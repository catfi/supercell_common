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

#ifndef ZILLIANS_NET_RDMA_IBCOMMON_H_
#define ZILLIANS_NET_RDMA_IBCOMMON_H_

#include "core-api/Prerequisite.h"
#include "core-api/Buffer.h"
#include <rdma/rdma_cma.h>

/**
 * We have several design rules to ensure the reliability of sending over RDMA.
 * Note that the following parameters are checked in IBCheckConfiguration()
 *
 * @see IBCheckConfiguration()
 */
//////////////////////////////////////////////////////////////////////////
//#define IB_ENABLE_DEBUG
//#define IB_ENABLE_ORDERING_CHECK
//#define IB_ENABLE_COMPLETION_DISPATCH

//////////////////////////////////////////////////////////////////////////
#define IB_DEFAULT_CONNECTOR_WAIT_TIMEOUT	2000
#define IB_DEFAULT_ADDR_RESOLVE_TIMEOUT		5000
#define IB_DEFAULT_ROUTE_RESOLVE_TIMEOUT	2000
#define IB_DEFAULT_CONNECTING_TIMEOUT     	50000

#define IB_DEFAULT_TIMEOUT					2000
#define IB_DEFAULT_LISTEN_BACKLOG			100
#define IB_DEFAULT_CQ_ENTRIES 				4096
#define IB_DEFAULT_WR_ENTRIES 				1024
#define IB_DEFAULT_MAX_POST_SEGMENTS		16

//////////////////////////////////////////////////////////////////////////
#define IB_DEFAULT_ACK_BUFFER_COUNT			8
#define IB_DEFAULT_DATA_BUFFER_COUNT		120

#define IB_DEFAULT_RECEIVE_BUFFER_COUNT		(IB_DEFAULT_DATA_BUFFER_COUNT + IB_DEFAULT_ACK_BUFFER_COUNT)
#define IB_DEFAULT_RECEIVE_BUFFER_SIZE		(16*1024)			// 16KB

// TODO need to re-consider the control buffer count...local <=> local send will be out of buffer, right now we just make it buffer so we won't use up the buffers
#define IB_DEFAULT_CONTROL_BUFFER_COUNT		1024*16
#define IB_DEFAULT_CONTROL_BUFFER_SIZE		128

#define IB_DEFAULT_THRESHOLD_BUFFER_ACK		16

#define IB_DEFAULT_THRESHOLD_CQ_ACK			16

#define IB_MINIMAL_MEMORY_USAGE				((IB_DEFAULT_RECEIVE_BUFFER_COUNT*IB_DEFAULT_RECEIVE_BUFFER_SIZE) + (IB_DEFAULT_CONTROL_BUFFER_COUNT*IB_DEFAULT_CONTROL_BUFFER_SIZE))

#define IB_DEFAULT_MAX_SEND_IN_FLIGHT		2048

//////////////////////////////////////////////////////////////////////////
#ifdef IB_ENABLE_DEBUG
#define IB_DEBUG(x) { LOG4CXX_DEBUG(mLogger, x); }
#define IB_ERROR(x) { LOG4CXX_ERROR(mLogger, x); }
#define IB_INFO(x)  { LOG4CXX_INFO (mLogger, x); }
#else
#define IB_DEBUG(x) { }
#define IB_ERROR(x) { LOG4CXX_ERROR(mLogger, x); }
#define IB_INFO(x)  { }
#endif

namespace zillians { namespace net { namespace rdma {

extern bool IBCheckConfiguration(log4cxx::LoggerPtr &mLogger);

} } }

#endif/*ZILLIANS_NET_RDMA_IBCOMMON_H_*/
