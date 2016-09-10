
#include "task_pool.hpp"

#include "base/threading/thread.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/memory/ref_counted.h"
#include "base/bind.h"
#include "base/lazy_instance.h"

class ThreadPool {
public:
  ThreadPool() 
    : worker_pool_( new base::SequencedWorkerPool(3, "worker_pool") ) 
  {
  }

  ~ThreadPool() {
    worker_pool_->Shutdown();
  }

  void PostTask( base::Closure task ) {
  	worker_pool_->PostWorkerTask(FROM_HERE, task);
  }

private:
  scoped_refptr<base::SequencedWorkerPool> worker_pool_;
};

static base::LazyInstance<ThreadPool> g_thread_pool = LAZY_INSTANCE_INITIALIZER;


namespace task_pool {

void PostTask( base::Closure task )
{
	g_thread_pool.Get().PostTask(task);
}

}// namespace taskpool

