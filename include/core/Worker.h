/**
 * Zillians MMO
 * Copyright (C) 2007-2009 Zillians.com, Inc.
 * For more information see http://www.zillians.com
 *
 * Zillians MMO is the library and runtime for massive multiplayer online game
 * development in utility computing model, which runs as a service for every
 * developer to build their virtual world running on our GPU-assisted machines.
 *
 * This is a close source library intended to be used solely within Zillians.com
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/**
 * @date Aug 26, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_WORKER_H_
#define ZILLIANS_WORKER_H_

#include "core/Prerequisite.h"
#include "core/ConditionVariable.h"
#include "core/Singleton.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <tbb/concurrent_queue.h>

/**
 * @brief The maximum concurrent blocking calls to a single worker instance.
 *
 * Since the worker relies on asynchronous completion reactor (i.e. the
 * boost::asio::io_service), all calls are executed in the internal thread
 * context. The blocking (synchronous) dispatch uses a condition variable
 * to wait for completion acknowledgment sent by the internal thread.
 */
#define ZILLIANS_WORKER_DEFAULT_MAXIMUM_CONCURRENT_CALLS 1024

namespace zillians {

/**
 * @brief Worker mimics boost::asio::io_service to serve as a task dispatcher.
 *
 * Worker is a generalized and simplified version of boost::asio::io_service,
 * providing an internal worker thread to execute specific functions.
 */
class Worker
{
	friend class WorkerGroup;
	Worker(zillians::ConditionVariable<uint32>** conditions, tbb::concurrent_bounded_queue<uint32>* slots, volatile bool* flags) :
		mTerminated(false),
		mConditionTableOwner(false),
		mThread(boost::bind(&Worker::run, this))
	{
		mConditionSlots = slots;
		mConditions = conditions;
		mCancellationFlags = flags;
	}

public:
	/**
	 * @brief Construct a worker object.
	 */
	Worker(std::size_t maxConcurrentCalls = ZILLIANS_WORKER_DEFAULT_MAXIMUM_CONCURRENT_CALLS) :
		mTerminated(false),
		mConditionTableOwner(true),
		mConditionTableSize(maxConcurrentCalls),
		mThread(boost::bind(&Worker::run, this))
	{
		mConditionSlots = new tbb::concurrent_bounded_queue<uint32>();
		mConditions = new zillians::ConditionVariable<uint32>*[mConditionTableSize];
		mCancellationFlags = new bool[mConditionTableSize];

		for(uint32 i=0;i<mConditionTableSize;++i)
		{
			mConditionSlots->push(i);
			mConditions[i] = new zillians::ConditionVariable<uint32>();
			mCancellationFlags[i] = false;
		}
	}

	/**
	 * @brief Destroy a worker object.
	 */
	virtual ~Worker()
	{
		stop();

		if(mConditionTableOwner)
		{
			for(uint32 i=0;i<mConditionTableSize;++i)
			{
				SAFE_DELETE(mConditions[i]);
			}
			SAFE_DELETE_ARRAY(mCancellationFlags);
			SAFE_DELETE_ARRAY(mConditions);
			SAFE_DELETE(mConditionSlots);
		}
	}

public:
	/**
	 * @brief Request the worker to invoke the given handler.
	 *
	 * This method is used to ask the worker to execute the given handler.
	 *
	 * It is guaranteed that the handler will be called from the internal
	 * thread context. The handler may be executed inside this method.
	 *
	 * @param handler The handler to be called. The worker will a copy of
	 * the handler object as required. The function signature of the handler
	 * must be: @code void handler(); @endcode
	 *
	 * @param blocking Set the flag for blocking invocation mode. False for
	 * asynchronous invocation, which is the default. True for synchronous
	 * invocation.
	 */
	template<typename CompletionHandler>//NOTE 20100728 Nothing - Tha name "CompletionHandler" sounds like it's a handler to be called after completion of the dispatched job, strange.
	inline void dispatch(CompletionHandler handler, bool blocking = false)
	{
		if(blocking)
		{
			void (Worker::*f)(uint32 /*key*/, boost::tuple<CompletionHandler> /*handler*/) = &Worker::wrap<CompletionHandler>;

			uint32 key = 0; mConditionSlots->pop(key);
			mCancellationFlags[key] = false;
			mIoService.dispatch(boost::bind(f, this, key, boost::make_tuple(handler)));
			uint32 dummy = 0; mConditions[key]->wait(dummy);
		}
		else
		{
			mIoService.dispatch(handler);
		}
	}

	template<typename CompletionHandler>
	inline int async(CompletionHandler handler)
	{
		void (Worker::*f)(uint32 /*key*/, boost::tuple<CompletionHandler> /*handler*/) = &Worker::wrap<CompletionHandler>;

		uint32 key = 0; mConditionSlots->pop(key);
		mCancellationFlags[key] = false;
		mConditions[key]->reset();
		mIoService.post(boost::bind(f, this, key, boost::make_tuple(handler)));
		return key;
	}

	inline void wait(int key)
	{
		uint32 dummy = 0;
		mConditions[key]->wait(dummy);
	}

	bool timed_wait(int key, const boost::system_time& absolute)
	{
		uint32 dummy = 0;
		return mConditions[key]->timed_wait(dummy, absolute);
	}

    template<typename DurationType>
    bool timed_wait(int key, const DurationType& relative)
    {
    	uint32 dummy = 0;
    	return mConditions[key]->timed_wait(dummy, relative);
    }

    void cancel(int key)
    {
    	mCancellationFlags[key] = true;
    }

	/**
	 * @brief Post the given handler to job queue of the worker and return.
	 *
	 * This method saves the given handler into the internal job queue,
	 * and returns immediately. The handler will not be executed inside
	 * this method.
	 *
	 * @param handler The handler to be called. The worker will a copy of
	 * the handler object as required. The function signature of the handler
	 * must be: @code void handler(); @endcode
	 */
	template<typename CompletionHandler>
	inline void post(CompletionHandler handler, bool blocking = false)
	{
		if(blocking)
		{
			void (Worker::*f)(uint32 /*key*/, boost::tuple<CompletionHandler> /*handler*/) = &Worker::wrap<CompletionHandler>;

			uint32 key = 0; mConditionSlots->pop(key);
			mCancellationFlags[key] = false;
			mIoService.post(boost::bind(f, this, key, boost::make_tuple(handler)));
			uint32 dummy = 0; mConditions[key]->wait(dummy);
		}
		else
		{
			mIoService.post(handler);
		}
	}

public:
	/**
	 * @brief The internal thread run procedure.
	 */
	void run()
	{
		// create a dummy work to avoid running out of job until stop() is explicitly called
		boost::asio::io_service::work w(mIoService);
		while(!mTerminated)
		{
			try
			{
				mIoService.run();
				break;
			}
			catch(std::exception& e)
			{
				printf("exception e: %s\n", e.what());
			}
		}
	}

	void stop()
	{
		if(!mTerminated)
		{
			mTerminated = true;
			mIoService.stop();
			if(mThread.joinable())
				mThread.join();
		}
	}

public:
	boost::asio::io_service& getIoService()
	{ return mIoService; }

protected:
	/**
	 * @brief Wrap the given handler with synchronous acknowledgment.
	 *
	 * @param key The acknowledgment key, which is basically the
	 * index of used condition variable.
	 *
	 * @param handler The handler to be called. he worker will a copy of
	 * the handler object as required. The function signature of the handler
	 * must be: @code void handler(); @endcode
	 */
	template<typename CompletionHandler>
	inline void wrap(uint32 key, boost::tuple<CompletionHandler> handler)
	{
		if(!mCancellationFlags[key])
		{
			boost::get<0>(handler)();
			mConditions[key]->signal(0);
			mConditionSlots->push(key);
		}
		else
		{
			mCancellationFlags[key] = false;
		}
	}

protected:
	boost::asio::io_service mIoService;
	bool mTerminated;
	bool mConditionTableOwner;
	uint32 mConditionTableSize;
	boost::thread mThread;
	tbb::concurrent_bounded_queue<uint32>* mConditionSlots;
	zillians::ConditionVariable<uint32>** mConditions;
	volatile bool* mCancellationFlags;
};

class WorkerGroup
{
public:
	struct load_balancing_t
	{
		enum type { round_robin, least_load_first };
	};

	explicit WorkerGroup(std::size_t workers = 1, std::size_t threads_per_worker = 16, load_balancing_t::type policy = load_balancing_t::round_robin, std::size_t maxConcurrentCalls = ZILLIANS_WORKER_DEFAULT_MAXIMUM_CONCURRENT_CALLS)
	{
		// initialize load balancing context
		mLoadBalancingPolicy = policy;
		if(mLoadBalancingPolicy == load_balancing_t::round_robin)
		{
			mLoadBalancingContext.round_robin.current = 0;
		}
		else if(mLoadBalancingPolicy == load_balancing_t::least_load_first)
		{
			BOOST_ASSERT("least-load-first policy is not yet implemented" && 0);
		}
		else
		{
			BOOST_ASSERT("unknown load balancing policy is provided" && 0);
		}

		// create shared condition variables and slots
		mConditionTableSize = workers * maxConcurrentCalls;
		mConditionSlots = new tbb::concurrent_bounded_queue<uint32>();
		mConditions = new zillians::ConditionVariable<uint32>*[mConditionTableSize];
		mCancellationFlags = new bool[mConditionTableSize];

		for(uint32 i=0;i<mConditionTableSize;++i)
		{
			mConditionSlots->push(i);
			mConditions[i] = new zillians::ConditionVariable<uint32>();
			mCancellationFlags[i] = false;
		}

		// create all workers with shared condition variables and slots
		mWorkerSize = workers;
		mWorkers = new Worker*[workers];
		for(std::size_t i=0;i<workers;++i)
		{
			mWorkers[i] = new Worker(mConditions, mConditionSlots, mCancellationFlags);

			// spawn default threads on each worker
			// (note that there's one thread associated with the worker by default, that's why we minus one here)
			for(std::size_t j=0;j<threads_per_worker - 1;++j)
			{
				boost::thread *t = new boost::thread(boost::bind(&Worker::run, mWorkers[i]));
				mWorkerThreads.push_back(t);
			}
		}
	}

	virtual ~WorkerGroup()
	{
		// stop all workers
		for(int i=0;i<mWorkerSize;++i)
		{
			mWorkers[i]->stop();
		}

		// join on all worker threads
		for(std::vector<boost::thread*>::iterator i = mWorkerThreads.begin(); i != mWorkerThreads.end(); ++i)
		{
			if((*i)->joinable())
			{
				(*i)->join();
			}
			delete (*i);
		}
		mWorkerThreads.clear();

		// destroy all workers
		for(int i=0;i<mWorkerSize;++i)
		{
			SAFE_DELETE(mWorkers[i]);
		}
		SAFE_DELETE_ARRAY(mWorkers);

		// destroy shared condition variables and slots
		for(uint32 i=0;i<mConditionTableSize;++i)
		{
			SAFE_DELETE(mConditions[i]);
		}
		SAFE_DELETE_ARRAY(mConditions);
		SAFE_DELETE(mConditionSlots);
	}

public:
	template<typename CompletionHandler>
	inline void dispatch(CompletionHandler handler, bool blocking = false)
	{
		if(mLoadBalancingPolicy == load_balancing_t::round_robin)
		{
			mWorkers[mLoadBalancingContext.round_robin.current]->dispatch(handler, blocking);
			++mLoadBalancingContext.round_robin.current;
			if(mLoadBalancingContext.round_robin.current >= mWorkerSize) mLoadBalancingContext.round_robin.current -= mWorkerSize;
		}
		else if(mLoadBalancingPolicy == load_balancing_t::least_load_first)
		{
			BOOST_ASSERT("least-load-first policy is not yet implemented" && 0);
		}
	}

	template<typename CompletionHandler>
	inline void post(CompletionHandler handler, bool blocking = false)
	{
		if(mLoadBalancingPolicy == load_balancing_t::round_robin)
		{
			mWorkers[mLoadBalancingContext.round_robin.current]->post(handler, blocking);
			++mLoadBalancingContext.round_robin.current;
			if(mLoadBalancingContext.round_robin.current >= mWorkerSize) mLoadBalancingContext.round_robin.current -= mWorkerSize;
		}
		else if(mLoadBalancingPolicy == load_balancing_t::least_load_first)
		{
			BOOST_ASSERT("least-load-first policy is not yet implemented" && 0);
		}
	}

	template<typename CompletionHandler>
	inline int async(CompletionHandler handler)
	{
		if(mLoadBalancingPolicy == load_balancing_t::round_robin)
		{
			mWorkers[mLoadBalancingContext.round_robin.current]->async(handler);
			++mLoadBalancingContext.round_robin.current;
			if(mLoadBalancingContext.round_robin.current >= mWorkerSize) mLoadBalancingContext.round_robin.current -= mWorkerSize;
		}
		else if(mLoadBalancingPolicy == load_balancing_t::least_load_first)
		{
			BOOST_ASSERT("least-load-first policy is not yet implemented" && 0);
		}
	}

	inline void wait(int key)
	{
		uint32 dummy = 0;
		mConditions[key]->wait(dummy);
	}

protected:
	int mWorkerSize;
	Worker** mWorkers;
	load_balancing_t::type mLoadBalancingPolicy;
	struct
	{
		struct
		{
			int current;
		} round_robin;

		struct
		{
			// NOT IMPLEMENT YET
		} least_load_first;
	} mLoadBalancingContext;
	std::vector<boost::thread*> mWorkerThreads;
	uint32 mConditionTableSize;
	tbb::concurrent_bounded_queue<uint32>* mConditionSlots;
	zillians::ConditionVariable<uint32>** mConditions;
	volatile bool* mCancellationFlags;
};

/**
 * GlobalWorker is a singleton version of Worker, so anyone can invoke functions asynchronously without having to new a Worker instance
 */
class GlobalWorker : public Worker, public Singleton<GlobalWorker>
{
public:
	explicit GlobalWorker(std::size_t defaultWokerSize = 16)
	{
		for(uint32 i=0;i<defaultWokerSize;++i)
		{
			boost::thread *t = new boost::thread(boost::bind(&Worker::run, this));
			mWorkerThreads.push_back(t);
		}
	}

	virtual ~GlobalWorker()
	{
		stop();
		for(std::vector<boost::thread*>::iterator i = mWorkerThreads.begin(); i != mWorkerThreads.end(); ++i)
		{
			if((*i)->joinable())
			{
				(*i)->join();
			}
			delete (*i);
		}
		mWorkerThreads.clear();
	}

private:
	std::vector<boost::thread*> mWorkerThreads;
};

}

#endif/*ZILLIANS_WORKER_H_*/
