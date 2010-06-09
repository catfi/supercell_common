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

#ifndef ZILLIANS_NET_RDMA_CONNECTIONHANDLERTEMPLATE_H_
#define ZILLIANS_NET_RDMA_CONNECTIONHANDLERTEMPLATE_H_

#include "core/Prerequisite.h"

using namespace zillians;

namespace zillians { namespace net { namespace rdma {

template <typename Connection>
struct ConnectionHandlerTemplate
{
	virtual void onConnected(shared_ptr<Connection> connection) = 0;
	virtual void onDisconnected(shared_ptr<Connection> connection) = 0;
	virtual void onError(shared_ptr<Connection> connection, int code) = 0;
};

template <typename Connection>
struct ConnectionHandlerBinder : public ConnectionHandlerTemplate<Connection>
{
	struct placeholders
	{
		static inline boost::arg<1> connection()
		{
			return boost::arg<1>();
		}

		static inline boost::arg<2> error_code()
		{
			return boost::arg<2>();
		}
	};

	typedef typename boost::function< void (shared_ptr<Connection> connection) > on_connected_handler_type;
	typedef typename boost::function< void (shared_ptr<Connection> connection) > on_disconnected_handler_type;
	typedef typename boost::function< void (shared_ptr<Connection> connection, int) > on_error_handler_type;

	ConnectionHandlerBinder(on_connected_handler_type on_connected, on_disconnected_handler_type on_disconnected, on_error_handler_type on_error) :
		connected_handler(on_connected), disconnected_handler(on_disconnected), error_handler(on_error)
	{ }

	virtual void onConnected(shared_ptr<Connection> connection)
	{
		connected_handler(connection);
	}

	virtual void onDisconnected(shared_ptr<Connection> connection)
	{
		disconnected_handler(connection);
	}

	virtual void onError(shared_ptr<Connection> connection, int code)
	{
		error_handler(connection, code);
	}

	on_connected_handler_type connected_handler;
	on_disconnected_handler_type disconnected_handler;
	on_error_handler_type error_handler;
};

} } }

#endif/*ZILLIANS_NET_RDMA_CONNECTIONHANDLERTEMPLATE_H_*/
