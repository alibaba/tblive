// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_STRINGS_UTF_OFFSET_STRING_CONVERSIONS_H_
#define BASE_STRINGS_UTF_OFFSET_STRING_CONVERSIONS_H_

#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace base {

// Like the conversions in utf_string_conversions.h, but also takes one or more
// |offset[s]_for_adjustment| representing insertion/selection points between
// characters: if |src| is "abcd", then 0 is before 'a', 2 is between 'b' and
// 'c', and 4 is at the end of the string.  Valid input offsets range from 0 to
// |src_len|.  On exit, each offset will have been modified to point at the same
// logical position in the output string.  If an offset cannot be successfully
// adjusted (e.g. because it points into the middle of a multibyte sequence), it
// will be set to string16::npos.
//
// |offset[s]_for_adjustment| may be NULL.
BASE_EXPORT bool UTF8ToUTF16AndAdjustOffset(const char* src,
                                            size_t src_len,
                                            string16* output,
                                            size_t* offset_for_adjustment);
BASE_EXPORT bool UTF8ToUTF16AndAdjustOffsets(
    const char* src,
    size_t src_len,
    string16* output,
    std::vector<size_t>* offsets_for_adjustment);

BASE_EXPORT string16 UTF8ToUTF16AndAdjustOffset(const base::StringPiece& utf8,
                                                size_t* offset_for_adjustment);
BASE_EXPORT string16 UTF8ToUTF16AndAdjustOffsets(
    const base::StringPiece& utf8,
    std::vector<size_t>* offsets_for_adjustment);

BASE_EXPORT std::string UTF16ToUTF8AndAdjustOffset(
    const base::StringPiece16& utf16,
    size_t* offset_for_adjustment);
BASE_EXPORT std::string UTF16ToUTF8AndAdjustOffsets(
    const base::StringPiece16& utf16,
    std::vector<size_t>* offsets_for_adjustment);

// Limiting function callable by std::for_each which will replace any value
// which is greater than |limit| with npos.  Typically this is called with a
// string length to clamp offsets into the string to [0, length] (as opposed to
// [0, length); see comments above).
template <typename T>
struct LimitOffset {
  explicit LimitOffset(size_t limit)
    : limit_(limit) {}

  void operator()(size_t& offset) {
    if (offset > limit_)
      offset = T::npos;
  }

  size_t limit_;
};

// Stack object which, on destruction, will update a vector of offsets based on
// any supplied adjustments.  To use, declare one of these, providing the
// address of the offset vector to adjust.  Then Add() any number of Adjustments
// (each Adjustment gives the |original_offset| of a substring and the lengths
// of the substring before and after transforming).  When the OffsetAdjuster
// goes out of scope, all the offsets in the provided vector will be updated.
class BASE_EXPORT OffsetAdjuster {
 public:
  struct BASE_EXPORT Adjustment {
    Adjustment(size_t original_offset,
               size_t original_length,
               size_t output_length);

    size_t original_offset;
    size_t original_length;
    size_t output_length;
  };

  explicit OffsetAdjuster(std::vector<size_t>* offsets_for_adjustment);
  ~OffsetAdjuster();

  void Add(const Adjustment& adjustment);

 private:
  void AdjustOffset(std::vector<size_t>::iterator offset);

  std::vector<size_t>* offsets_for_adjustment_;
  std::vector<Adjustment> adjustments_;
};

}  // namespace base

#endif  // BASE_STRINGS_UTF_OFFSET_STRING_CONVERSIONS_H_
