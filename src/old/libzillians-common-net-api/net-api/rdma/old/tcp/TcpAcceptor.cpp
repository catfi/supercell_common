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

#include "net-api/sys/tcp/TcpAcceptor.h"
#include "net-api/sys/tcp/TcpNetEngine.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

namespace zillians { namespace net {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr TcpAcceptor::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.sys.tcp.TcpAcceptor"));

/////////////////////////////////ServerHandler/////////////////////////////////////////
TcpAcceptor::TcpAcceptor(TcpNetEngine* engine)
{
	mEngine = engine;
	mAcceptInfo.handle = INVALID_HANDLE;
	mStatus = IDLE;
}

TcpAcceptor::~TcpAcceptor()
{
	TCP_DEBUG("DTOR");
	cleanup();
	TCP_DEBUG("DTOR_COMPLETE");
}

//////////////////////////////////////////////////////////////////////////
bool TcpAcceptor::accept(shared_ptr<Poller> poller, shared_ptr<InetSocketAddress> address, AcceptorCallback callback)
{
	if(mAcceptInfo.handle != INVALID_HANDLE)
	{
		mStatus = ERROR;
		return false;
	}

	mAcceptorCallback = callback;

	mStatus = ACCEPTING;

	mAcceptInfo.poller = poller;

	mAcceptInfo.handle = ::socket(address->getSocketAddress()->sa_family, SOCK_STREAM, 0);
	if(mAcceptInfo.handle == INVALID_HANDLE)
	{
		TCP_ERROR("fail to create accept socket, maybe system resources ran out, err = " << strerror(errno));
		mStatus = ERROR;
		errno = 0;
		return false;
	}

	if(::fcntl(mAcceptInfo.handle, F_SETFL, (::fcntl(mAcceptInfo.handle, F_GETFL, 0)) | O_NONBLOCK) == -1)
	{
		TCP_ERROR("fail to set non-blocking I/O on accept socket, err = " << strerror(errno));
		mStatus = ERROR;
		errno = 0;
		return false;
	}

	const int optEnable = 1;
	if(::setsockopt(mAcceptInfo.handle, SOL_SOCKET, SO_REUSEADDR, &optEnable, sizeof(optEnable)) == -1)
	{
		// TODO log warning fail to set reuse address
		TCP_ERROR("fail to enable address-reuse socket option, err = " << strerror(errno));
		errno = 0;
	}

#if ZN_DEFER_ACCEPT_SUPPORT
	const int optTimeout = 3000;
	if(::setsockopt(mAcceptInfo.handle, IPPROTO_TCP, TCP_DEFER_ACCEPT, &optTimeout, sizeof(optTimeout)) == -1)
	{
		// TODO log warning fail to enable defer accept
		TCP_ERROR("fail to enable defer accept optimization, err = " << strerror(errno));
		errno = 0;
	}
#endif

	// get the sockaddr structure from InetAddress object
	if(::bind(mAcceptInfo.handle, address->getSocketAddress(), address->getSocketAddressSize()) == -1)
	{
		TCP_ERROR("fail to bind on address, err = " << strerror(errno));
		mStatus = ERROR;
		errno = 0;
		return false;
	}

	if(::listen(mAcceptInfo.handle, SOMAXCONN) == -1)
	{
		TCP_ERROR("fail to listen on address, err = " << strerror(errno));
		mStatus = ERROR;
		errno = 0;
		return false;
	}

	// start the libev watcher
	mAcceptInfo.watcher.set(mAcceptInfo.handle, ev::READ);
	mAcceptInfo.watcher.set< TcpAcceptor, &TcpAcceptor::handleChannelEvent > (this);
	mAcceptInfo.poller->start(&mAcceptInfo.watcher);

	return true;
}

//////////////////////////////////////////////////////////////////////////
void TcpAcceptor::handleChannelEvent(ev::io &w, int revent)
{
	if(mStatus != ACCEPTING)
		return;

	struct sockaddr_in addr;
	socklen_t addrlen = (socklen_t)sizeof(sockaddr_in);

	// reset errno
	errno = 0;

	// try to accept the connection
	handle_t conn_id = ::accept(mAcceptInfo.handle, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
	if(UNLIKELY(conn_id == -1))
	{
		TCP_ERROR("accept failed, early TCP socket termination with invalid conn id, err = " << strerror(errno));
		mAcceptorCallback(shared_ptr<TcpConnection>(), errno);
		errno = 0;
		return;
	}
	else if(UNLIKELY(errno > 0))
	{
		TCP_ERROR("accept failed, early TCP socket termination, err = " << strerror(errno));
		mAcceptorCallback(shared_ptr<TcpConnection>(), errno);
		::close(conn_id);
		errno = 0;
		return;
	}

	// set to non-blocking I/O
	if(fcntl(conn_id, F_SETFL, (fcntl(conn_id, F_GETFL, 0)) | O_NONBLOCK) == -1)
	{
		TCP_ERROR("fail to enable non-blocking I/O, err = " << strerror(errno));
		mAcceptorCallback(shared_ptr<TcpConnection>(), errno);
		::close(conn_id);
		errno = 0;
		return;
	}

	shared_ptr<TcpConnection> connection = TcpConnection::create(mEngine, conn_id);
	mEngine->addConnection(connection);

	connection->start(mAcceptInfo.poller);

	mAcceptorCallback(connection, 0);

	mEngine->getDispatcher()->dispatchConnectionEvent(TcpDispatcher::CONNECTED, connection);
}

void handleTimeoutEvent(ev::timer& w, int revent)
{
}

//////////////////////////////////////////////////////////////////////////
void TcpAcceptor::cleanup()
{
	if(mAcceptInfo.handle == INVALID_HANDLE)
		return;

	mAcceptInfo.poller->stop(&mAcceptInfo.watcher);

	// TODO here we should wait until the watcher is actually stoped
	::shutdown(mAcceptInfo.handle, SHUT_RDWR);
	::close(mAcceptInfo.handle);
	errno = 0;

	mAcceptInfo.handle = INVALID_HANDLE;
}

//////////////////////////////////////////////////////////////////////////
void TcpAcceptor::cancel()
{
	if(mStatus == ACCEPTING)
	{
		mStatus = CANCELING;

		// notify TcpNetEngine about the completion
		shared_ptr<TcpAcceptor> shared_from_this(mWeakThis);
		mEngine->acceptorCompleted(shared_from_this);

		cleanup();
	}
}

} }
