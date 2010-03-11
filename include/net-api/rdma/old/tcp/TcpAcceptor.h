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

#ifndef TCPACCEPTOR_H_
#define TCPACCEPTOR_H_

#include "core-api/Prerequisite.h"
#include "net-api/sys/Poller.h"
#include "net-api/sys/address/InetSocketAddress.h"
#include "net-api/sys/tcp/TcpCommon.h"
#include "net-api/sys/tcp/TcpConnection.h"

using namespace std;
using namespace zillians;

namespace zillians { namespace net {

class TcpNetEngine;

class TcpAcceptor
{
public:
	typedef boost::function2<void, SharedPtr< TcpConnection >, int > AcceptorCallback;

public:
	TcpAcceptor(TcpNetEngine* engine);
	~TcpAcceptor();

public:
	static SharedPtr<TcpAcceptor> create(void* e)
	{
		TcpNetEngine* engine = reinterpret_cast<TcpNetEngine*>(e);
		SharedPtr<TcpAcceptor> p(new TcpAcceptor(engine));
		p->mWeakThis = p;
		return p;
	}

public:
	bool accept(SharedPtr<Poller> poller, SharedPtr<InetSocketAddress> address, AcceptorCallback callback);
	void cancel();

public:
	void handleChannelEvent(ev::io &w, int revent);
	void handleTimeoutEvent(ev::timer& w, int revent);

private:
	void cleanup();

public:
	enum Status
	{
		IDLE,
		ACCEPTING,
		CANCELING,
		CANCELED,
		CLOSING,
		CLOSED,
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

	struct
	{
		ev::io watcher;
		ev::timer timeout;
		SharedPtr<Poller> poller;
		handle_t handle;
	} mAcceptInfo;

	WeakPtr<TcpAcceptor> mWeakThis;

	AcceptorCallback mAcceptorCallback;
};

} }

#endif /* TCPACCEPTOR_H_ */
