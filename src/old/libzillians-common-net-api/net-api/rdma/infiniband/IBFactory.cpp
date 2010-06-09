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

#include "net-api/rdma/infiniband/IBFactory.h"
#include <fcntl.h>

extern "C" struct ibv_mr* zillians_ibv_reg_mr(struct ibv_pd* pd, void* addr, size_t length);
extern "C" int zillians_ibv_dereg_mr(struct ibv_mr* pd);

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr IBFactory::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.rdma.infiniband.IBFactory"));

//////////////////////////////////////////////////////////////////////////
IBFactory::IBFactory()
{ }

IBFactory::~IBFactory()
{ }

//////////////////////////////////////////////////////////////////////////
shared_ptr<ibv_context> IBFactory::createDeviceContext(ibv_device* device)
{
	ibv_context* context = ibv_open_device(device);
	IB_DEBUG("create device context = " << context);
	return shared_ptr<ibv_context>(context, IBFactory::destroyDeviceContext);
}

shared_ptr<rdma_event_channel> IBFactory::createEventChannel()
{
    rdma_event_channel* ec = rdma_create_event_channel();

    if(!ec)
    {
    	IB_ERROR("fail to create RDMA event channel, check user privilege or device resources");
		BOOST_ASSERT(ec != NULL);
    }

    //::fcntl(ec->fd, F_SETFL, O_NONBLOCK);
    IB_DEBUG("create rdma event channel = " << ec);
    return shared_ptr<rdma_event_channel>(ec, IBFactory::destroyEventChannel);
}

shared_ptr<rdma_cm_id> IBFactory::createManagementId(rdma_event_channel* ec, void* context, rdma_port_space ps)
{
    rdma_cm_id* id = NULL;
    rdma_create_id(ec, &id, context, ps);

    if(!id)
    {
    	IB_ERROR("fail to create management id (cm_id), check user privilege or device resources");
		BOOST_ASSERT(id != NULL);
    }

    IB_DEBUG("create rdma cm id = " << id);
    return shared_ptr<rdma_cm_id>(id, IBFactory::destroyManagementId);
}

shared_ptr<ibv_pd> IBFactory::createProtectionDomain(ibv_context* c)
{
    ibv_pd* pd = ibv_alloc_pd(c);
    if(!pd)
    {
    	IB_ERROR("fail to create protection domain, check user privilege or device resources");
		BOOST_ASSERT(pd != NULL);
    }

    IB_DEBUG("create ib protection domain = " << pd);
    return shared_ptr<ibv_pd>(pd, IBFactory::destroyProtectionDomain);

}

shared_ptr<ibv_comp_channel> IBFactory::createCompletionChannel(ibv_context* c)
{
    ibv_comp_channel* cc = ibv_create_comp_channel(c);
    if(!cc)
    {
    	IB_ERROR("fail to create completion channel, check user privilege or device resources");
    	BOOST_ASSERT(cc != NULL);
    }

    //::fcntl(cc->fd, F_SETFL, O_NONBLOCK);
    IB_DEBUG("create ib completion channel = " << cc);
    return shared_ptr<ibv_comp_channel>(cc, IBFactory::destroyCompletionChannel);

}

shared_ptr<ibv_cq> IBFactory::createCompletionQueue(ibv_context* c, int cqe, void* context, ibv_comp_channel* cc)
{
    ibv_cq* cq = ibv_create_cq(c, cqe, context, cc, 0);
    if(!cq)
    {
    	IB_ERROR("fail to create completion queue, check user privilege or device resources");
    	BOOST_ASSERT(cq != NULL);
    }

    IB_DEBUG("create ib completion queue = " << cq);
    return shared_ptr<ibv_cq>(cq, IBFactory::destroyCompletionQueue);
}

void IBFactory::createQueuePair(rdma_cm_id* id, ibv_pd* pd, ibv_qp_init_attr* attr)
{
	rdma_create_qp(id, pd, attr);
	if(!id->qp)
	{
		IB_ERROR("fail to create queue pair, check user privilege or device resources");
		BOOST_ASSERT(id->qp != NULL);
	}

	IB_DEBUG("create rdma queue pair = " << id->qp);
}

shared_ptr<ibv_mr> IBFactory::createMemoryRegion(ibv_pd* pd, void* addr, size_t size)
{
	ibv_mr* mr = zillians_ibv_reg_mr(pd, addr, size);
	if(!mr)
	{
		IB_ERROR("fail to create memory region, check user privilege or device resources");
		BOOST_ASSERT(mr != NULL);
	}

	IB_DEBUG("create memory region = " << mr);

	return shared_ptr<ibv_mr>(mr, IBFactory::destroyMemoryRegion);
}

//////////////////////////////////////////////////////////////////////////
void IBFactory::destroyDeviceContext(ibv_context* context)
{
	BOOST_ASSERT(context);
	IB_DEBUG("destroy device context = " << context);
	int err = ibv_close_device(context);
	if(err)
	{
		IB_ERROR("fail to destroy device context, err = " << err);
	}
}

void IBFactory::destroyEventChannel(rdma_event_channel* ec)
{
	if(ec)
	{
		IB_DEBUG("destroy rdma event channel = " << ec);
		rdma_destroy_event_channel(ec);
	}
}

void IBFactory::destroyManagementId(rdma_cm_id* id)
{
	if(id)
	{
		if(id->qp)
		{
			IB_DEBUG("destroy rdma queue pair = " << id->qp);
			rdma_destroy_qp(id);
		}

		IB_DEBUG("destroy rdma cm id = " << id);
		int err = rdma_destroy_id(id);
		if(err)
		{
			IB_ERROR("fail to destroy rdma cm id, err = " << err);
		}
	}
}

void IBFactory::destroyProtectionDomain(ibv_pd* pd)
{
	if(pd)
	{
		IB_DEBUG("destroy ib protection domain = " << pd);
		int err = ibv_dealloc_pd(pd);
		if(err)
		{
			IB_ERROR("fail to destroy ib protection domain, err = " << err);
		}
	}
}

void IBFactory::destroyCompletionChannel(ibv_comp_channel* cc)
{
	if(cc)
	{
		IB_DEBUG("destroy ib completion channel = " << cc);
		int err = ibv_destroy_comp_channel(cc);
		if(err)
		{
			IB_ERROR("fail to destroy ib completion channel, err = " << err);
		}
	}
}

void IBFactory::destroyCompletionQueue(ibv_cq* cq)
{
	if(cq)
	{
		IB_DEBUG("destroy ib completion queue = " << cq);
		int err = ibv_destroy_cq(cq);
		if(err)
		{
			IB_ERROR("fail to destroy ib completion queue, err = " << err);
		}
	}
}

void IBFactory::destroyQueuePair(rdma_cm_id* id)
{
	if(id)
	{
		if(id->qp)
		{
			IB_DEBUG("destroy rdma queue pair = " << id->qp);
			rdma_destroy_qp(id);
			id->qp = NULL;
		}
	}
}

void IBFactory::destroyMemoryRegion(ibv_mr* mr)
{
	if(mr)
	{
		IB_DEBUG("destroy memory region = " << mr);
		int err = zillians_ibv_dereg_mr(mr);
		if(err)
		{
			IB_ERROR("fail to destroy memory region, err = " << err);
		}
	}
}

} } }
