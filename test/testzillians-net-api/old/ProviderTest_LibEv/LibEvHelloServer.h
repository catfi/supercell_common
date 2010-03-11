#ifndef LIBEVHELLOSERVER_H_
#define LIBEVHELLOSERVER_H_

#include "LibEvHello.h"
#include "net-api/MessageHandler.h"
#include "net-api/Channel.h"
#include "net-api/ChannelListener.h"
#include "net-api/provider/ev/LibEvNetEngine.h"

#include <cstring>
#include <string>

using namespace zillians;

class ServerHelloMessageHandler : public MessageHandler
{
public:
	virtual void onMessage(int32 type, Channel *channel, Message *message) 
	{
		channel->send(message);
	}
};

class ServerChannelListener : public ChannelListener
{
public:
	ServerChannelListener() { }
	virtual ~ServerChannelListener() { }
	
public:
	virtual void onConnected(Channel *channel)
	{
		printf("[server] client connected!!\n");
	}
	
	virtual void onDisconnected(Channel *channel)
	{
		printf("[server] client disconnected!!\n");
	}
};

void serverThreadProc(std::string& localAddress)
{
	LibEvNetEngine engine;
	
	HelloMessageFactory factory;
	ServerHelloMessageHandler handler;
	engine.getMessageDispatcher()->registerType(HelloMessage::MESSAGE_TYPE, &factory, &handler);
	
	ServerChannelListener listener;
	engine.getChannelDispatcher()->registerListener(&listener);
	
	Address *local = NULL;  engine.getResolver()->resolve(localAddress, &local);
	
	engine.listen(local);
	
	engine.run();
	
}

#endif /*LIBEVHELLOSERVER_H_*/
