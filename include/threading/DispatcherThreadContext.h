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

#ifndef ZILLIANS_THREADING_DISPATCHERTHREADCONTEXT_H_
#define ZILLIANS_THREADING_DISPATCHERTHREADCONTEXT_H_

#include "core/Prerequisite.h"
#include "core/SharedPtr.h"
#include "core/ContextHub.h"
#include "threading/Dispatcher.h"
#include "threading/DispatcherNetwork.h"
#include "threading/DispatcherDestination.h"
#include "threading/DispatcherThreadSignaler.h"

namespace zillians { namespace threading {

template<typename Message>
class DispatcherThreadContext : public ContextHub<ContextOwnership::transfer>
{
public:
	DispatcherThreadContext(DispatcherNetwork<Message>* dispatcher, uint32 id, uint32 max_thread_id) : mId(id), mMaxThreadId(max_thread_id), mDispatcher(dispatcher)
	{ }

	~DispatcherThreadContext()
	{
		mDispatcher->distroyThreadContext(mId);
	}

public:
	uint32 getIdentity() const
	{ return mId; }

	DispatcherNetwork<Message>* getDispatcherNetwork() const
	{ return mDispatcher; }

	DispatcherThreadSignaler& getSignaler()
	{ return mSignaler; }

public:
	shared_ptr<DispatcherDestination<Message> > createDestination(uint32 dest)
	{
		return shared_ptr<DispatcherDestination<Message> >(new DispatcherDestination<Message>(mDispatcher, mId, dest));
	}

public:
	bool read(/*OUT*/ uint32& source, /*OUT*/ Message& message, bool blocking = false)
	{
		uint32 n = 1;
		return read(&source, &message, n, blocking);
	}

	/**
	 * Read the first message available from any pipes
	 * @param source
	 * @param message
	 */
	bool read(/*OUT*/ uint32* source, /*OUT*/ Message* message, /*INOUT*/ uint32& count, bool blocking = false)
	{
		uint64 signals = 0;
		uint32 n = 0;

		if(blocking)
			signals = mSignaler.poll(mId);
		else
			signals = mSignaler.check();

		if(signals)
		{
			for(int i = 0; i < mMaxThreadId && n < count; ++i)
			{
				if(signals & uint64 (1) << i)
				{
					mSignaler.bitZeroSet(i);
					for(; n < count; ++n)
					{
						if(!mDispatcher->read(i, mId, &message[n]))
							break;

						if(source)
							source[n] = i;
					}
				}
//				mSignaler.reset();
			}
			BOOST_ASSERT(n > 0);
		}

		return n > 0;
	}

private:
	DispatcherThreadSignaler mSignaler;
	DispatcherNetwork<Message>* mDispatcher;
	uint32 mId;
	uint32 mMaxThreadId;
};

} }

#endif /* ZILLIANS_THREADING_DISPATCHERTHREADCONTEXT_H_ */
