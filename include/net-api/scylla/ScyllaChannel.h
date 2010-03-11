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
 * @date Jul 2, 2009 sdk - Initial version created.
 */

#ifndef ZILLIANS_NET_SCYLLACHANNEL_H_
#define ZILLIANS_NET_SCYLLACHANNEL_H_

#include "core-api/Prerequisite.h"
#include "core-api/BufferManager.h"
#include "core-api/ConditionVariable.h"
#include "core-api/Worker.h"
#include "net-api/Channel.h"
#include "net-api/group/CloseProcessGroup.h"

#define SCYLLACHANNEL_CVARRAY_SIZE	5000

namespace zillians { namespace net {

class ScyllaChannel : public Channel
{
	friend class ScyllaChannelEngine;

public:
	ScyllaChannel(group::CloseProcessGroup* cpg, SharedPtr<Worker> worker);
	virtual ~ScyllaChannel();

public:
	virtual void send(uint32 type, SharedPtr<Buffer> buffer);
	virtual void acksend(uint32 type, SharedPtr<Buffer> buffer);
	virtual void acksendAsync(uint32 type, SharedPtr<Buffer> buffer, AsyncSendCompletionHandler handler);

protected:
	virtual void handleAckMessage(uint32 cvId, const boost::system::error_code& ec);

private:
	void doAcksendAsync(uint32 type, SharedPtr<Buffer> buffer, AsyncSendCompletionHandler handler);

protected:
	group::CloseProcessGroup* mCloseProcessGroup;
	SharedPtr<Worker> mAcksendWorker;

private:
	ConditionVariable<boost::system::error_code>** mConditionVarArray;
	tbb::concurrent_bounded_queue<uint32> mConditionVarQueue;
};

} }

#endif/*ZILLIANS_NET_SCYLLACHANNEL_H_*/
