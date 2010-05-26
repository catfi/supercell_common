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

#include "net-api/rdma/infiniband/IBTransportRW.h"

namespace zillians { namespace net { namespace rdma {


//////////////////////////////////////////////////////////////////////////
IBTransportRW::IBTransportRW(IBConnection* connection) : mConnection(connection)
{ }

IBTransportRW::~IBTransportRW()
{ }

//////////////////////////////////////////////////////////////////////////
uint64 IBTransportRW::registerDirect(shared_ptr<Buffer> buffer, bool pinned)
{
	// hold the buffer into a temporary storage to keep the reference so that it wouldn't be freed
	// register the buffer (pinned) and take buffer pointer as sink id
	// send the sink id <--> buffer pointer mapping to remote
}

void IBTransportRW::unregisterDirect(uint64 sink)
{
	// unregister the buffer (unpinned) and send unregisteration info to the remote
}

void IBTransportRW::write(uint64 sink, std::size_t offset, shared_ptr<Buffer> buffer, std::size_t size)
{
	// get the remote buffer pointer from sink id
	// post RDMA write
	// wait for completion by calling ibv_poll_cq(scq, 1, wc) (on the send completion queue)
}

void IBTransportRW::writeAsync(uint64 sink, std::size_t offset, shared_ptr<Buffer> buffer, std::size_t size, CompletionHandler handler)
{
	// get the buffer pointer from sink id
	// post RDMA write
	// insert the request info into the queue so that workCompleted() would handle it (and call the completion handler) later
}

void IBTransportRW::read(shared_ptr<Buffer> buffer, uint64 sink, std::size_t offset, std::size_t size)
{
	// get the remote buffer pointer from sink id
	// post RDMA read
	// wait for completion by calling ibv_poll_cq(scq, 1, wc) (on the send completion queue)
}

void IBTransportRW::readAsync(shared_ptr<Buffer> buffer, uint64 sink, std::size_t offset, std::size_t size, CompletionHandler handler)
{
	// get the buffer pointer from sink id
	// post RDMA read
	// insert the request info into the queue so that workCompleted() would handle it (and call the completion handler) later
}

//////////////////////////////////////////////////////////////////////////
void IBTransportRW::workCompleted(const ibv_wc& wc)
{

}

//////////////////////////////////////////////////////////////////////////
void IBTransportRW::handleWriteCompleted(const ibv_wc &wc)
{

}

void IBTransportRW::handleReadCompleted(const ibv_wc &wc)
{

}

} } }
