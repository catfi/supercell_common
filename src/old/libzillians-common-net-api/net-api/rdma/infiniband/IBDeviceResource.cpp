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

#include "net-api/rdma/infiniband/IBDeviceResource.h"
#include "net-api/rdma/infiniband/IBFactory.h"

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr IBDeviceResource::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.rdma.infiniband.IBDeviceResource"));

//////////////////////////////////////////////////////////////////////////
IBDeviceResource::IBDeviceResource(ibv_context* context)
{
	mContext = context;
	mGPD = IBFactory::createProtectionDomain(context);
	LOG4CXX_DEBUG(mLogger, "create global protection domain (" << mGPD.get() << ") for device context (" << mContext << ")");
}

IBDeviceResource::~IBDeviceResource()
{
	BOOST_ASSERT(mGPD.use_count() == 1L);
	BOOST_ASSERT(mGMR.use_count() == 1L);
}

void IBDeviceResource::regGlobalMemoryRegion(byte* address, size_t size)
{
	LOG4CXX_DEBUG(mLogger, "register global memory region (" << mGPD.get() << ")");
	mGMR = IBFactory::createMemoryRegion(mGPD.get(), address, size);
}

} } }
