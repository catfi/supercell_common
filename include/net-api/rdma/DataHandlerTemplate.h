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
/**
 * @date Feb 19, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_RDMA_DATAHANDLERTEMPLATE_H_
#define ZILLIANS_NET_RDMA_DATAHANDLERTEMPLATE_H_

#include "core-api/Prerequisite.h"

using namespace zillians;

namespace zillians { namespace net { namespace rdma {

template <typename Connection>
struct DataHandlerTemplate
{
	virtual void handle(uint32 type, SharedPtr<Buffer> b, SharedPtr<Connection> connection) = 0;
};

template <typename Connection>
struct DataHandlerBinder : public DataHandlerTemplate<Connection>
{
	struct placeholders
	{
		static inline boost::arg<1> connection()
		{
		  return boost::arg<1>();
		}

		static inline boost::arg<2> type()
		{
		  return boost::arg<2>();
		}

		static inline boost::arg<3> buffer()
		{
		  return boost::arg<3>();
		}
	};

	typedef typename boost::function< void (SharedPtr<Connection> connection, uint32, SharedPtr<Buffer>) > handler_type;

	DataHandlerBinder(handler_type h) : handler(h) { }

	virtual void handle(uint32 type, SharedPtr<Buffer> b, SharedPtr<Connection> connection)
	{
		handler(connection, type, b);
	}
	handler_type handler;
};

} } }

#endif/*ZILLIANS_NET_RDMA_DATAHANDLERTEMPLATE_H_*/
