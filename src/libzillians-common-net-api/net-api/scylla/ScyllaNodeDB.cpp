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

#include "net-api/scylla/ScyllaNodeDB.h"

using namespace zillians::net::group;

namespace zillians { namespace net {

log4cxx::LoggerPtr ScyllaNodeDB::mLogger(log4cxx::Logger::getLogger("zillians.net.ScyllaNodeDB"));

//////////////////////////////////////////////////////////////////////////
ScyllaNodeDB::ScyllaNodeDB()
{
}

ScyllaNodeDB::~ScyllaNodeDB()
{
}

//////////////////////////////////////////////////////////////////////////
void ScyllaNodeDB::setChannel(const UUID& id, shared_ptr<Channel> channel)
{
	LOG4CXX_INFO(mLogger, "setChannel: " << id);
	ChannelMap::accessor iter;
	if(!mChannels.find(iter, id))
	{
		mChannels.insert(std::make_pair(id, channel));
	}
	else
	{
		iter->second = channel;
	}
}

void ScyllaNodeDB::unsetChannel(const UUID& id)
{
	ChannelMap::accessor iter;
	if(mChannels.find(iter, id))
	{
		mChannels.erase(iter);
	}
}

bool ScyllaNodeDB::queryChannel(const UUID& id, shared_ptr<Channel>* channel)
{
	ChannelMap::const_accessor iter;
	if(!mChannels.find(iter, id))
	{
		return false;
	}
	else
	{
		*channel = iter->second;
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
void ScyllaNodeDB::setIdMapping(const CloseProcessGroup::MemberInfo& memberInfo, const UUID& id)
{
	IdMap::accessor iter;
	if(!mIdMap.find(iter, memberInfo))
	{
		mIdMap.insert(std::make_pair(memberInfo, id));
	}
	else
	{
		iter->second = id;
	}
}

void ScyllaNodeDB::unsetIdMapping(const CloseProcessGroup::MemberInfo& memberInfo)
{
	IdMap::accessor iter;
	if(mIdMap.find(iter, memberInfo))
	{
		mIdMap.erase(iter);
	}
}

bool ScyllaNodeDB::queryIdMapping(const CloseProcessGroup::MemberInfo& memberInfo, UUID& id)
{
	IdMap::const_accessor iter;
	if(!mIdMap.find(iter, memberInfo))
	{
		return false;
	}
	else
	{
		id = iter->second;
		return true;
	}
}

} }
