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

#include "LibEvQueueEngine.h"

#include "net-api/queue/impl/ev/LibEvNetEngine.h"
#include "net-api/address/InetAddress.h"
#include "net-api/address/InetSocketAddress.h"

namespace zillians {

//////////////////////////////////////////////////////////////////////////
LibEvQueueEngine::LibEvQueueEngine()
{
	mDaemon    = new LibEvDaemon();
	mAcceptor  = new LibEvAcceptor(this);
	mConnector = new LibEvConnector(this);

	mMessageDispatcher    = new LibEvMessageDispatcher();
	mQueueEventDispatcher = new LibEvQueueEventDispatcher();
	mResolver             = new LibEvResolver();
}

LibEvQueueEngine::~LibEvQueueEngine()
{
	SAFE_DELETE(mResolver);
	SAFE_DELETE(mMessageDispatcher);
	SAFE_DELETE(mQueueEventDispatcher);

	SAFE_DELETE(mAcceptor);
	SAFE_DELETE(mConnector);
	SAFE_DELETE(mDaemon);
}

//////////////////////////////////////////////////////////////////////////
int32 LibEvQueueEngine::run()
{ return mDaemon->run(); }

int32 LibEvQueueEngine::terminate()
{return mDaemon->terminate(); }

//////////////////////////////////////////////////////////////////////////
int32 LibEvQueueEngine::connect(const Address *remote)
{
	const InetSocketAddress *address = static_cast<const InetSocketAddress*>(remote);
	return mConnector->connect(address);
}

int32 LibEvQueueEngine::listen(const Address *local)
{
	const InetSocketAddress *address = static_cast<const InetSocketAddress*>(local);
	return mAcceptor->listen(address);
}

//////////////////////////////////////////////////////////////////////////
LibEvDaemon* LibEvQueueEngine::getDaemon()
{ return mDaemon; }

LibEvAcceptor* LibEvQueueEngine::getAcceptor()
{ return mAcceptor; }

LibEvConnector* LibEvQueueEngine::getConnector()
{ return mConnector; }

LibEvResolver* LibEvQueueEngine::getResolver()
{ return mResolver; }

LibEvMessageDispatcher* LibEvQueueEngine::getMessageDispatcher()
{ return mMessageDispatcher; }

LibEvQueueEventDispatcher* LibEvQueueEngine::getQueueEventDispatcher()
{ return mQueueEventDispatcher; }

//////////////////////////////////////////////////////////////////////////
void LibEvQueueEngine::createQueue(const handle_t &handle, const InetSocketAddress &address, LibEvQueue** q)
{
	*q = new LibEvQueue(handle, address, this);
	mQueueMap[handle] = *q;
}

void LibEvQueueEngine::destroyQueue(LibEvQueue* q)
{
	LibEvQueueMap::iterator it = mQueueMap.find(q->getHandle());
	if(it != mQueueMap.end())
	{
		mQueueMap.erase(it);
		SAFE_DELETE(q);
	}
}

}
