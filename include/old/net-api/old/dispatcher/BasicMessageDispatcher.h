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

#ifndef BASICMESSAGEDISPATCHER_H_
#define BASICMESSAGEDISPATCHER_H_

#include "networking/Message.h"
#include "networking/MessageDispatcher.h"
#include "networking/MessageFactory.h"
#include "networking/MessageHandler.h"
#include "networking/Channel.h"
#include <ext/hash_map>

namespace zillians {

class BasicMessageDispatcher : public MessageDispatcher
{
public:
	BasicMessageDispatcher();
	virtual ~BasicMessageDispatcher();

public:
	virtual void registerDefault(MessageFactory* factory, MessageHandler* handler);
	virtual void unregisterDefault();

	virtual void registerType(int32 type, MessageFactory* factory, MessageHandler* handler);
	virtual void unregisterType(int32 type);

public:
	void dispatch(int32 type, std::size_t length, Channel* channel, ByteBuffer* bb);

private:
	static log4cxx::LoggerPtr mLogger;

private:
	class DispatchInfo
	{
	public:
		DispatchInfo(MessageFactory* _factory = NULL, MessageHandler* _handler = NULL) : factory(_factory), handler(_handler)  { }
		DispatchInfo(const DispatchInfo &info) : factory(info.factory), handler(info.handler) { }
	public:
		MessageFactory* factory;
		MessageHandler* handler;
	};

	typedef __gnu_cxx::hash_map<int32, DispatchInfo> DispatchMap;

	DispatchInfo mDefaultDispatch;
	DispatchMap  mDispatchMap;
};

}

#endif /*BASICMESSAGEDISPATCHER_H_*/
