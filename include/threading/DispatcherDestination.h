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

#ifndef ZILLIANS_THREADING_DISPATCHERDESTINATION_H_
#define ZILLIANS_THREADING_DISPATCHERDESTINATION_H_

#include "core/Prerequisite.h"
#include "core/SharedPtr.h"
#include "core/ContextHub.h"
#include "threading/DispatcherNetwork.h"

namespace zillians { namespace threading {

template<typename Message>
class DispatcherDestination : public ContextHub<ContextOwnership::transfer>
{
public:
	DispatcherDestination(DispatcherNetwork<Message>* dispatcher, uint32 sourceId, uint32 destId)
	{
		mDispatcher = dispatcher;
		mDestinationId = destId;
		mSouceId = sourceId;
	}

	~DispatcherDestination()
	{ }

public:
	DispatcherNetwork<Message>* getDispatcherNetwork() const
	{ return mDispatcher; }

public:
	void write(const Message& message)
	{
		write(&message, 1);
	}

	void write(const Message* message, uint32 count)
	{
		for(int i = 0; i < count - 1; ++i)
			mDispatcher->write(mSouceId, mDestinationId, message[i], true);
		mDispatcher->write(mSouceId, mDestinationId, message[count-1], false);
	}

	bool read(Message* message, bool blocking = false)
	{
		return read(message, 1, blocking);
	}

	bool read(Message* messages, uint32 count, bool blocking = false)
	{
		uint32 n = 0;

		if(UNLIKELY(blocking))
		{
			while(!mDispatcher->read(mSouceId, mDestinationId, messages)) { }
			++n;
		}

		for(;n < count; ++n)
		{
			if(!mDispatcher->read(mSouceId, mDestinationId, messages))
				break;
		}

		return n > 0U;
	}

private:
	DispatcherNetwork<Message>* mDispatcher;
	uint32 mDestinationId;
	uint32 mSouceId;
};

} }

#endif /* ZILLIANS_THREADING_DISPATCHERDESTINATION_H_ */
