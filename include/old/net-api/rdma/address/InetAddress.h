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

#ifndef ZILLIANS_NET_RDMA_INETADDRESS_H_
#define ZILLIANS_NET_RDMA_INETADDRESS_H_

#include "core-api/Prerequisite.h"
#include <netinet/in.h>

namespace zillians { namespace net { namespace rdma {

class InetAddress
{
public:
	InetAddress();
	InetAddress(const std::string &hostName, int32 family = AF_UNSPEC);
	InetAddress(const byte* address, size_t addrlen, bool mapv4tov6 = false);
	InetAddress(const uint32 address);
	InetAddress(const InetAddress& obj);
	~InetAddress();

public:
	int32 setByHostName(const std::string &hostName, int32 family = AF_UNSPEC);
	int32 setByAddress(const byte* address, size_t addrlen, bool mapv4tov6 = false);
	int32 setByAddress(const uint32 address);

public:
	enum SupportedAddressType
	{
		INET = AF_INET,
		INET6 = AF_INET6,
	};

public:
	bool isValid();
	int32 getFamily() const;
	const byte* getInetAddress() const;
	const size_t getInetAddressSize() const;

public:
	const char* toString(bool resolveHostName = false) const;

private:
	int mFamily;
	union
	{
		struct in_addr in4;
#if ZN_IPV6_SUPPORT
		struct in6_addr in6;
#endif
	} mAddress;
};

} } }

#endif/*ZILLIANS_NET_RDMA_INETADDRESS_H_*/
