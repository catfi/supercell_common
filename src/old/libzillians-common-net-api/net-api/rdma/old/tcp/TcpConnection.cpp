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

#include "networking/sys/tcp/TcpConnection.h"
#include "networking/sys/tcp/TcpNetEngine.h"

namespace zillians { namespace net {

#define BUFFER_HEADER_SIZE  (sizeof(uint32) + sizeof(uint32))
#define BUFFER_HEADER_TYPE(x)      (*(uint32*)((byte*)x + 0))
#define BUFFER_HEADER_LENGTH(x)    (*(uint32*)((byte*)x + sizeof(uint32)))

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr TcpConnection::mLogger(log4cxx::Logger::getLogger("zillians.common.networking.sys.tcp.TcpConnection"));

//////////////////////////////////////////////////////////////////////////
TcpConnection::TcpConnection(TcpNetEngine* engine, handle_t id)
{
	mEngine = engine;
	mSocket = id;

	mConnected = true;

	mReadBuffer = createBuffer(TCP_DEFAULT_RECEIVE_BUFFER_SIZE);

	mIOV = NULL;
	setMaxSendInFlight(TCP_DEFAULT_MAX_SEND_IN_FLIGHT);
}

TcpConnection::~TcpConnection()
{
	close();
	SAFE_DELETE_ARRAY(mIOV);
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<Buffer> TcpConnection::createBuffer(size_t size)
{
	ASSERT(mEngine->getBufferManager());

	shared_ptr<Buffer> buffer = mEngine->getBufferManager()->createBuffer(size);

	if(!buffer)
	{
		TCP_ERROR("[ERROR] BufferManager returns NULL buffer! OUT OF MEMORY!")
	}

	return buffer;
}

bool TcpConnection::send(uint32 type, shared_ptr<Buffer> buffer)
{
	ASSERT(buffer->dataSize() > 0);

	if(!mConnected)
		return false;

	if(buffer->dataSize() > TCP_DEFAULT_MXA_SEND_SIZE)
	{
		TCP_ERROR("the buffer is too big to be sent over TcpConnection, the maximum allowed size = " << TCP_DEFAULT_MXA_SEND_SIZE);
		return false;
	}

	tbb::spin_mutex::scoped_lock lock_send(mSendLock);

	if(requestSend())
	{
		return directSend(type, buffer);
	}
	else
	{
		return queueSend(type, buffer);
	}
}

void TcpConnection::close()
{
	if(!mConnected)
		return;

	mConnected = false;

	// if the socket has been closed, just return
	if(mSocket == INVALID_HANDLE)
	{
		LOG4CXX_DEBUG(mLogger, "close on invalid handle!");
		return;
	}

	// remove itself from TcpNetEngine
	shared_ptr<TcpConnection> shared_from_this(mWeakThis);
	mEngine->removeConnection(shared_from_this);

	// dispatch to upper application
	mEngine->getDispatcher()->dispatchConnectionEvent(TcpDispatcher::DISCONNECTED, shared_from_this);

	// stop all LibEv watchers
	stop();

	// shutdown and close socket connection
	if(mSocket != INVALID_HANDLE)
	{
		// TODO change to two way handshake shutdown protocol
		::shutdown(mSocket, SHUT_RDWR);
		::close(mSocket);
		mSocket = INVALID_HANDLE;
	}

	// clean up message queue
	mSendRequestQueue.clear();
}

void TcpConnection::setTimeout(int32 ms)
{
	//TODO: mTimeoutWatcher.set()
}

//////////////////////////////////////////////////////////////////////////
void TcpConnection::setMaxSendInFlight(int32 maxSendInFlight)
{
	mMaxSendInFlight = maxSendInFlight;

	tbb::spin_mutex::scoped_lock lock_queue(mSendRequestQueueLock);

	SAFE_DELETE_ARRAY(mIOV);

	if(mMaxSendInFlight < 0)
	{
		mMaxIOVCount = UIO_MAXIOV;
		mIOV = new iovec[mMaxIOVCount];
	}
	else
	{
		mMaxIOVCount = min(mMaxSendInFlight*2, UIO_MAXIOV);
		mIOV = new iovec[mMaxIOVCount];
	}
}

//////////////////////////////////////////////////////////////////////////
bool TcpConnection::directSend(uint32 type, shared_ptr<Buffer> buffer)
{
	TCP_DEBUG("[DirectSend] send direct, type = " << type << ", buffer size = " << buffer->dataSize());

	// create request object
	TcpConnection::SendRequest request;
	request.processed = 0;
	BUFFER_HEADER_TYPE(request.header) = type;
	BUFFER_HEADER_LENGTH(request.header) = (uint32)buffer->dataSize();
	request.buffer = buffer;

	// create the iovec list
	iovec iov[2];
	iov[0].iov_base = (void*)request.header;
	iov[0].iov_len = BUFFER_HEADER_SIZE;
	iov[1].iov_base = request.buffer->rptr();
	iov[1].iov_len = BUFFER_HEADER_LENGTH(request.header);

	// calculate the total size
	uint32 total_size = BUFFER_HEADER_SIZE + BUFFER_HEADER_LENGTH(request.header);

	// send via writev
	uint32 total_sent = ::writev(mSocket, (const iovec*)iov, 2);

	TCP_DEBUG("[DirectSend] total_sent = " << total_sent << ", total_size = " << total_size << ", iov_count = 2");
	// check the status
	if(total_sent == -1)
	{
		if(errno == EAGAIN)
		{
			TCP_DEBUG("[DirectSend] writev EAGAIN, start watcher");

			mSendRequestQueue.push_back(request);

			// start the write watcher
			mPoller->start(&mWriteWatcher);
		}
		else
		{
			TCP_ERROR("[DirectSend] writev returns " << total_sent << ", err = " << strerror(errno));
			close();
			return false;
		}
	}
	else if(total_sent < total_size)
	{
		request.processed = total_sent;
		mSendRequestQueue.push_back(request);

		// start the write watcher
		mPoller->start(&mWriteWatcher);

		TCP_DEBUG("[DirectSend] partial write, start watcher");
	}
	else
	{
		TCP_DEBUG("[DirectSend] full write completed right away");
	}

	return true;
}

bool TcpConnection::queueSend(uint32 type, shared_ptr<Buffer> buffer)
{
	TCP_DEBUG("[QueueSend] send queued, type = " << type << ", buffer size = " << buffer->dataSize());

	// create request object
	TcpConnection::SendRequest request;
	request.processed = 0;
	BUFFER_HEADER_TYPE(request.header) = type;
	BUFFER_HEADER_LENGTH(request.header) = (uint32)buffer->dataSize();
	request.buffer = buffer;

	tbb::spin_mutex::scoped_lock lock(mSendRequestQueueLock);

	if(mMaxSendInFlight >= 0 && mSendRequestQueue.size() >= mMaxSendInFlight)
	{
		TCP_DEBUG("[QueueSend] send queue reaches the maximum size, return fail");
		return false;
	}

	mSendRequestQueue.push_back(request);

	return true;
}

//////////////////////////////////////////////////////////////////////////
void TcpConnection::handleDeviceRead(ev::io &w, int revent)
{
	while(true)
	{
		if(UNLIKELY(mConnected == false))
		{
			TCP_ERROR("[HandleDeviceRead] receiving from closed connection");
			w.stop();
			break;
		}

		// read from socket
		const int recv_length = ::recv(mSocket, mReadBuffer->wptr(), mReadBuffer->freeSize(), 0);
		TCP_DEBUG("[HandleDeviceRead] recv_length = " << recv_length);

		if(UNLIKELY(recv_length < 1))
		{
			if(LIKELY(recv_length == -1 && errno == EAGAIN))
			{
				// non-blocking read pending, wait for next available data
			}
			else if(recv_length == 0)
			{
				// remote close connection
				TCP_DEBUG("[HandleDeviceRead] remote connection closed");
				close();
			}
			else
			{
				// something goes wrong here
				TCP_ERROR("[HandleDeviceRead] error while receiving, recv returns = " << recv_length << ", err = " << strerror(errno));
				close();
			}
			break;
		}

		// reset read timeout timerServerHandler
		if(mTimeoutWatcher.is_active()) mTimeoutWatcher.again();

		// advance buffer
		mReadBuffer->wpos(mReadBuffer->wpos() + recv_length);

		// parse the stream and slice it into packets
		while(mReadBuffer->dataSize() >= BUFFER_HEADER_SIZE)
		{
			// get message type and length
			uint32 buffer_type   = BUFFER_HEADER_TYPE(mReadBuffer->rptr());
			uint32 buffer_length = BUFFER_HEADER_LENGTH(mReadBuffer->rptr());

			TCP_DEBUG("[HandleDeviceRead] receiving buffer_type = " << buffer_type << ", buffer_length = " << buffer_length);

			// if there's no enough data received, break
			if(mReadBuffer->dataSize() < (uint32)BUFFER_HEADER_SIZE + (uint32)buffer_length)
			{
				TCP_DEBUG("[HandleDeviceRead] partial message received, recv length = " << recv_length << ", data size = " << mReadBuffer->dataSize() << ", expected buffer length = " << buffer_length);
				break;
			}

			// slice (copy) the buffer
			shared_ptr<Buffer> buffer = createBuffer(buffer_length);
			buffer->writeArray(mReadBuffer->rptr() + BUFFER_HEADER_SIZE, buffer_length);
			//shared_ptr<Buffer> buffer(new Buffer(mReadBuffer->rptr() + BUFFER_HEADER_SIZE, buffer_length));
			//buffer->wpos(buffer_length);

			// advance the read pointer
			mReadBuffer->rpos(mReadBuffer->rpos() + BUFFER_HEADER_SIZE + buffer_length);

			// dispatch the buffer to application
			shared_ptr<TcpConnection> shared_from_this(mWeakThis);
			mEngine->getDispatcher()->dispatchDataEvent(buffer_type, buffer, shared_from_this);
		}

		// move remain buffer (if any) to the head of the buffer
		mReadBuffer->crunch();
	}
}

void TcpConnection::handleDeviceWrite(ev::io &w, int revent)
{
	if(UNLIKELY(!mConnected))
	{
		TCP_ERROR("[HandleDeviceWrite] sending to closed connection");
		w.stop();
	}

	while(true)
	{
		tbb::spin_mutex::scoped_lock lock_send(mSendLock);
		// start trying to flush the queue
		//tbb::spin_mutex::scoped_lock lock_queue;

		//lock_queue.acquire(mSendRequestQueueLock);

		// quit if the send request queue is empty
		if(mSendRequestQueue.empty())
		{
			//tbb::spin_mutex::scoped_lock lock_watcher(mWriteWatcherLock);

			// all request in send request queue are finished without error, stop write watcher
			mWriteWatcher.stop();

			TCP_DEBUG("[HandleDeviceWrite] all write completed, stop write watcher");

			break;
		}

		// compile send request into iovec
		uint32 iov_count = 0;
		uint32 total_size = 0;
		uint32 req_count = 0;
		for(TcpConnection::RequestQueue::iterator it = mSendRequestQueue.begin(); it != mSendRequestQueue.end() && iov_count < mMaxIOVCount-1; ++it, ++req_count)
		{
			if(it->processed < BUFFER_HEADER_SIZE)
			{
				mIOV[iov_count].iov_base = ((byte*)it->header) + it->processed;
				mIOV[iov_count].iov_len = BUFFER_HEADER_SIZE - it->processed;
				total_size += mIOV[iov_count].iov_len;
				++iov_count;

				mIOV[iov_count].iov_base = it->buffer->rptr();
				mIOV[iov_count].iov_len = it->buffer->dataSize();
				total_size += mIOV[iov_count].iov_len;
				++iov_count;
			}
			else
			{
				mIOV[iov_count].iov_base = it->buffer->rptr() + (it->processed - BUFFER_HEADER_SIZE);
				mIOV[iov_count].iov_len = it->buffer->dataSize() - (it->processed - BUFFER_HEADER_SIZE);
				total_size += mIOV[iov_count].iov_len;
				++iov_count;
			}
		}

		ASSERT(iov_count <= mMaxIOVCount);

		// send via writev
		errno = 0;
		ssize_t total_sent = ::writev(mSocket, (const iovec*)mIOV, iov_count);

		//lock_queue.release();

		TCP_DEBUG("[HandleDeviceWrite] total_sent = " << total_sent << ", total_size = " << total_size << ", iov_count = " << iov_count);

		if(total_sent == -1)
		{
			if(errno == EAGAIN)
			{
				if(errno == EAGAIN)
				{
					TCP_DEBUG("[HandleDeviceWrite] writev EAGAIN");
				}
				else
				{
					TCP_ERROR("[HandleDeviceWrite] writev error returns " << total_sent << ", err = " << strerror(errno));
					close();
				}
			}
			else
			{
				TCP_ERROR("[HandleDeviceWrite] writev error returns = " << total_sent << ", err = " << strerror(errno));
				close();
			}

			// wait for next write event (if there's any)
			break;
		}
		else if(total_sent < total_size)
		{
			//lock_queue.acquire(mSendRequestQueueLock);

			while(!mSendRequestQueue.empty())
			{
				if(total_sent + mSendRequestQueue.front().processed < BUFFER_HEADER_SIZE + mSendRequestQueue.front().buffer->dataSize())
				{
					mSendRequestQueue.front().processed += total_sent;
					TCP_DEBUG("last element processed = " << mSendRequestQueue.front().processed);
					total_sent = 0;
					break;
				}
				else
				{
					total_sent -= mSendRequestQueue.front().buffer->dataSize() + BUFFER_HEADER_SIZE - mSendRequestQueue.front().processed;
					mSendRequestQueue.erase(mSendRequestQueue.begin());
				}
			}

			//lock_queue.release();

			// wait for next write event
			break;
		}
		else if(total_sent == total_size)
		{
			//lock_queue.acquire(mSendRequestQueueLock);

			mSendRequestQueue.erase(mSendRequestQueue.begin(), mSendRequestQueue.begin() + req_count);

			//lock_queue.release();
		}
		else
		{
			TCP_ERROR("[HandleDeviceWrite] total_sent is more then total_size!! total_sent = " << total_sent << ", total_size = " << total_size);
		}
	}
}

void TcpConnection::handleDeviceTimeout(ev::timer &w, int revent)
{
}

//////////////////////////////////////////////////////////////////////////
bool TcpConnection::requestSend()
{
	if(!mConnected)
		return false;

	{
		//tbb::spin_mutex::scoped_lock lock_watcher(mWriteWatcherLock);
		if(mWriteWatcher.is_active())
			return false;
	}

	{
		//tbb::spin_mutex::scoped_lock lock_queue(mSendRequestQueueLock);
		if(mSendRequestQueue.size() > 0)
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void TcpConnection::start(shared_ptr<Poller> poller)
{
	mPoller = poller;

	// bind read event to handleDeviceRead()
	mReadWatcher.set(mSocket, ev::READ);
	mReadWatcher.set< TcpConnection, &TcpConnection::handleDeviceRead > (this);

	// bind write event to handleDeviceWrite()
	mWriteWatcher.set(mSocket, ev::WRITE);
	mWriteWatcher.set< TcpConnection, &TcpConnection::handleDeviceWrite > (this);

	// TODO set the global timeout (need a place to store these configuration)
	mTimeoutWatcher.set (0.0, 10.0);
	mTimeoutWatcher.set< TcpConnection, &TcpConnection::handleDeviceTimeout > (this);

	// start read/write/timeout watchers
	mPoller->start(&mReadWatcher);
}

void TcpConnection::stop()
{
	// stop all LibEv watchers
	if(mWriteWatcher.is_active())   mPoller->stop(&mWriteWatcher);
	if(mReadWatcher.is_active())    mPoller->stop(&mReadWatcher);
	if(mTimeoutWatcher.is_active()) mPoller->stop(&mTimeoutWatcher);
}

} }
