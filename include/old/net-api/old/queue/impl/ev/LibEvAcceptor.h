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

#ifndef LIBEVACCEPTOR_H_
#define LIBEVACCEPTOR_H_

#include "core/Types.h"
#include "networking/address/InetSocketAddress.h"
#include "networking/queue/impl/ev/LibEvWrapper.h"

namespace zillians {

// forward declaration
class LibEvQueueEngine;

class LibEvAcceptor
{
	friend class LibEvDaemon;

public:
	LibEvAcceptor(LibEvQueueEngine *ref);
	virtual ~LibEvAcceptor();

public:
	int32 listen(const InetSocketAddress* local);
	void close();

public:
	void handleConnectionAccepted(ev::io &w, int revent);

private:
	static log4cxx::LoggerPtr mLogger;

private:
	LibEvQueueEngine *mEngineRef;
	handle_t mHandle;
	InetSocketAddress mAddress;
	ev::io mAcceptWatcher;
};

}

#endif /*LIBEVACCEPTOR_H_*/
