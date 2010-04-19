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

#include "net-api/queue/impl/ev/LibEvAcceptor.h"
#include "net-api/queue/impl/ev/LibEvNetEngine.h"
#include "net-api/queue/impl/ev/LibEvChannel.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

namespace zillians {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr LibEvAcceptor::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.sys.tcp.TcpAcceptor"));

//////////////////////////////////////////////////////////////////////////
LibEvAcceptor::LibEvAcceptor(LibEvQueueEngine* ref)
{
	mEngineRef = ref;
	mHandle = INVALID_HANDLE;
}

LibEvAcceptor::~LibEvAcceptor()
{
	this->close();
}

//////////////////////////////////////////////////////////////////////////
int32 LibEvAcceptor::listen(const InetSocketAddress *local)
{
	if(mHandle != INVALID_HANDLE)
		return ZN_ERROR;

	mHandle = ::socket(local->getSocketAddress()->sa_family, SOCK_STREAM, 0);
	if(mHandle == INVALID_HANDLE)
	{
		LOG4CXX_FATAL(mLogger, "Fail to create accept socket, maybe system resources ran out, errno = " << Errno::detail());
		return ZN_ERROR;
	}

	if(::fcntl(mHandle, F_SETFL, (::fcntl(mHandle, F_GETFL, 0)) | O_NONBLOCK) == -1)
	{
		LOG4CXX_FATAL(mLogger, "Fail to set non-blocking I/O on accept socket, errno = " << Errno::detail());
		return ZN_ERROR;
	}

	const int optEnable = 1;
	if(::setsockopt(mHandle, SOL_SOCKET, SO_REUSEADDR, &optEnable, sizeof(optEnable)) == -1)
	{
		// TODO log warning fail to set reuse address
		LOG4CXX_WARN(mLogger, "Fail to enable address-reuse socket option, errno = " << Errno::detail());
	}

#if ZN_DEFER_ACCEPT_SUPPORT
	const int optTimeout = 3000;
	if(::setsockopt(mHandle, IPPROTO_TCP, TCP_DEFER_ACCEPT, &optTimeout, sizeof(optTimeout)) == -1)
	{
		// TODO log warning fail to enable defer accept
		LOG4CXX_WARN(mLogger, "Fail to enable defer accept optimization, errno = " << Errno::detail());
	}
#endif

	// get the sockaddr structure from InetAddress object
	if(::bind(mHandle, local->getSocketAddress(), local->getSocketAddressSize()) == -1)
	{
		LOG4CXX_ERROR(mLogger, "Fail to bind on address, errno = " << Errno::detail());
		return ZN_ERROR;
	}

	if(::listen(mHandle, SOMAXCONN) == -1)
	{
		LOG4CXX_ERROR(mLogger, "Fail to listen on address, errno = " << Errno::detail());
		return ZN_ERROR;
	}

	// start the libev watcher
	mAcceptWatcher.set(mHandle, ev::READ);
	mAcceptWatcher.set< LibEvAcceptor, &LibEvAcceptor::handleConnectionAccepted > (this);
	mEngineRef->getDaemon()->getReadProcessor()->evIoStart(&mAcceptWatcher);

	mAddress = *local;

	return ZN_OK;
}
//////////////////////////////////////////////////////////////////////////
void LibEvAcceptor::handleConnectionAccepted(ev::io &w, int revent)
{
	struct sockaddr_in* addr = new sockaddr_in;
	socklen_t addrlen = (socklen_t)sizeof(sockaddr_in);

	Errno::reset();

	handle_t conn = ::accept(mHandle, (struct sockaddr*)addr, (socklen_t*)&addrlen);
	if(UNLIKELY(conn == -1))
	{
		SAFE_DELETE(addr);
		LOG4CXX_WARN(mLogger, "Accept failed, early TCP socket termination with invalid conn id, errno = " << Errno::detail());
		return;
	}
	else if(UNLIKELY(errno > 0))
	{
		LOG4CXX_WARN(mLogger, "Accept failed, early TCP socket termination, errno = " << Errno::detail());
		SAFE_DELETE(addr);
		::close(conn);
		return;
	}

	// set to non-blocking I/O
	if(fcntl(conn, F_SETFL, (fcntl(conn, F_GETFL, 0)) | O_NONBLOCK) == -1)
	{
		LOG4CXX_ERROR(mLogger, "Fail to enable non-blocking I/O, errno = " << Errno::detail());
		SAFE_DELETE(addr);
		::close(conn);
		return;
	}

	LibEvQueue *q = NULL;
	mEngineRef->createQueue(conn, mAddress, &q);

	if(!q)
	{
		LOG4CXX_ERROR(mLogger, "Fail to create network queue object, insufficient memory, connection closed");
		SAFE_DELETE(addr);
		::close(conn);
		return;
	}

	mEngineRef->getQueueEventDispatcher()->dispatchQueueConnected(q);
}

void LibEvAcceptor::close()
{
	if(mHandle == INVALID_HANDLE)
		return;

	mEngineRef->getDaemon()->getReadProcessor()->evIoStop(&mAcceptWatcher);
	::shutdown(mHandle, SHUT_RDWR);
	::close(mHandle);

	mHandle = INVALID_HANDLE;
}

}
