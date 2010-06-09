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

#include "RdmaNetEngineTestServer.h"
#include "RdmaNetEngineTestCommon.h"
#include "networking/sys/Poller.h"

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr RdmaNetEngineTestServer::mLogger(log4cxx::Logger::getLogger("RdmaNetEngineTestServer"));

//////////////////////////////////////////////////////////////////////////
RdmaNetEngineTestServer::RdmaNetEngineTestServer()
{
}

RdmaNetEngineTestServer::~RdmaNetEngineTestServer()
{

}

//////////////////////////////////////////////////////////////////////////
void RdmaNetEngineTestServer::run(std::string &address)
{
}

//////////////////////////////////////////////////////////////////////////
void RdmaNetEngineTestServer::onConnected(shared_ptr<RdmaConnection> connection)
{
	LOG4CXX_DEBUG(mLogger, "CONNECTED");

	BufferInfo info;
	info.length = 8192*1024;

	shared_ptr<Buffer> rb = connection->createBuffer(info.length);
	info.id = connection->registrerDirect(rb);

	shared_ptr<Buffer> b = connection->createBuffer(sizeof(info));
	b->writeAny(info);
	connection->send(5, b);
}

void RdmaNetEngineTestServer::onDisconnected(shared_ptr<RdmaConnection> connection)
{
	LOG4CXX_DEBUG(mLogger, "DISCONNECTED");
}

void RdmaNetEngineTestServer::onError(shared_ptr<RdmaConnection> connection, int code)
{
	printf("ERROR\n");
}

//////////////////////////////////////////////////////////////////////////
void RdmaNetEngineTestServer::handle(uint32 type, shared_ptr<Buffer> b, shared_ptr<RdmaConnection> connection)
{
	LOG4CXX_DEBUG(mLogger, "type = " << type << ", length = " << b->dataSize());

	if(type == 2)
	{
		size_t size = b->dataSize();
		for(int i=0;i<size/sizeof(int);++i)
		{
			int x; b->read(x);
			if(x != i)
			{
				LOG4CXX_ERROR(mLogger, "DATA_ERROR AT #" << i << " = " << x);
			}
		}
		LOG4CXX_DEBUG(mLogger, "Orz");
	}
}
