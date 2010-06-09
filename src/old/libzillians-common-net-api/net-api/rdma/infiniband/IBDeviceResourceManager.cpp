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

#include "net-api/rdma/infiniband/IBDeviceResourceManager.h"
#include "net-api/rdma/infiniband/IBFactory.h"

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr IBDeviceResourceManager::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.rdma.infiniband.IBDeviceResourceManager"));

//////////////////////////////////////////////////////////////////////////
IBDeviceResourceManager::IBDeviceResourceManager()
{
	mGlobalMemoryRegion.address = NULL;
	mGlobalMemoryRegion.size = 0;
}

IBDeviceResourceManager::~IBDeviceResourceManager()
{
	// clean up all IBDeviceResource object in the hash map
	mResourceMap.clear();
}

//////////////////////////////////////////////////////////////////////////
void IBDeviceResourceManager::regGlobalMemoryRegion(byte* address, size_t size)
{
	if(mGlobalMemoryRegion.address != address || mGlobalMemoryRegion.size != size)
	{
		for(tResourceMap::iterator it = mResourceMap.begin(); it != mResourceMap.end(); ++it)
		{
			it->second->regGlobalMemoryRegion(address, size);
		}
	}

	mGlobalMemoryRegion.address = address;
	mGlobalMemoryRegion.size = size;
}

shared_ptr<IBDeviceResource> IBDeviceResourceManager::getResource(ibv_context* context)
{
	// TODO check if std::map allow concurrent read access
	tResourceMap::iterator it = mResourceMap.find(context);

	if(it == mResourceMap.end())
	{
		shared_ptr<IBDeviceResource> resource(new IBDeviceResource(context));

		if(mGlobalMemoryRegion.address != NULL && mGlobalMemoryRegion.size > 0)
			resource->regGlobalMemoryRegion(mGlobalMemoryRegion.address, mGlobalMemoryRegion.size);

		mResourceMap[context] = resource;

		LOG4CXX_DEBUG(mLogger, "creating IBDeviceResource which has ibv_context pointer = " << context);

		return resource;
	}

	return it->second;
}

} } }
