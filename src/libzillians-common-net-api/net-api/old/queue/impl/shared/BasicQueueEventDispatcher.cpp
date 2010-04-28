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

#include "net-api/queue/impl/shared/BasicQueueEventDispatcher.h"

namespace zillians {

//////////////////////////////////////////////////////////////////////////
BasicQueueEventDispatcher::BasicQueueEventDispatcher()
{
}

BasicQueueEventDispatcher::~BasicQueueEventDispatcher()
{
}

//////////////////////////////////////////////////////////////////////////
void BasicQueueEventDispatcher::registerListener(QueueEventListener* listener)
{
	mDispatchList.insert(listener);
}

void BasicQueueEventDispatcher::unregisterListener(QueueEventListener* listener)
{
	DispatchList::iterator it = mDispatchList.find(listener);
	if(it != mDispatchList.end())
	{
		mDispatchList.erase(it);
	}
}

//////////////////////////////////////////////////////////////////////////
void BasicQueueEventDispatcher::dispatchQueueConnected(Queue* q)
{
	for(DispatchList::iterator it = mDispatchList.begin(); it != mDispatchList.end(); it++)
	{
		(*it)->onQueueConnected(q);
	}
}

void BasicQueueEventDispatcher::dispatchQueueDisconnected(Queue* q)
{
	for(DispatchList::iterator it = mDispatchList.begin(); it != mDispatchList.end(); it++)
	{
		(*it)->onQueueDisconnected(q);
	}
}

void BasicQueueEventDispatcher::dispatchQueueError(Queue* q)
{
	for(DispatchList::iterator it = mDispatchList.begin(); it != mDispatchList.end(); it++)
	{
		(*it)->onQueueDisconnected(q);
	}
}

}
