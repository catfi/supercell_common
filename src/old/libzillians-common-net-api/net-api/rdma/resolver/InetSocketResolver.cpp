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

#include "net-api/rdma/resolver/InetSocketResolver.h"
#include "net-api/rdma/address/InetAddress.h"
#include "net-api/rdma/address/InetSocketAddress.h"

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
InetSocketResolver::InetSocketResolver()
{
}

InetSocketResolver::~InetSocketResolver()
{
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<InetSocketAddress> InetSocketResolver::resolve(std::string host)
{
	std::string token(":");

	size_t pos = host.find_last_of(token);
	if(pos == std::string::npos)
		return shared_ptr<InetSocketAddress>();

	InetAddress inetAddr;
	std::string name = host.substr(0, pos);

	if(name.compare("*") == 0)
		inetAddr.setByHostName(name, InetAddress::INET6);
	else
		inetAddr.setByHostName(name);

	if(!inetAddr.isValid())
		return shared_ptr<InetSocketAddress>();

	std::string port = host.substr(pos+1);

	int nport = atoi(port.c_str());
	if(nport <= 0 || nport >= 65536)
		return shared_ptr<InetSocketAddress>();

	return shared_ptr<InetSocketAddress>(new InetSocketAddress(inetAddr, nport));
}

} } }
