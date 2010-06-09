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

#ifndef TCPCONNECTION_H_
#define TCPCONNECTION_H_

#include "core-api/Prerequisite.h"
#include "core-api/Buffer.h"
#include "core-api/HashMap.h"
#include "net-api/sys/Poller.h"
#include "net-api/sys/tcp/TcpCommon.h"

#include "tbb/atomic.h"
#include "tbb/spin_mutex.h"

namespace zillians { namespace net {

class TcpNetEngine;

class TcpConnection
{
	friend class TcpConnector;
	friend class TcpAcceptor;

public:
	typedef handle_t HandleType;

public:
	TcpConnection(TcpNetEngine* engine, handle_t id);
	virtual ~TcpConnection();

	//////////////////////////////////////////////////////////////////////
	//// Public Interfaces
	//////////////////////////////////////////////////////////////////////
	static shared_ptr<TcpConnection> create(TcpNetEngine* engine, handle_t id)
	{
		shared_ptr<TcpConnection> p(new TcpConnection(engine, id));
		p->mWeakThis = p;
		return p;
	}

	inline HandleType getHandle() {	return mSocket; }

	shared_ptr<Buffer> createBuffer(size_t size);
	bool send(uint32 type, shared_ptr<Buffer> buffer);

	void close();

	void setTimeout(int32 ms);

	inline void resetContext() { mContext.reset(); }
	inline void setContext(shared_ptr<void> context) { mContext = context; }
	inline shared_ptr<void> getContext() { return mContext; }

	//////////////////////////////////////////////////////////////////////
	//// Parameter Adjust
	//////////////////////////////////////////////////////////////////////
	int32 mMaxSendInFlight;
	void setMaxSendInFlight(int32 maxSendInFlight);

private:
	bool directSend(uint32 type, shared_ptr<Buffer> buffer);
	bool queueSend(uint32 type, shared_ptr<Buffer> buffer);

public:
	void handleDeviceRead(ev::io &w, int revent);
	void handleDeviceWrite(ev::io &w, int revent);
	void handleDeviceTimeout(ev::timer &w, int revent);

private:
	bool completeDeviceWrite(ev::io &w);

private:
	bool requestSend();

private:
	void start(shared_ptr<Poller> poller);
	void stop();

private:
	struct SendRequest
	{
		uint32 processed;
		byte header[TCP_DEFAULT_BUFFER_HEADER_SIZE];
		shared_ptr<Buffer> buffer;
	};

	typedef std::vector<SendRequest> RequestQueue;
	//typedef tbb::concurrent_vector<SendRequest> RequestQueue;
	//typedef tbb::concurrent_queue<SendRequest> RequestQueue;

	RequestQueue mSendRequestQueue;
	tbb::spin_mutex mSendRequestQueueLock;

	tbb::spin_mutex mSendLock;

	ev::io mReadWatcher;
	ev::io mWriteWatcher;
	tbb::spin_mutex mWriteWatcherLock;
	ev::timer mTimeoutWatcher;

private:
	static log4cxx::LoggerPtr mLogger;

	TcpNetEngine* mEngine;
	WeakPtr<TcpConnection> mWeakThis;

	shared_ptr<Poller> mPoller;
    bool mConnected;

    shared_ptr<Buffer> mReadBuffer;

    handle_t mSocket;

    uint32 mMaxIOVCount;
	iovec* mIOV;

	shared_ptr<void> mContext;
};

} }

#endif /* TCPCONNECTION_H_ */
