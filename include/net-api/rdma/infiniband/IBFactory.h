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

#ifndef ZILLIANS_NET_RDMA_IBFACTORY_H_
#define ZILLIANS_NET_RDMA_IBFACTORY_H_

#include "core-api/Prerequisite.h"
#include "net-api/rdma/infiniband/IBCommon.h"

namespace zillians { namespace net { namespace rdma {

class IBFactory
{
public:
	static SharedPtr<ibv_context>			createDeviceContext(ibv_device* device);
	static SharedPtr<rdma_event_channel>	createEventChannel();
	static SharedPtr<rdma_cm_id> 			createManagementId(rdma_event_channel* ec, void* context, rdma_port_space ps);
	static SharedPtr<ibv_pd>				createProtectionDomain(ibv_context* c);
	static SharedPtr<ibv_comp_channel>		createCompletionChannel(ibv_context* c);
	static SharedPtr<ibv_cq> 				createCompletionQueue(ibv_context* c, int cqe, void* context, ibv_comp_channel* cc);
	static SharedPtr<ibv_mr>				createMemoryRegion(ibv_pd* pd, void* addr, size_t size);

	static void destroyDeviceContext(ibv_context* context);
	static void destroyEventChannel(rdma_event_channel* ec);
	static void destroyManagementId(rdma_cm_id* id);
	static void destroyProtectionDomain(ibv_pd* pd);
	static void destroyCompletionChannel(ibv_comp_channel* cc);
	static void destroyCompletionQueue(ibv_cq* cq);
	static void destroyMemoryRegion(ibv_mr* mr);

	static void	createQueuePair(rdma_cm_id* id, ibv_pd* pd, ibv_qp_init_attr* attr);
	static void destroyQueuePair(rdma_cm_id* id);

private:
	IBFactory();
	~IBFactory();

private:
	static log4cxx::LoggerPtr mLogger;
};

} } }

#endif/*ZILLIANS_NET_RDMA_IBFACTORY_H_*/
