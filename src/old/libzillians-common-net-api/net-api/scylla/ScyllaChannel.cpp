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
 * @date Jul 24, 2009 rocet - Initial version created.
 */

#include "networking/scylla/ScyllaChannel.h"
#include "networking/scylla/ScyllaAckMessage.h"

using namespace zillians::networking::group;

namespace zillians { namespace net {

//////////////////////////////////////////////////////////////////////////

ScyllaChannel::ScyllaChannel(CloseProcessGroup* cpg, shared_ptr<Worker> worker):
	mCloseProcessGroup(cpg), mAcksendWorker(worker)
{
	mConditionVarArray = new ConditionVariable<boost::system::error_code>*[SCYLLACHANNEL_CVARRAY_SIZE];
	for (uint32 i = 0UL; i < SCYLLACHANNEL_CVARRAY_SIZE; ++i)
	{
		mConditionVarArray[i] = new ConditionVariable<boost::system::error_code>;
		mConditionVarQueue.push(i);
	}
}

ScyllaChannel::~ScyllaChannel()
{
	for(uint32 i = 0UL; i < SCYLLACHANNEL_CVARRAY_SIZE; ++i)
	{
		if(mConditionVarArray[i])
		{
			SAFE_DELETE(mConditionVarArray[i]);
		}
	}
	SAFE_DELETE_ARRAY(mConditionVarArray);
}

//////////////////////////////////////////////////////////////////////////
void ScyllaChannel::send(uint32 type, shared_ptr<Buffer> buffer)
{
	mCloseProcessGroup->send(type, buffer);
}

void ScyllaChannel::acksend(uint32 type, shared_ptr<Buffer> buffer)
{
	uint32 cvId;
	mConditionVarQueue.pop(cvId);

	ScyllaAckMessage ackMessage;
	ackMessage.mIsAck = false;
	ackMessage.mCvId = cvId;
	ackMessage.mOriginalMessageType = type;
	ackMessage.mOriginalMessageSize = buffer->dataSize();
	shared_ptr<Buffer> buf(new Buffer(Buffer::probeSize(ackMessage) + ackMessage.mOriginalMessageSize));
	buf->writeSerializable(ackMessage);
	buf->append(*buffer);

	mCloseProcessGroup->send(ScyllaAckMessage::TYPE, buf);

	boost::system::error_code ec;
	mConditionVarArray[cvId]->wait(ec);
	mConditionVarQueue.push(cvId);
	if (ec)
	{
		throw boost::system::system_error(ec);
	}
}

void ScyllaChannel::acksendAsync(uint32 type, shared_ptr<Buffer> buffer, AsyncSendCompletionHandler handler)
{
	mAcksendWorker->dispatch(boost::bind(&ScyllaChannel::doAcksendAsync, this, type, buffer, handler));
}

//////////////////////////////////////////////////////////////////////////
void ScyllaChannel::handleAckMessage(uint32 cvId, const boost::system::error_code& ec)
{
	mConditionVarArray[cvId]->signal(ec);
}

//////////////////////////////////////////////////////////////////////////
void ScyllaChannel::doAcksendAsync(uint32 type, shared_ptr<Buffer> buffer, AsyncSendCompletionHandler handler)
{
	boost::system::error_code ec;
	try
	{
		acksend(type, buffer);
	}
	catch (boost::system::system_error& e)
	{
		ec = e.code();
	}
	handler(ec, buffer->dataSize());
}

} }
