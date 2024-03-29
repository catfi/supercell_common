/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
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
 * @date Jun 8, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_THREADING_DISPATCHER_H_
#define ZILLIANS_THREADING_DISPATCHER_H_

#include "core/Prerequisite.h"
#include "core/Semaphore.h"
#include "core/AtomicQueue.h"
#include "threading/DispatcherThreadContext.h"
#include "threading/DispatcherNetwork.h"

#define ZILLIANS_DISPATCHER_MAX_THREADS		63
#define ZILLIANS_DISPATCHER_PIPE_CHUNK_SIZE	256

namespace zillians { namespace threading {

template<typename Message>
class Dispatcher : public DispatcherNetwork<Message>
{
public:
	typedef atomic::AtomicPipe<Message, ZILLIANS_DISPATCHER_PIPE_CHUNK_SIZE> ContextPipe;

public:
	Dispatcher(uint32 max_dispatcher_threads = ZILLIANS_DISPATCHER_MAX_THREADS) : mMaxThreadContextCount(max_dispatcher_threads)
	{
		BOOST_ASSERT(max_dispatcher_threads <= ZILLIANS_DISPATCHER_MAX_THREADS);

		mPipes = new ContextPipe*[max_dispatcher_threads * max_dispatcher_threads];
		mSignalers = new DispatcherThreadSignaler*[max_dispatcher_threads];
		mAttachedFlags = new bool[mMaxThreadContextCount];

		for(uint32 i = 0; i < mMaxThreadContextCount; ++i)
		{
			mSignalers[i] = NULL;
			mAttachedFlags[i] = false;
		}

		for(uint32 i = 0; i < mMaxThreadContextCount * mMaxThreadContextCount; ++i)
		{
			mPipes[i] = new ContextPipe();
		}
	}

	~Dispatcher()
	{
		for(uint32 i=0;i<mMaxThreadContextCount;++i)
		{
			BOOST_ASSERT(!mAttachedFlags[i]);
		}

		for(uint32 i = 0; i < mMaxThreadContextCount * mMaxThreadContextCount; ++i)
		{
			SAFE_DELETE(mPipes[i]);
		}

		SAFE_DELETE_ARRAY(mPipes);
		SAFE_DELETE_ARRAY(mSignalers);
		SAFE_DELETE_ARRAY(mAttachedFlags);
	}

public:
	shared_ptr<DispatcherThreadContext<Message> > createThreadContext(int contextId = -1)
	{
		if(contextId == -1)
		{
			for(uint32 i = 0; i < mMaxThreadContextCount; ++i)
			{
				if(!mAttachedFlags[i])
				{
					contextId = i;
					break;
				}
			}
		}

		if(contextId < 0)
		{
			BOOST_ASSERT("out of thread context" && 0);
		}

		BOOST_ASSERT(mAttachedFlags[contextId] == false && "context already assigned");

		mAttachedFlags[contextId] = true;
		shared_ptr<DispatcherThreadContext<Message> > context = shared_ptr<DispatcherThreadContext<Message> >(new DispatcherThreadContext<Message>(this, contextId, mMaxThreadContextCount));

		// store the signaler object into the local signaler array
		mSignalers[contextId] = &context->getSignaler();

		return context;
	}	

	virtual void distroyThreadContext(uint32 contextId)
	{
		BOOST_ASSERT(mAttachedFlags[contextId] == true);
		mAttachedFlags[contextId] = false;
		mSignalers[contextId] = NULL;
	}

public:
	virtual void write(uint32 source, uint32 destination, const Message& message, bool incomplete)
	{
		ContextPipe* pipes = mPipes[source * mMaxThreadContextCount + destination];
		pipes->write(message, incomplete);

		if(!incomplete)
		{
			pipes->flush();
			mSignalers[destination]->signal(source);
		}
	}

	virtual bool read(uint32 source, uint32 destination, Message* message)
	{
		return mPipes[source * mMaxThreadContextCount + destination]->read(message);
	}

private:
	ContextPipe** mPipes;
	DispatcherThreadSignaler** mSignalers;
	bool* mAttachedFlags;
	uint32 mMaxThreadContextCount;
};

} }

#endif /* ZILLIANS_THREADING_DISPATCHER_H_ */
