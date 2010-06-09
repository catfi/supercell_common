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

#ifndef TCPCONNECTOR_H_
#define TCPCONNECTOR_H_

#include "core/Prerequisite.h"
#include "networking/sys/Poller.h"
#include "networking/sys/address/InetSocketAddress.h"
#include "networking/sys/tcp/TcpCommon.h"
#include "networking/sys/tcp/TcpConnection.h"

using namespace std;
using namespace zillians;

namespace zillians { namespace net {

class TcpNetEngine;

class TcpConnector
{
public:
	typedef boost::function2<void, shared_ptr< TcpConnection >, int > ConnectorCallback;

public:
	TcpConnector(TcpNetEngine* engine);
	~TcpConnector();

public:
	static shared_ptr<TcpConnector> create(void* e)
	{
		TcpNetEngine* engine = reinterpret_cast<TcpNetEngine*>(e);
		shared_ptr<TcpConnector> p(new TcpConnector(engine));
		p->mWeakThis = p;
		return p;
	}

public:
	bool connect(shared_ptr<Poller> poller, shared_ptr<InetSocketAddress> address, ConnectorCallback callback);
	void cancel();

public:
	void handleChannelEvent(ev::io& w, int revent);
	void handleTimeoutEvent(ev::timer& w, int revent);

private:
	void handleConnected();

private:
	void cleanup();

public:
	enum Status
	{
		IDLE,
		CONNECTING,
		CONNECTED,
		CANCELING,
		CANCELED,
		ERROR,
	} mStatus;

	inline Status status()
	{
		return mStatus;
	}

private:
	static log4cxx::LoggerPtr mLogger;

private:
	TcpNetEngine* mEngine;

private:
	struct
	{
		ev::io watcher;
		ev::timer timeout;
		shared_ptr<Poller> poller;
		handle_t handle;
		shared_ptr<TcpConnection> connection;
	} mConnectInfo;

	WeakPtr<TcpConnector> mWeakThis;

	ConnectorCallback mConnectorCallback;
};

} }

#endif /* TCPCONNECTOR_H_ */