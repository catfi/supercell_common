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

#ifndef ZILLIANS_NET_RDMA_IBNETENGINE_H_
#define ZILLIANS_NET_RDMA_IBNETENGINE_H_

#include "networking/rdma/RdmaNetEngineTemplate.h"
#include "networking/rdma/infiniband/IBCommon.h"
#include "networking/rdma/infiniband/IBConnection.h"
#include "networking/rdma/infiniband/IBConnector.h"
#include "networking/rdma/infiniband/IBAcceptor.h"
#include "networking/rdma/infiniband/IBDispatcher.h"

namespace zillians { namespace net { namespace rdma {

class IBNetEngine : public RdmaNetEngineTemplate <IBConnection, IBConnector, IBAcceptor, IBDispatcher>
{
public:
	IBNetEngine()
	{
		IBCheckConfiguration(mLogger);
	}
};

} } }

#endif/*ZILLIANS_NET_RDMA_IBNETENGINE_H_*/
