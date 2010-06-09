//
// Zillians MMO
// Copyright (C) 2007-2008 Zillians.com, Inc.
// For more information see http://www.zillians.com
//
// Zillians MMO is the library and runtime for massive multiplayer online game
// development in utility computing model, which runs as a service for every
// developer to build their virtual world running on our GPU-assisted machines
//
// This is a close source library intended to be used solely within Zillians.com
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Contact Information: info@zillians.com
//

#ifndef LIBEVCONNECTOR_H_
#define LIBEVCONNECTOR_H_

#include "core/Types.h"
#include "networking/address/InetSocketAddress.h"
#include "networking/queue/impl/ev/LibEvWrapper.h"
#include <ext/hash_set>

namespace zillians {

// forward declaration
class LibEvQueueEngine;

struct LibEvAsyncConn
{
	handle_t handle;
	ev::io watcherConnected;
	ev::timer watcherTimeout;
};

class LibEvConnector
{
	friend class LibEvDaemon;
	friend class LibEvAsyncConn;

public:
	LibEvConnector(LibEvQueueEngine *ref);
	virtual ~LibEvConnector();

public:
	int32 connect(const InetSocketAddress *remote);
	void close();

/*
public:
	void handleConnected(ev::io &w, int revent);
	void handleTimeout(ev::timer &w, int revent);
*/

private:
	static log4cxx::LoggerPtr mLogger;

private:
	LibEvQueueEngine* mEngineRef;

public:
	__gnu_cxx::hash_set<LibEvAsyncConn*> mRequests;
};

}

#endif /*LIBEVCONNECTOR_H_*/
