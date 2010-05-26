/*
 * RdmaChatRoomTestClient.h
 *
 *  Created on: Feb 22, 2009
 *      Author: bbbb
 */


#include "RdmaChatRoomTestCommon.h"
#include "RdmaChatRoomTestMsg.cpp"

using namespace zillians;
using namespace zillians::net;

class RdmaChatRoomTestClient : public RdmaDataHandler, public RdmaConnectionHandler
{
public:
	RdmaChatRoomTestClient();
	virtual ~RdmaChatRoomTestClient();

public:
	virtual void onConnected(shared_ptr<RdmaConnection> connection);
	virtual void onDisconnected(shared_ptr<RdmaConnection> connection);
	virtual void onError(shared_ptr<RdmaConnection> connection, int code);

public:
	void handleStdin(ev::io &w, int revents);
	virtual void handle(uint32 type, shared_ptr<Buffer> b, shared_ptr<RdmaConnection> connection);

public:
	string mUserName;
	shared_ptr<RdmaConnection> mConnection;
	shared_ptr<RdmaChatRoomTestMsg> mSendMsg;

	enum ChatRoomClientStatus
	{
		IDLE,
		CONNECTED,
		LOGINOK,
	}mStatu;
};
