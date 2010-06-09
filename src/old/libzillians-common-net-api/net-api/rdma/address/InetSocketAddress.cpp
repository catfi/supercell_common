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

#include "networking/rdma/address/InetSocketAddress.h"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
InetSocketAddress::InetSocketAddress()
{
	::memset(&mAddress, 0, sizeof(mAddress));
}

/*
InetSocketAddress::InetSocketAddress(const sockaddr* saddr, size_t len)
{
	//setSocketAddress(sa, len);
}*/

InetSocketAddress::InetSocketAddress(const InetAddress &address, const ushort port)
{
	::memset(&mAddress, 0, sizeof(mAddress));
	setInetAddress(address);
	setPort(port);
}

InetSocketAddress::InetSocketAddress(const InetSocketAddress& obj)
{
	setSocketAddress(obj.getSocketAddress(), obj.getSocketAddressSize());
}


InetSocketAddress::~InetSocketAddress()
{

}

//////////////////////////////////////////////////////////////////////////
char* InetSocketAddress::toString()
{
	static char buf[1024] = "";
	sprintf(buf, "%s:%d", getInetAddress().toString(), getPort());
	return buf;
}

void* InetSocketAddress::toInternal()
{
	return (void*)&mAddress;
}

//////////////////////////////////////////////////////////////////////////
void InetSocketAddress::setPort(uint16_t port)
{
	if(mAddress.sin4.sin_family == AF_INET)
	{
		mAddress.sin4.sin_port = htons(port);
	}
#if ZN_IPV6_SUPPORT
	else if(mAddress.sin6.sin6_family == AF_INET6)
	{
		mAddress.sin6.sin6_port = htons(port);
	}
#endif
	else
	{
		assert(false);
	}
}

uint16_t InetSocketAddress::getPort() const
{
	if(mAddress.sin4.sin_family == AF_INET)
	{
		return ntohs(mAddress.sin4.sin_port);
	}
#if ZN_IPV6_SUPPORT
	else if(mAddress.sin6.sin6_family == AF_INET6)
	{
		return ntohs(mAddress.sin6.sin6_port);
	}
#endif
	else
	{
		assert(false);
		return 0;
	}
}

void InetSocketAddress::setInetAddress(const InetAddress &address)
{
	if(address.getFamily() == AF_INET)
	{
		mAddress.sin4.sin_family = AF_INET;
		::memcpy(&mAddress.sin4.sin_addr, address.getInetAddress(), address.getInetAddressSize());
	}
#if ZN_IPV6_SUPPORT
	else if(address.getFamily() == AF_INET6)
	{
		mAddress.sin6.sin6_family = AF_INET6;
		::memcpy(&mAddress.sin6.sin6_addr, address.getInetAddress(), address.getInetAddressSize());
	}
#endif
	else
	{
		assert(false);
	}
}

const InetAddress InetSocketAddress::getInetAddress() const
{
	if(mAddress.sa.sa_family == AF_INET)
	{
		return InetAddress((const byte*)&mAddress.sin4.sin_addr, sizeof(struct in_addr), false);
	}
#if ZN_IPV6_SUPPORT
	else if(mAddress.sa.sa_family == AF_INET6)
	{
		return InetAddress((const byte*)&mAddress.sin6.sin6_addr, sizeof(struct in6_addr), false);
	}
#endif
	else
	{
		assert(false);
	}
}

void InetSocketAddress::setSocketAddress(const sockaddr* sa, size_t len)
{
	if(len == sizeof(sockaddr_in))
	{
		::memcpy(&mAddress.sin4, sa, len);
	}
#if ZN_IPV6_SUPPORT
	else if(len == sizeof(sockaddr_in6))
	{
		::memcpy(&mAddress.sin6, sa, len);
	}
#endif
	else
	{
		assert(false);
	}
}

sockaddr* InetSocketAddress::getSocketAddress() const
{
	return (sockaddr*)&mAddress;
}

size_t InetSocketAddress::getSocketAddressSize() const
{
	if(mAddress.sin4.sin_family == AF_INET)
	{
		return sizeof(struct sockaddr_in);
	}
#if ZN_IPV6_SUPPORT
	else if(mAddress.sin6.sin6_family == AF_INET6)
	{
		return sizeof(struct sockaddr_in6);
	}
#endif
	else
	{
		return 0;
	}
}

} } }
