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

#ifndef ZILLIANS_NET_RDMA_IBDEVICERESOURCEMANAGER_H_
#define ZILLIANS_NET_RDMA_IBDEVICERESOURCEMANAGER_H_

#include "core-api/Prerequisite.h"
#include "core-api/Singleton.h"
#include "net-api/rdma/infiniband/IBCommon.h"
#include "net-api/rdma/infiniband/IBDeviceResource.h"

namespace zillians { namespace net { namespace rdma {

/**
 * @brief IBDeviceResourceManager creates and manages all IBDeviceResource objects
 *
 * IBDeviceResourceManager will enumerate all Infiniband devices in its constructor
 * and create corresponding IBDeviceResource. Users are responsible for initialize
 * the global buffer, which will be registered with the IBDeviceResource to create
 * global memory region later.
 *
 * @see IBDeviceResource
 */
class IBDeviceResourceManager : public zillians::Singleton<IBDeviceResourceManager>
{
public:
	IBDeviceResourceManager();
	virtual ~IBDeviceResourceManager();

public:
	int resourceCount();
	ibv_context* getContext(int id);
	SharedPtr<IBDeviceResource> getResource(ibv_context* context);

public:
	void regGlobalMemoryRegion(byte* address, size_t size);

private:
	static log4cxx::LoggerPtr mLogger;

private:
	struct
	{
		byte* address;
		size_t size;
	} mGlobalMemoryRegion;
private:
	typedef std::map<ibv_context*, SharedPtr<IBDeviceResource> > tResourceMap;
	tResourceMap mResourceMap;
};

} } }

#endif/*ZILLIANS_NET_RDMA_IBDEVICERESOURCEMANAGER_H_*/
