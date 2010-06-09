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


#include "RdmaChatRoomTestClient.h"
#include "RdmaChatRoomTestServer.h"
#include "RdmaChatRoomTestCommon.h"

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
	gSingletonPool.buffer_manager = shared_ptr<RdmaBufferManager>(new RdmaBufferManager(POOL_SIZE));
}

void finiSingleton()
{
	gSingletonPool.device_resource_manager.reset();
	gSingletonPool.buffer_manager.reset();
}
void printUsage()
{
	fprintf(stderr, "%s <server|client> <listen_address|connecting_address>\n", "RdmaChatRoomTest");
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
		shared_ptr<RdmaDispatcher> dispatcher(new RdmaDispatcher());
		shared_ptr<Poller> poller(new Poller(ev_loop_new(0)));
		shared_ptr<RdmaNetEngine> netEngine(new RdmaNetEngine());

		ev::io stdinWatcher;
		stdinWatcher.set(fileno(stdin), ev::READ);

		if(strcmp(argv[1], "server") == 0)
		{
			shared_ptr<RdmaChatRoomTestServer> server(new RdmaChatRoomTestServer());

			dispatcher->registerDefaultDataHandler(server);
			dispatcher->registerConnectionHandler(server);

			stdinWatcher.set<RdmaChatRoomTestServer, &RdmaChatRoomTestServer::handleStdin>(server.get());

			netEngine->setDispatcher(dispatcher);
			netEngine->setBufferManager(gSingletonPool.buffer_manager);
			netEngine->accept(poller, address, boost::bind(AcceptorHandler, _1, _2));

			poller->start(&stdinWatcher);
			cout<<"run pollor..."<<endl;
			poller->run();
		}

		else if(strcmp(argv[1], "client") == 0)
		{
			shared_ptr<RdmaChatRoomTestClient> client(new RdmaChatRoomTestClient());

			dispatcher->registerDefaultDataHandler(client);
			dispatcher->registerConnectionHandler(client);

			stdinWatcher.set<RdmaChatRoomTestClient, &RdmaChatRoomTestClient::handleStdin>(client.get());

			netEngine->setDispatcher(dispatcher);
			netEngine->setBufferManager(gSingletonPool.buffer_manager);
			netEngine->connect(poller, address, boost::bind(ConnectorHandler, _1, _2));

			poller->start(&stdinWatcher);
			cout<<"run pollor..."<<endl;
			poller->run();
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


