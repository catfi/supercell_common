/*
 * RdmaChatRoomTestServer.h
 *
 *  Created on: Feb 22, 2009
 *      Author: bbbb
 */


#include "RdmaChatRoomTestCommon.h"

using namespace zillians;
using namespace zillians::net;

typedef map<string, shared_ptr<RdmaConnection> >::iterator myIterator;

class RdmaChatRoomTestServer : public RdmaDataHandler, public RdmaConnectionHandler
{
public:
	RdmaChatRoomTestServer();
	virtual ~RdmaChatRoomTestServer();

public:
	virtual void onConnected(shared_ptr<RdmaConnection> connection);
	virtual void onDisconnected(shared_ptr<RdmaConnection> connection);
	virtual void onError(shared_ptr<RdmaConnection> connection, int code);

public:
	void updateClientList();

public:
	void handleStdin(ev::io &w, int revents);
	virtual void handle(uint32 type, shared_ptr<Buffer> b, shared_ptr<RdmaConnection> connection);

public:
	map<string, shared_ptr<RdmaConnection> > mConnectionContainer;
	std::string mClientList;
};
