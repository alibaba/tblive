// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_BASE_UPLOAD_FILE_ELEMENT_READER_H_
#define NET_BASE_UPLOAD_FILE_ELEMENT_READER_H_

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/gtest_prod_util.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "net/base/upload_element_reader.h"

namespace base {
class TaskRunner;
}

namespace net {

class FileStream;

// An UploadElementReader implementation for file.
class NET_EXPORT UploadFileElementReader : public UploadElementReader {
 public:
  // |task_runner| is used to perform file operations. It must not be NULL.
  UploadFileElementReader(base::TaskRunner* task_runner,
                          const base::FilePath& path,
                          uint64 range_offset,
                          uint64 range_length,
                          const base::Time& expected_modification_time);
  virtual ~UploadFileElementReader();

  const base::FilePath& path() const { return path_; }
  uint64 range_offset() const { return range_offset_; }
  uint64 range_length() const { return range_length_; }
  const base::Time& expected_modification_time() const {
    return expected_modification_time_;
  }

  // UploadElementReader overrides:
  virtual const UploadFileElementReader* AsFileReader() const OVERRIDE;
  virtual int Init(const CompletionCallback& callback) OVERRIDE;
  virtual uint64 GetContentLength() const OVERRIDE;
  virtual uint64 BytesRemaining() const OVERRIDE;
  virtual int Read(IOBuffer* buf,
                   int buf_length,
                   const CompletionCallback& callback) OVERRIDE;

 private:
  // Deletes FileStream with |task_runner| to avoid blocking the IO thread.
  // This class is used as a template argument of scoped_ptr.
  class FileStreamDeleter {
   public:
    explicit FileStreamDeleter(base::TaskRunner* task_runner);
    ~FileStreamDeleter();
    void operator() (FileStream* file_stream) const;

   private:
    scoped_refptr<base::TaskRunner> task_runner_;
  };

  typedef scoped_ptr<FileStream, FileStreamDeleter> ScopedFileStreamPtr;

  FRIEND_TEST_ALL_PREFIXES(UploadDataStreamTest, FileSmallerThanLength);
  FRIEND_TEST_ALL_PREFIXES(HttpNetworkTransactionTest,
                           UploadFileSmallerThanLength);
  FRIEND_TEST_ALL_PREFIXES(HttpNetworkTransactionSpdy2Test,
                           UploadFileSmallerThanLength);
  FRIEND_TEST_ALL_PREFIXES(HttpNetworkTransactionSpdy3Test,
                           UploadFileSmallerThanLength);

  // Resets this instance to the uninitialized state.
  void Reset();

  // This method is used to implement Init().
  void OnInitCompleted(ScopedFileStreamPtr* file_stream,
                       uint64* content_length,
                       const CompletionCallback& callback,
                       int result);

  // This method is used to implement Read().
  void OnReadCompleted(ScopedFileStreamPtr file_stream,
                       const CompletionCallback& callback,
                       int result);

  // Sets an value to override the result for GetContentLength().
  // Used for tests.
  struct NET_EXPORT_PRIVATE ScopedOverridingContentLengthForTests {
    ScopedOverridingContentLengthForTests(uint64 value);
    ~ScopedOverridingContentLengthForTests();
  };

  scoped_refptr<base::TaskRunner> task_runner_;
  const base::FilePath path_;
  const uint64 range_offset_;
  const uint64 range_length_;
  const base::Time expected_modification_time_;
  ScopedFileStreamPtr file_stream_;
  uint64 content_length_;
  uint64 bytes_remaining_;
  base::WeakPtrFactory<UploadFileElementReader> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(UploadFileElementReader);
};

// An UploadElementReader implementation for file which performs file operation
// synchronously.
// Use this class only if the thread is IO allowed.
class NET_EXPORT UploadFileElementReaderSync : public UploadElementReader {
 public:
  UploadFileElementReaderSync(const base::FilePath& path,
                              uint64 range_offset,
                              uint64 range_length,
                              const base::Time& expected_modification_time);
  virtual ~UploadFileElementReaderSync();

  // UploadElementReader overrides:
  virtual int Init(const CompletionCallback& callback) OVERRIDE;
  virtual uint64 GetContentLength() const OVERRIDE;
  virtual uint64 BytesRemaining() const OVERRIDE;
  virtual int Read(IOBuffer* buf,
                   int buf_length,
                   const CompletionCallback& callback) OVERRIDE;

 private:
  const base::FilePath path_;
  const uint64 range_offset_;
  const uint64 range_length_;
  const base::Time expected_modification_time_;
  scoped_ptr<FileStream> file_stream_;
  uint64 content_length_;
  uint64 bytes_remaining_;

  DISALLOW_COPY_AND_ASSIGN(UploadFileElementReaderSync);
};

}  // namespace net

#endif  // NET_BASE_UPLOAD_FILE_ELEMENT_READER_H_
