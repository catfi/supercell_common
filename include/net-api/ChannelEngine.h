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

#ifndef ZILLIANS_NET_CHANNELENGINE_H_
#define ZILLIANS_NET_CHANNELENGINE_H_

#include "core-api/Prerequisite.h"
#include "net-api/Channel.h"

namespace zillians { namespace net {

class ChannelEngine
{
public:
	ChannelEngine() { }
	virtual ~ChannelEngine() { }

public:
	virtual shared_ptr<Channel> findChannel(const UUID& destination) = 0;

public:
	typedef boost::function< void (const UUID& indentfier) > ChannelOnlineListener;
	typedef boost::function< void (const UUID& indentfier) > ChannelOfflineListener;
	typedef boost::function< void (const UUID& indentfier, const boost::system::error_code&) > ChannelErrorListener;

	virtual void registerAnyChannelListener(ChannelOnlineListener onOnline, ChannelOfflineListener onOffline, ChannelErrorListener onError) = 0;
	virtual void unregisterAnyChannelListener() = 0;

	virtual void registerChannelListener(const UUID& destination, ChannelOnlineListener onOnline, ChannelOfflineListener onOffline, ChannelErrorListener onError) = 0;
	virtual void unregisterChannelListener(const UUID& destination) = 0;

public:
	typedef boost::function< void (shared_ptr<Channel>, uint32, shared_ptr<Buffer>) > DataHandler;

	virtual void registerDefaultDataHandler(DataHandler handler) = 0;
	virtual void unregisterDefaultDataHandler() = 0;

	virtual void registerDataHandler(uint32 type, DataHandler handler) = 0;
	virtual void unregisterDataHandler(uint32 type) = 0;

public:
	virtual void run() = 0;
	virtual void terminate() = 0;
};

} }

#endif/*ZILLIANS_NET_CHANNELENGINE_H_*/
