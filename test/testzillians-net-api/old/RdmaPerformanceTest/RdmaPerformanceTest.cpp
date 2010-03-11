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
 * @date Feb 16, 2009 chitat - Initial version created.
 */

#include "RdmaPerformanceTest.h"
#include "net-api/sys/rdma/RdmaDeviceResourceManager.h"

#include "net-api/sys/buffer_manager/RdmaBufferManager.h"

#include "net-api/sys/Poller.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>

#include <tbb/tick_count.h>

//////////////////////////////////////////////////////////////////////////
RdmaPerformanceTestServer::RdmaPerformanceTestServer()
{

}

RdmaPerformanceTestServer::~RdmaPerformanceTestServer()
{

}

//////////////////////////////////////////////////////////////////////////
void RdmaPerformanceTestServer::onConnected(SharedPtr<RdmaConnection> connection)
{
	connection->setMaxSendInFlight(-1);
	mConnection = connection;
	mBuffer = mConnection->createBuffer(5);
	mBuffer->zero();
	mBuffer->wpos(mBuffer->freeSize());
	LOG4CXX_INFO(mLogger ,"CONNECTED");
}

void RdmaPerformanceTestServer::onDisconnected(SharedPtr<RdmaConnection> connection)
{
	LOG4CXX_INFO(mLogger ,"DISCONNECTED");
}

void RdmaPerformanceTestServer::onError(SharedPtr<RdmaConnection> connection, int code)
{
	LOG4CXX_ERROR(mLogger, "client error, connection ptr = " << connection.get() << ", error code = " << code);
}

//////////////////////////////////////////////////////////////////////////
void RdmaPerformanceTestServer::handle(uint32 type, SharedPtr<Buffer> b, SharedPtr<RdmaConnection> connection)
{

	switch(type)
	{
		case 0:
		{
				break;
		}
		case 3:
		{
			BufferInfo info;
			info.length = 8192*1024*10;

			SharedPtr<Buffer> rb = connection->createBuffer(info.length);
			info.id = connection->registrerDirect(rb);

			SharedPtr<Buffer> cb = connection->createBuffer(sizeof(info));
			cb->writeAny(info);
			connection->send(3, cb);

			break;
		}
		default:
		{
			connection->send(type,mBuffer);
			LOG4CXX_INFO(mLogger ,"resend type "<< type <<" message ");
			break;
		}
	}

}

void RdmaPerformanceTestServer::run()
{
	;
}

/////////////////////////////////////////////////////////////////////////
//RdmaPerformanceTestClient
////////////////////////////////////////////////////////////////////////
RdmaPerformanceTestClient::RdmaPerformanceTestClient()
{
}

RdmaPerformanceTestClient::~RdmaPerformanceTestClient()
{

}


//////////////////////////////////////////////////////////////////////////

void RdmaPerformanceTestClient::onConnected(SharedPtr<RdmaConnection> connection)
{
	connection->setMaxSendInFlight(-1);
	mConnection = connection;
	mLock = false;
	LOG4CXX_INFO(mLogger,"CONNECTED" );
	tbb::tbb_thread t(boost::bind(&RdmaPerformanceTestClient::run, this));
	LOG4CXX_INFO(mLogger,"Finish onConnected" );


}
void RdmaPerformanceTestClient::onDisconnected(SharedPtr<RdmaConnection> connection)
{
	LOG4CXX_INFO(mLogger,"DISCONNECTED" );
}

void RdmaPerformanceTestClient::onError(SharedPtr<RdmaConnection> connection, int code)
{
	LOG4CXX_ERROR(mLogger, "client error, connection ptr = " << connection.get() << ", error code = " << code);
}

//////////////////////////////////////////////////////////////////////////
void RdmaPerformanceTestClient::handle(uint32 type, SharedPtr<Buffer> b, SharedPtr<RdmaConnection> connection)
{
	switch(type)
	{
		case 1:
		{
			mEndTime = tbb::tick_count::now();

			LOG4CXX_INFO(mLogger,"\t Rdma Sent 16kb " << SENDITERATION << " times takes "<< (mEndTime - mSendstartTime).seconds()*1000.0 <<" ms" );
			LOG4CXX_INFO(mLogger,"\t Rdma Sent Bandwidth =" << (16.0*SENDITERATION)/((mEndTime - mSendstartTime).seconds()*1.0*1024)<<"MB/s"   );
			mLock = false;
			break;
		}
		case 2:
		{
			mEndTime = tbb::tick_count::now();
			LOG4CXX_INFO(mLogger, "\t Rdma BigSent "<< mSize/(1024*1024) <<" MB "<< BIGSENDITERATION <<" times takes "<< (mEndTime - mBigSendstartTime).seconds()*1000.0 <<" ms");
			LOG4CXX_INFO(mLogger,"\t Rdma BigSent Bandwidth = "<< (mSize*BIGSENDITERATION*1.0)/(((mEndTime - mBigSendstartTime).seconds()*1.0)*(1024*1024)) <<" MB/s" );
			mLock = false;
			break;
		}
		case 3:
		{
			BufferInfo info;
			b->readAny(info);
			SharedPtr<Buffer> rb = connection->createBuffer(info.length);
			rb->wpos(rb->freeSize());
			mSize = info.length;
			mDirectSendstartTime = tbb::tick_count::now();
			connection->sendDirect(4, rb, info.id);
			break;
		}
		case 4:
		{
			mEndTime = tbb::tick_count::now();
			LOG4CXX_INFO(mLogger ,"\t Rdma DirectSent "<<mSize/(1024*1024) <<" MB takes "<< (mEndTime - mDirectSendstartTime).seconds()*1000.0 <<" ms " );
			LOG4CXX_INFO(mLogger ,"\t Rdma DirectSent Bandwidth = "<<((mSize*1.0)/ ((mEndTime - mDirectSendstartTime).seconds()*1.0*1024*1024)) <<" MB " );
			mLock = false;
			break;
		}
	}

}

void RdmaPerformanceTestClient::run()
{



	LOG4CXX_INFO(mLogger ,"SendTest Start:");
	doSendTest();
	while(mLock)
		tbb::this_tbb_thread::yield();


	LOG4CXX_INFO(mLogger ,"BigSendTest Start:");
	doBigSendTest();
	while(mLock)
		tbb::this_tbb_thread::yield();


	LOG4CXX_INFO(mLogger ,"DirectSendTest Start:");
	doDirectSendTest();
	while(mLock)
		tbb::this_tbb_thread::yield();

	mConnection->close();

}
void RdmaPerformanceTestClient::doSendTest()
{

	mLock=true;
	mSendstartTime = tbb::tick_count::now();
	for(int i=0;i<SENDITERATION-1 ;i++)
	{
		mBuffer = mConnection->createBuffer(16*1024);
		mBuffer->wpos(mBuffer->freeSize());
		mConnection->send(0,mBuffer);
	}
	mBuffer = mConnection->createBuffer(16*1024);
	mBuffer->wpos(mBuffer->freeSize());

	mConnection->send(1,mBuffer);



}
void RdmaPerformanceTestClient::doBigSendTest()
{
	mLock = true;
	mBigSendstartTime = tbb::tick_count::now();
	for(int i=0;i<BIGSENDITERATION-1 ;i++)
	{
		mBuffer = mConnection->createBuffer(mSize);
		mBuffer->wpos(mBuffer->freeSize());

		mConnection->send(0,mBuffer);
	}
	mBuffer = mConnection->createBuffer(mSize);
	mBuffer->wpos(mBuffer->freeSize());

	mConnection->send(2,mBuffer);


}
void RdmaPerformanceTestClient::doDirectSendTest()
{
	mLock = true;
	mDirectSendstartTime = tbb::tick_count::now();

	mBuffer = mConnection->createBuffer(1);
	mBuffer->wpos(mBuffer->freeSize());

	mConnection->send(3,mBuffer);

}


//////////////////////////////////////////////////////////////////////////
//Main
//////////////////////////////////////////////////////////////////////////
#define MB 1024L*1024L

struct SingletonPool
{
	SharedPtr<RdmaDeviceResourceManager> device_resource_manager;
	SharedPtr<RdmaBufferManager> buffer_manager;
} gSingletonPool;

void initSingleton()
{
	gSingletonPool.device_resource_manager = SharedPtr<RdmaDeviceResourceManager>(new RdmaDeviceResourceManager());
	gSingletonPool.buffer_manager = SharedPtr<RdmaBufferManager>(new RdmaBufferManager(800*MB + RDMA_MINIMAL_MEMORY_USAGE + 10*MB));
}

void finiSingleton()
{
	gSingletonPool.device_resource_manager.reset();
	gSingletonPool.buffer_manager.reset();
}

void printUsage()
{
	fprintf(stderr, "%s <server|client> <listen_address|connecting_address> <MessageSize(MB)(MAX 500)>\n", "RdmaPerformanceTest");
}

void ConnectorHandler(SharedPtr<RdmaConnection> connection, int err)
{
	printf("ConnectorHandler: err = %d\n", err);
}

void AcceptorHandler(SharedPtr<RdmaConnection> connection, int err)
{
	printf("AcceptorHandler: err = %d\n", err);
}

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		printUsage();
		return -1;
	}

	// configure the log4cxx to default
	log4cxx::BasicConfigurator::configure();

	std::string address(argv[2]);

	// initialize singleton classes
	initSingleton();


	SharedPtr<RdmaDispatcher> d(new RdmaDispatcher());
	SharedPtr<Poller> p(new Poller(ev_loop_new(0)));

	SharedPtr<RdmaNetEngine> engine(new RdmaNetEngine());

	engine->setDispatcher(d);
	engine->setBufferManager(gSingletonPool.buffer_manager);


	if(strcmp(argv[1], "server") == 0)
	{
		SharedPtr< RdmaPerformanceTestServer > Server( new RdmaPerformanceTestServer() );



		d->registerDefaultDataHandler(Server);
		d->registerConnectionHandler(Server);

		Server->mSize = atoi(argv[3])*1024*1024 ;

		engine->accept(p, address, boost::bind(AcceptorHandler, _1, _2));

		p->run();



	}
	else if(strcmp(argv[1], "client")==0)
	{

		SharedPtr<RdmaPerformanceTestClient> Client(new RdmaPerformanceTestClient());


		d->registerDefaultDataHandler(Client);
		d->registerConnectionHandler(Client);

		Client->mSize = atoi(argv[3])*1024*1024;

		engine->connect(p, address, boost::bind(ConnectorHandler, _1, _2));



		p->run();

	}
	else
	{
		printUsage();
		return -1;
	}




}
