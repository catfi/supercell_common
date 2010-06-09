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
 * @date Feb 14, 2009 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "networking/sys/rdma/RdmaDeviceResourceManager.h"
#include "networking/sys/rdma/RdmaNetEngine.h"
#include "networking/sys/Poller.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <tbb/tbb_thread.h>

#include "networking/sys/buffer_manager/RdmaBufferManager.h"

#include "RdmaNetEngineTestClient.h"
#include "RdmaNetEngineTestServer.h"
#include "RdmaNetEngineTestCommon.h"

#define MB 1024L*1024L

using namespace zillians;
using namespace zillians::networking;

struct SingletonPool
{
	shared_ptr<RdmaDeviceResourceManager> device_resource_manager;
	shared_ptr<RdmaBufferManager> buffer_manager;
} gSingletonPool;

void initSingleton()
{
	gSingletonPool.device_resource_manager = shared_ptr<RdmaDeviceResourceManager>(new RdmaDeviceResourceManager());
	gSingletonPool.buffer_manager = shared_ptr<RdmaBufferManager>(new RdmaBufferManager(256*MB));
}

void finiSingleton()
{
	gSingletonPool.device_resource_manager.reset();
	gSingletonPool.buffer_manager.reset();
}

void printUsage()
{
	fprintf(stderr, "%s <server|client> <listen_address|connecting_address>\n", "RdmaNetEngineTest");
}

void ConnectorHandler(shared_ptr<RdmaConnection> connection, int err)
{
	printf("ConnectorHandler: err = %d\n", err);
}

void AcceptorHandler(shared_ptr<RdmaConnection> connection, int err)
{
	printf("AcceptorHandler: err = %d\n", err);
}

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		printUsage();
		return -1;
	}

	// configure the log4cxx to default
	log4cxx::BasicConfigurator::configure();

	std::string address(argv[2]);

	// initialize singleton classes
	initSingleton();

	// start and run the network engine
	{
		shared_ptr<RdmaDispatcher> d (new RdmaDispatcher());
		shared_ptr<Poller> p(new Poller(ev_loop_new(0)));

		shared_ptr<RdmaNetEngine> engine(new RdmaNetEngine());

		engine->setDispatcher(d);
		engine->setBufferManager(gSingletonPool.buffer_manager);

		if(strcmp(argv[1], "server") == 0)
		{
			shared_ptr<RdmaNetEngineTestServer> server(new RdmaNetEngineTestServer());

			d->registerDefaultDataHandler(server);
			d->registerConnectionHandler(server);

			engine->accept(p, address, boost::bind(AcceptorHandler, _1, _2));

			p->run();
		}
		else if(strcmp(argv[1], "client") == 0)
		{
			shared_ptr<RdmaNetEngineTestClient> client(new RdmaNetEngineTestClient());

			d->registerDefaultDataHandler(client);
			d->registerConnectionHandler(client);

			engine->connect(p, address, boost::bind(ConnectorHandler, _1, _2));

			p->run();
		}
		else
		{
			printUsage();
			return -1;
		}


	}

	// finalize singleton classes
	finiSingleton();

	return 0;
}
