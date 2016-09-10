#pragma once
#include "base/callback.h"
#include "base/threading/thread.h"
#include "base/memory/ref_counted.h"
#include "list"
typedef base::Callback<void()> AsyncInitWorkTaskGroupUnitedCallback;

class CAsyncInitWorkTaskGroup:public base::RefCountedThreadSafe<CAsyncInitWorkTaskGroup>
{
public:
	struct WorkTask
	{
		WorkTask():delay(0)
		{

		}
		WorkTask(const base::Callback<void()> &_task, const base::PoolMgrSequenceToken &_token, base::PoolMgrWorkerShutdown _shutdown)
			:task(_task), token(_token), shutdown(_shutdown), delay(0)
		{

		}
		WorkTask(const base::Callback<void()> &_task, unsigned long _delay)
			:task(_task), delay(_delay)
		{

		}
		base::Callback<void()> task;
		base::PoolMgrSequenceToken token;
		base::PoolMgrWorkerShutdown shutdown;
		unsigned long delay;
	};

	typedef std::list<WorkTask> WorkTaskList;

	struct WorkTaskManifest
	{
		WorkTaskList taskList;
		bool inited;
	};

	CAsyncInitWorkTaskGroup():m_callThread("asyncInitWorkTaskController")
	{
		base::Thread::Options options;
		options.message_loop_type = base::MessageLoop::TYPE_DEFAULT;
		m_callThread.StartWithOptions(options);
	}

	HRESULT InitWorkTask(const std::string &poolName, int threadCount)
	{
		base::AutoLock lock(m_lock);
		WorkTaskManifest manifest;
		manifest.inited = false;
		m_poolName2WorkTaskList[poolName] = manifest;
		m_callThread.message_loop()->PostTask(FROM_HERE, base::Bind(&CAsyncInitWorkTaskGroup::AsyncInitWorkTask, this, poolName, threadCount));
		return S_OK;
	}

	HRESULT UninitWorkTask(const std::string &poolName, const AsyncInitWorkTaskGroupUnitedCallback &callback)
	{
		base::AutoLock lock(m_lock);
		m_poolName2WorkTaskList.erase(poolName);
		m_callThread.message_loop()->PostTask(FROM_HERE, base::Bind(&CAsyncInitWorkTaskGroup::AsyncUninitWorkTask, this, poolName, callback));
		return S_OK;
	}

	HRESULT QueueWorkTask(const std::string &poolName, const base::Callback<void()> &task, const base::PoolMgrSequenceToken &token = base::PoolMgrSequenceToken(), base::PoolMgrWorkerShutdown shutdown = base::TASK_BLOCK_SHUTDOWN)
	{
		base::AutoLock lock(m_lock);
		auto it = m_poolName2WorkTaskList.find(poolName);
		if (it != m_poolName2WorkTaskList.end())
		{
			if (it->second.inited)
			{
				WorkTaskList taskList;
				taskList.push_back(WorkTask(task, token, shutdown));
				RunWorkTask(poolName, taskList);
			}
			else
			{
				it->second.taskList.push_back(WorkTask(task, token, shutdown));
			}
			return S_OK;
		}
		return E_FAIL;
	}

	HRESULT QueueTimerTask(const std::string &poolName, const base::Callback<void()> &task, unsigned long delay)
	{
		base::AutoLock lock(m_lock);
		auto it = m_poolName2WorkTaskList.find(poolName);
		if (it != m_poolName2WorkTaskList.end())
		{
			if (it->second.inited)
			{
				WorkTaskList taskList;
				taskList.push_back(WorkTask(task, delay));
				RunWorkTask(poolName, taskList);
			}
			else
			{
				it->second.taskList.push_back(WorkTask(task, delay));
			}
			return S_OK;
		}
		return E_FAIL;
	}

	void AsyncInitWorkTask(const std::string &poolName, int threadCount)
	{
		prg::InitWorkTask(threadCount, poolName);

		base::AutoLock lock(m_lock);
		auto it = m_poolName2WorkTaskList.find(poolName);
		if (it != m_poolName2WorkTaskList.end())
		{
			it->second.inited = true;
			RunWorkTask(poolName, it->second.taskList);
            it->second.taskList.clear();
		}
	}

	void AsyncUninitWorkTask(const std::string &poolName, AsyncInitWorkTaskGroupUnitedCallback callback)
	{
		prg::UninitWorkTask(poolName);
		callback.Run();
	}

	HRESULT RunWorkTask(const std::string &poolName, const WorkTaskList &taskList)
	{
        HRESULT hr = S_OK;
		auto it = taskList.begin();
		for (; it != taskList.end(); it++)
		{
			if (it->delay != 0)
			{
				hr = prg::QueueTimerTask(it->task, it->delay, poolName)?S_OK:E_FAIL;
			}
			else
			{
				if (it->token.valid)
				{
					hr = prg::QueueWorkTask(it->task, poolName, it->shutdown)?S_OK:E_FAIL;
				}
				else
				{
					hr = prg::QueueSequencedWorkTask(it->token, it->task, poolName, it->shutdown)?S_OK:E_FAIL;
				}
			}
		}
        return hr;
	}

private:
	base::Thread m_callThread;
	base::Lock m_lock;
	std::map<std::string, WorkTaskManifest> m_poolName2WorkTaskList;
};