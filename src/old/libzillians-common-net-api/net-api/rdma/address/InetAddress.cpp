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

#include "networking/rdma/address/InetAddress.h"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
InetAddress::InetAddress()
{
	mFamily = AF_UNSPEC;
	::memset(&mAddress, 0, sizeof(mAddress));
}

InetAddress::InetAddress(const std::string &hostName, int family)
{
	if(setByHostName(hostName, family) != ZN_OK)
	{
		mFamily = AF_UNSPEC;
		::memset(&mAddress, 0, sizeof(mAddress));
	}
}

InetAddress::InetAddress(const byte* address, size_t addrlen, bool mapv4tov6)
{
	if(setByAddress(address, addrlen, mapv4tov6) != ZN_OK)
	{
		mFamily = AF_UNSPEC;
		::memset(&mAddress, 0, sizeof(mAddress));
	}
}

InetAddress::InetAddress(const uint32 address)
{
	if(setByAddress(address) != ZN_OK)
	{
		mFamily = AF_UNSPEC;
		::memset(&mAddress, 0, sizeof(mAddress));
	}
}

InetAddress::InetAddress(const InetAddress& obj)
{
	mFamily = obj.mFamily;
	::memcpy(&mAddress, &obj.mAddress, sizeof(mAddress));
}

InetAddress::~InetAddress()
{

}

//////////////////////////////////////////////////////////////////////////
int32 InetAddress::setByHostName(const std::string &hostName, int32 family)
{
	struct addrinfo hints;
	struct addrinfo *result = NULL;
	::memset(&hints, 0, sizeof(addrinfo));

	// if the host name is "*", set it to INADDR_ANY or
	if(hostName.compare("*") == 0)
	{
		if(family == AF_INET)
		{
			mFamily = AF_INET;
			mAddress.in4.s_addr = INADDR_ANY;
			return ZN_OK;
		}
#if ZN_IPV6_SUPPORT
		else if(family == AF_INET6)
		{
			mFamily = AF_INET6;
			mAddress.in6 = in6addr_any;
			return ZN_OK;
		}
#endif
		return ZN_ERROR;
	}

#if ZN_IPV6_SUPPORT
	if(family == AF_UNSPEC || family == AF_INET6)
	{
		// try to resolve as IPv6 first
		hints.ai_family = AF_INET6;
		if(::getaddrinfo(hostName.c_str(), NULL, &hints, &result))
		{
			// resolve failed,
			if(family == AF_INET6)
			{
				// if it's AF_INET6, just return
				if(result)	::freeaddrinfo(result);
				return ZN_ERROR;
			}
			else
			{
				// otherwise, which is AF_UNSPEC, try to resolve as AF_INET
				family = AF_INET;
			}
		}
		else
		{
       		family = AF_INET6;
		}
	}
#else
	if(family == AF_UNSPEC) family = AF_INET;
#endif

	if(!result)
	{
		if(family == AF_INET)
		{
			hints.ai_family = AF_INET;
			if(::getaddrinfo(hostName.c_str(), NULL, &hints, &result))
			{
				// resolve failed
				if(result)	::freeaddrinfo(result);
				return ZN_ERROR;
			}
		}
		else
		{
			// family is not valid (possible options are AF_UNSPEC, AF_INET, AF_INET6)
			return ZN_ERROR;
		}
	}

	// here we've successfully resolved the address
	mFamily = family;

#if ZN_IPV6_SUPPORT
	if(result->ai_addrlen == sizeof(struct sockaddr_in6))
	{
		const sockaddr_in6* addrv6 = reinterpret_cast<const sockaddr_in6*>(result->ai_addr);
		::memcpy(&mAddress, &addrv6->sin6_addr, sizeof(struct in6_addr));
	}
	else
	{
		const sockaddr_in* addrv4 = reinterpret_cast<const sockaddr_in*>(result->ai_addr);
		::memcpy(&mAddress, &addrv4->sin_addr, sizeof(struct in_addr));
	}
#else
	assert(result->ai_addrlen == sizeof(struct sockaddr_in));
	const sockaddr_in* addrv4 = reinterpret_cast<const sockaddr_in*>(result->ai_addr);
	::memcpy(&mAddress, &addrv4->sin_addr, sizeof(struct in_addr));
#endif

	::freeaddrinfo(result);

	return ZN_OK;
}

int32 InetAddress::setByAddress(const byte* address, size_t addrlen, bool mapv4tov6)
{
	if(addrlen == sizeof(struct in_addr))
	{
		if(!mapv4tov6)
		{
			mFamily = AF_INET;

			uint ipv4 = htonl(*reinterpret_cast<const uint*>(address));
			::memcpy(&mAddress.in4, &ipv4, addrlen);

			return ZN_OK;
		}
		else
		{
			mFamily = AF_INET;

			uint ipv4 = htonl(*reinterpret_cast<const uint*>(address));
			::memcpy(&mAddress.in4, &ipv4, addrlen);
#if ZN_IPV6_SUPPORT
			if(ipv4 == htonl(INADDR_ANY))
			{
				struct in6_addr ipv6 = in6addr_any;
				::memcpy(&mAddress.in6, &ipv6, sizeof(ipv6));
			}
			else
			{
				::memset(&mAddress.in6, 0, 16);
				mAddress.in6.s6_addr[10] = mAddress.in6.s6_addr[11] = 0xff;
				::memcpy(&mAddress.in6.s6_addr[12], &ipv4, sizeof(ipv4));
			}
#endif
			return ZN_OK;
		}
	}
#if ZN_IPV6_SUPPORT
	else if(addrlen == sizeof(struct in6_addr))
	{
		mFamily = AF_INET6;

		::memcpy(&mAddress.in6, address, addrlen);

		return ZN_OK;
	}
#endif
	else
	{
		return ZN_ERROR;
	}
}

int32 InetAddress::setByAddress(const uint32 address)
{
	return setByAddress((const byte*)&address, sizeof(uint32));
}

//////////////////////////////////////////////////////////////////////////
bool InetAddress::isValid()
{
	return (mFamily == AF_INET || mFamily == AF_INET6) ? true : false;
}

int32 InetAddress::getFamily() const
{
	return mFamily;
}

const byte* InetAddress::getInetAddress() const
{
	return (const byte*)&mAddress;
}

const size_t InetAddress::getInetAddressSize() const
{
	if(mFamily == AF_INET)
		return sizeof(struct in_addr);
#if ZN_IPV6_SUPPORT
	else if(mFamily == AF_INET6)
		return sizeof(struct in6_addr);
#endif
	else
		return 0;
}

const char* InetAddress::toString(bool resolveHostName) const
{
	if(resolveHostName)
	{
		// TODO implement host name lookup
	}
	else
	{
#ifdef __ZN_WINDOWS__
		// TODO this only support IPv4, how about IPv6?
		return ::inet_ntoa(mAddress.in4);
#else
		// TODO this only support IPv4, how about IPv6?
		return ::inet_ntoa(mAddress.in4);
#endif
	}
}

} } }
