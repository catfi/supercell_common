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
#include "RdmaChatRoomTestServer.h"
#include "RdmaChatRoomTestMsg.cpp"

using namespace zillians;
using namespace zillians::net;


RdmaChatRoomTestServer::RdmaChatRoomTestServer()
{
}

RdmaChatRoomTestServer::~RdmaChatRoomTestServer()
{
}

void RdmaChatRoomTestServer::onConnected(SharedPtr<RdmaConnection> connection)
{
	std::cout<<"Client connected."<<std::endl;
}

void RdmaChatRoomTestServer::onDisconnected(SharedPtr<RdmaConnection> connection)
{
	std::cout<<"Client disconnected."<<std::endl;

	for(myIterator it = mConnectionContainer.begin(); it != mConnectionContainer.end(); ++it)
	{
		if(it->second.get() == connection.get())
		{
			mConnectionContainer.erase(it);
			updateClientList();
		}
	}
}

void RdmaChatRoomTestServer::onError(SharedPtr<RdmaConnection> connection, int code)
{
	std::cout << "ERROR" << std::endl;
}

void RdmaChatRoomTestServer::updateClientList()
{
	mClientList = 	 "\n======= UserList =====\n";
	for(myIterator it = mConnectionContainer.begin(); it != mConnectionContainer.end(); ++it)
	{
		mClientList.append(it->first);
		mClientList.append("\n");
	}
	mClientList.append("======================\n");
	std::cout << mClientList << std::endl;
}

void RdmaChatRoomTestServer::handleStdin(ev::io &w, int revents)
{
	string cmd;
	std::cin>>cmd;

	if(cmd == "")
	{
		return;
	}

	else if(cmd == "who" || cmd == "w")
	{
		std::cout<< mClientList << std::endl;
	}

	else
	{
		std::cout << "Undefined command \"" << cmd << "\"." << std::endl;
	}

}

void RdmaChatRoomTestServer::handle(uint32 type, SharedPtr<Buffer> b, SharedPtr<RdmaConnection> connection)
{
	SharedPtr<RdmaChatRoomTestMsg> msg(new RdmaChatRoomTestMsg());
	msg->unpackSenderReceiver(type, b);

	switch(type)
	{
	case RdmaChatRoomTestMsg::LOGIN:
		{
			SharedPtr<RdmaChatRoomTestMsg> sendMsg(new RdmaChatRoomTestMsg());
			sendMsg->createBuffer(connection);

//			Avoid userName conflict
			if(mConnectionContainer.find(msg->getSender()) != mConnectionContainer.end())
			{
				sendMsg->sendMsg(RdmaChatRoomTestMsg::LOGINFAIL, "server", "", "");
			}
			else
			{
				mConnectionContainer.insert(pair<string, SharedPtr<RdmaConnection> > (msg->getSender(), connection));
				updateClientList();
				sendMsg->sendMsg(RdmaChatRoomTestMsg::LOGINOK, "server", "", mClientList);
			}
		}
		break;

	case RdmaChatRoomTestMsg::BROADCAST:
		{
			for(myIterator it = mConnectionContainer.begin(); it != mConnectionContainer.end(); ++it)
			{
				it->second->send(type, b);
			}
		}
		break;
	case RdmaChatRoomTestMsg::UNICAST:
		{
			myIterator it = mConnectionContainer.find(msg->getReceiver());
			if(it != mConnectionContainer.end())
				it->second->send(type, b);
		}
		break;

	case RdmaChatRoomTestMsg::WHO:
		{
			SharedPtr<RdmaChatRoomTestMsg> sendMsg(new RdmaChatRoomTestMsg());
			sendMsg->createBuffer(connection);
			sendMsg->sendMsg(RdmaChatRoomTestMsg::UNICAST,"server", msg->getSender() , mClientList);
		}
		break;
	}
}
