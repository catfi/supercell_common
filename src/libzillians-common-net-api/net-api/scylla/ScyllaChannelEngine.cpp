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
 * @date Jun 30, 2009 sdk - Initial version created.
 */

#include "net-api/scylla/ScyllaChannelEngine.h"
#include "net-api/scylla/ScyllaChannel.h"

#define GLOBAL_GROUP_UUID	"3fbcb6e8-b583-11de-8d64-001d92648328"

using namespace zillians::net;
using namespace zillians::net::group;

namespace zillians { namespace net {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr ScyllaChannelEngine::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.scylla.ScyllaChannelEngine"));

//////////////////////////////////////////////////////////////////////////
struct null_deleter
{
    void operator()(void const *) const
    {
    }
};

//////////////////////////////////////////////////////////////////////////
ScyllaChannelEngine::ScyllaChannelEngine(const UUID& localIdentifier, SharedPtr<ScyllaNodeDB> db):
	defaultChannelOfflineListener(NULL),
	defaultChannelErrorListener(NULL)
{
	mNodeDB = db;
	mLocalIdentifier = localIdentifier;
	mCloseProcessGroupManager = new CloseProcessGroupManager();
	mWorker.reset(new Worker());

	{
		// join global group
		CloseProcessGroup* cpg = new CloseProcessGroup();

		CloseProcessGroup::MembershipDispatcher* membershipDispatcher = new CloseProcessGroup::MembershipDispatcher();
		membershipDispatcher->bind(
				GLOBAL_GROUP_UUID,
				boost::bind(&ScyllaChannelEngine::listenChannelOnline, this, placeholders::membership::group_name, placeholders::membership::source_info, placeholders::membership::reason),
				boost::bind(&ScyllaChannelEngine::listenChannelOffline, this, placeholders::membership::group_name, placeholders::membership::source_info, placeholders::membership::reason),
				boost::bind(&ScyllaChannelEngine::listenChannelError, this, placeholders::membership::group_name, placeholders::membership::source_info, placeholders::membership::reason)
		);
		cpg->setMemberDispatcher(membershipDispatcher);

		CloseProcessGroup::DataDispatcher* dataDispatcher = new CloseProcessGroup::DataDispatcher(256);
		dataDispatcher->bind<ScyllaHelloMessage>(
				boost::bind(&ScyllaChannelEngine::handleHelloMessage, this,
						placeholders::data::source_ref,
						placeholders::data::message_ref
		));
		cpg->setDataDispatcher(dataDispatcher);

		BOOST_ASSERT(cpg->join(GLOBAL_GROUP_UUID));
		mCloseProcessGroupManager->attach(cpg);

		ScyllaChannel* scyllaChannel = new ScyllaChannel(cpg, mWorker);
		scyllaChannel->setIdentifier(GLOBAL_GROUP_UUID);
		SharedPtr<Channel> channel;
		channel.reset();
		channel = boost::static_pointer_cast<Channel>(SharedPtr<ScyllaChannel>(scyllaChannel, null_deleter()));

		mNodeDB->setChannel(GLOBAL_GROUP_UUID, channel);
	}

	{
		// join local group
		CloseProcessGroup* cpg = new CloseProcessGroup();
		CloseProcessGroup::DataDispatcher* dataDispatcher = new CloseProcessGroup::DataDispatcher(256);
		dataDispatcher->bind(
				ScyllaAckMessage::TYPE,
				boost::bind(&ScyllaChannelEngine::handleAckMessage, this,
						placeholders::data::source_ref,
						placeholders::data::type,
						placeholders::data::buffer_ref,
						placeholders::data::size
		));
		cpg->setDataDispatcher(dataDispatcher);

		BOOST_ASSERT(cpg->join(mLocalIdentifier));
		mCloseProcessGroupManager->attach(cpg);

		ScyllaChannel* scyllaChannel = new ScyllaChannel(cpg, mWorker);
		scyllaChannel->setIdentifier(mLocalIdentifier);
		SharedPtr<Channel> channel;
		channel.reset();
		channel = boost::static_pointer_cast<Channel>(SharedPtr<ScyllaChannel>(scyllaChannel, null_deleter()));

		mNodeDB->setChannel(mLocalIdentifier, channel);
	}
}

ScyllaChannelEngine::~ScyllaChannelEngine()
{
	terminate();
}

//////////////////////////////////////////////////////////////////////////
SharedPtr<Channel> ScyllaChannelEngine::findChannel(const UUID& destination)
{
	SharedPtr<Channel> channel;	channel.reset();
	if(!mNodeDB->queryChannel(destination, &channel))
	{
		CloseProcessGroup* cpg = new CloseProcessGroup();
		BOOST_ASSERT(cpg->join(destination));
		ScyllaChannel* scyllaChannel = new ScyllaChannel(cpg, mWorker);
		scyllaChannel->setIdentifier(destination);
		channel = boost::static_pointer_cast<Channel>(SharedPtr<ScyllaChannel>(scyllaChannel, null_deleter()));

		mNodeDB->setChannel(destination, channel);
	}

	return channel;
}

//////////////////////////////////////////////////////////////////////////
void ScyllaChannelEngine::listenChannelOnline(const char* groupName, CloseProcessGroup::MemberInfo& memberInfo, MembershipChangeReason reason)
{
	LOG4CXX_INFO(mLogger, "(" << memberInfo.nodeId << ", " << memberInfo.processId << ") is online");

	SharedPtr<Channel> channel; channel.reset();
	if (!mNodeDB->queryChannel(GLOBAL_GROUP_UUID, &channel))
	{
		LOG4CXX_ERROR(mLogger, "can't find global channel");
	}
	else
	{
		ScyllaHelloMessage hello;
		hello.mIdentity = mLocalIdentifier;
		SharedPtr<Buffer> buf(new Buffer(Buffer::probeSize(hello)));
		buf->writeSerializable(hello);
		channel->send(ScyllaHelloMessage::TYPE, buf);
	}

	if (defaultChannelOnlineListener != NULL)
	{
		defaultChannelOnlineListener(groupName);
	}

	ChannelOnlineListenerMap::const_accessor iter;
	if(mChannelOnlineListenerMap.find(iter, groupName))
	{
		(iter->second)(groupName);
	}
}

void ScyllaChannelEngine::listenChannelOffline(const char* groupName, CloseProcessGroup::MemberInfo& memberInfo, MembershipChangeReason reason)
{
	UUID identifier;
	if (!mNodeDB->queryIdMapping(memberInfo, identifier))
	{
		LOG4CXX_ERROR(mLogger, "can't find id (" << memberInfo.nodeId << ", " << memberInfo.processId << ")");
		return;
	}

	LOG4CXX_INFO(mLogger, "channel " << identifier << " is offline");

	SharedPtr<Channel> channel; channel.reset();
	if (mNodeDB->queryChannel(identifier, &channel))
	{
		SharedPtr<ScyllaChannel> scyllaChannel = boost::dynamic_pointer_cast<ScyllaChannel>(channel);
		scyllaChannel->mCloseProcessGroup->leave(identifier);
	}

	if (defaultChannelOfflineListener != NULL)
	{
		defaultChannelOfflineListener(identifier);
	}

	ChannelOfflineListenerMap::const_accessor iter;
	if(mChannelOfflineListenerMap.find(iter, identifier))
	{
		(iter->second)(identifier);
	}

	mNodeDB->unsetIdMapping(memberInfo);
	mNodeDB->unsetChannel(identifier);
}

void ScyllaChannelEngine::listenChannelError(const char* groupName, CloseProcessGroup::MemberInfo& memberInfo, MembershipChangeReason reason)
{
	UUID identifier;
	if (!mNodeDB->queryIdMapping(memberInfo, identifier))
	{
		LOG4CXX_ERROR(mLogger, "can't find id (" << memberInfo.nodeId << ", " << memberInfo.processId << ")");
		return;
	}

	LOG4CXX_INFO(mLogger, "channel " << identifier << " error occured");

	boost::system::error_code ec;
	if (reason == MEMBER_NODEDOWN)
	{
		ec = boost::system::system_error(boost::asio::error::connection_reset).code();
	}
	else if (reason == MEMBER_PROCDOWN)
	{
		ec = boost::system::system_error(boost::asio::error::connection_reset).code();
	}
	else
	{
		ec = boost::system::system_error(boost::asio::error::connection_reset).code();
	}

	if (defaultChannelErrorListener != NULL)
	{
		defaultChannelErrorListener(identifier, ec);
	}

	ChannelErrorListenerMap::const_accessor iter;
	if(mChannelErrorListenerMap.find(iter, identifier))
	{
		(iter->second)(identifier, ec);
	}
}

void ScyllaChannelEngine::registerAnyChannelListener(ChannelOnlineListener onOnline, ChannelOfflineListener onOffline, ChannelErrorListener onError)
{
	defaultChannelOnlineListener = onOnline;
	defaultChannelOfflineListener = onOffline;
	defaultChannelErrorListener = onError;
}

void ScyllaChannelEngine::unregisterAnyChannelListener()
{
	defaultChannelOnlineListener = NULL;
	defaultChannelOfflineListener = NULL;
	defaultChannelErrorListener = NULL;
}

void ScyllaChannelEngine::registerChannelListener(const UUID& destination, ChannelOnlineListener onOnline, ChannelOfflineListener onOffline, ChannelErrorListener onError)
{
	{
		ChannelOnlineListenerMap::accessor iter;
		if(!mChannelOnlineListenerMap.find(iter, destination))
		{
			mChannelOnlineListenerMap.insert(std::make_pair(destination, onOnline));
		}
		else
		{
			iter->second = onOnline;
		}
	}

	{
		ChannelOfflineListenerMap::accessor iter;
		if(!mChannelOfflineListenerMap.find(iter, destination))
		{
			mChannelOfflineListenerMap.insert(std::make_pair(destination, onOffline));
		}
		else
		{
			iter->second = onOffline;
		}
	}

	{
		ChannelErrorListenerMap::accessor iter;
		if(!mChannelErrorListenerMap.find(iter, destination))
		{
			mChannelErrorListenerMap.insert(std::make_pair(destination, onError));
		}
		else
		{
			iter->second = onError;
		}
	}
}

void ScyllaChannelEngine::unregisterChannelListener(const UUID& destination)
{
	{
		ChannelOnlineListenerMap::accessor iter;
		if(mChannelOnlineListenerMap.find(iter, destination))
		{
			mChannelOnlineListenerMap.erase(iter);
		}
	}

	{
		ChannelOfflineListenerMap::accessor iter;
		if(mChannelOfflineListenerMap.find(iter, destination))
		{
			mChannelOfflineListenerMap.erase(iter);
		}
	}

	{
		ChannelErrorListenerMap::accessor iter;
		if(mChannelErrorListenerMap.find(iter, destination))
		{
			mChannelErrorListenerMap.erase(iter);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void ScyllaChannelEngine::handleData(CloseProcessGroup::SourceInfo& source, uint32 type, SharedPtr<Buffer>& buffer, std::size_t size, DataHandler handler)
{
	UUID identifier;
	CloseProcessGroup::MemberInfo memberInfo;
	memberInfo.nodeId = source.nodeId;
	memberInfo.processId = source.processId;

	if (mNodeDB->queryIdMapping(memberInfo, identifier))
	{
		SharedPtr<Channel> channel; channel.reset();
		channel = findChannel(identifier);
		handler(channel, type, buffer);
	}
}

void ScyllaChannelEngine::registerDefaultDataHandler(DataHandler handler)
{
	SharedPtr<Channel> channel; channel.reset();
	if (mNodeDB->queryChannel(mLocalIdentifier, &channel))
	{
		SharedPtr<ScyllaChannel> scyllaChannel = boost::dynamic_pointer_cast<ScyllaChannel>(channel);
		CloseProcessGroup::DataDispatcher* dataDispatcher = scyllaChannel->mCloseProcessGroup->getDataDispatcher();
		if (dataDispatcher == NULL)
		{
			dataDispatcher = new CloseProcessGroup::DataDispatcher(256);
		}

		dataDispatcher->bindAny(
				boost::bind(
						&ScyllaChannelEngine::handleData, this,
						placeholders::data::source_ref,
						placeholders::data::type,
						placeholders::data::buffer_ref,
						placeholders::data::size,
						handler
		));
	}
}
void ScyllaChannelEngine::unregisterDefaultDataHandler()
{
	SharedPtr<Channel> channel; channel.reset();
	if (mNodeDB->queryChannel(mLocalIdentifier, &channel))
	{
		SharedPtr<ScyllaChannel> scyllaChannel = boost::dynamic_pointer_cast<ScyllaChannel>(channel);
		CloseProcessGroup::DataDispatcher* dataDispatcher = scyllaChannel->mCloseProcessGroup->getDataDispatcher();
		if (dataDispatcher != NULL)
		{
			dataDispatcher->unbindAny();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void ScyllaChannelEngine::registerDataHandler(uint32 type, DataHandler handler)
{
	SharedPtr<Channel> channel; channel.reset();
	if (mNodeDB->queryChannel(mLocalIdentifier, &channel))
	{
		SharedPtr<ScyllaChannel> scyllaChannel = boost::dynamic_pointer_cast<ScyllaChannel>(channel);
		CloseProcessGroup::DataDispatcher* dataDispatcher = scyllaChannel->mCloseProcessGroup->getDataDispatcher();
		if (dataDispatcher == NULL)
		{
			dataDispatcher = new CloseProcessGroup::DataDispatcher(256);
		}

		dataDispatcher->bind(
				type,
				boost::bind(
						&ScyllaChannelEngine::handleData, this,
						placeholders::data::source_ref,
						placeholders::data::type,
						placeholders::data::buffer_ref,
						placeholders::data::size,
						handler
		));
	}
}

void ScyllaChannelEngine::unregisterDataHandler(uint32 type)
{
	SharedPtr<Channel> channel; channel.reset();
	if (mNodeDB->queryChannel(mLocalIdentifier, &channel))
	{
		SharedPtr<ScyllaChannel> scyllaChannel = boost::dynamic_pointer_cast<ScyllaChannel>(channel);
		CloseProcessGroup::DataDispatcher* dataDispatcher = scyllaChannel->mCloseProcessGroup->getDataDispatcher();
		if (dataDispatcher != NULL)
		{
			dataDispatcher->unbind(type);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void ScyllaChannelEngine::run()
{
	mCloseProcessGroupManager->run();
}

void ScyllaChannelEngine::terminate()
{
	mCloseProcessGroupManager->stop();
}

//////////////////////////////////////////////////////////////////////////
void ScyllaChannelEngine::handleHelloMessage(CloseProcessGroup::SourceInfo& source, ScyllaHelloMessage& message)
{
	LOG4CXX_INFO(mLogger, "received hello message from (" << source.nodeId << ", " << source.processId << "): uuid=" << message.mIdentity);

	CloseProcessGroup::MemberInfo memberInfo;
	memberInfo.nodeId = source.nodeId;
	memberInfo.processId = source.processId;
	mNodeDB->setIdMapping(memberInfo, message.mIdentity);
}

void ScyllaChannelEngine::handleAckMessage(CloseProcessGroup::SourceInfo& source, uint32 type, SharedPtr<Buffer>& buffer, std::size_t size)
{
	//uint32 cvId, messageType;
	//*buffer >> cvId >> messageType;
	SharedPtr<ScyllaAckMessage> message = SharedPtr<ScyllaAckMessage>(new ScyllaAckMessage());
	buffer->readSerializable(*message);

	UUID identifier;
	CloseProcessGroup::MemberInfo memberInfo;
	memberInfo.nodeId = source.nodeId;
	memberInfo.processId = source.processId;
	if (!mNodeDB->queryIdMapping(memberInfo, identifier))
	{
		LOG4CXX_ERROR(mLogger, "can't find id (" << memberInfo.nodeId << ", " << memberInfo.processId << ")");
		return;
	}

	if (message->mIsAck)
	{
		LOG4CXX_INFO(mLogger, "received ack message");
		// Ack Message
		boost::system::error_code ec;
		*buffer >> ec;

		SharedPtr<Channel> channel; channel.reset();
		if (!mNodeDB->queryChannel(identifier, &channel))
		{
			LOG4CXX_ERROR(mLogger, "can't find channel (" << identifier << ")");
			return;
		}
		SharedPtr<ScyllaChannel> scyllaChannel = boost::dynamic_pointer_cast<ScyllaChannel>(channel);

		scyllaChannel->handleAckMessage(message->mCvId, ec);
	}
	else
	{
		LOG4CXX_INFO(mLogger, "received acksend message");
		// AckSend Message
		boost::system::error_code ec;
		try
		{
			SharedPtr<Channel> channel; channel.reset();
			if (!mNodeDB->queryChannel(mLocalIdentifier, &channel))
			{
				LOG4CXX_ERROR(mLogger, "can't find channel (" << mLocalIdentifier << ")");
				return;
			}
			SharedPtr<ScyllaChannel> scyllaChannel = boost::dynamic_pointer_cast<ScyllaChannel>(channel);

			CloseProcessGroup::DataDispatcher* dataDispatcher = scyllaChannel->mCloseProcessGroup->getDataDispatcher();
			BOOST_ASSERT(dataDispatcher != NULL);

			dataDispatcher->dispatch(source, message->mOriginalMessageType, buffer, message->mOriginalMessageSize);
		}
		catch (boost::system::system_error& e)
		{
			ec = e.code();
			LOG4CXX_ERROR(mLogger, "catched error: " << e.code().message());
		}

		ScyllaAckMessage ackMessage;
		ackMessage.mIsAck = true;
		ackMessage.mCvId = message->mCvId;
		ackMessage.mOriginalMessageType = 0;
		ackMessage.mOriginalMessageSize = 0;
		SharedPtr<Buffer> buf(new Buffer(Buffer::probeSize(ackMessage) + Buffer::probeSize(ec)));
		buf->writeSerializable(ackMessage);
		*buf << ec;

		SharedPtr<Channel> remoteChannel = findChannel(identifier);
		remoteChannel->send(ScyllaAckMessage::TYPE, buf);
	}
}

} }
