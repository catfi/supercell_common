/*
 * RdmaChatRoomTestServer.h
 *
 *  Created on: Feb 22, 2009
 *      Author: bbbb
 */


#include "RdmaChatRoomTestCommon.h"

using namespace zillians;
using namespace zillians::net;

typedef map<string, SharedPtr<RdmaConnection> >::iterator myIterator;

class RdmaChatRoomTestServer : public RdmaDataHandler, public RdmaConnectionHandler
{
public:
	RdmaChatRoomTestServer();
	virtual ~RdmaChatRoomTestServer();

public:
	virtual void onConnected(SharedPtr<RdmaConnection> connection);
	virtual void onDisconnected(SharedPtr<RdmaConnection> connection);
	virtual void onError(SharedPtr<RdmaConnection> connection, int code);

public:
	void updateClientList();

public:
	void handleStdin(ev::io &w, int revents);
	virtual void handle(uint32 type, SharedPtr<Buffer> b, SharedPtr<RdmaConnection> connection);

public:
	map<string, SharedPtr<RdmaConnection> > mConnectionContainer;
	std::string mClientList;
};
