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

#include "net-api/dispatcher/BasicMessageDispatcher.h"

namespace zillians {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr BasicMessageDispatcher::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.provider.shared.BasicMessageDispatcher"));

//////////////////////////////////////////////////////////////////////////
BasicMessageDispatcher::BasicMessageDispatcher()
{
	mDefaultDispatch.factory = NULL;
	mDefaultDispatch.handler = NULL;
}

BasicMessageDispatcher::~BasicMessageDispatcher()
{
}

//////////////////////////////////////////////////////////////////////////
void BasicMessageDispatcher::registerDefault(MessageFactory* factory, MessageHandler* handler)
{
	mDefaultDispatch.factory = factory;
	mDefaultDispatch.handler = handler;
}

void BasicMessageDispatcher::unregisterDefault()
{

}

void BasicMessageDispatcher::registerType(int32 type, MessageFactory* factory, MessageHandler* handler)
{
	mDispatchMap[type] = DispatchInfo(factory, handler);
}

void BasicMessageDispatcher::unregisterType(int type)
{
	DispatchMap::iterator it = mDispatchMap.find(type);
	if(it != mDispatchMap.end())
	{
		mDispatchMap.erase(it);
	}
}

//////////////////////////////////////////////////////////////////////////
void BasicMessageDispatcher::dispatch(int32 type, size_t length, Channel* channel, ByteBuffer* bb)
{
	MessageFactory *factory = NULL;
	MessageHandler *handler = NULL;

	DispatchMap::iterator it = mDispatchMap.find(type);
	if(it != mDispatchMap.end())
	{
		DispatchInfo dispatch = it->second;
		factory = dispatch.factory;
		handler = dispatch.handler;
	}
	else
	{
		factory = mDefaultDispatch.factory;
		handler = mDefaultDispatch.handler;
	}

	if(UNLIKELY(!factory || !handler))
	{
		LOG4CXX_DEBUG(mLogger, "Fail to dispatch message, invalid factory or handler");
		return;
	}

	Message *message = NULL;
	factory->create(type, &message);
	{
		if(!message)
		{
			LOG4CXX_DEBUG(mLogger, "Fail to create message object, message type = " << type);
			return;
		}

		if(UNLIKELY(message->decode(*bb, length) != ZN_OK))
		{
			LOG4CXX_DEBUG(mLogger, "Fail to decode message object, message type = " << type);
			return;
		}
		handler->onMessage(type, channel, message);
	}
	factory->destroy(type, message);
}

}
