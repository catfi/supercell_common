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

#ifndef ZILLIANS_NET_RDMA_IBDEVICERESOURCE_H_
#define ZILLIANS_NET_RDMA_IBDEVICERESOURCE_H_

#include "core-api/Prerequisite.h"
#include "net-api/rdma/infiniband/IBCommon.h"

namespace zillians { namespace net { namespace rdma {

/**
 * @brief IBDeviceResource is a wrapper for Global Protection Domain and Global Memory Region on each Infiniband HCA
 *
 * IBDeviceResource is a helper class to wrap up the global protection domain and
 * global memory region to be used in IBConnection. Right now the protection domain
 * and memory region feature on Infiniband imposes some architecture design restriction,
 * so, as we are in a trusted cluster environment, we abandon these security features
 * from Infiniband and use a global protection domain and global memory region shared
 * among all RDMA connections. As a result, all connection within the same HCA will
 * have the same IBDeviceResource and use the same local key (lkey) and remote key (rkey)
 * for ibv_post_send and ibv_post_recv. This is not safe, but probably the best solution
 * to minimize the design restriction imposed by RDMA.
 *
 * @see IBDeviceResourceManager, IBConnection
 */
class IBDeviceResource
{
	friend class IBDeviceResourceManager;
public:
	IBDeviceResource(ibv_context* context);
	~IBDeviceResource();

private:
	void regGlobalMemoryRegion(byte* address, size_t size);

public:
	inline SharedPtr<ibv_pd> getGlobalProtectionDomain() const { return mGPD; }
	inline SharedPtr<ibv_mr> getGlobalMemoryRegion() const { return mGMR; }

private:
	static log4cxx::LoggerPtr mLogger;

private:
	ibv_context* mContext;
	SharedPtr<ibv_pd> mGPD;
	SharedPtr<ibv_mr> mGMR;
};

} } }

#endif/*ZILLIANS_NET_RDMA_IBDEVICERESOURCE_H_*/
