//
// Zillians MMO
// Copyright (C) 2007-2008 Zillians.com, Inc.
// For more information see http://www.zillians.com
//
// Zillians MMO is the library and runtime for massive multiplayer online game
// development in utility computing model, which runs as a service for every
// developer to build their virtual world running on our GPU-assisted machines
//
// This is a close source library intended to be used solely within Zillians.com
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Contact Information: info@zillians.com
//

#include "networking/queue/impl/ev/LibEvConnector.h"
#include "networking/queue/impl/ev/LibEvNetEngine.h"

namespace zillians {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr LibEvConnector::mLogger(log4cxx::Logger::getLogger("zillians.common.networking.queue.impl.ev.LibEvConnector"));

//////////////////////////////////////////////////////////////////////////
LibEvConnector::LibEvConnector(LibEvQueueEngine* ref)
{
	mEngineRef = ref;
}

LibEvConnector::~LibEvConnector()
{
}

/////////////////////////////////////////////////////////////////////////
int32 LibEvConnector::connect(const InetSocketAddress *remote)
{
	handle_t sock = ::socket(remote->getSocketAddress()->sa_family, SOCK_STREAM, 0);
	if(sock == INVALID_HANDLE)
	{
		LOG4CXX_FATAL(mLogger, "Fail to create connecting socket, maybe system resources ran out, errno = " << Errno::detail());
		return ZN_ERROR;
	}

	if(::connect(sock, remote->getSocketAddress(), remote->getSocketAddressSize()) == -1)
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
				LOG4CXX_FATAL(mLogger, "Unrecognized connect errno = " << << Errno::detail());
			}
			::close(sock);
			return ZN_ERROR;
		}
#else
		if(errno != EINPROGRESS)
		{
			switch(errno)
			{
			case ECONNREFUSED:
				LOG4CXX_DEBUG(mLogger, "Connection refused"); break;
			case ETIMEDOUT:
				LOG4CXX_DEBUG(mLogger, "Connect timeout, closed"); break;
			case ENETUNREACH:
				LOG4CXX_DEBUG(mLogger, "Network unreachable"); break;
			case EAGAIN:
				LOG4CXX_DEBUG(mLogger, "Unknown error again"); break;
			default:
				LOG4CXX_FATAL(mLogger, "Unrecognized connect errno = " << Errno::detail());
			}
			::close(sock);
			return ZN_ERROR;
		}
#endif
	}

	// if we successfully get connected, set to non-blocking I/O
	if(::fcntl(sock, F_SETFL, (::fcntl(sock, F_GETFL, 0)) | O_NONBLOCK) == -1)
	{
		LOG4CXX_FATAL(mLogger, "Fail to set non-blocking I/O on connecting socket, errno = " << Errno::detail());
		::close(sock);
		return ZN_ERROR;
	}

	// create the network queue and notify queue event listener
	LibEvQueue *q = NULL;
	mEngineRef->createQueue(sock, *remote, &q);
	mEngineRef->getQueueEventDispatcher()->dispatchQueueConnected(q);

	/*
	LibEvAsyncConn* asyncConn = new LibEvAsyncConn();
	asyncConn->handle = sock;

	asyncConn->watcherConnected.set(sock, ev::WRITE);
	asyncConn->watcherConnected.set< LibEvConnector, &LibEvConnector::handleConnected > (this);
	asyncConn->watcherConnected.set_ref_obj(asyncConn);

	asyncConn->watcherTimeout.set (0.0, 10.0);
	asyncConn->watcherTimeout.set< LibEvConnector, &LibEvConnector::handleTimeout > (this);
	asyncConn->watcherTimeout.set_ref_obj(asyncConn);

	// since the current execution context is different from processor thread context,
	// use the evXXX methods in processor to guarantee thread-safety
	mEngineRef->getDaemon()->getWriteProcessor()->evIoStart(&asyncConn->watcherConnected);
	mEngineRef->getDaemon()->getWriteProcessor()->evTimerStart(&asyncConn->watcherTimeout);
	*/

	return ZN_OK;
}

void LibEvConnector::close()
{
	// TODO close all items
}

/////////////////////////////////////////////////////////////////////////
/*
void LibEvConnector::handleConnected(ev::io &w, int revent)
{
	LibEvAsyncConn* asyncConn = reinterpret_cast<LibEvAsyncConn*>(w.get_ref_obj());

	if(asyncConn->watcherConnected.is_active())
	{
		LibEvQueue *q = NULL;
		mEngineRef->createQueue(asyncConn->handle, &q);
		mEngineRef->getQueueEventDispatcher()->dispatchQueueConnected(q);
	}

	asyncConn->watcherConnected.stop();
	asyncConn->watcherTimeout.stop();
}

void LibEvConnector::handleTimeout(ev::timer &w, int revent)
{
	LibEvAsyncConn* asyncConn = reinterpret_cast<LibEvAsyncConn*>(w.get_ref_obj());

	if(asyncConn->watcherTimeout.is_active())
	{
		LOG4CXX_DEBUG(mLogger, "Connect timeout, closed");
		::close(asyncConn->handle);
	}

	asyncConn->watcherConnected.stop();
	asyncConn->watcherTimeout.stop();
}*/

}
