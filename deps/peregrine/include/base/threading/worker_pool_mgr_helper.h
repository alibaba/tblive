// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once
#include "base/namespace.h"
#include "base/threading/worker_pool_mgr.h"
#include "base/memory/ref_counted.h"
#include "base/callback.h"
#include "base/bind.h"
#include "base/memory/weak_ptr.h"
#include "base/memory/linked_ptr.h"

BEGIN_NAMESPACE(prg)

struct TimerexHandler : base::SupportsWeakPtr<TimerexHandler>
    , base::RefCountedThreadSafe<TimerexHandler> {
base::PoolMgrSequenceToken token;
};

struct simple_task_closure
{
	simple_task_closure() { }
	virtual ~simple_task_closure() { };
	virtual void Invoke(void) = 0;
};

struct timerex_task_closure: simple_task_closure
{
	timerex_task_closure(base::Closure const& f, base::PoolMgrSequenceToken &_token) 
		: functor(f), token(_token)
	{
	}

	timerex_task_closure(timerex_task_closure const& rhs)
		: functor(rhs.functor)
		, WeakHandler(rhs.WeakHandler)
		, token(rhs.token)
	{
	}

	virtual void Invoke(void) { functor.Run(); }

	base::Closure functor;
	base::WeakPtr<TimerexHandler> WeakHandler;
	base::PoolMgrSequenceToken token;
};

inline void InitWorkTask(int max_threads, std::string const& poolname)
{
	base::CWorkerPoolMgr::instance().InitWorkerPool(max_threads, poolname);
}

inline void UninitWorkTask(std::string const& poolname)
{
	base::CWorkerPoolMgr::instance().UninitWorkerPool(poolname);
}

inline bool GenerateSequenceToken(const std::string &poolname, base::PoolMgrSequenceToken &poolmgrtoken)
{
	return base::CWorkerPoolMgr::instance().GenerateSequenceToken(poolname, poolmgrtoken);
}

inline bool QueueWorkTask(
	base::Closure const& task, 
	const std::string &poolname,
	base::PoolMgrWorkerShutdown shutdown = base::TASK_BLOCK_SHUTDOWN
	)
{
	return base::CWorkerPoolMgr::instance().PostTask(task, poolname, shutdown);
}

inline bool QueueSequencedWorkTask(
	const base::PoolMgrSequenceToken &poolmgrtoken,
	base::Closure const& task, 
	const std::string &poolname,
	base::PoolMgrWorkerShutdown shutdown = base::TASK_BLOCK_SHUTDOWN
	)
{
	return base::CWorkerPoolMgr::instance().PostSequencedTask(poolmgrtoken, task, poolname, shutdown);
}

inline bool QueueTimerTask(
	base::Closure const& task, 
	unsigned long delay,
	const std::string &poolname)
{
	return base::CWorkerPoolMgr::instance().PostDelayedTask(task, poolname, delay);
}

inline bool QueueWorkTaskAndReply(
	base::Closure const& task, 
	base::Closure const& reply,
	const std::string &poolname,
	base::PoolMgrWorkerShutdown shutdown = base::TASK_BLOCK_SHUTDOWN
	)
{
	return base::CWorkerPoolMgr::instance().PostTaskAndReply(task, reply, poolname, shutdown);
}

inline bool QueueSequencedWorkTaskAndReply(
	const base::PoolMgrSequenceToken &poolmgrtoken,
	base::Closure const& task, 
	base::Closure const& reply,
	const std::string &poolname,
	base::PoolMgrWorkerShutdown shutdown = base::TASK_BLOCK_SHUTDOWN
	)
{
	return base::CWorkerPoolMgr::instance().PostSequencedTaskAndReply(poolmgrtoken, task, reply, poolname, shutdown);
}

// 重复定时任务回调函数
inline void circle_timerex_func(void* parameter, std::string name, unsigned long delay)
{
	// auto_ptr 会自动释放这一轮的对象
	std::auto_ptr<timerex_task_closure> pclosure(reinterpret_cast<timerex_task_closure*>(parameter));

	scoped_refptr<TimerexHandler> sp = pclosure->WeakHandler.get();
	if (sp)
	{
		try
		{
			// 抛到下一轮
			timerex_task_closure * next_pclosure = new timerex_task_closure(*pclosure);
			base::Closure next_turn = base::Bind(&circle_timerex_func, base::Unretained(next_pclosure), name, delay);
			base::CWorkerPoolMgr::instance().PostSequencedDelayedTask(next_pclosure->token, next_turn, name, delay);
			pclosure->Invoke();
		}
		catch(...)
		{
		}
	}
}

// 统一线程销毁
inline void destroy_timerex_func(scoped_refptr<TimerexHandler> spHandler)
{
	spHandler = NULL;
}

inline scoped_refptr<TimerexHandler> QueueTimerexTask(
	base::Closure const& task, 
	unsigned long delay,
	const std::string &poolname)
{
	// 创建任务闭包
	base::PoolMgrSequenceToken token;
	base::CWorkerPoolMgr::instance().GenerateSequenceToken(poolname, token);

	scoped_refptr<TimerexHandler> SharedHandler(new TimerexHandler());
	SharedHandler->token = token;

	timerex_task_closure* pclosure = new timerex_task_closure(task, token);
	pclosure->WeakHandler = SharedHandler->AsWeakPtr();

	base::Closure closure = base::Bind(&circle_timerex_func, base::Unretained(pclosure), poolname, delay);
	if (base::CWorkerPoolMgr::instance().PostSequencedDelayedTask(token, closure, poolname, delay))
		return SharedHandler;

	assert( "not inited taskpool used" );
	delete pclosure;
	return scoped_refptr<TimerexHandler>();
}

inline void DestroyTimerexTask(
	scoped_refptr<TimerexHandler> &spHandler,
	const std::string &poolname
	)
{
	base::Closure closure = base::Bind(&destroy_timerex_func, spHandler);
	base::CWorkerPoolMgr::instance().PostSequencedTask(spHandler->token, closure, poolname);
	spHandler = NULL;
}

END_NAMESPACE()