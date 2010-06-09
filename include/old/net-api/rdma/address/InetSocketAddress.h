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

#ifndef ZILLIANS_NET_RDMA_INETSOCKETADDRESS_H_
#define ZILLIANS_NET_RDMA_INETSOCKETADDRESS_H_

#include "core/Prerequisite.h"
#include "networking/rdma/Address.h"
#include "networking/rdma/address/InetAddress.h"
#include <sys/socket.h>

namespace zillians { namespace net { namespace rdma {

class InetSocketAddress : public Address
{
public:
	InetSocketAddress();
	InetSocketAddress(const InetAddress &address, const ushort port);
	InetSocketAddress(const InetSocketAddress& obj);
	~InetSocketAddress();

public:
	virtual char* toString();
	virtual void* toInternal();

public:
	void setPort(const uint16_t port);
	uint16_t getPort() const;

	void setInetAddress(const InetAddress &address);
	const InetAddress getInetAddress() const;

	void setSocketAddress(const sockaddr* sa, size_t len);
	sockaddr* getSocketAddress() const;

	size_t getSocketAddressSize() const;

private:
	union
	{
		struct sockaddr sa;
		struct sockaddr_in  sin4;
#if ZN_IPV6_SUPPORT
		struct sockaddr_in6 sin6;
#endif
	} mAddress;
};

} } }

#endif/*ZILLIANS_NET_RDMA_INETSOCKETADDRESS_H_*/
