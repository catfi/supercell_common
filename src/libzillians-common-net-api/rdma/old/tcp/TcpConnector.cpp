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

#include "net-api/sys/tcp/TcpConnector.h"
#include "net-api/sys/tcp/TcpNetEngine.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

namespace zillians { namespace net {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr TcpConnector::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.sys.tcp.TcpConnector"));

//////////////////////////////////////////////////////////////////////////
TcpConnector::TcpConnector(TcpNetEngine* engine) : mEngine(engine)
{
	TCP_DEBUG("CTOR");
	mConnectInfo.handle = INVALID_HANDLE;
	mStatus = IDLE;
}

TcpConnector::~TcpConnector()
{
	TCP_DEBUG("DTOR");
	cleanup();
	TCP_DEBUG("DTOR_COMPLETE");
}

/////////////////////////////////////////////////////////////////////////
bool TcpConnector::connect(SharedPtr<Poller> poller, SharedPtr<InetSocketAddress> address, ConnectorCallback callback)
{
	mConnectorCallback = callback;

	mConnectInfo.handle = ::socket(address->getSocketAddress()->sa_family, SOCK_STREAM, 0);
	if(mConnectInfo.handle == INVALID_HANDLE)
	{
		TCP_ERROR("fail to create connecting socket, maybe system resources ran out, err = " << strerror(errno));
		errno = 0;
		return false;
	}

	mStatus = CONNECTING;

	mConnectInfo.poller = poller;

	// set to non-blocking I/O
	if(::fcntl(mConnectInfo.handle, F_SETFL, (::fcntl(mConnectInfo.handle, F_GETFL, 0)) | O_NONBLOCK) == -1)
	{
		TCP_ERROR("fail to set non-blocking I/O on connecting socket, err = " << strerror(errno));
		::close(mConnectInfo.handle);
		errno = 0;
		return false;
	}

	// asynchronously connect to remove
	if(::connect(mConnectInfo.handle, address->getSocketAddress(), address->getSocketAddressSize()) == -1)
	{
#ifdef __ZN_WINDOWS__
		int errno = ::WSAGetLastError();
		if(err != WSAEWOULDBLOCK)
		{
			switch(err)
			{
			case WSAECONNREFUSED:
			case WSAENETUNREACH:
			case WSAETIMEDOUT:
				break;
			default:
				LOG4CXX_FATAL(mLogger, "Unrecognized connect err = " << << strerror(errno));
			}
			::close(mConnectInfo.handle);
			return false;
		}
#else
		if(errno != EINPROGRESS)
		{
			switch(errno)
			{
			case ECONNREFUSED:
				TCP_ERROR("connection refused"); break;
			case ETIMEDOUT:
				TCP_ERROR("connect timeout, closed"); break;
			case ENETUNREACH:
				TCP_ERROR("network unreachable"); break;
			case EAGAIN:
				TCP_ERROR("unknown error again"); break;
			default:
				TCP_ERROR("unrecognized connect err = " << strerror(errno));
			}
			::close(mConnectInfo.handle);
			errno = 0;
			return false;
		}
		else
		{
			// the connect request is pending, create watchers
			mConnectInfo.watcher.set(mConnectInfo.handle, ev::WRITE);
			mConnectInfo.watcher.set<TcpConnector, &TcpConnector::handleChannelEvent>(this);
			mConnectInfo.poller->start(&mConnectInfo.watcher);

			mConnectInfo.timeout.set(0.0, 5.0);
			mConnectInfo.timeout.set<TcpConnector, &TcpConnector::handleTimeoutEvent>(this);
			//mConnectInfo.poller->start(&mConnectInfo.timeout);

			errno = 0;
		}
#endif
	}
	else
	{
		// the connection is established right away
		handleConnected();
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////
void TcpConnector::handleChannelEvent(ev::io &w, int revent)
{
	if(mStatus != CONNECTING)
		return;

	handleConnected();
}

void TcpConnector::handleConnected()
{
	// stop all watchers
	if(mConnectInfo.watcher.is_active()) mConnectInfo.watcher.stop();
	if(mConnectInfo.timeout.is_active()) mConnectInfo.timeout.stop();

	// create connection object
	mConnectInfo.connection = TcpConnection::create(mEngine, mConnectInfo.handle);

	// use the poller to start the connection
	mConnectInfo.connection->start(mConnectInfo.poller);

	// add connection object to TcpNetEngine
	mEngine->addConnection(mConnectInfo.connection);

	// dispatch connected event
	mEngine->getDispatcher()->dispatchConnectionEvent(TcpDispatcher::CONNECTED, mConnectInfo.connection);

	mStatus = CONNECTED;

	// notify TcpNetEngine about the completion
	SharedPtr<TcpConnector> shared_from_this(mWeakThis);
	mEngine->connectorCompleted(shared_from_this);

	mConnectorCallback(mConnectInfo.connection, 0);

	cleanup();
}

void TcpConnector::handleTimeoutEvent(ev::timer &w, int revent)
{
	// stop all watchers
	if(mConnectInfo.watcher.is_active()) mConnectInfo.watcher.stop();
	if(mConnectInfo.timeout.is_active()) mConnectInfo.timeout.stop();

	mStatus = ERROR;

	// notify TcpNetEngine about the completion
	SharedPtr<TcpConnector> shared_from_this(mWeakThis);
	mEngine->connectorCompleted(shared_from_this);

	mConnectorCallback(SharedPtr<TcpConnection>(), -1);

	cleanup();
}

/////////////////////////////////////////////////////////////////////////
void TcpConnector::cleanup()
{
	if(mConnectInfo.handle == INVALID_HANDLE)
		return;

	if(mConnectInfo.watcher.is_active()) mConnectInfo.poller->stop(&mConnectInfo.watcher);
	if(mConnectInfo.timeout.is_active()) mConnectInfo.poller->stop(&mConnectInfo.timeout);

	// TODO here we should wait until the watcher is actually stoped
	//::shutdown(mConnectInfo.handle, SHUT_RDWR);
	//::close(mConnectInfo.handle);
	//errno = 0;

	mConnectInfo.handle = INVALID_HANDLE;
}

//////////////////////////////////////////////////////////////////////////
void TcpConnector::cancel()
{
	if(mStatus == CONNECTING)
	{
		mStatus = CANCELING;
		cleanup();
	}
}

} }
