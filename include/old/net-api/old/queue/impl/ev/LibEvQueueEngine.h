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

#ifndef LIBEVQUEUEENGINE_H_
#define LIBEVQUEUEENGINE_H_

#include "networking/Address.h"
#include "networking/queue/QueueEngine.h"
#include "networking/queue/impl/ev/LibEvQueue.h"
#include "networking/queue/impl/ev/LibEvAcceptor.h"
#include "networking/queue/impl/ev/LibEvConnector.h"
#include "networking/address/InetSocketAddress.h"
#include "networking/queue/impl/ev/LibEvDaemon.h"
#include "networking/queue/impl/ev/LibEvResolver.h"
#include "networking/queue/impl/ev/dispatcher/LibEvMessageDispatcher.h"
#include "networking/queue/impl/ev/dispatcher/LibEvQueueEventDispatcher.h"
#include <ext/hash_map>

namespace zillians {

typedef __gnu_cxx::hash_map<handle_t, LibEvQueue*> LibEvQueueMap;

class LibEvQueueEngine : public QueueEngine
{
public:
	LibEvQueueEngine();
	virtual ~LibEvQueueEngine();

public:
	virtual int32 run();
	virtual int32 terminate();

public:
	virtual int32 connect(const Address *remote);
	virtual int32 listen(const Address *local);

public:
	LibEvDaemon*     getDaemon();
	LibEvAcceptor*   getAcceptor();
	LibEvConnector*  getConnector();

	virtual LibEvMessageDispatcher*    getMessageDispatcher();
	virtual LibEvQueueEventDispatcher* getQueueEventDispatcher();
	virtual LibEvResolver*             getResolver();

public:
	void createQueue(const handle_t &handle, const InetSocketAddress &address, LibEvQueue** q);
	void destroyQueue(LibEvQueue* q);

private:
	LibEvDaemon*     mDaemon;
	LibEvAcceptor*   mAcceptor;
	LibEvConnector*  mConnector;
	LibEvResolver*   mResolver;

	LibEvMessageDispatcher*    mMessageDispatcher;
	LibEvQueueEventDispatcher* mQueueEventDispatcher;

protected:
	LibEvQueueMap  mQueueMap;
};

}

#endif /* LIBEVQUEUEENGINE_H_ */
