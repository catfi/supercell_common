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

using namespace zillians;
using namespace zillians::net;


class RdmaChatRoomTestMsg{

public:
	RdmaChatRoomTestMsg()
	{

	}
	~RdmaChatRoomTestMsg()
	{

	}

public:
	enum ChatRoomMsgType
	{
		LOGIN,
		LOGINFAIL,
		LOGINOK,
		BROADCAST,
		UNICAST,
		WHO,
	}mType;

private:
	string mSender;
	string mReceiver;
	string mText;
	SharedPtr<RdmaConnection> mConnection;
	SharedPtr<Buffer> mBuf;

public:
	ChatRoomMsgType getType()
	{
		return mType;
	}

	string getSender()
	{
		return mSender;
	}

	string getReceiver()
	{
		return mReceiver;
	}

	string getText()
	{
		return mText;
	}

public:
	void createBuffer(SharedPtr<RdmaConnection> connection){
		SharedPtr<Buffer> buf(connection->createBuffer(BUFFER_SIZE));
		mConnection = connection;
		mBuf = buf;
	}

public:
	void sendMsg(ChatRoomMsgType type, string sender, string receiver, string text)
	{
		mBuf->clear();
		switch(type){
			case LOGIN:
				(*mBuf.get()) << sender;
				break;
			case LOGINOK:
				(*mBuf.get()) << sender;
				(*mBuf.get()) << text;
				break;
			case LOGINFAIL:
				(*mBuf.get()) << sender;
				break;
			case BROADCAST:
				(*mBuf.get()) << sender;
				(*mBuf.get()) << text;
				break;
			case UNICAST:
				(*mBuf.get()) << sender;
				(*mBuf.get()) << receiver;
				(*mBuf.get()) << text;
				break;
			case WHO:
				(*mBuf.get()) << sender;
				break;
			default:
				std::cout << "Unknown ChatRoomMsg Type." << std::endl;
				return;
			}
//		std::cout << "==========Msg=========\nsender : " << sender << std::endl << "receiver : " << receiver << std::endl << "Text : " << text << std::endl;
		mConnection->send(type, mBuf);
	}

//Unpack information for clients.
	void unpackMsg(int32 type, SharedPtr<Buffer> buf)
	{
		switch(type){
			case LOGINFAIL:
				(*buf.get()) >> mSender;
				break;
			case LOGINOK:
				(*buf.get()) >> mSender;
				(*buf.get()) >> mText;
				break;
			case BROADCAST:
				(*buf.get()) >> mSender;
				(*buf.get()) >> mText;
				break;
			case UNICAST:
				(*buf.get()) >> mSender;
				(*buf.get()) >> mReceiver;
				(*buf.get()) >> mText;
				break;
			default:
				std::cout << "Unknown ChatRoomMsg Type." << std::endl;
				return;
		}
//		std::cout << "==========Msg=========\nsender : " << mSender << std::endl << "receiver : " << mReceiver << std::endl << "Text : " << mText << std::endl;
		buf->rpos(0);
	}

//Unpack information for server to forward message.
	void unpackSenderReceiver(int32 type, SharedPtr<Buffer> buf)
	{
		switch(type){
			case LOGIN:
				(*buf.get()) >> mSender;
				break;
			case BROADCAST:
				break;
			case UNICAST:
				(*buf.get()) >> mSender;
				(*buf.get()) >> mReceiver;
				break;
			case WHO:
				(*buf.get()) >> mSender;
				break;
			default:
				std::cout << "Unknown ChatRoomMsg Type." << std::endl;
				return;
		}
//		std::cout << "==========Msg=========\nsender : " << mSender << std::endl << "receiver : " << mReceiver << std::endl << "Text : " << mText << std::endl;
		buf->rpos(0);
	}
};
