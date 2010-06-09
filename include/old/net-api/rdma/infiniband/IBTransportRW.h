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
 * @date Mar 30, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_RDMA_IBTRANSPORTRW_H_
#define ZILLIANS_NET_RDMA_IBTRANSPORTRW_H_

#include "net-api/rdma/infiniband/IBTransport.h"

namespace zillians { namespace net { namespace rdma {

/**
 * @brief Support RDMA Read/Write transport semantics
 */
template <bool Concurrent = true>
class IBTransportRW : public IBTransport
{
public:
	IBTransportRW(IBConnection* connection)
	{

	}

	virtual ~IBTransportRW()
	{

	}

public:
	uint64 registerDirect(shared_ptr<Buffer> buffer, bool pinned = true)
	{

	}

	void unregisterDirect(uint64 sink)
	{

	}

	void write(uint64 sink, std::size_t offset, shared_ptr<Buffer> buffer, std::size_t size)
	{

	}

	void writeAsync(uint64 sink, std::size_t offset, shared_ptr<Buffer> buffer, std::size_t size, CompletionHandler handler)
	{

	}

	void read(shared_ptr<Buffer> buffer, uint64 sink, std::size_t offset, std::size_t size)
	{

	}

	void readAsync(shared_ptr<Buffer> buffer, uint64 sink, std::size_t offset, std::size_t size, CompletionHandler handler)
	{

	}

public:
	virtual void workCompleted(const ibv_wc& wc)
	{

	}

private:
	void handleWriteCompleted(const ibv_wc &wc)
	{

	}

	void handleReadCompleted(const ibv_wc &wc)
	{

	}

private:
	typedef boost::function< void (const boost::system::error_code&, const std::size_t&) > CompletionHandler;

private:
	IBConnection* mConnection;

	typedef tbb::concurrent_hash_map<uint64, shared_ptr<Buffer> > BufferRefHolder;
	ConcurrentBufferRefHolder mLocalConcurrentBufferHolder;
	ConcurrentBufferRefHolder mRemoteConcurrentBufferHolder;
};

} } }

#endif /* ZILLIANS_NET_RDMA_IBTRANSPORTRW_H_ */
