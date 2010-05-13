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

#include "core-api/Prerequisite.h"
#include "core-api/ConditionVariable.h"
#include "core-api/Singleton.h"
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
#define MAXIMUM_CONCURRENT_CALLS 1024

namespace zillians {

/**
 * @brief Worker mimics boost::asio::io_service to serve as a task dispatcher.
 *
 * Worker is a generalized and simplified version of boost::asio::io_service,
 * providing an internal worker thread to execute specific functions.
 */
class Worker
{
public:
	/**
	 * @brief Construct a worker object.
	 */
	Worker() :
		mTerminated(false),
		mThread(boost::bind(&Worker::run, this))
	{
		for(uint32 i=0;i<MAXIMUM_CONCURRENT_CALLS;++i)
		{
			mAvailableConditionSlots.push(i);
			mConditions[i] = new zillians::ConditionVariable<uint32>();
		}
	}

	/**
	 * @brief Destroy a worker object.
	 */
	virtual ~Worker()
	{
		stop();
		for(uint32 i=0;i<MAXIMUM_CONCURRENT_CALLS;++i)
		{
			SAFE_DELETE(mConditions[i]);
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
	template<typename CompletionHandler>
	inline void dispatch(CompletionHandler handler, bool blocking = false)
	{
		if(blocking)
		{
			void (Worker::*f)(uint32 /*key*/, boost::tuple<CompletionHandler> /*handler*/) = &Worker::wrap<CompletionHandler>;

			uint32 key = 0; mAvailableConditionSlots.pop(key);
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

		uint32 key = 0; mAvailableConditionSlots.pop(key);

		mConditions[key]->reset();
		mIoService.post(boost::bind(f, this, key, boost::make_tuple(handler)));
		return key;
	}

	inline void wait(int key)
	{
		uint32 dummy = 0;
		mConditions[key]->wait(dummy);
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

			uint32 key = 0; mAvailableConditionSlots.pop(key);
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
			mThread.join();
		}
	}

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
		boost::get<0>(handler)();
		mConditions[key]->signal(0);
		mAvailableConditionSlots.push(key);
	}

protected:
	bool mTerminated;
	boost::asio::io_service mIoService;
	boost::thread mThread;
	tbb::concurrent_bounded_queue<uint32> mAvailableConditionSlots;
	boost::array<zillians::ConditionVariable<uint32>*, MAXIMUM_CONCURRENT_CALLS> mConditions;
};

/**
 * GlobalWorker is a singleton version of Worker, so anyone can invoke functions asynchronously without having to new a Worker instance
 */
class GlobalWorker : public Worker, public Singleton<GlobalWorker>
{
public:
	explicit GlobalWorker(std::size_t defaultWokerSize = 16)
	{
		for(int i=0;i<defaultWokerSize;++i)
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
