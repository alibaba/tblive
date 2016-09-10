// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// DataPack represents a read-only view onto an on-disk file that contains
// (key, value) pairs of data.  It's used to store static resources like
// translation strings and images.

#ifndef BASE_RESOURCE_DATA_PACK_H_
#define BASE_RESOURCE_DATA_PACK_H_

#include <map>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/platform_file.h"
#include "base/strings/string_piece.h"

#include "base/resource/resource_handle.h"
#include "base/base_export.h"

namespace base {
class FilePath;
class MemoryMappedFile;
class RefCountedStaticMemory;
}

namespace base {

class BASE_EXPORT DataPack : public ResourceHandle {
 public:
  DataPack();
  virtual ~DataPack();

  // Load a pack file from |path|, returning false on error.
  bool LoadFromPath(const base::FilePath& path);

  // Loads a pack file from |file|, returning false on error.
  bool LoadFromFile(base::PlatformFile file);

  // Writes a pack file containing |resources| to |path|. If there are any
  // text resources to be written, their encoding must already agree to the
  // |textEncodingType| specified. If no text resources are present, please
  // indicate BINARY.
  static bool WritePack(const base::FilePath& path,
                        const std::map<uint16, base::StringPiece>& resources,
                        TextEncodingType textEncodingType);

  // ResourceHandle implementation:
  virtual bool HasResource(uint16 resource_id) const OVERRIDE;
  virtual bool GetStringPiece(uint16 resource_id,
                              base::StringPiece* data) const OVERRIDE;
  virtual base::RefCountedStaticMemory* GetStaticMemory(
      uint16 resource_id) const OVERRIDE;
  virtual TextEncodingType GetTextEncodingType() const OVERRIDE;

 private:
  // Does the actual loading of a pack file. Called by Load and LoadFromFile.
  bool LoadImpl();

  // The memory-mapped data.
  scoped_ptr<base::MemoryMappedFile> mmap_;

  // Number of resources in the data.
  size_t resource_count_;

  // Type of encoding for text resources.
  TextEncodingType text_encoding_type_;

  DISALLOW_COPY_AND_ASSIGN(DataPack);
};

}  // namespace base

#endif  // BASE_RESOURCE_DATA_PACK_H_
