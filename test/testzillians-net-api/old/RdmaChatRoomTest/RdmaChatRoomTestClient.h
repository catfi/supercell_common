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
	virtual void onConnected(SharedPtr<RdmaConnection> connection);
	virtual void onDisconnected(SharedPtr<RdmaConnection> connection);
	virtual void onError(SharedPtr<RdmaConnection> connection, int code);

public:
	void handleStdin(ev::io &w, int revents);
	virtual void handle(uint32 type, SharedPtr<Buffer> b, SharedPtr<RdmaConnection> connection);

public:
	string mUserName;
	SharedPtr<RdmaConnection> mConnection;
	SharedPtr<RdmaChatRoomTestMsg> mSendMsg;

	enum ChatRoomClientStatus
	{
		IDLE,
		CONNECTED,
		LOGINOK,
	}mStatu;
};
