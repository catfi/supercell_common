//
// Zillians MMO
// Copyright (C) 2007-2008 Zillians.com, Inc.
// For more information see http://www.zillians.com
//
// Zillians MMO is the library and runtime for massive multiplayer online game
// development in utility computing model, which runs as a service for every
// developer to build their virtual world running on our GPU-assisted machines
//
// This is a close source library intended to be used solely within Zillians.com
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Contact Information: info@zillians.com
//

#ifndef LIBEVQUEUE_H_
#define LIBEVQUEUE_H_

#include "core/ByteBuffer.h"
#include "core/Callback.h"
#include "networking/Message.h"
#include "networking/queue/Queue.h"
#include "networking/queue/QueueEngine.h"
#include "networking/address/InetSocketAddress.h"
#include "networking/queue/impl/ev/LibEvWrapper.h"
#include "tbb/concurrent_queue.h"
#include "tbb/spin_mutex.h"

namespace zillians {

// forward declaration
class LibEvQueueEngine;

class LibEvQueue : public Queue
{
public:
	LibEvQueue(const handle_t &handle, const InetSocketAddress &address, LibEvQueueEngine *ref);
	virtual ~LibEvQueue();

public:
	virtual Address* getAddress() const;

public:
	virtual int32 send(Message *message);
	virtual void close();

public:
	handle_t getHandle();

protected:
	bool compilePendingMessages();

	void handleDeviceRead(ev::io &w, int revent);
	void handleDeviceWrite(ev::io &w, int revent);
	void handleDeviceTimeout(ev::timer &w, int revent);

private:
	bool completeDeviceWrite(ev::io &w);

private:
	static log4cxx::LoggerPtr mLogger;

private:
	LibEvQueueEngine* mEngineRef;
	handle_t mHandle;

private:
	InetSocketAddress mAddress;

	tbb::concurrent_queue<Message*> mPendingWrites;
	Message* mWriteOnHold;

	ev::io mReadWatcher;
	ByteBuffer mReadBuffer;

	ev::io mWriteWatcher;
	ByteBuffer mWriteBuffer;

	bool mWriteRequestCompleted;
	tbb::spin_mutex mLockWriteRequestCompleted;
	//Callback0* mLockWriteRequestHandledCallback;

	tbb::atomic<bool> mConnected;
	ev::timer mTimeoutWatcher;
};

}

#endif /* LIBEVQUEUE_H_ */
