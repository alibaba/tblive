// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_WORKER_POOL_MGR_H_
#define BASE_THREADING_WORKER_POOL_MGR_H_

#include <cstddef>
#include <string>
#include <map>

#include "base/base_export.h"
#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/task_runner.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/synchronization/lock.h"

namespace base {

enum PoolMgrWorkerShutdown {
	// Tasks posted with this mode that have not started executing at
	// shutdown will be deleted rather than executed. However, any tasks that
	// have already begun executing when shutdown is called will be allowed
	// to continue, and will block shutdown until completion.
	//
	// Note: Because Shutdown() may block while these tasks are executing,
	// care must be taken to ensure that they do not block on the thread that
	// called Shutdown(), as this may lead to deadlock.
	TASK_SKIP_ON_SHUTDOWN,

	// Tasks posted with this mode will block shutdown until they're
	// executed. Since this can have significant performance implications,
	// use sparingly.
	//
	// Generally, this should be used only for user data, for example, a task
	// writing a preference file.
	//
	// If a task is posted during shutdown, it will not get run since the
	// workers may already be stopped. In this case, the post operation will
	// fail (return false) and the task will be deleted.
	TASK_BLOCK_SHUTDOWN,
};

class PoolMgrSequenceToken {
public:
	PoolMgrSequenceToken(SequencedWorkerPool::SequenceToken &_token): token(_token)
	{
		valid = token.IsValid();
	}

	PoolMgrSequenceToken():valid(false)
	{

	}

	PoolMgrSequenceToken(const PoolMgrSequenceToken &that): token(that.token)
	{
		valid = token.IsValid();
	}

	PoolMgrSequenceToken& operator=(const PoolMgrSequenceToken &that)
	{
		token = that.token;
		valid = token.IsValid();
		return *this;
	}

public:
	SequencedWorkerPool::SequenceToken token;
	bool valid;
};

class BASE_EXPORT CWorkerPoolMgr
{
public:
	typedef scoped_refptr<base::SequencedWorkerPool> TaskPoolPtr;

	CWorkerPoolMgr();

	~CWorkerPoolMgr();

public:
	// 必须在进程开始后在UI线程上初始化
	static CWorkerPoolMgr &instance() {
        CR_DEFINE_STATIC_LOCAL( CWorkerPoolMgr, obj, () );
		return obj;
	}

	// 添加自定义的业务线程池
	void InitWorkerPool(int max_threads, std::string const& poolname);

	void UninitWorkerPool(std::string const& poolname);

	bool PostTask(const base::Closure& task, std::string const& poolname, PoolMgrWorkerShutdown shutdown = TASK_BLOCK_SHUTDOWN);

	bool PostDelayedTask(const base::Closure& task, std::string const& poolname, unsigned long delay);

	bool PostTaskAndReply(const base::Closure& task, const base::Closure& reply, std::string const& poolname, PoolMgrWorkerShutdown shutdown = TASK_BLOCK_SHUTDOWN);

	bool GenerateSequenceToken(std::string const& poolname, PoolMgrSequenceToken &poolmgrtoken);

	bool PostSequencedTask(const PoolMgrSequenceToken &poolmgrtoken, const base::Closure& task, std::string const& poolname, PoolMgrWorkerShutdown shutdown = TASK_BLOCK_SHUTDOWN);
	
	bool PostSequencedDelayedTask(const PoolMgrSequenceToken &poolmgrtoken, const base::Closure& task, std::string const& poolname, unsigned long delay);
	
	bool PostSequencedTaskAndReply(const PoolMgrSequenceToken &poolmgrtoken, const base::Closure& task, const base::Closure& reply, std::string const& poolname, PoolMgrWorkerShutdown shutdown = TASK_BLOCK_SHUTDOWN);

private:
	//线程池map
	std::map<std::string, TaskPoolPtr> m_customTaskpoolMap;
    base::Lock m_mutex;
};

}  // namespace base

#endif  // BASE_THREADING_WORKER_POOL_MGR_H_
