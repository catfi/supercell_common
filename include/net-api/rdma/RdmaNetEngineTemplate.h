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

#ifndef ZILLIANS_NET_RDMA_NETENGINETEMPLATE_H_
#define ZILLIANS_NET_RDMA_NETENGINETEMPLATE_H_

#include "core-api/Prerequisite.h"
#include "core-api/HashMap.h"
#include "core-api/BufferManager.h"
#include "net-api/rdma/Poller.h"
#include "net-api/rdma/address/InetSocketAddress.h"
#include "net-api/rdma/resolver/InetSocketResolver.h"
#include "core-api/Future.h"
#include "tbb/spin_mutex.h"

//using namespace std;
using namespace zillians;

namespace zillians { namespace net { namespace rdma {

template <class Connection, class Connector, class Acceptor, class Dispatcher>
class RdmaNetEngineTemplate
{
public:
	typedef typename Connection::HandleType HandleType;
	typedef boost::function2<void, shared_ptr< Connection >, int > ConnectorCallback;
	typedef boost::function2<void, shared_ptr< Connection >, int > AcceptorCallback;
	typedef boost::function2<void, shared_ptr< Connection >, int > ConnectionErrorCallback;

public:
	RdmaNetEngineTemplate() : mAddressResolver(new InetSocketResolver())
	{
		mStopped = false;
	}

	virtual ~RdmaNetEngineTemplate()
	{
		shutdown();
	}

public:
	shared_ptr< Future<Connector> > connect(shared_ptr<Poller> poller, std::string address, ConnectorCallback callback)
	{
		BOOST_ASSERT(mDispatcher.get() != NULL);

		shared_ptr<Connector> connector = Connector::create(this);

		// try to resolve address
		shared_ptr<InetSocketAddress> inetAddress = mAddressResolver->resolve(address);
		if(!inetAddress)
		{
			return shared_ptr< Future<Connector> >();
		}

		// insert the connector
		typename tConnectorContainer::accessor a;
		mConnectorContainer.insert(a, connector.get());
		a->second = connector;

		// call connector to connect
		connector->connect(poller, inetAddress, callback);

		// return the future object (future template)
		shared_ptr< Future<Connector> > f(new Future<Connector>(connector));

		return f;
	}

	shared_ptr< Future<Acceptor> > accept(shared_ptr<Poller> poller, std::string address, AcceptorCallback callback)
	{
		BOOST_ASSERT(mDispatcher.get() != NULL);

		shared_ptr<Acceptor> acceptor = Acceptor::create(this);

		// try to resolve address
		shared_ptr<InetSocketAddress> inetAddress = mAddressResolver->resolve(address);
		if(!inetAddress)
		{
			return shared_ptr< Future<Acceptor> >();
		}

		// insert the acceptor
		typename tAcceptorContainer::accessor a;
		mAcceptorContainer.insert(a, acceptor.get());
		a->second = acceptor;

		// call acceptor to start accepting incoming connection
		acceptor->accept(poller, inetAddress, callback);

		// return the future object (future template)
		shared_ptr< Future<Acceptor> > f(new Future<Acceptor>(acceptor));
		return f;
	}

	inline shared_ptr<Dispatcher> getDispatcher()					{ return mDispatcher; }
	inline void setDispatcher(shared_ptr<Dispatcher> dispatcher)		{ mDispatcher = dispatcher; }

	inline shared_ptr<BufferManager> getBufferManager()				{ return mBufferManager; }
	inline void setBufferManager(shared_ptr<BufferManager> manager)	{ mBufferManager = manager; }

public:
	void shutdown()
	{
		if(mStopped) return;

		// TODO maybe work, maybe not....Orz, check begin() and end() thread-safety later
		for(typename tConnectionContainer::iterator it = mConnectionContainer.begin();
		    it != mConnectionContainer.end(); ++it)
		{
			if(it->second.get())
			{
				it->second->close();
			}
		}

		while(!mConnectionContainer.empty())
		{
			usleep(200); // sleep for 200ms and keep waiting until all connection are safely removed from the container
		}

		mStopped = true;
	}

public:
	void connectorCompleted(shared_ptr<Connector> connector)
	{
		// remove the connector object from the connector container
		typename tConnectorContainer::accessor a;

		if(UNLIKELY(!mConnectorContainer.find(a, connector.get()))) { BOOST_ASSERT(false); }

		mConnectorContainer.erase(a);
	}

	void acceptorCompleted(shared_ptr<Acceptor> acceptor)
	{
		// remove the acceptor object from the acceptor container
		typename tAcceptorContainer::accessor a;

		if(UNLIKELY(!mAcceptorContainer.find(a, acceptor.get()))) { BOOST_ASSERT(false); }

		mAcceptorContainer.erase(a);
	}

public:
	void addConnection(shared_ptr<Connection> connection)
	{
		typename tConnectionContainer::accessor a;

		mConnectionContainer.insert(a, connection->getHandle());

		a->second = connection;
	}

	shared_ptr<Connection> getConnection(HandleType handle)
	{
		typename tConnectionContainer::accessor a;

		if(UNLIKELY(!mConnectionContainer.find(a, handle))) { BOOST_ASSERT(false); }

		return a->second;
	}

	void removeConnection(shared_ptr<Connection> connection)
	{
		typename tConnectionContainer::accessor a;

		if(UNLIKELY(!mConnectionContainer.find(a, connection->getHandle()))) { BOOST_ASSERT(false); }

		mConnectionContainer.erase(a);
	}

private:
	shared_ptr<Dispatcher> mDispatcher;
	shared_ptr<BufferManager> mBufferManager;

	shared_ptr<InetSocketResolver> mAddressResolver;
	bool mStopped;

private:
	typedef tbb::concurrent_hash_map< HandleType, shared_ptr<Connection> > tConnectionContainer;
	typedef tbb::concurrent_hash_map< Connector*, shared_ptr<Connector> > tConnectorContainer;
	typedef tbb::concurrent_hash_map< Acceptor*, shared_ptr<Acceptor> > tAcceptorContainer;

	tConnectionContainer mConnectionContainer;
	tConnectorContainer mConnectorContainer;
	tAcceptorContainer mAcceptorContainer;

protected:
	static log4cxx::LoggerPtr mLogger;
};

//////////////////////////////////////////////////////////////////////////
template <class Connection, class Connector, class Acceptor, class Dispatcher>
log4cxx::LoggerPtr RdmaNetEngineTemplate<Connection,Connector,Acceptor,Dispatcher>::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.sys.NetEngine"));

} } }

#endif/*ZILLIANS_NET_RDMA_NETENGINETEMPLATE_H_*/
