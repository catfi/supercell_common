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
// Contact Information: info@0zillians.com
//

#include "net-api/queue/impl/ev/LibEvQueue.h"
#include "net-api/queue/impl/ev/LibEvQueueEngine.h"

#define MESSAGE_HEADER_SIZE  (sizeof(int) + sizeof(uint))
#define MESSAGE_TYPE(x)      (*(int*)((byte*)x + 0))
#define MESSAGE_LENGTH(x)    (*(uint*)((byte*)x + sizeof(uint)))

namespace zillians {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr LibEvQueue::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.provider.ev.LibEvQueue"));

//////////////////////////////////////////////////////////////////////////
LibEvQueue::LibEvQueue(const handle_t &handle, const InetSocketAddress &address, LibEvNetEngine* ref)
{
	mEngineRef = ref;

	mHandle = handle;
	mAddress = address;

	mWriteRequestCompleted = true;
	mWriteOnHold = NULL;
	mConnected = true;

	// setup watchers
	mReadWatcher.set(mHandle, ev::READ);
	mReadWatcher.set< LibEvQueue, &LibEvQueue::handleDeviceRead > (this);

	mWriteWatcher.set(mHandle, ev::WRITE);
	mWriteWatcher.set< LibEvQueue, &LibEvQueue::handleDeviceWrite > (this);

	// TODO set the global timeout (need a place to store these configuration)
	mTimeoutWatcher.set (0.0, 10.0);
	mTimeoutWatcher.set< LibEvQueue, &LibEvQueue::handleDeviceTimeout > (this);

	// link read/write/timeout watchers with ev processors
	mEngineRef->getDaemon()->getReadProcessor()->evIoStart(&mReadWatcher);
	//mEngineRef->getDaemon()->getWriteProcessor()->evIoStart(&mWriteWatcher);
	// TODO implement some configuration to config socket timeout
	//mEngineRef->getDaemon()->getReadProcessor()->evTimerStart(&mTimeoutWatcher);
}

LibEvQueue::~LibEvQueue()
{
	close();
}

//////////////////////////////////////////////////////////////////////////
handle_t LibEvQueue::getHandle()
{
	return mHandle;
}

//////////////////////////////////////////////////////////////////////////
Address* LibEvQueue::getAddress() const
{
	return (Address*)&mAddress;
}

int32 LibEvQueue::send(Message *message)
{
	// TODO put some maximum pending message limit here, and return false when reaching this limit
	// if the socket has been closed, just return failed
	if(UNLIKELY(!message))
		return ZN_ERROR;

	if(!mConnected)
		return ZN_ERROR;

	if(true)
	{
		tbb::spin_mutex::scoped_lock lock(mLockWriteRequestCompleted);

		if(mWriteRequestCompleted == true)
		{
			//LOG4CXX_DEBUG(mLogger, "Schedule restarting write watcher");
			mEngineRef->getDaemon()->getWriteProcessor()->evIoStart(&mWriteWatcher);
			mWriteRequestCompleted = false;
		}

		SAFE_ADDREF(message);
		mPendingWrites.push(message);
	}

	return ZN_OK;
}

void LibEvQueue::close()
{
	if(!mConnected)
		return;

	mConnected = false;

	LOG4CXX_DEBUG(mLogger, "Try to close Queue");
	// if the socket has been closed, just return
	if(mHandle == INVALID_HANDLE)
	{
		LOG4CXX_DEBUG(mLogger, "Close on invalid handle!");
		return;
	}

	// dispatch close Queue event
	mEngineRef->getQueueEventDispatcher()->dispatchDisconnected(this);

	// stop all LibEv watchers
	if(mWriteWatcher.is_active())   mEngineRef->getDaemon()->getWriteProcessor()->evIoStop(&mWriteWatcher);
	if(mReadWatcher.is_active())    mEngineRef->getDaemon()->getReadProcessor()->evIoStop(&mReadWatcher);
	if(mTimeoutWatcher.is_active()) mEngineRef->getDaemon()->getReadProcessor()->evTimerStop(&mTimeoutWatcher);

	// shutdown and close socket connection
	if(mHandle != INVALID_HANDLE)
	{
		::shutdown(mHandle, SHUT_RDWR);
		::close(mHandle);
		mHandle = INVALID_HANDLE;
	}

	// clean up message queue
	while(UNLIKELY(!mPendingWrites.empty()))
	{
		Message* next = NULL;
		mPendingWrites.pop_if_present(next);
		if(UNLIKELY(!next)) break;
		SAFE_RELEASE(next);
	}
	SAFE_RELEASE(mWriteOnHold);
}

//////////////////////////////////////////////////////////////////////////
bool LibEvQueue::compilePendingMessages()
{
	bool result = false;
	int count = 0;

	// move pending write buffer to ongoing write buffer
	while(UNLIKELY(!mPendingWrites.empty()))
	{
		Message* next = NULL;

		if(mWriteOnHold)
		{
			next = mWriteOnHold;
			mWriteOnHold = NULL;
		}
		else
		{
			mPendingWrites.pop_if_present(next);
		}
		if(UNLIKELY(!next)) break;

		const size_t msgLength = next->length();
		const size_t packetLength = msgLength + MESSAGE_HEADER_SIZE;

		// make sure the message size is smaller then the allocated write buffer
		if(UNLIKELY(packetLength > mWriteBuffer.allocatedSize()))
		{
			LOG4CXX_DEBUG(mLogger, "Message is bigger then write buffer, skipped");
			SAFE_RELEASE(next);
			continue;
		}

		// make sure the free size of write buffer is bigger then message size
		//LOG4CXX_DEBUG(mLogger, "packetLength = " << packetLength << ", freeSize() = " << mWriteBuffer.freeSize());
		if(UNLIKELY(packetLength > mWriteBuffer.freeSize()))
		{
			if(packetLength <= mWriteBuffer.allocatedSize() - mWriteBuffer.dataSize())
			{
				// move all data to the beginning of the buffer
				//LOG4CXX_DEBUG(mLogger, "Try to crunch the buffer");
				mWriteBuffer.crunch();
			}
			else
			{
				// we ran out the write buffer, let's wait for next time
				// since we can't push the message back to the end of the concurrent queue,
				// just save it in a dummy variable to process that later
				//LOG4CXX_DEBUG(mLogger, "Hold on buffer full");
				mWriteOnHold = next;
				break;
			}
		}

		// fill the header
		MESSAGE_TYPE(mWriteBuffer.wptr()) = next->type();
		MESSAGE_LENGTH(mWriteBuffer.wptr()) = msgLength;

		// remember the last position to go back later (we need to fill the header later if there's inconsistency in message format)
		const size_t lastPos = mWriteBuffer.wpos(mWriteBuffer.wpos() + MESSAGE_HEADER_SIZE);

		// try to encode the message
		if(UNLIKELY(next->encode(mWriteBuffer) != ZN_OK))
		{
			LOG4CXX_DEBUG(mLogger, "Message fail to encode, skipped");
			SAFE_RELEASE(next);

			// roll back
			mWriteBuffer.wpos(lastPos - MESSAGE_HEADER_SIZE);
			continue;
		}
		else
		{
			const size_t currPos = mWriteBuffer.wpos();
			const size_t encodeLength = currPos - lastPos;

			// assertion check: if the encoded message is different from length(), log
			if(encodeLength != msgLength)
			{
				LOG4CXX_DEBUG(mLogger, "Message format inconsistency! message type = " << next->type() << ", encode length = " <<  encodeLength << ", given length = " << msgLength);

				// rev back to fill the header
				mWriteBuffer.wpos(lastPos - MESSAGE_HEADER_SIZE);

				// fill the header with correct encoded length
				MESSAGE_TYPE(mWriteBuffer.wptr()) = next->type();
				MESSAGE_LENGTH(mWriteBuffer.wptr()) = encodeLength;

				// move the the end of message
				mWriteBuffer.wpos(currPos);
			}

			result = true;
		}

		count++;
		SAFE_RELEASE(next);
	}

	//printf("compilePendingMessage count = %d\n", count);
	return result;
}

void LibEvQueue::handleDeviceRead(ev::io &w, int revent)
{
	// NOTE: here we're in the execution context of reader thread (which is the same as timeout checker thread)
	// TODO implement QoS, throttle each connection receiving speed to make it fair and avoid DoS

	while(true)
	{
		if(UNLIKELY(mConnected == false))
		{
			LOG4CXX_DEBUG(mLogger, "Receiving from closed connection");
			w.stop();
			break;
		}

		// read from socket
		const int recvLength = ::recv(mHandle, mReadBuffer.wptr(), mReadBuffer.freeSize(), 0);
		if(UNLIKELY(recvLength < 1))
		{
			if(LIKELY(recvLength == -1 && Errno::code() == EAGAIN))
			{
				// non-blocking read pending, wait for next available data
				break;
			}
			else if(recvLength == 0)
			{
				// remote close connection
				LOG4CXX_DEBUG(mLogger, "Remote connection closed");
				close();
				break;
			}
			else
			{
				// something goes wrong here
				LOG4CXX_DEBUG(mLogger, "Error while receiving, recv returns = " << recvLength << ", errno = " << Errno::detail());
				close();
			}
			break;
		}

		// reset read timeout timer
		//LOG4CXX_DEBUG(mLogger, "Timer Agained");
		if(mTimeoutWatcher.is_active()) mTimeoutWatcher.again();

		// advance buffer
		mReadBuffer.wpos(mReadBuffer.wpos() + recvLength);

		int msgCount = 0;
		// parse the stream and slice it into packets
		while(mReadBuffer.dataSize() >= MESSAGE_HEADER_SIZE)
		{
			// get message type and length
			int msgType = MESSAGE_TYPE(mReadBuffer.rptr());
			int msgLength = MESSAGE_LENGTH(mReadBuffer.rptr());

			ASSERT(msgLength > 0);

			//LOG4CXX_INFO(mLogger, "Receiving msg_type = " << msgType << ", msg_length = " << msgLength);

			// if there's no enough data received, break
			if(mReadBuffer.dataSize() < (long)MESSAGE_HEADER_SIZE + (long)msgLength)
			{
				//LOG4CXX_DEBUG(mLogger, "Partial message received, recv length = " << recvLength << ", dispatched msg count = " << msgCount << ", data size = " << mReadBuffer.dataSize() << ", msg length = " << msgLength);
				break;
			}

			// advance buffer by header size
			const size_t lastPos = mReadBuffer.rpos(mReadBuffer.rpos() + MESSAGE_HEADER_SIZE);

			// create message by message factory
			mEngineRef->getMessageDispatcher()->dispatch(msgType, msgLength, this, &mReadBuffer);
			++msgCount;

			// advance buffer by message size (this makes sure even if the decoder messed up, we can still get correct message offset for following messages)
			mReadBuffer.rpos(lastPos + msgLength);
		}

		// move remain buffer (if any) to the head
		mReadBuffer.crunch();
	}

}

void LibEvQueue::handleDeviceWrite(ev::io &w, int revent)
{
	// NOTE: here we're in the execution context of writer thread
	// TODO implement QoS, throttle each connection sending speed to make it fair and avoid DoS

	while(true)
	{
		if(UNLIKELY(mConnected == false))
		{
			LOG4CXX_DEBUG(mLogger, "Sending to closed connection");
			w.stop();
			break;
		}

		// try to fill up the buffer
		compilePendingMessages();

		if(LIKELY(mWriteBuffer.dataSize() > 0))
		{
			// if there's data in the write buffer, send it
			const int written = ::send(mHandle, mWriteBuffer.rptr(), mWriteBuffer.dataSize(), 0);

			// check if write failed (due to system error or insufficient buffer or others)
			if(UNLIKELY(written == -1))
			{
				if(UNLIKELY(Errno::code() != EAGAIN))
				{
					LOG4CXX_DEBUG(mLogger, "Fail to send, errno = " << Errno::detail());
					// probably the socket is closed remotely or some internally error just happened
					close();
					break;
				}
				else
				{
					//LOG4CXX_DEBUG(mLogger, "OS send buffer full, wait for next trigger, errno = " << Errno::detail() << ", data size = " << mWriteBuffer.dataSize());
					// we've used up all available system socket buffer, let's wait for next time
					break;
				}
			}

			// advance write buffer
			mWriteBuffer.rpos(mWriteBuffer.rpos() + written);
		}
		else
		{
			if(completeDeviceWrite(w))
			{
				// all writes are completed
				break;
			}
		}

	}
}

void LibEvQueue::handleDeviceTimeout(ev::timer &w, int revent)
{
	// NOTE: here we're in the execution context of reader thread (which is the same as timeout checker thread)

	// where the time is out, close the connection and log it
	LOG4CXX_DEBUG(mLogger, "read timeout, close connection");
	close();
}

bool LibEvQueue::completeDeviceWrite(ev::io &w)
{
	tbb::spin_mutex::scoped_lock lock(mLockWriteRequestCompleted);

	// then try to fill up the buffer again if there're still some pending messages
	if(UNLIKELY(!mPendingWrites.empty()))
	{
		return false;
	}
	else
	{
		// we dont need to ask processor to stop the watcher because we're in the same execution context of writer thread
		// so we just call watcher's stop()
		//LOG4CXX_DEBUG(mLogger, "Complete write, stop watcher");

		//mEngineRef->getDaemon()->getWriteProcessor()->evIoStop(&mWriteWatcher);
		w.stop();

		ASSERT(mPendingWrites.empty());
		ASSERT(!mWriteRequestCompleted);

		mWriteRequestCompleted = true;

		return true;
	}
}

}
