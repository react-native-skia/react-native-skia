// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Dumps PartitionAlloc's heap into a file.

#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include "base/allocator/partition_allocator/partition_alloc_config.h"
#include "base/allocator/partition_allocator/partition_ref_count.h"
#include "base/allocator/partition_allocator/partition_root.h"
#include "base/allocator/partition_allocator/thread_cache.h"
#include "base/bits.h"
#include "base/check.h"
#include "base/command_line.h"
#include "base/files/file.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/memory/page_size.h"
#include "base/strings/stringprintf.h"
#include "base/thread_annotations.h"
#include "base/values.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/snappy/src/snappy.h"
#include "tools/memory/partition_allocator/inspect_utils.h"

namespace partition_alloc::tools {

using partition_alloc::internal::kInvalidBucketSize;
using partition_alloc::internal::kSuperPageSize;
using partition_alloc::internal::PartitionPage;
using partition_alloc::internal::PartitionPageSize;
#if BUILDFLAG(USE_BACKUP_REF_PTR)
using partition_alloc::internal::PartitionRefCountPointer;
#endif  // BUILDFLAG(USE_BACKUP_REF_PTR)
using partition_alloc::internal::PartitionSuperPageExtentEntry;
using partition_alloc::internal::SystemPageSize;
using partition_alloc::internal::ThreadSafe;

// See https://www.kernel.org/doc/Documentation/vm/pagemap.txt.
struct PageMapEntry {
  uint64_t pfn_or_swap : 55;
  uint64_t soft_dirty : 1;
  uint64_t exclusively_mapped : 1;
  uint64_t unused : 4;
  uint64_t file_mapped_or_shared_anon : 1;
  uint64_t swapped : 1;
  uint64_t present : 1;
};
static_assert(sizeof(PageMapEntry) == sizeof(uint64_t), "Wrong bitfield size");

absl::optional<PageMapEntry> EntryAtAddress(int pagemap_fd, uintptr_t address) {
  constexpr size_t kPageShift = 12;
  off_t offset = (address >> kPageShift) * sizeof(PageMapEntry);
  if (lseek(pagemap_fd, offset, SEEK_SET) != offset)
    return absl::nullopt;

  PageMapEntry entry;
  if (read(pagemap_fd, &entry, sizeof(PageMapEntry)) != sizeof(PageMapEntry))
    return absl::nullopt;

  return {entry};
}

class HeapDumper {
 public:
  HeapDumper(pid_t pid, int pagemap_fd)
      : pagemap_fd_(pagemap_fd), reader_(pid) {}
  ~HeapDumper() {
    for (const auto& p : super_pages_) {
      munmap(p.second, kSuperPageSize);
    }
    if (local_root_copy_mapping_base_) {
      munmap(local_root_copy_mapping_base_, local_root_copy_mapping_size_);
    }
  }

  bool FindRoot() {
    root_address_ = FindRootAddress(reader_);
    CHECK(root_address_);
    auto root = RawBuffer<PartitionRoot<ThreadSafe>>::ReadFromProcessMemory(
        reader_, root_address_);
    CHECK(root);
    root_ = *root;

    // Since the heap if full of pointers, copying the data to the local address
    // space doesn't allow to follow the pointers, or to call most member
    // functions on the local objects.
    //
    // To make it easier to work with, we copy some objects in the local address
    // space at the *same* address used in the remote process. This is not
    // guaranteed to work though, since the addresses can already be mapped in
    // the local process. However, since we are targeting 64 bit Linux, with
    // ASLR executing again should solve the problem in most cases.
    //
    // Copy at the same address as in the remote process. Since the root is not
    // page-aligned in the remote process, need to pad the mapping a bit.
    size_t size_to_map = ::base::bits::AlignUp(
        sizeof(PartitionRoot<ThreadSafe>) + SystemPageSize(), SystemPageSize());
    uintptr_t address_to_map =
        ::base::bits::AlignDown(root_address_, SystemPageSize());
    char* local_memory = CreateMappingAtAddress(address_to_map, size_to_map);
    if (!local_memory) {
      LOG(WARNING) << base::StringPrintf(
          "Cannot map memory at %lx",
          reinterpret_cast<uintptr_t>(address_to_map));
      return false;
    }
    local_root_copy_ = local_memory;

    memcpy(reinterpret_cast<void*>(root_address_), root_.get(),
           sizeof(PartitionRoot<ThreadSafe>));
    local_root_copy_mapping_base_ = reinterpret_cast<void*>(address_to_map);
    local_root_copy_mapping_size_ = size_to_map;

    return true;
  }

  bool DumpSuperPages() {
    std::vector<uintptr_t> super_pages;
    // There is no list of super page, only a list of extents. Walk the extent
    // list to get all superpages.
    uintptr_t extent_address =
        reinterpret_cast<uintptr_t>(root_.get()->first_extent);
    while (extent_address) {
      auto extent = RawBuffer<PartitionSuperPageExtentEntry<ThreadSafe>>::
          ReadFromProcessMemory(reader_, extent_address);
      uintptr_t first_super_page_address = SuperPagesBeginFromExtent(
          reinterpret_cast<PartitionSuperPageExtentEntry<ThreadSafe>*>(
              extent_address));
      for (uintptr_t super_page = first_super_page_address;
           super_page < first_super_page_address +
                            extent->get()->number_of_consecutive_super_pages *
                                kSuperPageSize;
           super_page += kSuperPageSize) {
        super_pages.push_back(super_page);
      }
      extent_address = reinterpret_cast<uintptr_t>(extent->get()->next);
    }

    LOG(WARNING) << "Found " << super_pages.size() << std::hex
                 << " super pages.";
    for (uintptr_t super_page : super_pages) {
      char* local_super_page =
          reader_.ReadAtSameAddressInLocalMemory(super_page, kSuperPageSize);
      if (!local_super_page) {
        LOG(WARNING) << base::StringPrintf("Cannot read from super page 0x%lx",
                                           super_page);
        continue;
      }
      super_pages_.emplace(super_page, local_super_page);
    }
    LOG(WARNING) << "Read all super pages";
    return true;
  }

  base::Value Dump() const {
    auto partition_page_to_value = [](uintptr_t offset,
                                      const char* data) -> base::Value {
      auto ret = base::Value(base::Value::Type::DICTIONARY);
      std::string value;
      if (offset == 0) {
        value = "metadata";
      } else if (offset == kSuperPageSize - PartitionPageSize()) {
        value = "guard";
      } else {
        value = "payload";
      }
      ret.SetKey("type", base::Value{value});

      if (value != "metadata" && value != "guard") {
        const auto* partition_page = PartitionPage<ThreadSafe>::FromAddr(
            reinterpret_cast<uintptr_t>(data + offset));
        ret.SetKey("page_index_in_span",
                   base::Value{partition_page->slot_span_metadata_offset});
        if (partition_page->slot_span_metadata_offset == 0 &&
            partition_page->slot_span_metadata.bucket) {
          const auto& slot_span_metadata = partition_page->slot_span_metadata;
          ret.SetKey("slot_size", base::Value{static_cast<int>(
                                      slot_span_metadata.bucket->slot_size)});
          ret.SetKey("is_active", base::Value{slot_span_metadata.is_active()});
          ret.SetKey("is_full", base::Value{slot_span_metadata.is_full()});
          ret.SetKey("is_empty", base::Value{slot_span_metadata.is_empty()});
          ret.SetKey("is_decommitted",
                     base::Value{slot_span_metadata.is_decommitted()});
          ret.SetKey("slots_per_span",
                     base::Value{static_cast<int>(
                         slot_span_metadata.bucket->get_slots_per_span())});
          ret.SetKey(
              "num_system_pages_per_slot_span",
              base::Value{static_cast<int>(
                  slot_span_metadata.bucket->num_system_pages_per_slot_span)});
          ret.SetKey("num_allocated_slots",
                     base::Value{slot_span_metadata.num_allocated_slots});
          ret.SetKey("num_unprovisioned_slots",
                     base::Value{slot_span_metadata.num_unprovisioned_slots});
        }
      }

      bool all_zeros = true;
      for (size_t i = 0; i < PartitionPageSize(); i++) {
        if (data[offset + i]) {
          all_zeros = false;
          break;
        }
      }
      ret.SetKey("all_zeros", base::Value{all_zeros});

      return ret;
    };
    auto super_page_to_value = [&](uintptr_t address,
                                   const char* data) -> base::Value {
      auto ret = base::Value(base::Value::Type::DICTIONARY);
      ret.SetKey("address", base::Value{base::StringPrintf("0x%lx", address)});

      auto partition_pages = base::Value(base::Value::Type::LIST);
      for (uintptr_t offset = 0; offset < kSuperPageSize;
           offset += PartitionPageSize()) {
        partition_pages.Append(partition_page_to_value(offset, data));
      }
      ret.SetKey("partition_pages", std::move(partition_pages));

      auto page_sizes = base::Value(base::Value::Type::LIST);
      // Looking at how well the heap would compress.
      const size_t page_size = base::GetPageSize();
      for (uintptr_t page_address = address;
           page_address < address + partition_alloc::internal::kSuperPageSize;
           page_address += page_size) {
        auto maybe_pagemap_entry = EntryAtAddress(pagemap_fd_, page_address);
        size_t uncompressed_size = 0, compressed_size = 0;

        bool all_zeros = true;
        for (size_t i = 0; i < page_size; i++) {
          if (reinterpret_cast<unsigned char*>(page_address)[i]) {
            all_zeros = false;
            break;
          }
        }

        bool should_report;
        if (!maybe_pagemap_entry) {
          // We cannot tell whether a page has been decommitted, but all-zero
          // likely indicates that. Only report data for pages that the other
          // pages.
          should_report = !all_zeros;
        } else {
          // If it's not in memory and not in swap, only the PTE exists.
          should_report =
              maybe_pagemap_entry->present || maybe_pagemap_entry->swapped;
        }

        if (should_report) {
          std::string compressed;
          uncompressed_size = page_size;
          // Use snappy to approximate what a fast compression algorithm
          // operating with a page granularity would do. This is not the
          // algorithm used in either Linux or macOS, but should give some
          // indication.
          compressed_size =
              snappy::Compress(reinterpret_cast<const char*>(page_address),
                               page_size, &compressed);
        }

        auto page_size_dict = base::Value(base::Value::Type::DICTIONARY);
        page_size_dict.SetKey("uncompressed",
                              base::Value{static_cast<int>(uncompressed_size)});
        page_size_dict.SetKey("compressed",
                              base::Value{static_cast<int>(compressed_size)});
        page_sizes.Append(std::move(page_size_dict));
      }
      ret.SetKey("page_sizes", std::move(page_sizes));

      return ret;
    };

    auto super_pages_value = base::Value(base::Value::Type::LIST);
    for (const auto& address_data : super_pages_) {
      super_pages_value.Append(
          super_page_to_value(address_data.first, address_data.second));
    }

    return super_pages_value;
  }

#if defined(PA_REF_COUNT_STORE_REQUESTED_SIZE)
  base::Value DumpAllocatedSizes() {
    // Note: Here and below, it is safe to follow pointers into the super page,
    // or to the root or buckets, since they share the same address in the this
    // process as in the Chromium process.

    // Since there is no tracking of full slot spans, the way to enumerate all
    // allocated memory is to walk the heap itself.
    base::Value ret = base::Value(base::Value::Type::LIST);

    for (const auto& address_data : super_pages_) {
      const char* data = address_data.second;
      // Exclude the first and last partition pagers: metadata and guard,
      // respectively.
      size_t partition_page_index = 1;
      while (partition_page_index < kSuperPageSize / PartitionPageSize() - 1) {
        uintptr_t slot_span_start = reinterpret_cast<uintptr_t>(
            data + partition_page_index * PartitionPageSize());
        const auto* partition_page =
            PartitionPage<ThreadSafe>::FromAddr(slot_span_start);
        // No bucket for PartitionPages that were never provisioned.
        if (!partition_page->slot_span_metadata.bucket) {
          partition_page_index++;
          continue;
        }

        const auto& metadata = partition_page->slot_span_metadata;
        if (metadata.is_decommitted() || metadata.is_empty()) {
          // Skip this entire slot span, since it doesn't hold live allocations.
          partition_page_index += metadata.bucket->get_pages_per_slot_span();
          continue;
        }

        base::Value slot_span_value =
            base::Value(base::Value::Type::DICTIONARY);
        slot_span_value.SetKey("start_address", base::Value{base::StringPrintf(
                                                    "0x%lx", slot_span_start)});
        slot_span_value.SetKey(
            "slot_size",
            base::Value{static_cast<int>(metadata.bucket->slot_size)});

        // There is no tracking of allocated slots, need to reconstruct
        // these as everything which is not in the freelist.
        std::vector<bool> free_slots(metadata.bucket->get_slots_per_span());
        auto* head = metadata.get_freelist_head();
        while (head) {
          size_t offset_in_slot_span =
              reinterpret_cast<uintptr_t>(head) - slot_span_start;
          size_t slot_number =
              metadata.bucket->GetSlotNumber(offset_in_slot_span);
          free_slots[slot_number] = true;
          head = head->GetNext(0);
        }

        base::Value allocated_sizes_value =
            base::Value(base::Value::Type::LIST);
        for (size_t slot_index = 0; slot_index < free_slots.size();
             slot_index++) {
          // Skip unprovisioned slots, which are always at the end of the slot
          // span.
          if (free_slots[slot_index] ||
              slot_index >= (metadata.bucket->get_slots_per_span() -
                             metadata.num_unprovisioned_slots)) {
            continue;
          }
          uintptr_t slot_address =
              slot_span_start + slot_index * metadata.bucket->slot_size;
          auto* ref_count = PartitionRefCountPointer(slot_address);
          uint32_t requested_size = ref_count->requested_size();

          // Address space dumping is not synchronized with allocation, meaning
          // that we can observe the heap in an inconsistent state. Skip
          // obviously-wrong entries.
          if (requested_size > metadata.bucket->slot_size || !requested_size)
            continue;

          allocated_sizes_value.Append(static_cast<int>(requested_size));
        }
        slot_span_value.SetKey("allocated_sizes",
                               std::move(allocated_sizes_value));

        ret.Append(std::move(slot_span_value));
        partition_page_index += metadata.bucket->get_pages_per_slot_span();
      }
    }

    return ret;
  }
#endif  // defined(PA_REF_COUNT_STORE_REQUESTED_SIZE)

  base::Value DumpBuckets() {
    auto ret = base::Value(base::Value::Type::LIST);
    for (const auto& bucket : root_.get()->buckets) {
      if (bucket.slot_size == kInvalidBucketSize)
        continue;

      auto bucket_value = base::Value(base::Value::Type::DICTIONARY);
      bucket_value.SetKey("slot_size",
                          base::Value{static_cast<int>(bucket.slot_size)});
      ret.Append(std::move(bucket_value));
    }

    return ret;
  }

 private:
  static uintptr_t FindRootAddress(RemoteProcessMemoryReader& reader)
      NO_THREAD_SAFETY_ANALYSIS {
    uintptr_t tcache_registry_address = IndexThreadCacheNeedleArray(reader, 1);
    auto registry = RawBuffer<ThreadCacheRegistry>::ReadFromProcessMemory(
        reader, tcache_registry_address);
    if (!registry)
      return 0;

    auto tcache_address =
        reinterpret_cast<uintptr_t>(registry->get()->list_head_);
    if (!tcache_address)
      return 0;

    auto tcache =
        RawBuffer<ThreadCache>::ReadFromProcessMemory(reader, tcache_address);
    if (!tcache)
      return 0;

    auto root_address = reinterpret_cast<uintptr_t>(tcache->get()->root_);
    return root_address;
  }

  const int pagemap_fd_;
  uintptr_t root_address_ = 0;
  RemoteProcessMemoryReader reader_;
  RawBuffer<PartitionRoot<ThreadSafe>> root_ = {};
  std::map<uintptr_t, char*> super_pages_ = {};

  char* local_root_copy_ = nullptr;

  void* local_root_copy_mapping_base_ = nullptr;
  size_t local_root_copy_mapping_size_ = 0;
};

}  // namespace partition_alloc::tools

int main(int argc, char** argv) {
  base::CommandLine::Init(argc, argv);

  auto* command_line = base::CommandLine::ForCurrentProcess();
  if (!command_line->HasSwitch("pid") || !command_line->HasSwitch("json")) {
    LOG(ERROR) << "Usage:" << argv[0] << " --pid=<PID> --json=<FILENAME>";
    return 1;
  }

  int pid = atoi(command_line->GetSwitchValueASCII("pid").c_str());
  LOG(WARNING) << "PID = " << pid;

  auto pagemap_fd = partition_alloc::tools::OpenPagemap(pid);
  partition_alloc::tools::HeapDumper dumper{pid, pagemap_fd.get()};

  {
    partition_alloc::tools::ScopedSigStopper stopper{pid};
    if (!dumper.FindRoot()) {
      LOG(WARNING) << "Cannot find (or copy) the root";
      return 1;
    }
    if (!dumper.DumpSuperPages()) {
      LOG(WARNING) << "Cannot dump (or copy) super pages.";
    }
  }

  auto overall_dump = base::Value(base::Value::Type::DICTIONARY);
  auto dump = dumper.Dump();
  overall_dump.SetKey("superpages", std::move(dump));

#if defined(PA_REF_COUNT_STORE_REQUESTED_SIZE)
  auto allocated_sizes = dumper.DumpAllocatedSizes();
  overall_dump.SetKey("allocated_sizes", std::move(allocated_sizes));
#endif  // defined(PA_REF_COUNT_STORE_REQUESTED_SIZE)

  auto buckets = dumper.DumpBuckets();
  overall_dump.SetKey("buckets", std::move(buckets));

  std::string json_string;
  bool ok = base::JSONWriter::WriteWithOptions(
      overall_dump, base::JSONWriter::Options::OPTIONS_PRETTY_PRINT,
      &json_string);

  if (ok) {
    base::FilePath json_filename = command_line->GetSwitchValuePath("json");
    auto f = base::File(json_filename, base::File::Flags::FLAG_CREATE_ALWAYS |
                                           base::File::Flags::FLAG_WRITE);
    if (f.IsValid()) {
      f.WriteAtCurrentPos(json_string.c_str(), json_string.size());
      LOG(WARNING) << "\n\nDumped JSON to " << json_filename;
      return 0;
    }
  }

  return 1;
}
