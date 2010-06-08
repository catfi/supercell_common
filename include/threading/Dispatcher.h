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

#include "core-api/Prerequisite.h"
#include "core-api/Semaphore.h"
#include "core-api/AtomicQueue.h"
#include "threading/DispatcherThread.h"
#include "threading/DispatcherNetwork.h"

#define ZILLIANS_DISPATCHER_MAX_THREADS		64
#define ZILLIANS_DISPATCHER_PIPE_CHUNK_SIZE	256

namespace zillians { namespace threading {

template<typename Message>
class Dispatcher : public DispatcherNetwork<Message>
{
public:
	Dispatcher(uint32 max_dispatcher_threads)
	{

	}

	~Dispatcher()
	{

	}

public:
	shared_ptr<DispatcherThread> createThread()
	{
		// create dispatcher thread object

		// store the signaler object into the local signaler array
	}

public:
	virtual void write(uint32 source, uint32 destination, const Message& message)
	{

	}

	virtual bool read(uint32 source, uint32 destination, Message* message)
	{
		return false;
	}

private:
	atomic::AtomicPipe<Message, ZILLIANS_DISPATCHER_PIPE_CHUNK_SIZE>** mPipes;
	Semaphore** mSignalers;
};

} }

#endif /* ZILLIANS_THREADING_DISPATCHER_H_ */
