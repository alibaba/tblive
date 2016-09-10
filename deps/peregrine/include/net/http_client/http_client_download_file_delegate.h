#ifndef NET_HTTP_CLIENT_DOWNLOAD_FILE_DELEGATE_H_
#define NET_HTTP_CLIENT_DOWNLOAD_FILE_DELEGATE_H_

#include "http_client_helper.h"
#include "net_request_context.h"
#include "http_transaction_adapter.h"

#include "net/base/net_errors.h"
#include "net/base/file_stream.h"
#include "net/base/io_buffer.h"
#include "base/file_util.h"
#include "base/threading/sequenced_worker_pool.h"

namespace net {

class HttpClientDownloadFileDelegate
  : public HttpTransactionAdapter::Delegate
{
 public:
  static void StartHttpTransction(
    const base::subtle::Atomic32 id,
    const std::wstring& url,
    const std::wstring& path,
    const scoped_refptr<StreamRequestDelegate>& delegate ) {

    HttpRequest req;
    req.url = url;
    req.method = L"GET";
    scoped_refptr<HttpTransactionAdapter> adapter= 
      NetRequestContext::GetInstance()->CreateHttpTransactionAdapter(
      make_scoped_refptr(new HttpClientDownloadFileDelegate(delegate, id, url, path)),
      id);
    adapter->Start(req);
  }

  HttpClientDownloadFileDelegate(const scoped_refptr<StreamRequestDelegate>& delegate,
    const base::subtle::Atomic32 id,
    const std::wstring& url,
                                 const std::wstring& path);
  
protected:
  virtual ~HttpClientDownloadFileDelegate();
  friend class HttpTransactionAdapter::Delegate;

public:
  virtual void OnReceivedHeaders(const scoped_refptr<HttpResponseHeaders>& headers) OVERRIDE ;
  
  virtual void OnReceivedData(const std::string& data) OVERRIDE ;
  
  virtual void OnError(int result) OVERRIDE;
  
  virtual void OnRequestComplete() OVERRIDE;
  
 private:
  void DoWriteFile() {
    if (file_) {
      if (!writing_file_
        && buffer_.size() > 0) {
        writing_file_ = true;
        scoped_refptr<IOBuffer> buffer = make_scoped_refptr(new IOBuffer(buffer_.size()));
        memcpy(buffer->data(), buffer_.c_str(), buffer_.size());
        file_->Write(buffer.get(), 
          buffer_.size(), 
          base::Bind(&HttpClientDownloadFileDelegate::OnWriteFileComplete, this));
      }
    }
  }

  void OnOpenFileComplete(int result) {
    if (result < 0) {
      OnError(result);
    }
  }

  void OnCloseFileComplete(int result) {
    if (result < 0) {
      OnError(result);
    } else {
      if (delegate_) {
        task_runner_->PostNonNestableTask(FROM_HERE,
          base::Bind(&StreamRequestDelegate::OnRequestComplete, delegate_));
      }
    }

    file_.reset();
  }

  void OnWriteFileComplete(int result) {
    writing_file_ = false;
    if (result < 0) {
      OnError(result);
    } else {
      buffer_ = buffer_.substr(result);
      DoWriteFile();
      CheckComplete();
    }
  }

  void CheckComplete() {
    if (download_file_finish_
      && buffer_.empty()) {
        if (net::OK == 
          NetRequestContext::GetInstance()->DeleteHttpTransactionAdapter(id_)) {
            // Download finish and async write file finish
            // Then, close file async first to make sure file generated
            if ( file_ ) {
              file_->Close( base::Bind(&HttpClientDownloadFileDelegate::OnCloseFileComplete, this) );
            }
        }
    }
  }
  
  static void OnDeleteFile(base::FilePath path)
  {
    base::DeleteFile(path, false);
  }
  
 protected:
  base::subtle::Atomic32 id_;
  base::SequencedWorkerPool::SequenceToken seq_token_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;

 private:
  scoped_refptr<StreamRequestDelegate> delegate_;
  scoped_ptr<FileStream> file_;
  std::string buffer_;
  bool writing_file_;
  bool download_file_finish_;
  base::FilePath path_;
};
}
#endif  // NET_HTTP_CLIENT_DOWNLOAD_FILE_DELEGATE_H_
