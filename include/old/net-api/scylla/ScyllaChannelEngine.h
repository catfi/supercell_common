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

#ifndef ZILLIANS_NET_SCYLLACHANNELENGINE_H_
#define ZILLIANS_NET_SCYLLACHANNELENGINE_H_

#include "core/Prerequisite.h"
#include "core/BufferManager.h"
#include "core/Worker.h"
#include "networking/ChannelEngine.h"
#include "networking/scylla/ScyllaNodeDB.h"
#include "networking/scylla/ScyllaAckMessage.h"
#include "networking/scylla/ScyllaHelloMessage.h"
#include "networking/group/CloseProcessGroup.h"
#include "networking/group/CloseProcessGroupManager.h"
#include <tbb/tbb_thread.h>
#include <tbb/concurrent_hash_map.h>

namespace zillians { namespace net {

class ScyllaChannelEngine : public ChannelEngine
{
public:
	ScyllaChannelEngine(const UUID& localIdentifier, shared_ptr<ScyllaNodeDB> db);
	virtual ~ScyllaChannelEngine();

public:
	virtual shared_ptr<Channel> findChannel(const UUID& destination);

public:
	virtual void registerAnyChannelListener(ChannelOnlineListener onOnline, ChannelOfflineListener onOffline, ChannelErrorListener onError);
	virtual void unregisterAnyChannelListener();

	virtual void registerChannelListener(const UUID& destination, ChannelOnlineListener onOnline, ChannelOfflineListener onOffline, ChannelErrorListener onError);
	virtual void unregisterChannelListener(const UUID& destination);

public:
	virtual void registerDefaultDataHandler(DataHandler handler);
	virtual void unregisterDefaultDataHandler();

	void registerDataHandler(uint32 type, DataHandler handler);
	void unregisterDataHandler(uint32 type);

public:
	virtual void run();
	virtual void terminate();

private:
	void listenChannelOnline(const char* groupName, group::CloseProcessGroup::MemberInfo& memberInfo, group::MembershipChangeReason reason);
	void listenChannelOffline(const char* groupName, group::CloseProcessGroup::MemberInfo& memberInfo, group::MembershipChangeReason reason);
	void listenChannelError(const char* groupName, group::CloseProcessGroup::MemberInfo& memberInfo, group::MembershipChangeReason reason);

//	void listenChannelOnline(const char* groupName, group::CloseProcessGroup::MemberInfo& memberInfo, group::MembershipChangeReason reason, ChannelOfflineListener onOnline);
//	void listenChannelOffline(const char* groupName, group::CloseProcessGroup::MemberInfo& memberInfo, group::MembershipChangeReason reason, ChannelOfflineListener onOffline);
//	void listenChannelError(const char* groupName, group::CloseProcessGroup::MemberInfo& memberInfo, group::MembershipChangeReason reason, ChannelErrorListener onError);

	void handleData(group::CloseProcessGroup::SourceInfo& source, uint32 type, shared_ptr<Buffer>& buffer, std::size_t size, DataHandler handler);

	void handleHelloMessage(group::CloseProcessGroup::SourceInfo& source, ScyllaHelloMessage& message);
	void handleAckMessage(group::CloseProcessGroup::SourceInfo& source, uint32 type, shared_ptr<Buffer>& buffer, std::size_t size);

private:
	ChannelOnlineListener defaultChannelOnlineListener;
	ChannelOfflineListener defaultChannelOfflineListener;
	ChannelErrorListener defaultChannelErrorListener;

	typedef tbb::concurrent_hash_map<UUID, ChannelOnlineListener, UUIDHasher> ChannelOnlineListenerMap;
	typedef tbb::concurrent_hash_map<UUID, ChannelOfflineListener, UUIDHasher> ChannelOfflineListenerMap;
	typedef tbb::concurrent_hash_map<UUID, ChannelErrorListener, UUIDHasher> ChannelErrorListenerMap;

	ChannelOnlineListenerMap mChannelOnlineListenerMap;
	ChannelOfflineListenerMap mChannelOfflineListenerMap;
	ChannelErrorListenerMap mChannelErrorListenerMap;

private:
	shared_ptr<ScyllaNodeDB> mNodeDB;
	UUID mLocalIdentifier;
	group::CloseProcessGroupManager* mCloseProcessGroupManager;
	shared_ptr<Worker> mWorker;

private:
	static log4cxx::LoggerPtr mLogger;
};

} }

#endif/*ZILLIANS_NET_SCYLLACHANNELENGINE_H_*/
