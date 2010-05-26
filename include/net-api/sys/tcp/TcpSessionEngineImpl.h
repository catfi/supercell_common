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
 * @date Aug 9, 2009 sdk - Initial version created.
 */


#ifndef ZILLIANS_NET_SYS_TCPSESSIONENGINEIMPL_H_
#define ZILLIANS_NET_SYS_TCPSESSIONENGINEIMPL_H_

#include "core-api/Prerequisite.h"
#include "util-api/StringUtil.h"
#include "net-api/sys/SessionCommon.h"
#include "net-api/sys/Session.h"
#include "net-api/sys/Placeholders.h"
#include "net-api/sys/Dispatcher.h"

namespace zillians { namespace net { namespace sys {

/**
 * This is a SessionEngineT specialization for boost::asio::ip::tcp (which is basically TCP)
 */
template <>
class SessionEngineT< SessionTransport::tcp >
{
public:
	typedef boost::asio::ip::tcp Protocol;
	typedef boost::asio::io_service IoService;
	typedef Protocol::endpoint Endpoint;
	typedef Protocol::acceptor Acceptor;
	typedef Protocol::resolver Resolver;
	typedef SessionT<Protocol> Session;
	typedef DispatcherT<Session> Dispatcher;

	typedef boost::function< void (Session&) > CloseCallback;
	typedef boost::function< void (Session&, const boost::system::error_code&) > ErrorCallback;

private:
	struct DispatcherContext
	{
		shared_ptr<Buffer> buffer;
		bool dispatchEnabled;
		CloseCallback onClose;
		ErrorCallback onError;
		Dispatcher* dispatcher;
	};

public:
	SessionEngineT(IoService* io = NULL)
	{
		mDispatcher = new Dispatcher(256);
		mAcceptor = NULL;

		if(io)
		{
			mIsIoServiceOwner = false;
			mIoService = io;
		}
		else
		{
			mIsIoServiceOwner = true;
			mIoService = new boost::asio::io_service;
		}

		mAcceptor = new Acceptor(*mIoService);
		mResolver = new Resolver(*mIoService);
	}

	~SessionEngineT()
	{
		if(mAcceptor->is_open())
		{
			boost::system::error_code error;
			mAcceptor->cancel(error);
			mAcceptor->close(error);
		}

		mResolver->cancel();

		if(mIsIoServiceOwner)
		{
			mIoService->stop();
			SAFE_DELETE(mIoService);
		}

		SAFE_DELETE(mDispatcher);
		SAFE_DELETE(mAcceptor);
		SAFE_DELETE(mResolver);
	}

	Session* createSession()
	{
		Session* p = new Session(*mIoService);
		return p;
	}

	void listen(const Protocol& type, uint16 port)
	{
		boost::system::error_code error;

		BOOST_ASSERT(mAcceptor != NULL);
		if(mAcceptor->is_open())
		{
			mAcceptor->cancel(error);
			mAcceptor->close(error);
		}

		mAcceptor->open(type, error);
		if(error) { throw boost::system::system_error(error); }

		// re-use the address (in some system, the listening port is not free immediately after program exit, so we have to re-use the port)
		boost::asio::socket_base::reuse_address option(true);
		mAcceptor->set_option(option);

		mAcceptor->bind(Endpoint(type, port), error);
		if(error) { throw boost::system::system_error(error); }

		mAcceptor->listen(SOMAXCONN, error);
		if(error) { throw boost::system::system_error(error); }
	}

	template <typename Handler>
	void listenAsync(const Protocol& type, uint16 port, Handler handler)
	{
		mIoService->dispatch(boost::bind(&SessionEngineT::doListenAsync<Handler>, this, type, port, boost::make_tuple(handler)));
	}

	void getListeningAddress(std::string& address, uint16& port)
	{
		if(!mAcceptor->is_open())
		{
			throw std::runtime_error("acceptor is not listening");
		}

		boost::system::error_code error;
		Protocol::endpoint ep = mAcceptor->local_endpoint(error);
		if(error) { throw boost::system::system_error(error); }

		address = ep.address().to_string();
		port = ep.port();
	}

	void accept(Session* session)
	{
		BOOST_ASSERT(mAcceptor->is_open());

		boost::system::error_code error;

		mAcceptor->accept(session->socket(), error);

		if(error) { throw boost::system::system_error(error); }
	}

	template <typename Handler>
	void acceptAsync(Session* session, Handler handler)
	{
		BOOST_ASSERT(session != NULL);
		void (SessionEngineT::*f)(Session* session, boost::tuple<Handler>) = &SessionEngineT::doAcceptAsync<Handler>;

		mIoService->dispatch(boost::bind(f, this, session, boost::make_tuple(handler)));
	}

	void connect(Session* session, const Protocol& type, const std::string& endpoint_name)
	{
		std::vector<std::string> tokens = zillians::StringUtil::tokenize(endpoint_name, ":", false);
		if(tokens.size() != 2)
			throw std::invalid_argument("given endpoint format is not correct, should be \"host_name:service_name\"");

		connect(session, type, tokens[0], tokens[1]);
	}

	void connect(Session* session, const Protocol& type, const std::string& host_name, const std::string& service_name)
	{
		boost::system::error_code error;

		Resolver::query query(type, host_name, service_name);
		Resolver::iterator it = mResolver->resolve(query, error);
		if(error) { throw boost::system::system_error(error); }

		session->socket().connect(*it, error);
		if(error) { throw boost::system::system_error(error); }
	}

	template <typename Handler>
	void connectAsync(Session* session, const Protocol& type, const std::string& endpoint_name, Handler handler)
	{
		std::vector<std::string> tokens = zillians::StringUtil::tokenize(endpoint_name, ":", false);
		if(tokens.size() != 2)
			throw std::invalid_argument("given endpoint format is not correct, should be \"host_name:service_name\"");

		connectAsync<Handler>(session, type, tokens[0], tokens[1], handler);
	}

	template <typename Handler>
	void connectAsync(Session* session, const Protocol& type, const std::string& host_name, const std::string& service_name, Handler handler)
	{
		BOOST_ASSERT(session != NULL);
		void (SessionEngineT::*f)(Session* session, const Protocol& type, const std::string& host_name, const std::string& service_name, boost::tuple<Handler>) = &SessionEngineT::doConnectAsync<Handler>;

		mIoService->dispatch(boost::bind(f, this, session, type, host_name, service_name, boost::make_tuple(handler)));
	}

	void startDispatch(Session* session, CloseCallback onClose, ErrorCallback onError, Dispatcher* dispatcher = NULL)
	{
		BOOST_ASSERT(session != NULL);
		if(!dispatcher) dispatcher = mDispatcher;
		mIoService->dispatch(boost::bind(&SessionEngineT::doStartDispatch, this, session, onClose, onError, dispatcher));
	}

	void updateDispatch(Session* session, CloseCallback onClose, ErrorCallback onError, Dispatcher* dispatcher = NULL)
	{
		BOOST_ASSERT(session != NULL);
		if(!dispatcher) dispatcher = mDispatcher;
		mIoService->dispatch(boost::bind(&SessionEngineT::doUpdateDispatch, this, session, onClose, onError, dispatcher));
	}

	void stopDispatch(Session* session)
	{
		BOOST_ASSERT(session != NULL);
		mIoService->dispatch(boost::bind(&SessionEngineT::doStopDispatch, this, session));
	}

	void schedule()
	{
		// NOT YET IMPLEMENTED
		throw std::runtime_error("TcpSessionEngine::schedule() hasn't been implemented");
	}

	template <typename Handler>
	void dispatch(Handler handler)
	{
		mIoService->dispatch(handler);
	}

	template <typename Handler>
	void post(Handler handler)
	{
		mIoService->post(handler);
	}

	void run()
	{
		boost::asio::io_service::work dummy(*mIoService);
		while(true)
		{
			try
			{
				mIoService->run();
				break;
			}
			catch(std::exception& ex)
			{
				printf("TcpSessionEngine::run() catches uncaught exception: %s", ex.what());
			}
		}
	}

	void stop()
	{
		mIoService->stop();
	}

//	IoService& getIoService()
//	{
//		return *mIoService;
//	}

	Dispatcher& getDispatcher()
	{
		return *mDispatcher;
	}

private:
	template <typename Handler>
	void doListenAsync(const Protocol& type, uint16 port, boost::tuple<Handler> handler)
	{
		//printf("damn\n");
		boost::system::error_code error;

		if(mAcceptor)
		{
			mAcceptor->cancel(error);
			mAcceptor->close(error);
			SAFE_DELETE(mAcceptor);
		}

		try
		{
			mAcceptor = new Acceptor(*mIoService);
		}
		catch(boost::system::system_error& e)
		{
			error = e.code();
			boost::get<0>(handler)(error);
			return;
		}

		mAcceptor->open(type, error);
		if(error) { boost::get<0>(handler)(error); return; }

		// re-use the address (in some system, the listening port is not free immediately after program exit, so we have to re-use the port)
		boost::asio::socket_base::reuse_address option(true);
		mAcceptor->set_option(option);

		mAcceptor->bind(Endpoint(type, port), error);
		if(error) { boost::get<0>(handler)(error); return; }

		mAcceptor->listen(SOMAXCONN, error);
		if(error) { boost::get<0>(handler)(error); return; }

		boost::get<0>(handler)(error);
	}

	template <typename Handler>
	void doAcceptAsync(Session* session, boost::tuple<Handler> handler)
	{
		void (SessionEngineT::*f)(const boost::system::error_code& ec, boost::tuple<Handler> handler) = &SessionEngineT::handleAcceptAsync<Handler>;

		mAcceptor->async_accept(session->socket(), boost::bind(f, this, boost::asio::placeholders::error, handler));
	}

	template <typename Handler>
	void handleAcceptAsync(const boost::system::error_code& ec, boost::tuple<Handler> handler)
	{
		boost::get<0>(handler)(ec);
	}

	template <typename Handler>
	void doConnectAsync(Session* session, const Protocol& type, const std::string& host_name, const std::string& service_name, boost::tuple<Handler> handler)
	{
		void (SessionEngineT::*f)(const boost::system::error_code& ec, boost::tuple<Handler> handler) = &SessionEngineT::handleConnectAsync<Handler>;

		typename Protocol::resolver resolver(*mIoService);
		typename Protocol::resolver::query query(type, host_name, service_name);
		typename Protocol::resolver::iterator it = resolver.resolve(query);

		session->socket().async_connect(*it, boost::bind(f, this, boost::asio::placeholders::error, handler));
	}

	template <typename Handler>
	void handleConnectAsync(const boost::system::error_code& ec, boost::tuple<Handler> handler)
	{
		boost::get<0>(handler)(ec);
	}

	void doStartDispatch(Session* session, CloseCallback onClose, ErrorCallback onError, Dispatcher* dispatcher)
	{
		DispatcherContext* ctx = session->getContext<DispatcherContext>();
		if(!ctx)
		{
			ctx = new DispatcherContext;
			ctx->buffer.reset(new Buffer(detail::MessageHeader::kMaxDataSize));
			session->setContext<DispatcherContext>(ctx);
		}

		ctx->dispatchEnabled = true;
		ctx->onClose = onClose;
		ctx->onError = onError;
		ctx->dispatcher = dispatcher;

		session->readAsync(
				ctx->buffer,
				boost::bind(&SessionEngineT::handleHeaderRead, this, session, placeholders::error),
				detail::MessageHeader::kHeaderSize);
	}

	void doUpdateDispatch(Session* session, CloseCallback onClose, ErrorCallback onError, Dispatcher* dispatcher)
	{
		DispatcherContext* ctx = session->getContext<DispatcherContext>();
		if(!ctx)
		{
			ctx = new DispatcherContext;
			ctx->buffer.reset(new Buffer(detail::MessageHeader::kMaxDataSize));
			session->setContext<DispatcherContext>(ctx);
		}

		ctx->onClose = onClose;
		ctx->onError = onError;
		ctx->dispatcher = dispatcher;
	}

	void doStopDispatch(Session* session)
	{
		DispatcherContext* ctx = session->getContext<DispatcherContext>();

		ctx->dispatchEnabled = false;
		ctx->onClose.clear();
		ctx->onError.clear();
		ctx->dispatcher = NULL;
	}

	void handleHeaderRead(Session* session, const boost::system::error_code& ec)
	{
		DispatcherContext* ctx = session->getContext<DispatcherContext>();

		if(!ec)
		{
			try
			{
				detail::MessageHeader header;
				*ctx->buffer >> header;

				if(UNLIKELY(header.size > detail::MessageHeader::kMaxDataSize))
				{
					throw boost::system::system_error(boost::asio::error::broken_pipe);
				}
				else if(header.size == 0)
				{
					handleBodyRead(session, header.type, header.size, ec);
				}
				else
				{
					ctx->buffer->clear();

					session->readAsync(
							ctx->buffer,
							boost::bind(&SessionEngineT::handleBodyRead, this, session, header.type, header.size, placeholders::error),
							header.size);
				}
			}
			catch(boost::system::system_error& e)
			{
				if(!ctx->onError.empty())
				{
					ctx->onError(*session, e.code());
				}

				if(!ctx->onClose.empty()) ctx->onClose(*session);
				if(session->socket().is_open())
				{
					boost::system::error_code error;
					session->socket().close(error);
				}
				session->markForDeletion();
			}
		}
		else if(ec == boost::asio::error::eof)
		{
			//LOG4CXX_INFO(mLogger, "client closes connection");
			if(!ctx->onClose.empty()) ctx->onClose(*session);
			if(session->socket().is_open())
			{
				boost::system::error_code error;
				session->socket().close(error);
			}
			session->markForDeletion();
		}
		else
		{
			//LOG4CXX_INFO(mLogger, "client handle message read header error, error = " << ec.message());
			if(!ctx->onError.empty())
			{
				ctx->onError(*session, ec);
			}

			if(!ctx->onClose.empty()) ctx->onClose(*session);
			if(session->socket().is_open())
			{
				boost::system::error_code error;
				session->socket().close(error);
			}
			session->markForDeletion();
		}
	}

	void handleBodyRead(Session* session, uint32 type, std::size_t bytes_requested, const boost::system::error_code& ec)
	{
		DispatcherContext* ctx = session->getContext<DispatcherContext>();

		if(!ec)
		{
			try
			{
				//mDispatcher->dispatch(type, *session, ctx->buffer, bytes_requested);
				ctx->dispatcher->dispatch(type, *session, ctx->buffer, bytes_requested);
				ctx->buffer->clear();

				if(ctx->dispatchEnabled)
				{
					session->readAsync(
							ctx->buffer,
							boost::bind(&SessionEngineT::handleHeaderRead, this, session, placeholders::error),
							detail::MessageHeader::kHeaderSize);
				}
			}
			catch(boost::system::system_error& e)
			{
				if(!ctx->onError.empty())
				{
					ctx->onError(*session, e.code());
				}

				if(!ctx->onClose.empty()) ctx->onClose(*session);
				if(session->socket().is_open())
				{
					boost::system::error_code error;
					session->socket().close(error);
				}
				session->markForDeletion();
			}
		}
		else if(ec == boost::asio::error::eof)
		{
			//LOG4CXX_INFO(mLogger, "client closes connection");
			if(!ctx->onClose.empty()) ctx->onClose(*session);
			if(session->socket().is_open())
			{
				boost::system::error_code error;
				session->socket().close(error);
			}
			session->markForDeletion();
		}
		else
		{
			//LOG4CXX_INFO(mLogger, "client handle message read body error, error = " << ec.message());
			if(!ctx->onError.empty())
			{
				ctx->onError(*session, ec);
			}

			if(!ctx->onClose.empty()) ctx->onClose(*session);
			if(session->socket().is_open())
			{
				boost::system::error_code error;
				session->socket().close(error);
			}
			session->markForDeletion();
		}
	}

private:
	Acceptor* mAcceptor;
	Dispatcher* mDispatcher;
	Resolver* mResolver;

	bool mIsIoServiceOwner;
	IoService* mIoService;
};

/// Pre-define TcpSessionEngine
typedef SessionEngineT< SessionTransport::tcp > TcpSessionEngine;

} } }

#endif/*ZILLIANS_NET_SYS_TCPSESSIONENGINEIMPL_H_*/
