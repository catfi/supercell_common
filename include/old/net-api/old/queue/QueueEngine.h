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

#ifndef QUEUEENGINE_H_
#define QUEUEENGINE_H_

#include "core-api/Types.h"
#include "net-api/Address.h"
#include "net-api/MessageDispatcher.h"
#include "net-api/queue/QueueEventDispatcher.h"
#include "net-api/Resolver.h"

namespace zillians {

class QueueEngine
{
public:
	virtual int32 run() = 0;
	virtual int32 terminate() = 0;

public:
	virtual int32 connect(const Address *remote) = 0;
	virtual int32 listen (const Address *local) = 0;

public:
	virtual MessageDispatcher*    getMessageDispatcher()    = 0;
	virtual QueueEventDispatcher* getQueueEventDispatcher() = 0;
	virtual Resolver*             getResolver()             = 0;
};

}

#endif /* QUEUEENGINE_H_ */
