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
 * @date Jul 3, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_SCYLLANODEDB_H_
#define ZILLIANS_NET_SCYLLANODEDB_H_

#include "core/Prerequisite.h"
#include "networking/scylla/ScyllaChannel.h"
#include <tbb/concurrent_hash_map.h>

namespace zillians { namespace net {

class ScyllaNodeDB
{
public:
	ScyllaNodeDB();
	virtual ~ScyllaNodeDB();

public:
	void setChannel(const UUID& id, shared_ptr<Channel> channel);
	void unsetChannel(const UUID& id);
	bool queryChannel(const UUID& id, shared_ptr<Channel>* channel);

public:
	void setIdMapping(const group::CloseProcessGroup::MemberInfo& memberInfo, const UUID& id);
	void unsetIdMapping(const group::CloseProcessGroup::MemberInfo& memberInfo);
	bool queryIdMapping(const group::CloseProcessGroup::MemberInfo& memberInfo, UUID& id);

private:
	typedef tbb::concurrent_hash_map<UUID, shared_ptr<Channel>, UUIDHasher> ChannelMap;
	ChannelMap mChannels;

private:
	typedef tbb::concurrent_hash_map<group::CloseProcessGroup::MemberInfo, UUID, group::CloseProcessGroup::MemberInfoHasher> IdMap;
	IdMap mIdMap;

private:
	static log4cxx::LoggerPtr mLogger;
};

} }

#endif/*ZILLIANS_NET_SCYLLANODEDB_H_*/
