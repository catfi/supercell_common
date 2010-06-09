/**
* Zillians MMO
* Copyright (C) 2007-2009 Zillians.com, Inc.
* For more information see http://www.zillians.com
*
* Zillians MMO is the library and runtime for massive multiplayer online game
* development in utility computing model, which runs as a service for every
* developer to build their virtual world running on our GPU-assisted machines.
*
* This is a close source library intended to be used solely within Zillians.com
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
* AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/**
* @date Feb 14, 2009 sdk - Initial version created.
*/



#include "RdmaChatRoomTestCommon.h"
#include "RdmaChatRoomTestClient.h"

using namespace zillians;
using namespace zillians::networking;

RdmaChatRoomTestClient::RdmaChatRoomTestClient()
{
	mStatu = IDLE;
}

RdmaChatRoomTestClient::~RdmaChatRoomTestClient()
{
}

void RdmaChatRoomTestClient::onConnected(shared_ptr<RdmaConnection> connection)
{
	mStatu = CONNECTED;
	std::cout << "CONNECTED" << std::endl;
	mConnection = connection;
	std::cout << "UserName : ";
	std::cin >> mUserName;
	shared_ptr<RdmaChatRoomTestMsg> sendMsg(new RdmaChatRoomTestMsg());
	mSendMsg = sendMsg;
	mSendMsg->createBuffer(mConnection);
	mSendMsg->sendMsg(RdmaChatRoomTestMsg::LOGIN, mUserName, "", "");
}

void RdmaChatRoomTestClient::onDisconnected(shared_ptr<RdmaConnection> connection)
{
	std::cout << "DISCONNECTED" << std::endl;
}

void RdmaChatRoomTestClient::onError(shared_ptr<RdmaConnection> connection, int code)
{
	std::cout << "ERROR" << std::endl;
}

void RdmaChatRoomTestClient::handleStdin(ev::io &w, int revents)
{

	if(mStatu != LOGINOK){
		std::cout << "You must login first." << std::endl;
		return;
	}
	string cmd;
	std::cin>>cmd;

	if(cmd == "")
	{
		return;
	}

	if(cmd == "broadcast" || cmd == "b")
	{
		string t, text=" ";
		cout << "Text : ";
		while(getline(std::cin, t))
		{
			if(t==".")break;
			text.append(t);
		}

		mSendMsg->sendMsg(RdmaChatRoomTestMsg::BROADCAST, mUserName, "", text);
	}

	else if(cmd == "send" | cmd == "s")
	{
		string receiver;
		std::cout << "Send to : ";
		cin>>receiver;

		string t, text=" ";
		std::cout << "Text : ";
		while(getline(std::cin, t))
		{
			if(t==".")break;
			text.append(t);
		}
		mSendMsg->sendMsg(RdmaChatRoomTestMsg::UNICAST, mUserName, receiver, text);
	}

	else if(cmd == "who" | cmd == "w")
	{
		mSendMsg->sendMsg(RdmaChatRoomTestMsg::WHO, mUserName, "", "");
	}

	else if(cmd == "statu")
	{
		std::cout<< "Statu: " << mStatu << std::endl;
	}

	else
	{
		std::cout << "Undefined command \"" << cmd << "\"." << std::endl;
		std::cout << "======= Command ======" << std::endl
				<< "  broadcast/b"<< std::endl
				<< "  send/s" << std::endl
				<< "  who/w" << std::endl
				<< "  statu" << std::endl
				<< "  * end message with \".\" on a line by itself." << std::endl
				<<   "======================" << std::endl;
	}
}

void RdmaChatRoomTestClient::handle(uint32 type, shared_ptr<Buffer> b, shared_ptr<RdmaConnection> connection)
{
	shared_ptr<RdmaChatRoomTestMsg> msg(new RdmaChatRoomTestMsg());
	msg->unpackMsg(type, b);

	switch(type)
	{
	case RdmaChatRoomTestMsg::LOGINOK:
		std::cout << "Login OK." << std::endl << msg->getText();
		mStatu = LOGINOK;
		break;

	case RdmaChatRoomTestMsg::LOGINFAIL:
		std::cout << "Login failed." << std::endl;
		std::cout << "UserName : ";
		std::cin >> mUserName;
		mSendMsg->sendMsg(RdmaChatRoomTestMsg::LOGIN, mUserName, "", "");
		break;

	case RdmaChatRoomTestMsg::BROADCAST:
		std::cout << msg->getSender() << " to everyone : " <<msg->getText() << std::endl;
		break;

	case RdmaChatRoomTestMsg::UNICAST:
		std::cout << msg->getSender() << " to you : " <<msg->getText() << std::endl;
		break;

	case RdmaChatRoomTestMsg::WHO:
		std::cout <<" User list : "<< std::endl;
//			TODO print user list
		break;

	}
}


