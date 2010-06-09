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
 * @date Aug 9, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_SYS_TCPSESSIONIMPL_H_
#define ZILLIANS_NET_SYS_TCPSESSIONIMPL_H_

#include "core-api/Prerequisite.h"
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread.hpp>
#include <tbb/atomic.h>
#include <tbb/queuing_mutex.h>
#include <tbb/spin_mutex.h>
#include <boost/scope_exit.hpp>

using namespace zillians;

namespace zillians { namespace net { namespace sys {

// TODO delete this when error occurred or disconnected
template<>
class SessionT< SessionTransport::tcp >
{
	typedef boost::asio::ip::tcp::socket Socket;
	typedef boost::asio::io_service IoService;

public:
	SessionT(IoService& io_service) :
		mIoService(io_service), mSocket(io_service)
	{
		mDeletionMark = false;
		mPendingCompletions = 0;
	}

	~SessionT()
	{
		BOOST_ASSERT(mPendingCompletions == 0);

		mTlsWriteBuffer.reset();
		mTlsReadBuffer.reset();
		for(uint32 i=0;i<ksMaximumSupportedContextTypes;++i)
		{
			mContext[i].reset();
		}
	}

	Socket& socket()
	{
		return mSocket;
	}

	void write(uint32 type, std::vector< shared_ptr<Buffer> >& buffers)
	{
		std::size_t bytes_written = 0;

		// calculate the total number of bytes to send
		std::size_t bytes_to_write = 0;
		for(std::vector< shared_ptr<Buffer> >::iterator it = buffers.begin(); it != buffers.end(); ++it) bytes_to_write += (*it)->dataSize();

		// ASSERTION the given data size cannot exceed the underlying transport limit
		BOOST_ASSERT(bytes_to_write <= boost::asio::detail::default_max_transfer_size - detail::MessageHeader::kHeaderSize);

		// if the total number of bytes to send is bigger than some threshold, use scatter send to avoid copying
		// otherwise send it in one single buffer (including the header)
		if(bytes_to_write > kScatterWriteThreshold)
		{
			BOOST_ASSERT(buffers.size() < ksMaxIOV - 1);

			// use thread-local buffer as header buffer
			Buffer* header_buffer = getTlsWriteBuffer();

			// encode the header
			detail::MessageHeader header;
			header.type = type;
			header.size = bytes_to_write;
			*header_buffer << header;

			// create asio write buffer vector to perform scatter write
			std::vector<boost::asio::mutable_buffer> write_buffers;

			// append header buffer to write buffer
			write_buffers.push_back( boost::asio::buffer(header_buffer->rptr(), detail::MessageHeader::kHeaderSize) );

			// append all given buffers to write buffer
			for(std::vector< shared_ptr<Buffer> >::iterator it = buffers.begin(); it != buffers.end(); ++it)
				write_buffers.push_back( boost::asio::buffer((*it)->rptr(), (*it)->dataSize()) );

			// perform asio synchronous write
			bytes_written = boost::asio::write(mSocket, write_buffers);

			header_buffer->clear();
		}
		else
		{
			// use thread-local buffer as write buffer
			Buffer* write_buffer = getTlsWriteBuffer();

			// encode the header
			detail::MessageHeader header;
			header.type = type;
			header.size = bytes_to_write;
			*write_buffer << header;

			// append all given buffers to write buffer
			for(std::vector< shared_ptr<Buffer> >::iterator it = buffers.begin(); it != buffers.end(); ++it)
			{
				// preserve the old read position so that it is not altered by append()
				std::size_t old_rpos = (*it)->rpos();
				write_buffer->append(**it);
				(*it)->rpos(old_rpos);
			}

			// perform asio synchronous write
			bytes_written = boost::asio::write(mSocket, boost::asio::buffer(write_buffer->rptr(), write_buffer->dataSize()));

			write_buffer->clear();
		}


		if(bytes_written != bytes_to_write + detail::MessageHeader::kHeaderSize)
		{
			throw boost::system::system_error(boost::asio::error::broken_pipe);
		}
		else
		{
			for(std::vector< shared_ptr<Buffer> >::iterator it = buffers.begin(); it != buffers.end(); ++it)
				(*it)->rskip((*it)->dataSize());
		}
	}

	void write(uint32 type, shared_ptr<Buffer>& buffer, std::size_t size = 0)
	{
		std::size_t bytes_written = 0;
		std::size_t bytes_to_write = (size == 0) ? buffer->dataSize() : size;
		BOOST_ASSERT(bytes_to_write <= buffer->dataSize());
		BOOST_ASSERT(bytes_to_write <= boost::asio::detail::default_max_transfer_size - detail::MessageHeader::kHeaderSize);

		if(bytes_to_write > kScatterWriteThreshold)
		{
			// use thread-local buffer as header buffer
			Buffer* header_buffer = getTlsWriteBuffer();

			// encode the header
			detail::MessageHeader header;
			header.type = type;
			header.size = bytes_to_write;
			*header_buffer << header;

			// create asio write buffer vector to perform scatter write
			std::vector<boost::asio::mutable_buffer> buffers;
			buffers.push_back( boost::asio::buffer(header_buffer->rptr(), detail::MessageHeader::kHeaderSize) );
			buffers.push_back( boost::asio::buffer(buffer->rptr(), buffer->dataSize()) );

			// perform asio synchronous write
			bytes_written = boost::asio::write(mSocket, buffers);

			header_buffer->clear();
		}
		else
		{
			// use thread-local buffer as write buffer
			Buffer* write_buffer = getTlsWriteBuffer();

			// encode the header
			detail::MessageHeader header;
			header.type = type;
			header.size = bytes_to_write;
			*write_buffer << header;

			// append the given buffer to write buffer
			// preserve the old read position so that it is not altered by append()
			std::size_t old_rpos = buffer->rpos();
			write_buffer->append(*buffer, bytes_to_write);
			buffer->rpos(old_rpos);

			// perform asio synchronous write
			bytes_written = boost::asio::write(mSocket, boost::asio::buffer(write_buffer->rptr(), write_buffer->dataSize()));

			write_buffer->clear();
		}

		if(bytes_written != bytes_to_write + detail::MessageHeader::kHeaderSize)
		{
			throw boost::system::system_error(boost::asio::error::broken_pipe);
		}
		else
		{
			buffer->rskip(bytes_written - detail::MessageHeader::kHeaderSize);
		}
	}

	template<typename M>
	void write(M& message)
	{
		// get the buffer from TLS
		Buffer* buffer = getTlsWriteBuffer();

		try
		{
			// skip the header first
			buffer->wskip(detail::MessageHeader::kHeaderSize);

			// serialize the data first so we know how large it is
			*buffer << message;
			size_t end_pos = buffer->wpos();

			// encode the header
			detail::MessageHeader header;
			header.type = M::TYPE;
			header.size = end_pos - detail::MessageHeader::kHeaderSize;

			BOOST_ASSERT(header.size <= boost::asio::detail::default_max_transfer_size - detail::MessageHeader::kHeaderSize);

			buffer->wpos(0);
			*buffer << header;
			buffer->wpos(end_pos);

			std::size_t bytes_written = boost::asio::write(mSocket, boost::asio::buffer(buffer->rptr(), buffer->dataSize()));

			if(bytes_written != buffer->dataSize())
			{
				throw boost::system::system_error(boost::asio::error::broken_pipe);
			}

			buffer->clear();
		}
		catch(std::exception& e)
		{
			throw e;
		}
	}

	/// Asynchronously write a collection of buffers to the session
	template<typename Handler>
	void writeAsync(uint32 type, std::vector< shared_ptr<Buffer> >& buffers, Handler handler)
	{
		// calculate the total number of bytes to send
		std::size_t bytes_to_write = 0;
		for(std::vector< shared_ptr<Buffer> >::iterator it = buffers.begin(); it != buffers.end(); ++it) bytes_to_write += (*it)->dataSize();

		// ASSERTION the given data size cannot exceed the underlying transport limit
		BOOST_ASSERT(bytes_to_write <= boost::asio::detail::default_max_transfer_size - detail::MessageHeader::kHeaderSize);

		// if the total number of bytes to send is bigger than some threshold, use scatter send to avoid copying
		// otherwise send it in one single buffer (including the header)
		if(bytes_to_write > kScatterWriteThreshold)
		{
			BOOST_ASSERT(buffers.size() < ksMaxIOV - 1);

			// create a local buffer to store headers
			shared_ptr<Buffer> header_buffer(new Buffer(detail::MessageHeader::kHeaderSize));

			// encode the header
			detail::MessageHeader header;
			header.type = type;
			header.size = bytes_to_write;
			*header_buffer << header;

			// create asio write buffer vector to perform scatter write
			std::vector<boost::asio::mutable_buffer> write_buffers;
			write_buffers.push_back( boost::asio::buffer(header_buffer->rptr(), header_buffer->dataSize()) );

			// insert all given buffer one by one
			for(std::vector< shared_ptr<Buffer> >::iterator it = buffers.begin(); it != buffers.end(); ++it)
				write_buffers.push_back( boost::asio::buffer((*it)->rptr(), (*it)->dataSize()) );

			// lock the structures for write queues
			tbb::queuing_mutex::scoped_lock lock(mSendLock);

			// if there's no on-going write event, write directly and push the operation to sending queue
			// otherwise push the operation onto pending queue
			if(mSendingQueue.empty())
			{
				void (SessionT::*f)(const boost::system::error_code&, const std::size_t, const std::size_t) = &SessionT::writeAsyncBufferCompleted<Handler>;

				// increment pending completion
				++mPendingCompletions;

				// perform asio async write
				boost::asio::async_write(
						mSocket,
						write_buffers,
						boost::bind(f, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, detail::MessageHeader::kHeaderSize + header.size));

				// push the write operation along with the completion handler into sending queue
				mSendingQueue.push_back(boost::make_tuple(handler, WriteOperation(header_buffer, buffers)));
			}
			else
			{
				// push the write operation along with the completion handler into pending queue
				mPendingQueue.push_back(boost::make_tuple(handler, WriteOperation(header_buffer, buffers)));
			}
		}
		else
		{
			// create a local buffer to store headers as well as given buffers
			shared_ptr<Buffer> write_buffer(new Buffer(detail::MessageHeader::kHeaderSize + bytes_to_write));

			// encode the header
			detail::MessageHeader header;
			header.type = type;
			header.size = bytes_to_write;
			*write_buffer << header;

			// append all given buffers to the write buffer
			for(std::vector< shared_ptr<Buffer> >::iterator it = buffers.begin(); it != buffers.end(); ++it)
			{
				// preserve the old read position so that it is not altered by append()
				std::size_t old_rpos = (*it)->rpos();
				write_buffer->append(**it);
				(*it)->rpos(old_rpos);
			}

			// consume all given buffers (since we have encoded them into a single write buffer
			for(std::vector< shared_ptr<Buffer> >::iterator it = buffers.begin(); it != buffers.end(); ++it)
			{
				(*it)->rskip((*it)->dataSize());
			}

			// lock the structures for write queues
			tbb::queuing_mutex::scoped_lock lock(mSendLock);

			// if there's no on-going write event, write directly and push the operation to sending queue
			// otherwise push the operation onto pending queue
			if(mSendingQueue.empty())
			{
				void (SessionT::*f)(const boost::system::error_code&, const std::size_t, const std::size_t) = &SessionT::writeAsyncBufferCompleted<Handler>;

				// increment pending completion
				++mPendingCompletions;

				// perform asio async write
				boost::asio::async_write(
						mSocket,
						boost::asio::buffer(write_buffer->rptr(), write_buffer->dataSize()),
						boost::bind(f, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, detail::MessageHeader::kHeaderSize + header.size));

				// push the write operation along with the completion handler into sending queue
				mSendingQueue.push_back(boost::make_tuple(handler, WriteOperation(write_buffer)));
			}
			else
			{
				// push the write operation along with the completion handler into pending queue
				mPendingQueue.push_back(boost::make_tuple(handler, WriteOperation(write_buffer)));
			}
		}
	}

	/// Asynchronously write a buffer to the session
	template<typename Handler>
	void writeAsync(uint32 type, shared_ptr<Buffer>& buffer, Handler handler, std::size_t size = 0)
	{
		// calculate the total number of bytes to send
		std::size_t bytes_to_write = (size == 0) ? buffer->dataSize() : size;

		// ASSERTION the number of bytes to write cannot exceed the actual data size of the given buffer
		BOOST_ASSERT(bytes_to_write <= buffer->dataSize());

		// ASSERTION the given data size cannot exceed the underlying transport limit
		BOOST_ASSERT(bytes_to_write <= boost::asio::detail::default_max_transfer_size - detail::MessageHeader::kHeaderSize);

		// if the total number of bytes to send is bigger than some threshold, use scatter send to avoid copying
		// otherwise send it in one single buffer (including the header)
		if(bytes_to_write > kScatterWriteThreshold)
		{
			// create a local buffer to store headers
			shared_ptr<Buffer> header_buffer(new Buffer(detail::MessageHeader::kHeaderSize));

			// encode the header
			detail::MessageHeader header;
			header.type = type;
			header.size = bytes_to_write;
			*header_buffer << header;

			// create asio write buffer vector to perform scatter write
			std::vector<boost::asio::mutable_buffer> buffers;
			buffers.push_back( boost::asio::buffer(header_buffer->rptr(), header_buffer->dataSize()) );
			buffers.push_back( boost::asio::buffer(buffer->rptr(), bytes_to_write) );

			// lock the structures for write queues
			tbb::queuing_mutex::scoped_lock lock(mSendLock);

			// if there's no on-going write event, write directly and push the operation to sending queue
			// otherwise push the operation onto pending queue
			if(mSendingQueue.empty())
			{
				void (SessionT::*f)(const boost::system::error_code&, const std::size_t, const std::size_t) = &SessionT::writeAsyncBufferCompleted<Handler>;

				// increment pending completion
				++mPendingCompletions;

				// perform asio async write
				boost::asio::async_write(
						mSocket,
						buffers,
						boost::bind(f, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, detail::MessageHeader::kHeaderSize + header.size));

				// push the write operation along with the completion handler into sending queue
				mSendingQueue.push_back(boost::make_tuple(handler, WriteOperation(header_buffer, buffer)));
			}
			else
			{
				// push the write operation along with the completion handler into pending queue
				mPendingQueue.push_back(boost::make_tuple(handler, WriteOperation(header_buffer, buffer)));
			}
		}
		else
		{
			// create a local buffer (which will be held in the write completion handler)
			shared_ptr<Buffer> write_buffer(new Buffer(detail::MessageHeader::kHeaderSize + bytes_to_write));

			detail::MessageHeader header;
			header.type = type;
			header.size = bytes_to_write;
			*write_buffer << header;

			// append the buffer into the write buffer
			write_buffer->append(*buffer, bytes_to_write);

			// lock the structures for write queues
			tbb::queuing_mutex::scoped_lock lock(mSendLock);

			// if there's no on-going write event, write directly and push the operation to sending queue
			// otherwise push the operation onto pending queue
			if(mSendingQueue.empty())
			{
				void (SessionT::*f)(const boost::system::error_code&, const std::size_t, const std::size_t) = &SessionT::writeAsyncBufferCompleted<Handler>;

				// increment pending completion
				++mPendingCompletions;

				// perform asio async write
				boost::asio::async_write(
						mSocket,
						boost::asio::buffer(write_buffer->rptr(), write_buffer->dataSize()),
						boost::bind(f, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, detail::MessageHeader::kHeaderSize + header.size));

				// push the write operation along with the completion handler into sending queue
				mSendingQueue.push_back(boost::make_tuple(handler, WriteOperation(write_buffer)));
			}
			else
			{
				// push the write operation along with the completion handler into pending queue
				mPendingQueue.push_back(boost::make_tuple(handler, WriteOperation(write_buffer)));
			}
		}
	}

	/// Asynchronously write a message structure to the session
	template<typename M, typename Handler>
	void writeAsync(M& message, Handler handler)
	{
		// create a local buffer (which will be held in the write completion handler)
		shared_ptr<Buffer> write_buffer(new Buffer(detail::MessageHeader::kHeaderSize + Buffer::probeSize(message)));

		// ASSERTION the given data size cannot exceed the underlying transport limit
		BOOST_ASSERT(write_buffer->dataSize() <= boost::asio::detail::default_max_transfer_size);

		// skip the header first
		write_buffer->wskip(detail::MessageHeader::kHeaderSize);

		// serialize the data first so we know how large it is
		*write_buffer << message;
		size_t end_pos = write_buffer->wpos();

		// format the header and save into buffer
		detail::MessageHeader header;
		header.type = M::TYPE;
		header.size = end_pos - detail::MessageHeader::kHeaderSize;
		write_buffer->wpos(0);
		*write_buffer << header;
		write_buffer->wpos(end_pos);

		// lock the structures for write queues
		tbb::queuing_mutex::scoped_lock lock(mSendLock);

		// if there's no on-going write event, write directly and push the operation to sending queue
		// otherwise push the operation onto pending queue
		if(mSendingQueue.empty())
		{
			void (SessionT::*f)(const boost::system::error_code&, const std::size_t, const std::size_t) = &SessionT::writeAsyncBufferCompleted<Handler>;

			// increment pending completion
			++mPendingCompletions;

			// perform asio async write
			boost::asio::async_write(
					mSocket,
					boost::asio::buffer(write_buffer->rptr(), write_buffer->dataSize()),
					boost::bind(f, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, detail::MessageHeader::kHeaderSize + header.size));

			// push the write operation along with the completion handler into sending queue
			mSendingQueue.push_back(boost::make_tuple(handler, WriteOperation(write_buffer)));
		}
		else
		{
			// push the write operation along with the completion handler into pending queue
			mPendingQueue.push_back(boost::make_tuple(handler, WriteOperation(write_buffer)));
		}
	}

	void read(shared_ptr<Buffer>& buffer, std::size_t size = 0)
	{
		std::size_t bytes_to_read = (size == 0) ? buffer->freeSize() : size;
		std::size_t bytes_read = boost::asio::read(mSocket, boost::asio::buffer(buffer->wptr(), bytes_to_read));
		buffer->wskip(bytes_read);

		if(bytes_to_read != bytes_read)
		{
			throw boost::system::system_error(boost::asio::error::broken_pipe);
		}
	}

//	void read(uint32& type, shared_ptr<BufferCollection> buffers)
//	{
//		BOOST_ASSERT(buffers->count() < 64);
//
//		Buffer* buffer = getTlsReadBuffer();
//
//		// read the header synchronously
//		boost::asio::read(mSocket, boost::asio::buffer(buffer->wptr(), detail::MessageHeader::kHeaderSize));
//		buffer->wskip(detail::MessageHeader::kHeaderSize);
//
//		// deserialize the header
//		detail::MessageHeader header;
//		*buffer >> header;
//		buffer->clear();
//
//		// make sure the requested data size is smaller than the buffer's available size
//		if(header.size > buffer->freeSize())
//		{
//			throw boost::system::system_error(boost::asio::error::no_buffer_space);
//		}
//
//		// create the scatter read buffers
//		std::vector<boost::asio::mutable_buffer> recv_buffers;
//		buffers->createReceiveBuffer(recv_buffers);
//
//		// read the data synchronously
//		std::size_t bytes_transferred = boost::asio::read(mSocket, recv_buffers);
//
//		if(header.size != bytes_transferred)
//		{
//			throw boost::system::system_error(boost::asio::error::broken_pipe);
//		}
//
//		buffers->rskip(bytes_transferred);
//	}

	void read(uint32& type, shared_ptr<Buffer> buffer)
	{
		// read the header synchronously
		boost::asio::read(mSocket, boost::asio::buffer(buffer->wptr(), detail::MessageHeader::kHeaderSize));
		buffer->wskip(detail::MessageHeader::kHeaderSize);

		// deserialize the header
		detail::MessageHeader header;
		*buffer >> header;
		buffer->clear();

		// make sure the requested data size is smaller than the buffer's available size
		if(header.size > buffer->freeSize())
		{
			throw boost::system::system_error(boost::asio::error::no_buffer_space);
		}

		// read the data synchronously
		std::size_t bytes_transferred = boost::asio::read(mSocket, boost::asio::buffer(buffer->wptr(), header.size));

		if(header.size != bytes_transferred)
		{
			throw boost::system::system_error(boost::asio::error::broken_pipe);
		}

		buffer->wskip(bytes_transferred);
	}

	template<typename M>
	void read(M& message)
	{
		// get the buffer from TLS
		Buffer* buffer = getTlsReadBuffer();

		try
		{
			// read the header synchronously
			boost::asio::read(mSocket, boost::asio::buffer(buffer->wptr(), detail::MessageHeader::kHeaderSize));
			buffer->wskip(detail::MessageHeader::kHeaderSize);

			// deserialize the header
			detail::MessageHeader header;
			*buffer >> header;
			buffer->clear();

			if(header.type != M::TYPE)
			{
				throw boost::system::system_error(boost::asio::error::broken_pipe);
			}

			// read the data synchronously
			boost::asio::read(mSocket, boost::asio::buffer(buffer->wptr(), header.size));
			buffer->wskip(header.size);

			// deserialize the message
			*buffer >> message;

			buffer->clear();
		}
		catch(std::exception& e)
		{
			buffer->clear();
			throw e;
		}

		buffer->clear();
	}

	template<typename Handler>
	void readAsync(shared_ptr<Buffer> buffer, Handler handler, std::size_t size = 0)
	{
		void (SessionT::*f)(const boost::system::error_code&, const std::size_t&, shared_ptr<Buffer>, boost::tuple<Handler>) = &SessionT::readAsyncCompleted<Handler>;

		std::size_t bytes_to_read = (size == 0) ? buffer->freeSize() : size;

		// increment pending completion
		++mPendingCompletions;

		boost::asio::async_read(
				mSocket,
				boost::asio::buffer(buffer->wptr(), bytes_to_read),
				boost::bind(f, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, buffer, boost::make_tuple(handler)));
	}

	 /// Asynchronously read a data structure from the socket
	template<typename M, typename Handler>
	void readAsync(M& message, Handler handler)
	{
		void (SessionT::*f)(const boost::system::error_code&, const std::size_t&, M&, boost::tuple<Handler>, shared_ptr<Buffer>) = &SessionT::readAsyncHeaderCompleted<M, Handler>;

		// create a local buffer (which will be held in the read completion handler)
		shared_ptr<Buffer> buffer(new Buffer(detail::MessageHeader::kMaxDataSize));

		// increment pending completion
		++mPendingCompletions;

		// issue a read operation to read exactly the number of bytes in a header
		boost::asio::async_read(
				mSocket,
				boost::asio::buffer(buffer->wptr(), detail::MessageHeader::kHeaderSize),
				boost::bind(f, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, boost::ref(message), boost::make_tuple(handler), buffer));
	}

	void close()
	{
		boost::system::error_code error;

		mSocket.close(error);
	}

	template<typename Handler>
	void closeAsync(Handler handler)
	{
		mIoService.dispatch(boost::bind(&SessionT::doClose<Handler>, this, boost::make_tuple(handler)));
	}

	template <typename T>
	inline void setContext(T* ctx)
	{
		refContext<T>() = shared_ptr<T>(ctx);
	}

	template <typename T>
	inline T* getContext()
	{
		shared_ptr<T> ctx = static_pointer_cast<T>(refContext<T>());
		return ctx.get();
	}

	template <typename T>
	inline void clearContext()
	{
		refContext<T>().reset();
	}

	void markForDeletion()
	{
		mDeletionMark = true;
	}

private:
	inline Buffer* getTlsReadBuffer()
	{
		Buffer* buffer = mTlsReadBuffer.get();
		if(!buffer)
		{
			buffer = new Buffer(detail::MessageHeader::kDefaultBufferSize);
			mTlsReadBuffer.reset(buffer);
		}
		return buffer;
	}

	inline Buffer* getTlsWriteBuffer()
	{
		Buffer* buffer = mTlsWriteBuffer.get();
		if(!buffer)
		{
			buffer = new Buffer(detail::MessageHeader::kDefaultBufferSize);
			mTlsWriteBuffer.reset(buffer);
		}
		return buffer;
	}

	template<typename Handler>
	void writeAsyncBufferCompleted(const boost::system::error_code& ec, const std::size_t bytes_transferred, const std::size_t expected_bytes_transferred)
	{
		tbb::queuing_mutex::scoped_lock lock(mSendLock);

		// if session closed, receiving zero-length write completion
		if(UNLIKELY(bytes_transferred == 0))
		{
			// invoke all callbacks in sending queue
			for(std::list<boost::tuple< WriteCompletionHandler, WriteOperation > >::iterator it = mSendingQueue.begin(); it != mSendingQueue.end(); it++)
			{
				boost::get<0>(*it)(ec, 0);
			}
			mSendingQueue.clear();

			// invoke all callbacks in pending queue
			for(std::list<boost::tuple< WriteCompletionHandler, WriteOperation > >::iterator it = mPendingQueue.begin(); it != mPendingQueue.end(); it++)
			{
				boost::get<0>(*it)(ec, 0);
			}
			mPendingQueue.clear();

		}
		else
		{
			{
				std::size_t remaining_bytes_completed = bytes_transferred;
				while(remaining_bytes_completed > 0 && !mSendingQueue.empty())
				{
					std::size_t completed_bytes = 0;
					boost::tuple< WriteCompletionHandler, WriteOperation >& op = mSendingQueue.front();

					bool all_consumed = true;
					for(std::vector< shared_ptr<Buffer> >::iterator it = boost::get<1>(op).buffers.begin(); it != boost::get<1>(op).buffers.end(); ++it)
					{
						if((*it).get())
						{
							std::size_t size = (*it)->dataSize();
							if(remaining_bytes_completed >= size)
							{
								(*it)->rskip(size);
								remaining_bytes_completed -= size;
								completed_bytes += size;
								(*it).reset();
							}
							else
							{
								(*it)->rskip(remaining_bytes_completed);
								completed_bytes += remaining_bytes_completed;
								all_consumed = false;
								break;
							}
						}
					}

					// TODO make exception safe here
					if(LIKELY(all_consumed))
					{
						boost::get<0>(op)(ec, completed_bytes);
						mSendingQueue.pop_front();
					}
					else
					{
						boost::system::error_code error(boost::asio::error::broken_pipe);
						boost::get<0>(op)(error, completed_bytes);
						break;
					}
				}
			}

			// NOTE the sending queue is supposed to be empty every time the send is completed
			if(UNLIKELY(!mSendingQueue.empty()))
			{
				BOOST_ASSERT(mSendingQueue.empty());
			}

			// for every item in the sending queue, create an entry in the buffer vector
			std::vector<boost::asio::mutable_buffer> buffers;
			std::size_t total_size = 0;
			for(std::list<boost::tuple< WriteCompletionHandler, WriteOperation > >::iterator it = mSendingQueue.begin(); it != mSendingQueue.end(); ++it)
			{
				for(std::vector< shared_ptr<Buffer> >::iterator it_buffers = boost::get<1>(*it).buffers.begin(); it_buffers != boost::get<1>(*it).buffers.end(); ++it_buffers)
				{
					if(LIKELY((*it_buffers).get()))
					{
						std::size_t size = (*it_buffers)->dataSize();
						buffers.push_back(boost::asio::buffer((*it_buffers)->rptr(), size));
						total_size += size;
					}
				}
			}

			// move all items in pending queue to sending queue
			while(!mPendingQueue.empty() && buffers.size() < ksMaxIOV)
			{
				boost::tuple< WriteCompletionHandler, WriteOperation >& op = mPendingQueue.front();

				std::size_t size = 0;
				for(std::vector< shared_ptr<Buffer> >::iterator it_buffers = boost::get<1>(op).buffers.begin(); it_buffers != boost::get<1>(op).buffers.end(); ++it_buffers)
				{
					if(LIKELY((*it_buffers).get()))
					{
						size += (*it_buffers)->dataSize();
					}
				}

				if(total_size + size >= boost::asio::detail::default_max_transfer_size)
					break;

				for(std::vector< shared_ptr<Buffer> >::iterator it_buffers = boost::get<1>(op).buffers.begin(); it_buffers != boost::get<1>(op).buffers.end(); ++it_buffers)
				{
					if(LIKELY((*it_buffers).get()))
					{
						size += (*it_buffers)->dataSize();
						buffers.push_back(boost::asio::buffer((*it_buffers)->rptr(), (*it_buffers)->dataSize()));
					}
				}

				total_size += size;

				mSendingQueue.push_back(op); mPendingQueue.pop_front();
			}

			if(buffers.size() > 0)
			{
				BOOST_ASSERT(total_size < boost::asio::detail::default_max_transfer_size);

				void (SessionT::*f)(const boost::system::error_code&, const std::size_t, const std::size_t) = &SessionT::writeAsyncBufferCompleted<Handler>;

				// increment pending completion
				++mPendingCompletions;

				boost::asio::async_write(
						mSocket,
						buffers,
						boost::bind(f, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, total_size));
			}
		}

		// finish up, decrement pending completion
		uint32 completion = --mPendingCompletions;
		if(UNLIKELY(mDeletionMark && completion == 0))
		{
			delete this;
			//printf("deleted\n");
		}
	}

	template <typename Handler>
	void readAsyncCompleted(const boost::system::error_code& ec, const std::size_t& bytes_transferred, shared_ptr<Buffer> buffer, boost::tuple<Handler> handler)
	{
		buffer->wskip(bytes_transferred);
		boost::get<0>(handler)(ec, bytes_transferred);

		// finish up, decrement pending completion
		uint32 completion = --mPendingCompletions;
		if(UNLIKELY(mDeletionMark && completion == 0))
		{
			delete this;
			//printf("deleted\n");
		}
	}


	/// Handle a completed read of a message header. The handler is passed using a tuple since boost::bind seems to have trouble binding a function object created using boost::bind as a parameter
	template<typename M, typename Handler>
	void readAsyncHeaderCompleted(const boost::system::error_code& ec, const std::size_t& bytes_transferred, M& message, boost::tuple<Handler> handler, shared_ptr<Buffer> buffer)
	{
		if(!ec)
		{
			// the header has been read, update the writing position
			buffer->wskip(detail::MessageHeader::kHeaderSize);

			detail::MessageHeader header;
			try
			{
				// de-serialize the header
				*buffer >> header;
				buffer->clear();
			}
			catch (std::exception& e)
			{
				// unable to decode data
				boost::system::error_code error(boost::asio::error::broken_pipe);
				boost::get<0>(handler)(error, bytes_transferred);
			}

			//printf("received header, type = %d, size = %d\n", header.type, header.size);
			// check the data size & type
			if(UNLIKELY(header.size > detail::MessageHeader::kMaxDataSize || header.type != M::TYPE))
			{
				boost::system::error_code error(boost::asio::error::broken_pipe);
				boost::get<0>(handler)(error, 0);
			}
			else
			{
				// everything is ok, we should continue reading the data into input buffer
				void (SessionT::*f)(const boost::system::error_code&, const std::size_t&, M&, boost::tuple<Handler>, shared_ptr<Buffer>) = &SessionT::readAsyncDataCompleted<M, Handler>;

				++mPendingCompletions;

				boost::asio::async_read(
						mSocket,
						boost::asio::buffer(buffer->wptr(), header.size),
						boost::bind(f, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, boost::ref(message), handler, buffer));
			}
		}
		else
		{
			boost::get<0>(handler)(ec, 0);
		}

		// finish up, decrement pending completion
		uint32 completion = --mPendingCompletions;
		if(UNLIKELY(mDeletionMark && completion == 0))
		{
			delete this;
			//printf("deleted\n");
		}
	}

	/// Handle a completed read of message data.
	template<typename M, typename Handler>
	void readAsyncDataCompleted(const boost::system::error_code& ec, const std::size_t& bytes_transferred, M& message, boost::tuple<Handler> handler, shared_ptr<Buffer> buffer)
	{
		if (ec)
		{
			//printf("received data error, ec = %s\n", ec.message().c_str());
			boost::get<0>(handler)(ec, 0);
		}
		else
		{
			buffer->wskip(bytes_transferred);

			// extract the data structure from the data just received
			try
			{
				// deserialize the message
				*buffer >> message;
			}
			catch (std::exception& e)
			{
				// unable to decode data
				boost::system::error_code error(boost::asio::error::broken_pipe);
				boost::get<0>(handler)(error, bytes_transferred);
			}

			try
			{
				// inform caller that data has been received ok
				boost::get<0>(handler)(ec, bytes_transferred);
			}
			catch(std::exception& e)
			{
			}
		}

		// finish up, decrement pending completion
		uint32 completion = --mPendingCompletions;
		if(UNLIKELY(mDeletionMark && completion == 0))
		{
			delete this;
			//printf("deleted\n");
		}
	}

	template<typename Handler>
	void doClose(boost::tuple<Handler> handler)
	{
		boost::system::error_code error;
		mSocket.cancel(error);
		mSocket.close(error);

		boost::get<0>(handler)();

		// finish up, decrement pending completion
		uint32 completion = --mPendingCompletions;
		if(UNLIKELY(mDeletionMark && completion == 0))
		{
			delete this;
			//printf("deleted\n");
		}
	}

	template <typename T>
	inline shared_ptr<void>& refContext()
	{
		static uint32 index = msContextIndexer++;
		BOOST_ASSERT(index < ksMaximumSupportedContextTypes);
		return mContext[index];
	}

private:
	IoService& mIoService;
	Socket mSocket;

	/// Thread-local buffer. Each thread has local buffer used to perform synchronous message read/write.
	boost::thread_specific_ptr<Buffer> mTlsWriteBuffer;
	boost::thread_specific_ptr<Buffer> mTlsReadBuffer;

	const static std::size_t kScatterWriteThreshold = 256UL;
	const static std::size_t ksMaximumSupportedContextTypes = 8UL;
	const static std::size_t ksMaxIOV = boost::asio::detail::max_iov_len;

	static tbb::atomic<uint32> msContextIndexer;
	shared_ptr<void> mContext[ksMaximumSupportedContextTypes];

	typedef boost::function< void (const boost::system::error_code&, const std::size_t&) > WriteCompletionHandler;

	tbb::queuing_mutex mSendLock;

	struct WriteOperation
	{
		WriteOperation()
		{ }

		WriteOperation(shared_ptr<Buffer> d)
		{ buffers.push_back(d); }

		WriteOperation(shared_ptr<Buffer> h, shared_ptr<Buffer> d)
		{ buffers.push_back(h); buffers.push_back(d); }

		WriteOperation(std::vector< shared_ptr<Buffer> >& b)
		{
			for(std::vector< shared_ptr<Buffer> >::iterator it = b.begin(); it != b.end(); ++it)
			{
				buffers.push_back(*it);
			}
		}

		WriteOperation(shared_ptr<Buffer> h, std::vector< shared_ptr<Buffer> >& b)
		{
			buffers.push_back(h);
			for(std::vector< shared_ptr<Buffer> >::iterator it = b.begin(); it != b.end(); ++it)
			{
				buffers.push_back(*it);
			}
		}

		std::vector< shared_ptr<Buffer> > buffers;
	};

	std::list< boost::tuple< WriteCompletionHandler, WriteOperation > > mPendingQueue;
	std::list< boost::tuple< WriteCompletionHandler, WriteOperation > > mSendingQueue;

	tbb::atomic<bool> mDeletionMark;
	tbb::atomic<uint32> mPendingCompletions;
};

/// Pre-define TcpSession
typedef SessionT< SessionTransport::tcp > TcpSession;

} } }

#endif/*ZILLIANS_NET_SYS_TCPSESSIONIMPL_H_*/
