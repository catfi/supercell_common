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

#ifndef RDMANETENGINETESTCLIENT_H_
#define RDMANETENGINETESTCLIENT_H_

#include "core-api/Prerequisite.h"
#include "net-api/sys/rdma/RdmaNetEngine.h"
#include "net-api/sys/rdma/RdmaDataHandler.h"
#include "net-api/sys/rdma/RdmaConnectionHandler.h"

using namespace zillians;
using namespace zillians::net;

class RdmaNetEngineTestClient : public RdmaDataHandler, public RdmaConnectionHandler
{
public:
	RdmaNetEngineTestClient();
	virtual ~RdmaNetEngineTestClient();

public:
	void run(std::string &address);

public:
	virtual void onConnected(SharedPtr<RdmaConnection> connection);
	virtual void onDisconnected(SharedPtr<RdmaConnection> connection);
	virtual void onError(SharedPtr<RdmaConnection> connection, int code);

public:
	virtual void handle(uint32 type, SharedPtr<Buffer> b, SharedPtr<RdmaConnection> connection);

private:
	static log4cxx::LoggerPtr mLogger;

};

#endif/*RDMANETENGINETESTCLIENT_H_*/
