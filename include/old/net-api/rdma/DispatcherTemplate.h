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
/**
 * @date Feb 16, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_RDMA_DISPATCHERTEMPLATE_H_
#define ZILLIANS_NET_RDMA_DISPATCHERTEMPLATE_H_

#include "core-api/Prerequisite.h"
#include "net-api/rdma/DataHandlerTemplate.h"
#include "net-api/rdma/ConnectionHandlerTemplate.h"
#include "net-api/rdma/CompletionHandlerTemplate.h"
#include "tbb/spin_rw_mutex.h"

using namespace zillians;

namespace zillians { namespace net { namespace rdma {

template <typename Connection, typename DataHandler = DataHandlerTemplate<Connection>, typename CompletionHandler = CompletionHandlerTemplate<Connection>, typename ConnectionHandler = ConnectionHandlerTemplate<Connection>  >
class DispatcherTemplate
{
public:
	enum EventType
	{
		DATA_READ,
		CONNECTED,
		DISCONNECTED,
	};

public:
	DispatcherTemplate()
	{ }

	virtual ~DispatcherTemplate()
	{ }

public:
	virtual void registerDefaultDataHandler(shared_ptr<DataHandler> dataHandler)
	{
		tbb::spin_rw_mutex::scoped_lock(mDataHandler.lock, true);
		mDataHandler.def = dataHandler;
	}

	virtual void unregisterDefaultDataHandler()
	{
		tbb::spin_rw_mutex::scoped_lock(mDataHandler.lock, true);
		mDataHandler.def.reset();
	}

	virtual void registerDataHandler(int32 type, shared_ptr<DataHandler> dataHandler)
	{
		tbb::spin_rw_mutex::scoped_lock(mDataHandler.lock, true);
		mDataHandler.map[type] = dataHandler;
	}

	virtual void unregisterDataHandler(int32 type)
	{
		tbb::spin_rw_mutex::scoped_lock(mDataHandler.lock, true);
		typename tDataHandlerMap::iterator it = mDataHandler.map.find(type);
		if(it == mDataHandler.map.end())
		{
			LOG4CXX_ERROR(mLogger, "Unable to un-register data handler for type " << type);
		}
		else
		{
			mDataHandler.map.erase(it);
		}
	}

public:
	virtual void registerConnectionHandler(shared_ptr<ConnectionHandler> connectionHandler)
	{
		tbb::spin_rw_mutex::scoped_lock(mConnectionHandler.lock, true);
		mConnectionHandler.map[connectionHandler.get()] = connectionHandler;
	}

	virtual void unregisterConnectionHandler(shared_ptr<ConnectionHandler> connectionHandler)
	{
		tbb::spin_rw_mutex::scoped_lock(mConnectionHandler.lock, true);
		typename tConnectionHandlerMap::iterator it = mConnectionHandler.map.find(connectionHandler.get());
		if(UNLIKELY(it == mConnectionHandler.map.end()))
		{
			LOG4CXX_ERROR(mLogger, "Unable to un-register completion handler");
		}
	}

public:
	virtual void registerCompletionHandler(shared_ptr<CompletionHandler> completionHandler)
	{
		tbb::spin_rw_mutex::scoped_lock(mCompletionHandler.lock, true);
		mCompletionHandler.map[completionHandler.get()] = completionHandler;
	}

	virtual void unregisterCompletionHandler(shared_ptr<CompletionHandler> completionHandler)
	{
		tbb::spin_rw_mutex::scoped_lock(mCompletionHandler.lock, true);
		typename tCompletionHandlerMap::iterator it = mCompletionHandler.map.find(completionHandler.get());
		if(UNLIKELY(it == mCompletionHandler.map.end()))
		{
			LOG4CXX_ERROR(mLogger, "Unable to un-register completion handler");
		}
	}

public:
	void dispatchDataEvent(uint32 type, shared_ptr<Buffer> b, shared_ptr<Connection> connection)
	{
		tbb::spin_rw_mutex::scoped_lock(mDataHandler.lock, false);
		typename tDataHandlerMap::iterator it = mDataHandler.map.find(type);
		if(UNLIKELY(it == mDataHandler.map.end()))
		{
			mDataHandler.def->handle(type, b, connection);
		}
		else
		{
			it->second->handle(type, b, connection);
		}
	}

	void dispatchCompletion(shared_ptr<Buffer> b, shared_ptr<Connection> connection)
	{
		tbb::spin_rw_mutex::scoped_lock(mCompletionHandler.lock, false);
		for(typename tCompletionHandlerMap::iterator it = mCompletionHandler.map.begin(); it != mCompletionHandler.map.end(); ++it)
		{
			it->second->onCompleted(b, connection);
		}
	}

	void dispatchConnectionEvent(int type, shared_ptr<Connection> connection)
	{
		tbb::spin_rw_mutex::scoped_lock(mConnectionHandler.lock, false);
		switch(type)
		{
		case CONNECTED:
			for(typename tConnectionHandlerMap::iterator it = mConnectionHandler.map.begin(); it != mConnectionHandler.map.end(); ++it)
			{
				it->second->onConnected(connection);
			}
			break;
		case DISCONNECTED:
			for(typename tConnectionHandlerMap::iterator it = mConnectionHandler.map.begin(); it != mConnectionHandler.map.end(); ++it)
			{
				it->second->onDisconnected(connection);
			}
			break;
		}
	}

private:
	static log4cxx::LoggerPtr mLogger;

private:
	typedef std::map<int32, shared_ptr<DataHandler> > tDataHandlerMap;
	typedef std::map<ConnectionHandler*, shared_ptr<ConnectionHandler> > tConnectionHandlerMap;
	typedef std::map<CompletionHandler*, shared_ptr<CompletionHandler> > tCompletionHandlerMap;

	struct
	{
		shared_ptr<DataHandler> def;
		tDataHandlerMap map;
		tbb::spin_rw_mutex lock;
	} mDataHandler;

	struct
	{
		tConnectionHandlerMap map;
		tbb::spin_rw_mutex lock;
	} mConnectionHandler;

	struct
	{
		tCompletionHandlerMap map;
		tbb::spin_rw_mutex lock;
	} mCompletionHandler;
};

//////////////////////////////////////////////////////////////////////////
template<class Connection, class DataHandler, class CompletionHandler, class ConnectionHandler>
log4cxx::LoggerPtr DispatcherTemplate<Connection,DataHandler,CompletionHandler,ConnectionHandler>::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.sys.rdma.Dispatcher"));

} } }

#endif/*ZILLIANS_NET_RDMA_DISPATCHERTEMPLATE_H_*/
