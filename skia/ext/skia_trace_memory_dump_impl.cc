// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skia/ext/skia_trace_memory_dump_impl.h"

#include "base/trace_event/memory_allocator_dump.h"
#include "base/trace_event/memory_dump_manager.h"
#include "base/trace_event/process_memory_dump.h"
#include "skia/ext/SkDiscardableMemory_chrome.h"

namespace skia {

namespace {
const char kMallocBackingType[] = "malloc";
}

SkiaTraceMemoryDumpImpl::SkiaTraceMemoryDumpImpl(
    base::trace_event::MemoryDumpLevelOfDetail level_of_detail,
    base::trace_event::ProcessMemoryDump* process_memory_dump)
    : SkiaTraceMemoryDumpImpl("", level_of_detail, process_memory_dump) {}

SkiaTraceMemoryDumpImpl::SkiaTraceMemoryDumpImpl(
    const std::string& dump_name_prefix,
    base::trace_event::MemoryDumpLevelOfDetail level_of_detail,
    base::trace_event::ProcessMemoryDump* process_memory_dump)
    : dump_name_prefix_(dump_name_prefix),
      process_memory_dump_(process_memory_dump),
      request_level_(
          level_of_detail == base::trace_event::MemoryDumpLevelOfDetail::LIGHT
              ? SkTraceMemoryDump::kLight_LevelOfDetail
              : SkTraceMemoryDump::kObjectsBreakdowns_LevelOfDetail) {}

SkiaTraceMemoryDumpImpl::~SkiaTraceMemoryDumpImpl() = default;

void SkiaTraceMemoryDumpImpl::dumpNumericValue(const char* dumpName,
                                               const char* valueName,
                                               const char* units,
                                               uint64_t value) {
  auto* dump = process_memory_dump_->GetOrCreateAllocatorDump(dumpName);
  dump->AddScalar(valueName, units, value);
}

void SkiaTraceMemoryDumpImpl::dumpStringValue(const char* dump_name,
                                              const char* value_name,
                                              const char* value) {
  auto* dump = process_memory_dump_->GetOrCreateAllocatorDump(dump_name);
  dump->AddString(value_name, "", value);
}

void SkiaTraceMemoryDumpImpl::setMemoryBacking(const char* dumpName,
                                               const char* backingType,
                                               const char* backingObjectId) {
  if (strcmp(backingType, kMallocBackingType) == 0) {
    auto* dump = process_memory_dump_->GetOrCreateAllocatorDump(dumpName);
    const char* system_allocator_name =
        base::trace_event::MemoryDumpManager::GetInstance()
            ->system_allocator_pool_name();
    if (system_allocator_name) {
      process_memory_dump_->AddSuballocation(dump->guid(),
                                             system_allocator_name);
    }
  } else {
    NOTREACHED();
  }
}

void SkiaTraceMemoryDumpImpl::setDiscardableMemoryBacking(
    const char* dumpName,
    const SkDiscardableMemory& discardableMemoryObject) {
  std::string name = dump_name_prefix_ + dumpName;
  DCHECK(!process_memory_dump_->GetAllocatorDump(name));
  const SkDiscardableMemoryChrome& discardable_memory_obj =
      static_cast<const SkDiscardableMemoryChrome&>(discardableMemoryObject);
  auto* dump = discardable_memory_obj.CreateMemoryAllocatorDump(
      name.c_str(), process_memory_dump_);
  DCHECK(dump);
}

SkTraceMemoryDump::LevelOfDetail SkiaTraceMemoryDumpImpl::getRequestedDetails()
    const {
  return request_level_;
}

bool SkiaTraceMemoryDumpImpl::shouldDumpWrappedObjects() const {
  // Chrome already dumps objects it imports into Skia. Avoid duplicate dumps
  // by asking Skia not to dump them.
  return false;
}

}  // namespace skia
