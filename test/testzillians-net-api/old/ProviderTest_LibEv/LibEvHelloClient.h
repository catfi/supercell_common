#ifndef LIBEVHELLOCLIENT_H_
#define LIBEVHELLOCLIENT_H_

#include "LibEvHello.h"
#include "net-api/MessageHandler.h"
#include "net-api/queue/Queue.h"
#include "net-api/queue/QueueEventListener.h"
#include "net-api/queue/impl/ev/LibEvNetEngine.h"
#include <tbb/tick_count.h>
#include <boost/bind.hpp>
#include <tbb/tbb_thread.h>

#include <cstring>
#include <string>

using namespace zillians;

tbb::tick_count gStart, gEnd;
tbb::tbb_thread *gSenderThread;

#define MESSAGE_COUNT 1000000L
void clientSenderProc(HelloMessageFactory* factory, Channel *channel)
{
	gStart = tbb::tick_count::now();

	HelloMessage *hello = NULL;
	for(uint32 i=0;i<MESSAGE_COUNT;++i)
	{
		factory->create(HelloMessage::MESSAGE_TYPE, (Message**)&hello);
		hello->id = i + 1;
		sprintf(hello->data, "");
		channel->send(hello);
		factory->destroy(HelloMessage::MESSAGE_TYPE, (Message*)hello);
	}
}

class ClientHelloMessageHandler : public MessageHandler
{
public:
	ClientHelloMessageHandler() { mCount = 0L; lastId = 0L; }
public:
	virtual void onMessage(int32 type, Channel *channel, Message *message)
	{
		HelloMessage* hello = static_cast<HelloMessage*>(message);

		if(hello->id == MESSAGE_COUNT - 1)
		{
			gEnd = tbb::tick_count::now();
			double elapsed = (gEnd - gStart).seconds();
			printf("[client] sending %d message takes %lf ms, throughput = %lf messages/sec\n", MESSAGE_COUNT,  elapsed * 1000.0, (double)MESSAGE_COUNT / elapsed);
			channel->close();
		}
	}

private:
	uint32 lastId;
	long mCount;
};

class ClientChannelListener : public QueueEventListener
{
public:
	ClientChannelListener(HelloMessageFactory* factory) { mFactory = factory; }
	virtual ~ClientChannelListener() { }

public:
	virtual void onQueueConnected(Queue *q)
	{
		printf("[client] server connected!!\n");

		gSenderThread = new tbb::tbb_thread(boost::bind(clientSenderProc, mFactory, q));
	}

	virtual void onDisconnected(Channel *channel)
	{
		printf("[client] server disconnected!!\n");

		gSenderThread->join();
		SAFE_DELETE(gSenderThread);
	}

private:
	HelloMessageFactory* mFactory;
};



void clientThreadProc(std::string &remoteAddress)
{
	LibEvNetEngine engine;

	HelloMessageFactory factory;
	ClientHelloMessageHandler handler;
	engine.getMessageDispatcher()->registerType(HelloMessage::MESSAGE_TYPE, &factory, &handler);

	ClientChannelListener listener(&factory);
	engine.getChannelDispatcher()->registerListener(&listener);

	Address *remote = NULL; engine.getResolver()->resolve(remoteAddress, &remote);

	engine.connect(remote);

	engine.run();
}

#endif /*LIBEVHELLOCLIENT_H_*/
