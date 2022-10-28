// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// clang-format off
#include <hb.h>
#include <hb-subset.h>
// clang-format on

#include "base/check.h"
#include "base/cxx17_backports.h"
#include "third_party/harfbuzz-ng/utils/hb_scoped.h"

namespace {

void TrySubset(hb_face_t* face,
               const hb_codepoint_t text[],
               const int text_length,
               const uint8_t flags) {
  bool drop_layout = flags & (1 << 1);
  unsigned input_set_flags =
      0                                                          //
      | ((flags & (1 << 0)) ? HB_SUBSET_FLAGS_NO_HINTING : 0)    //
      | ((flags & (1 << 2)) ? HB_SUBSET_FLAGS_RETAIN_GIDS : 0);  //

  HbScoped<hb_subset_input_t> input(hb_subset_input_create_or_fail());
  hb_subset_input_set_flags(input.get(), input_set_flags);
  hb_set_t* codepoints = hb_subset_input_unicode_set(input.get());

  if (!drop_layout) {
    hb_set_del(hb_subset_input_set(input.get(), HB_SUBSET_SETS_DROP_TABLE_TAG),
               HB_TAG('G', 'S', 'U', 'B'));
    hb_set_del(hb_subset_input_set(input.get(), HB_SUBSET_SETS_DROP_TABLE_TAG),
               HB_TAG('G', 'P', 'O', 'S'));
    hb_set_del(hb_subset_input_set(input.get(), HB_SUBSET_SETS_DROP_TABLE_TAG),
               HB_TAG('G', 'D', 'E', 'F'));
  }

  for (int i = 0; i < text_length; i++) {
    hb_set_add(codepoints, text[i]);
  }

  HbScoped<hb_face_t> result(hb_subset_or_fail(face, input.get()));
  if (!result) {
    // Subset failed, so nothing to check.
    return;
  }
  HbScoped<hb_blob_t> blob(hb_face_reference_blob(result.get()));
  uint32_t length;
  const char* data = hb_blob_get_data(blob.get(), &length);

  // Access all the blob data
  uint32_t bytes_count = 0;
  if (data) {
    for (uint32_t i = 0; i < length; ++i) {
      if (data[i])
        ++bytes_count;
    }
  }
  CHECK(bytes_count || !length);
}

}  // namespace

constexpr size_t kMaxInputLength = 16800;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size > kMaxInputLength)
    return 0;

  const char* data_ptr = reinterpret_cast<const char*>(data);
  HbScoped<hb_blob_t> blob(hb_blob_create(
      data_ptr, size, HB_MEMORY_MODE_READONLY, nullptr, nullptr));
  HbScoped<hb_face_t> face(hb_face_create(blob.get(), 0));

  // Test hb_set API
  {
    HbScoped<hb_set_t> output(hb_set_create());
    hb_face_collect_unicodes(face.get(), output.get());
  }

  uint8_t subset_flags = 0;
  const hb_codepoint_t text[] = {'A', 'B', 'C', 'D', 'E', 'X', 'Y',
                                 'Z', '1', '2', '3', '@', '_', '%',
                                 '&', ')', '*', '$', '!'};

  TrySubset(face.get(), text, base::size(text), subset_flags);

  hb_codepoint_t text_from_data[16];
  if (size > sizeof(text_from_data) + 1) {
    memcpy(text_from_data, data + size - sizeof(text_from_data),
           sizeof(text_from_data));
    subset_flags = data[size - sizeof(text_from_data) - 1];
    size_t text_size = base::size(text_from_data);
    TrySubset(face.get(), text_from_data, text_size, subset_flags);
  }

  return 0;
}
