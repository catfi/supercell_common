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

#include "core-api/ConditionVariable.h"
#include "core-api/ObjectPool.h"
#include "net-api/rdma/infiniband/IBConnection.h"
#include "net-api/rdma/infiniband/IBNetEngine.h"
#include "net-api/rdma/infiniband/IBDeviceResourceManager.h"
#include "net-api/rdma/buffer_manager/IBBufferManager.h"
#include "tbb/tbb_thread.h"

/**
 * _HOLD_BUFFER macro is used to add buffer reference to a buffer reference holder to prevent the buffer object being freed automatically
 * @see _UNHOLD_BUFFER, _GET_BUFFER
 */
#ifdef IB_ENABLE_DEBUG
#define _HOLD_BUFFER(holder, ref, buffer) \
	{ \
		IBConnection::BufferRefHolder::accessor accessor; \
		if(holder.insert(accessor, reinterpret_cast<uint64>(ref))) \
		{ \
			accessor->second = buffer; \
			/*IB_DEBUG("" #holder " hold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")");*/ \
			++mDEBUG_HoldBuffer; \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to hold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")"); \
		} \
	}
#else
#define _HOLD_BUFFER(holder, ref, buffer) \
	{ \
		IBConnection::BufferRefHolder::accessor accessor; \
		if(holder.insert(accessor, reinterpret_cast<uint64>(ref))) \
		{ \
			accessor->second = buffer; \
			/*IB_DEBUG("" #holder " hold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")");*/ \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to hold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")"); \
		} \
	}
#endif

/**
 * _GET_BUFFER macro is used to get buffer reference from a buffer reference holder
 * @see _HOLD_BUFFER
 */
#define _GET_BUFFER(holder, ref, buffer) \
	{ \
		IBConnection::BufferRefHolder::accessor accessor; \
		if(holder.find(accessor, reinterpret_cast<uint64>(ref))) \
		{ \
			buffer = accessor->second; \
			/*IB_DEBUG(mLogger, "" #holder "return buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")");*/ \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to get buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")"); \
		} \
	}

/**
 * _GET_AND_UNHOLD_BUFFER macro is used to get buffer reference from a buffer reference holder and erase it right away
 * @see _HOLD_BUFFER
 */
#ifdef IB_ENABLE_DEBUG
#define _GET_AND_UNHOLD_BUFFER(holder, ref, buffer) \
	{ \
		IBConnection::BufferRefHolder::accessor accessor; \
		if(holder.find(accessor, reinterpret_cast<uint64>(ref))) \
		{ \
			buffer = accessor->second; \
			holder.erase(accessor); \
			/*IB_DEBUG(mLogger, "" #holder "return buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")");*/ \
			--mDEBUG_HoldBuffer; \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to get & unhold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")"); \
		} \
	}
#else
#define _GET_AND_UNHOLD_BUFFER(holder, ref, buffer) \
	{ \
		IBConnection::BufferRefHolder::accessor accessor; \
		if(holder.find(accessor, reinterpret_cast<uint64>(ref))) \
		{ \
			buffer = accessor->second; \
			holder.erase(accessor); \
			/*IB_DEBUG(mLogger, "" #holder "return buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")");*/ \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to get & unhold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")"); \
		} \
	}
#endif

/**
 * _UNHOLD_BUFFER macro is used to remove buffer reference from a buffer reference holder to free up the buffer object
 * @see _HOLD_BUFFER
 */
#ifdef IB_ENABLE_DEBUG
#define _UNHOLD_BUFFER(holder, ref) \
	{ \
		IBConnection::BufferRefHolder::accessor accessor; \
		if(holder.find(accessor, reinterpret_cast<uint64>(ref))) \
		{ \
			holder.erase(accessor); \
			/*IB_DEBUG(mLogger, "" #holder " unhold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")");*/ \
			--mDEBUG_HoldBuffer; \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to unhold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")"); \
		} \
	}
#else
#define _UNHOLD_BUFFER(holder, ref) \
	{ \
		IBConnection::BufferRefHolder::accessor accessor; \
		if(holder.find(accessor, reinterpret_cast<uint64>(ref))) \
		{ \
			holder.erase(accessor); \
			/*IB_DEBUG(mLogger, "" #holder " unhold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")");*/ \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to unhold buffer reference for ref = " << reinterpret_cast<uint64>(ref) << " ( at " << __LINE__ << ")"); \
		} \
	}
#endif
/**
 * _HOLD_REF macro is used to hold "value" which can be referenced through "ref"
 * @see _UNHOLD_REF
 */
#ifdef IB_ENABLE_DEBUG
#define _HOLD_REF(holder, ref, value) \
	{ \
		IBConnection::RefHolder::accessor accessor; \
		if(holder.insert(accessor, ref)) \
		{ \
			accessor->second = value; \
			/*IB_DEBUG("" #holder " hold reference value for ref = " << ref << ", value = " << value << " ( at " << __LINE__ << ")");*/ \
			++mDEBUG_HoldRef; \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to hold reference value for ref = " << ref << " ( at " << __LINE__ << ")"); \
		} \
	}
#else
#define _HOLD_REF(holder, ref, value) \
	{ \
		IBConnection::RefHolder::accessor accessor; \
		if(holder.insert(accessor, ref)) \
		{ \
			accessor->second = value; \
			/*IB_DEBUG("" #holder " hold reference value for ref = " << ref << ", value = " << value << " ( at " << __LINE__ << ")");*/ \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to hold reference value for ref = " << ref << " ( at " << __LINE__ << ")"); \
		} \
	}
#endif

/**
 * _GET_REF macro is used to get the holded value from "ref" (the value is stored into "value")
 * @see _HOLD_REF
 */
#define _GET_REF(holder, ref, value) \
	{ \
		IBConnection::RefHolder::accessor accessor; \
		if(holder.find(accessor, ref)) \
		{ \
			value = accessor->second; \
			/*IB_DEBUG("" #holder " returns reference value for ref = " << ref << ", value = " << value << " ( at " << __LINE__ << ")");*/ \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to return reference value for ref = " << ref << " ( at " << __LINE__ << ")"); \
		} \
	}

/**
 * _GET_AND_UNHOLD_REF macro is used to get the holded value and unhold it right away
 * @see _HOLD_REF
 */
#ifdef IB_ENABLE_DEBUG
#define _GET_AND_UNHOLD_REF(holder, ref, value) \
	{ \
		IBConnection::RefHolder::accessor accessor; \
		if(holder.find(accessor, ref)) \
		{ \
			value = accessor->second; \
			holder.erase(accessor); \
			/*IB_DEBUG("" #holder " returns & unhold reference value for ref = " << ref << ", value = " << value << " ( at " << __LINE__ << ")");*/ \
			--mDEBUG_HoldRef; \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to return & unhold reference value for ref = " << ref << " ( at " << __LINE__ << ")"); \
		} \
	}
#else
#define _GET_AND_UNHOLD_REF(holder, ref, value) \
	{ \
		IBConnection::RefHolder::accessor accessor; \
		if(holder.find(accessor, ref)) \
		{ \
			value = accessor->second; \
			holder.erase(accessor); \
			/*IB_DEBUG("" #holder " returns & unhold reference value for ref = " << ref << ", value = " << value << " ( at " << __LINE__ << ")");*/ \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to return & unhold reference value for ref = " << ref << " ( at " << __LINE__ << ")"); \
		} \
	}
#endif

/**
 * _UNHOLD_REF macro is used to unhold the referenced value
 * @see _HOLD_REF
 */
#ifdef IB_ENABLE_DEBUG
#define _UNHOLD_REF(holder, ref) \
	{ \
		IBConnection::RefHolder::accessor accessor; \
		if(holder.find(accessor, ref)) \
		{ \
			holder.erase(accessor); \
			/*IB_DEBUG("" #holder " unhold reference value for ref = " << ref << " ( at " << __LINE__ << ")");*/ \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to return reference value for ref = " << ref << " ( at " << __LINE__ << ")"); \
		} \
	}
#else
#define _UNHOLD_REF(holder, ref) \
	{ \
		IBConnection::RefHolder::accessor accessor; \
		if(holder.find(accessor, ref)) \
		{ \
			holder.erase(accessor); \
			/*IB_DEBUG("" #holder " unhold reference value for ref = " << ref << " ( at " << __LINE__ << ")");*/ \
		} \
		else \
		{ \
			IB_ERROR("[ERROR] " #holder " fail to return reference value for ref = " << ref << " ( at " << __LINE__ << ")"); \
		} \
	}
#endif

namespace zillians { namespace net { namespace rdma {

//////////////////////////////////////////////////////////////////////////
struct PooledConditionVariable : public zillians::ConditionVariable<int>, public zillians::ConcurrentObjectPool<PooledConditionVariable>
{ };

void gSignalConditionVariable(zillians::ConditionVariable<int>* cv, int error)
{
	cv->signal(error);
}

//////////////////////////////////////////////////////////////////////////
const uint32 IBConnection::CONTROL_CREDIT_UPDATE 		= 1;//0x10000000;
const uint32 IBConnection::CONTROL_LARGE_BUFFER_SEND	= 2;//0x20000000;
const uint32 IBConnection::CONTROL_LARGE_BUFFER_ACK		= 3;//0x30000000;
const uint32 IBConnection::CONTROL_ACCESS_EXCHANGE		= 4;//0x40000000;
const uint32 IBConnection::CONTROL_SEND_DIRECT_SIGNAL 	= 5;//0x50000000;
const uint32 IBConnection::CONTROL_REG_DIRECT_BUFFER	= 6;//0x60000000;
const uint32 IBConnection::CONTROL_UNREG_DIRECT_BUFFER	= 7;//0x70000000;

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr IBConnection::mLogger(log4cxx::Logger::getLogger("zillians.common.net-api.rdma.infiniband.IBConnection"));

//////////////////////////////////////////////////////////////////////////
IBConnection::IBConnection(IBNetEngine* engine, shared_ptr<rdma_cm_id> id)
{
	IB_DEBUG("CTOR");
	mEngine = engine;

	// given id should be valid
	BOOST_ASSERT(!!id);

	// save rdma_cm_id
	mVerbs.id = id;

	// get device resource from device resource manager
	mDeviceResource = IBDeviceResourceManager::instance()->getResource(id->verbs);

	// initialize queue pair related structure
	ensureQueuePair();

    // watchers initialized disabled
    mWatchersStarted = false;


    // for first time we request notification of the send/recv completion queue
    notifyRecv();
    notifySend();

    // create a bunch of buffer and "post receive" on them
    mConnected = true;

    // initialize flow control
    mFlowControl.recvCredit			= 0;
    mFlowControl.xmitCredit			= IB_DEFAULT_DATA_BUFFER_COUNT + IB_DEFAULT_ACK_BUFFER_COUNT;
    mFlowControl.outstandingSend	= 0;

    mFlowControl.batchRecvAck			= 0;
    mFlowControl.batchSendAck			= 0;

#ifdef IB_ENABLE_DEBUG
    mDEBUG_TotalEvents = 0;
    mDEBUG_TotalPostSend = 0;
    mDEBUG_TotalPostRecv = 0;
    mDEBUG_HoldBuffer = 0;
    mDEBUG_HoldRef = 0;

    mCreditUpdateSeq = 0;
#endif

    if(IB_DEFAULT_MAX_SEND_IN_FLIGHT >= 0)
    {
    	mSendRequestQueue.set_capacity(IB_DEFAULT_MAX_SEND_IN_FLIGHT);
    	mMaxSendInFlight = IB_DEFAULT_MAX_SEND_IN_FLIGHT;
    }
    else
    {
    	mMaxSendInFlight = -1;
    }

    mControlBufferQueue.set_capacity(IB_DEFAULT_CONTROL_BUFFER_COUNT);

    // create the send/receive buffer and pre-post to receive
    for(int i=0;i<IB_DEFAULT_RECEIVE_BUFFER_COUNT;++i)
    {
    	shared_ptr<Buffer> recvBuffer = createBuffer(IB_DEFAULT_RECEIVE_BUFFER_SIZE);

    	//_HOLD_BUFFER(mRecvBufferHolder, recvBuffer.get(), recvBuffer);

    	CompletionInfo* info = new CompletionInfo;
    	info->buffer = recvBuffer;

    	if(!postRecv(recvBuffer.get(), info))
    	{
    		SAFE_DELETE(info);
    	}
    }

    // create stocked buffer for control messages
    for(int i=0;i<IB_DEFAULT_CONTROL_BUFFER_COUNT;++i)
    {
    	shared_ptr<Buffer> controlBuffer = createBuffer(IB_DEFAULT_CONTROL_BUFFER_SIZE);
    	mControlBufferQueue.push(controlBuffer);
    }
}

IBConnection::~IBConnection()
{
	IB_DEBUG("DTOR");

	// stop watching on event channel and CQ completion channel
	stop();

	// force to close RDMA connection
	close();

	// shutting down Infiniband Verbs
	// NOTE we force the destruction order to make it safe (QP -> CQ -> CCHANNEL -> MR -> PD -> CM_ID -> RDMA EVENT CHANNEL)
	// TODO re-factor this code to make it more consistent
	IBFactory::destroyQueuePair(mVerbs.id.get());

	mVerbs.scq.reset();
	mVerbs.rcq.reset();

	mVerbs.cchannel.reset();

	mVerbs.id.reset();
	mVerbs.rchannel.reset();

	IB_DEBUG("DTOR_COMPLETE");
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<Buffer> IBConnection::createBuffer(size_t size)
{
	BOOST_ASSERT(mEngine->getBufferManager());

	shared_ptr<Buffer> buffer = mEngine->getBufferManager()->createBuffer(size);

	if(!buffer)
	{
		IB_ERROR("[ERROR] BufferManager returns NULL buffer! OUT OF MEMORY! Requested size = " << size);
	}

	return buffer;
}

bool IBConnection::sendThrottled(uint32 type, shared_ptr<Buffer> buffer)
{
	if(mSendThrottle.slow_down)
	{
		tbb::tick_count::interval_t t(mSendThrottle.slow_factor);
		tbb::this_tbb_thread::sleep(t);
	}

	bool halt = false;
	while(!send(type, buffer))
	{
		halt = true;

		mSendThrottle.slow_factor += mSendThrottle.slow_step;
		mSendThrottle.ok_threshold = mSendThrottle.observation_time/mSendThrottle.slow_factor;
		LOG4CXX_INFO(mLogger, "sending too fast, halt for a moment and resend, slow_factor = " << mSendThrottle.slow_factor);
		tbb::tick_count::interval_t t(mSendThrottle.slow_factor);
		tbb::this_tbb_thread::sleep(t);

		if(!mSendThrottle.slow_down)
		{
			LOG4CXX_INFO(mLogger, "enable application-level flow control");
		}
		mSendThrottle.slow_down = true;

		if(!mConnected) return false;
	}

	if(!halt && mSendThrottle.slow_down)
	{
		++mSendThrottle.ok_count;

		if(mSendThrottle.ok_count > mSendThrottle.ok_threshold)
		{
			mSendThrottle.slow_factor -= mSendThrottle.slow_step;
			mSendThrottle.ok_threshold = mSendThrottle.observation_time/mSendThrottle.slow_factor;

			LOG4CXX_INFO(mLogger, "reducing rate, slow_factor = " << mSendThrottle.slow_factor);

			if(mSendThrottle.slow_factor <= 0.0005)
			{
				LOG4CXX_INFO(mLogger, "disable application-level flow control");
				mSendThrottle.slow_down = false;
			}
			mSendThrottle.ok_count = 0;
		}
	}

	return true;
}

bool IBConnection::send(uint32 type, shared_ptr<Buffer> buffer)
{
	IB_DEBUG("send general message length = " << buffer->dataSize() << ", type = " << type << ", buffer = " << buffer.get());

	if(buffer->dataSize() == 0)
	{
		IB_ERROR("[ERROR] zero size message send requested!");
		return false;
	}

	tbb::queuing_mutex::scoped_lock lock(mSendLock);

	if(requestSend())
	{
		IB_DEBUG("send right away");
		return postGeneral(type, buffer);
	}
	else
	{
		IB_DEBUG("send queued");
		SendRequest request;
		request.req = SendRequest::GENERAL;
		request.type = type;
		request.buffer = buffer;

		bool result = mSendRequestQueue.try_push(request);

		if(!result)
		{
			IB_ERROR("[ERROR] EXCEED MAXIMUM SENDS IN FLIGHT! (IB_DEFAULT_MAX_SEND_IN_FLIGHT = " << IB_DEFAULT_MAX_SEND_IN_FLIGHT << ") (mSendRequestQueue.size() = " << mSendRequestQueue.size() << ")");
		}

		return result;
	}
}

//////////////////////////////////////////////////////////////////////////
uint64 IBConnection::registerDirect(shared_ptr<Buffer> buffer)
{
	// hold the buffer
	_HOLD_BUFFER(mRemoteDirectBufferHolder, buffer.get(), buffer);

	// send control message to tell remote about the id -> address mapping
	sendControlRegDirectBuffer(reinterpret_cast<uint64>(buffer.get()), reinterpret_cast<uint64>(buffer->wptr()));

	// return the buffer address as sink_id
	return reinterpret_cast<uint64>(buffer.get());
}

void IBConnection::unregisterDirect(uint64 sink_id)
{
	// send control message to tell remote about the id de-registeration
	sendControlUnregDirectBuffer(sink_id);

	// unhold the buffer
	Buffer* buffer = reinterpret_cast<Buffer*>(sink_id);
	_UNHOLD_BUFFER(mRemoteDirectBufferHolder, buffer);
}


bool IBConnection::write(uint64 sink, std::size_t offset, shared_ptr<Buffer> buffer, std::size_t size)
{
	bool result = true;

	// lookup the remote address by id
	uint32 rkey = mRemoteAccess.rkey;
	uint64 address = 0;
	uint64 source_id = reinterpret_cast<uint64>(buffer.get());

	_GET_REF(mRemoteDirectBufferRefHolder, sink, address);
	BOOST_ASSERT(address != 0);

	// create condition variable
	PooledConditionVariable* cv = new PooledConditionVariable;

	// create completion info
	CompletionInfo* info = new CompletionInfo;
	info->buffer = buffer;
	info->handler = boost::bind(gSignalConditionVariable, cv, _1);

	// post RDMA WRITE
	result = postWrite(address, rkey, (size == 0) ? buffer->dataSize() : size, buffer.get(), info);

	// if something goes wrong, clean up the completion info (otherwise the object is deleted in handleSendCompletionRdmaWrite)
	if(!result)
	{
		SAFE_DELETE(info);
	}
	else
	{
		// wait for completion and get the result
		int error; cv->wait(error);
		if(!error) result = false;
	}

	SAFE_DELETE(cv);

	return result;
}

bool IBConnection::writeAsync(uint64 sink, std::size_t offset, shared_ptr<Buffer> buffer, std::size_t size, CompletionHandler handler)
{
	bool result = true;

	// lookup the remote address by id
	uint32 rkey = mRemoteAccess.rkey;
	uint64 address = 0;
	uint64 source_id = reinterpret_cast<uint64>(buffer.get());

	_GET_REF(mRemoteDirectBufferRefHolder, sink, address);
	BOOST_ASSERT(address != 0);

	// create completion info
	CompletionInfo* info = new CompletionInfo;
	info->buffer = buffer;
	info->handler = handler;

	// post RDMA WRITE
	result = postWrite(address, rkey, (size == 0) ? buffer->dataSize() : size, buffer.get(), info);

	// if something goes wrong, clean up the completion info (otherwise the object is deleted in handleSendCompletionRdmaWrite)
	if(!result)
	{
		SAFE_DELETE(info);
	}

	return result;
}

bool IBConnection::read(shared_ptr<Buffer> buffer, uint64 sink, std::size_t offset, std::size_t size)
{
	bool result = true;

	// lookup the remote address by id
	uint32 rkey = mRemoteAccess.rkey;
	uint64 address = 0;
	uint64 source_id = reinterpret_cast<uint64>(buffer.get());

	_GET_REF(mRemoteDirectBufferRefHolder, sink, address);
	BOOST_ASSERT(address != 0);

	// create condition variable
	PooledConditionVariable* cv = new PooledConditionVariable;

	// create completion info
	CompletionInfo* info = new CompletionInfo;
	info->buffer = buffer;
	info->handler = boost::bind(gSignalConditionVariable, cv, _1);

	// post RDMA WRITE
	result = postRead(address, rkey, (size == 0) ? buffer->freeSize() : size, buffer.get(), info);

	// if something goes wrong, clean up the completion info (otherwise the object is deleted in handleSendCompletionRdmaRead)
	if(!result)
	{
		SAFE_DELETE(info);
	}
	else
	{
		// wait for completion and get the result
		int error; cv->wait(error);
		if(!error) result = false;
	}

	SAFE_DELETE(cv);

	return result;
}

bool IBConnection::readAsync(shared_ptr<Buffer> buffer, uint64 sink, std::size_t offset, std::size_t size, CompletionHandler handler)
{
	bool result = true;

	// lookup the remote address by id
	uint32 rkey = mRemoteAccess.rkey;
	uint64 address = 0;
	uint64 source_id = reinterpret_cast<uint64>(buffer.get());

	_GET_REF(mRemoteDirectBufferRefHolder, sink, address);
	BOOST_ASSERT(address != 0);

	// create completion info
	CompletionInfo* info = new CompletionInfo;
	info->buffer = buffer;
	info->handler = handler;

	// post RDMA WRITE
	result = postRead(address, rkey, (size == 0) ? buffer->freeSize() : size, buffer.get(), info);

	// if something goes wrong, clean up the completion info (otherwise the object is deleted in handleSendCompletionRdmaRead)
	if(!result)
	{
		SAFE_DELETE(info);
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////
void IBConnection::close()
{
	// ack all remaining events
	if(mFlowControl.batchRecvAck > 0)
	{
		IB_DEBUG("shutting down, ack remaining recv cq events (# of cq event = " << mFlowControl.batchRecvAck << ")");
		ibv_ack_cq_events(mVerbs.rcq.get(), mFlowControl.batchRecvAck);
		mFlowControl.batchRecvAck = 0;
	}
	if(mFlowControl.batchSendAck > 0)
	{
		IB_DEBUG("shutting down, ack remaining send cq events (# of cq event = " << mFlowControl.batchSendAck << ")");
		ibv_ack_cq_events(mVerbs.scq.get(), mFlowControl.batchSendAck);
		mFlowControl.batchSendAck = 0;
	}

	// clean up buffers
	mRecvBufferHolder.clear();
	printf("mRecvBufferHolder buffer holder cleared\n");
	mSendBufferHolder.clear();
	printf("mSendBufferHolder buffer holder cleared\n");

	mLocalDirectBufferHolder.clear();
	printf("mLocalDirectBufferHolder buffer holder cleared\n");
	mRemoteDirectBufferHolder.clear();
	printf("mRemoteDirectBufferHolder buffer holder cleared\n");

	mLargeBufferAckRefHolder.clear();
	printf("mLargeBufferAckRefHolder buffer holder cleared\n");

	mLocalDirectBufferRefHolder.clear();
	printf("mLocalDirectBufferRefHolder buffer holder cleared\n");
	mRemoteDirectBufferRefHolder.clear();
	printf("mRemoteDirectBufferRefHolder buffer holder cleared\n");

	mControlBufferQueue.clear();
	mSendRequestQueue.clear();

	if(mConnected)
	{
		rdma_disconnect(mVerbs.id.get());

		mConnected = false;
	}
}

//////////////////////////////////////////////////////////////////////////
void IBConnection::setMaxSendInFlight(int32 maxSendInFlight)
{
	mSendRequestQueue.set_capacity(maxSendInFlight);

	mMaxSendInFlight = maxSendInFlight;

	// re-calculate the control buffer count
	if(mMaxSendInFlight >= 0)
	{
		int nControlBufferCount = ( (mMaxSendInFlight + IB_DEFAULT_WR_ENTRIES)*2 / IB_DEFAULT_THRESHOLD_BUFFER_ACK ) + 1;
		if(nControlBufferCount > IB_DEFAULT_CONTROL_BUFFER_COUNT)
		{
			mControlBufferQueue.set_capacity(nControlBufferCount);
		}
		else
		{
			mControlBufferQueue.set_capacity(IB_DEFAULT_CONTROL_BUFFER_COUNT);
		}
	}
	else
	{
		mControlBufferQueue.set_capacity(-1);
	}

}

//////////////////////////////////////////////////////////////////////////
void IBConnection::handleDataEvent(ev::io& w, int revent)
{
#ifdef IB_ENABLE_DEBUG
	++mDEBUG_TotalEvents;
	IB_DEBUG("[DEBUG] BEGIN data handle TOTAL EVENTS = " << (uint32)mDEBUG_TotalEvents << ", TOTAL POST SEND = " << (uint32)mDEBUG_TotalPostSend << ", TOTAL POST RECV = " << (uint32)mDEBUG_TotalPostRecv);
	IB_DEBUG("[DEBUG] BEGIN data handle TOTAL BUFFER HOLD = " << (uint32)mDEBUG_HoldBuffer << ", TOTAL REF HOLD = " << (uint32)mDEBUG_HoldRef);
	//debugFlowControl();
#endif

	notifyRecv();
	notifySend();

    // find out which completion queue has the event
    ibv_cq* cq = NULL;
    void* ctx = NULL;
    bool scq_or_rcq = false;

    if(ibv_get_cq_event(mVerbs.cchannel.get(), &cq, &ctx) == -1)
    {
    	if(errno == EAGAIN)
    		return;

    	// result is not EAGAIN, faulty result!
    	IB_DEBUG("fail to get completion queue event, errno = " << errno);
    }

    // batch acknowledge the event when number of events exceed half of the CQ size
    if(cq == mVerbs.scq.get())
    {
    	scq_or_rcq = true;
        if(++mFlowControl.batchSendAck > IB_DEFAULT_THRESHOLD_CQ_ACK)
        {
            ibv_ack_cq_events(cq, mFlowControl.batchSendAck);
            mFlowControl.batchSendAck = 0;
        }
    }
    else if(cq == mVerbs.rcq.get())
    {
    	scq_or_rcq = false;
        if(++mFlowControl.batchRecvAck > IB_DEFAULT_THRESHOLD_CQ_ACK)
        {
            ibv_ack_cq_events(cq, mFlowControl.batchRecvAck);
            mFlowControl.batchRecvAck = 0;
        }
    }
    else
    {
    	BOOST_ASSERT(cq == mVerbs.scq.get() || cq == mVerbs.rcq.get());
    }

    // notify completion queue before any further processing (so that we will be acked upon next completion happened)
    if(scq_or_rcq) ibv_req_notify_cq(mVerbs.scq.get(), 0);
    else           ibv_req_notify_cq(mVerbs.rcq.get(), 0);

    bool emptyCQ = true;
    int res = 0;
    while(true)
    {
        ibv_wc wc;
        if(scq_or_rcq)
        {
        	if((res = ibv_poll_cq(mVerbs.scq.get(), 1, &wc)) != 1) break;
        }
        else
        {
        	if((res = ibv_poll_cq(mVerbs.rcq.get(), 1, &wc)) != 1) break;
        }

        emptyCQ = false;

        if(wc.status != IBV_WC_SUCCESS)
        {
            // errorCallback(*this);
            // TODO Probably need to flush queues at this point or just disconnect here
#ifdef IB_ENABLE_DEBUG
        	IB_ERROR("[ERROR] FAILED TO POLL CQ, STATUS = " << ibv_wc_status_str(wc.status));
#endif
        	/*
        	int flush_num = 0;
            if(scq_or_rcq)
            {
            	while((flush_num = ibv_poll_cq(mVerbs.scq.get(), IB_DEFAULT_CQ_ENTRIES, &wc)) > 0)
            	{
            		IB_ERROR("[ERROR] FLUSH " << flush_num << " CQ ENTRIES");
            	}
            	break;
            }
            else
            {
            	while((flush_num = ibv_poll_cq(mVerbs.rcq.get(), IB_DEFAULT_CQ_ENTRIES, &wc)) > 0)
            	{
            		IB_ERROR("[ERROR] FLUSH " << flush_num << " CQ ENTRIES");
            	}
            	break;
            }*/
        	continue;
        }

        if(scq_or_rcq)
        {
#ifdef IB_ENABLE_DEBUG
        	--mDEBUG_TotalPostSend;
#endif

#ifdef IB_ENABLE_ORDERING_CHECK
        	uint64 wr_id_in_queue = 0;
        	mPostSendOrderQueue.pop(wr_id_in_queue);
        	if(wr_id_in_queue != wc.wr_id)
        	{
        		IB_ERROR("[ERROR] POST SEND NOT IN ORDER! EXPECT " << wr_id_in_queue << " BUT GOT " << wc.wr_id);
        	}
#endif
        	// for SEND completion
        	if(wc.opcode == IBV_WC_RDMA_WRITE)
        	{
        		// for RDMA WRITE completion
        		handleSendCompletionRdmaWrite(wc);
        	}
        	else
        	{
				if(wc.opcode == IBV_WC_RDMA_READ)
				{
					// for RDMA READ completion
					handleSendCompletionRdmaRead(wc);
				}
				else
				{
					if(wc.wc_flags & IBV_WC_WITH_IMM)
					{
						// for GENERAL SEND completion
						handleSendCompletionGeneralBuffer(wc);
					}
					else
					{
						// for CONTROL MESSAGE SEND completion
						handleSendCompletionControlBuffer(wc);
					}
				}
        	}

        	{
				tbb::queuing_mutex::scoped_lock lock(mSendLock);

				--mFlowControl.outstandingSend;

				if(mFlowControl.outstandingSend < IB_DEFAULT_WR_ENTRIES / 2)
				{
					flushSendRequestQueue();
				}
        	}
        }
        else
        {
        	// for RECEIVE completion
#ifdef IB_ENABLE_DEBUG
        	--mDEBUG_TotalPostRecv;
#endif

#ifdef IB_ENABLE_ORDERING_CHECK
        	uint64 wr_id_in_queue = 0;
        	mPostRecvOrderQueue.pop(wr_id_in_queue);
        	if(wr_id_in_queue != wc.wr_id)
        	{
        		IB_ERROR("[ERROR] POST RECV NOT IN ORDER! EXPECT " << wr_id_in_queue << " BUT GOT " << wc.wr_id);
        	}
#endif

        	++mFlowControl.recvCredit;
        	sendCreditIfNecessary();

    		// if the RECEIVE completion comes with IMM, it's a general message; otherwise it's a control message
        	if(wc.wc_flags & IBV_WC_WITH_IMM)
        	{
       			handleRecvCompletionGeneralBuffer(wc);
        	}
        	else
        	{
        		handleRecvCompletionControlBuffer(wc);
        	}
        }
    }

    if(emptyCQ)
    {
    	IB_DEBUG("[BOGOS SPURIOUS COMPLETION] EMPTY CQ, RES = " << res);
    }

#ifdef IB_ENABLE_DEBUG
    IB_DEBUG("[DEBUG] AFTER data handle TOTAL EVENTS = " << (uint32)mDEBUG_TotalEvents << ", TOTAL POST SEND = " << (uint32)mDEBUG_TotalPostSend << ", TOTAL POST RECV = " << (uint32)mDEBUG_TotalPostRecv);
    IB_DEBUG("[DEBUG] AFTER data handle TOTAL BUFFER HOLD = " << (uint32)mDEBUG_HoldBuffer << ", TOTAL REF HOLD = " << (uint32)mDEBUG_HoldRef);
    //debugFlowControl();
#endif
}

void IBConnection::handleChannelEvent(ev::io& w, int revent)
{
	int err = 0;
	rdma_cm_event* event = NULL;
	err = rdma_get_cm_event(mVerbs.rchannel.get(), &event);

	switch(event->event)
	{
		case RDMA_CM_EVENT_DISCONNECTED:
		{
			shared_ptr<IBConnection> shared_from_this(mWeakThis);

			err = rdma_ack_cm_event(event);

			// stop this connection
			stop();
			// get the buffer reference from wr_id
			// set to disconnected state (we cannot call rdma_disconnect here, otherwise we would have a corrupted heap!!!)
			close();

			//err = rdma_ack_cm_event(event);

			// remove itself from NetEngine
			mEngine->removeConnection(shared_from_this);

			// notify upper user
			mEngine->getDispatcher()->dispatchConnectionEvent(IBDispatcher::DISCONNECTED, shared_from_this);

			break;
		}
		default:
		{
			IB_DEBUG("capture miscellous event " << rdma_event_str(event->event));

			err = rdma_ack_cm_event(event);

			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void IBConnection::handleSendCompletionRdmaWrite(const ibv_wc &wc)
{
	// get the completion info from wr_id
	CompletionInfo* info = reinterpret_cast<CompletionInfo*>(wc.wr_id);

	if(wc.status == IBV_WC_SUCCESS)
	{
		// skip read pointer by number of bytes written
		info->buffer->rskip(wc.byte_len);

		// invoke the completion handler if necessary
		if(!info->handler.empty())	info->handler(1);
	}
	else
	{
		// invoke the completion handler if necessary
		if(!info->handler.empty())	info->handler(0);
	}

	// delete the completion info object
	SAFE_DELETE(info);
}

void IBConnection::handleSendCompletionRdmaRead(const ibv_wc &wc)
{
	// get the completion info from wr_id
	CompletionInfo* info = reinterpret_cast<CompletionInfo*>(wc.wr_id);

	if(wc.status == IBV_WC_SUCCESS)
	{
		// skip read pointer by number of bytes written
		info->buffer->wskip(wc.byte_len);

		// invoke the completion handler if necessary
		if(!info->handler.empty())	info->handler(1);
	}
	else
	{
		// invoke the completion handler if necessary
		if(!info->handler.empty())	info->handler(0);
	}

	// delete the completion info object
	SAFE_DELETE(info);
}

void IBConnection::handleSendCompletionControlBuffer(const ibv_wc &wc)
{
	// get the completion info from wr_id
	CompletionInfo* info = reinterpret_cast<CompletionInfo*>(wc.wr_id);

	Buffer* b = reinterpret_cast<Buffer*>(wc.wr_id);

	IB_DEBUG("[COMPLETION] [SEND] CONTROL BUFFER SEND, buffer = " << b);

	// push the control buffer back to "stocked control buffer container"
	returnControlBuffer(info->buffer);

	// delete the completion info object
	SAFE_DELETE(info);
}

void IBConnection::handleSendCompletionGeneralBuffer(const ibv_wc &wc)
{
	// get the completion info from wr_id
	CompletionInfo* info = reinterpret_cast<CompletionInfo*>(wc.wr_id);

	//IB_DEBUG("[COMPLETION] [SEND] GENERAL BUFFER SEND, buffer = " << b);

#ifdef IB_ENABLE_COMPLETION_DISPATCH
	// dispatch completion
	shared_ptr<Buffer> sb;

	_GET_AND_UNHOLD_BUFFER(mSendBufferHolder, b, sb);
	shared_ptr<IBConnection> shared_from_this(mWeakThis);

	mEngine->getDispatcher()->dispatchCompletion(sb, shared_from_this);
#else
	// remove the buffer from "send buffer reference holder"
    //_UNHOLD_BUFFER(mSendBufferHolder, b);
#endif
	// delete the completion info object
	SAFE_DELETE(info);
}

void IBConnection::handleRecvCompletionControlBuffer(const ibv_wc &wc)
{
	// get the buffer reference from wr_id
	CompletionInfo* info = reinterpret_cast<CompletionInfo*>(wc.wr_id);

    info->buffer->rpos(0); info->buffer->wpos(wc.byte_len);

    IB_DEBUG("[COMPLETION] [RECV] CONTROL BUFFER RECV, buffer = " << b);

	// NOTE the type of control message is embedded in the buffer itself, but general buffer are using imm field to store buffer types
	uint32 type; info->buffer->read(type);
	switch(type)
	{
		case CONTROL_CREDIT_UPDATE:
		{
			handleControlCreditUpdate(info->buffer.get());
			break;
		}
		case CONTROL_LARGE_BUFFER_SEND:
		{
			handleControlLargeBufferSend(info->buffer.get());
			break;
		}
		case CONTROL_LARGE_BUFFER_ACK:
		{
			handleControlLargeBufferAck(info->buffer.get());
			break;
		}
		case CONTROL_ACCESS_EXCHANGE:
		{
			handleControlAccessExchange(info->buffer.get());
			break;
		}
		case CONTROL_SEND_DIRECT_SIGNAL:
		{
			handleControlSendDirectSignal(info->buffer.get());
			break;
		}
		case CONTROL_REG_DIRECT_BUFFER:
		{
			handleControlRegDirectBuffer(info->buffer.get());
			break;
		}
		case CONTROL_UNREG_DIRECT_BUFFER:
		{
			handleControlUnregDirectBuffer(info->buffer.get());
			break;
		}
		default:
		{
			LOG4CXX_FATAL(mLogger, "receive unknown control message type = " << type);
		}
	}

	// as we know all control buffer handler will not hold the buffer for future use, so here we reuse the buffer
	info->buffer->clear();
	postRecv(info->buffer.get(), info);
}

void IBConnection::handleRecvCompletionGeneralBuffer(const ibv_wc &wc)
{
	// get the buffer reference from wr_id
	CompletionInfo* info = reinterpret_cast<CompletionInfo*>(wc.wr_id);

    info->buffer->rpos(0); info->buffer->wpos(wc.byte_len);

    IB_DEBUG("[COMPLETION] [RECV] GENERAL BUFFER RECV, buffer = " << b);

	// dispatch buffer through IBDispatcher
    shared_ptr<IBConnection> shared_from_this(mWeakThis);

	// NOTE we store the type of general buffer in the imm field (in the host order but not in network order as we assume all machines are in little endian format)
	uint32 type = wc.imm_data;
	mEngine->getDispatcher()->dispatchDataEvent(type, info->buffer, shared_from_this);

	// if the buffer is still referenced by upper application, create another buffer to post receive
	if(info->buffer.use_count() > 2L)
	{
		IB_DEBUG("[COMPLETION] [RECV] buffer dispatched but still in use, create new buffer (use count = " << info->buffer.use_count() << ")");

		// create another buffer
		shared_ptr<Buffer> newBuffer = createBuffer(IB_DEFAULT_RECEIVE_BUFFER_SIZE);

		// replace the buffer
		info->buffer = newBuffer;

		// post to receive
		postRecv(newBuffer.get(), info);
	}
	else
	{
		IB_DEBUG("[COMPLETION] [RECV] buffer dispatched and reused");
		// reuse the buffer by reseting and re-posting it
		info->buffer->clear();
		postRecv(info->buffer.get(), info);
	}
}

//////////////////////////////////////////////////////////////////////////
void IBConnection::start(shared_ptr<Poller> poller)
{
	if(mWatchersStarted)
	{
		mPoller->stop(&mWatchers.dataRead);
		mPoller->stop(&mWatchers.channelRead);
		mWatchersStarted = false;
	}

	mPoller = poller;

	// create event watchers on RDMA event channel and CQ completion event channel
	mWatchers.dataRead.set(mVerbs.cchannel->fd, ev::READ);
	mWatchers.dataRead.set<IBConnection, &IBConnection::handleDataEvent>(this);

	mWatchers.channelRead.set(mVerbs.rchannel->fd, ev::READ);
	mWatchers.channelRead.set<IBConnection, &IBConnection::handleChannelEvent>(this);

	mPoller->start(&mWatchers.dataRead);
	mPoller->start(&mWatchers.channelRead);

	mWatchersStarted = true;
}

void IBConnection::stop()
{
	if(mWatchersStarted)
	{
		// TODO we need to place a callback here so that we can proceed to disconnect or object cleanup after poller stop watching
		// TODO use some signal object here
		mPoller->stop(&mWatchers.dataRead);
		mPoller->stop(&mWatchers.channelRead);

		mWatchersStarted = false;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool IBConnection::postGeneral(uint32 type, shared_ptr<Buffer> buffer)
{
	bool result = true;

	IB_DEBUG("[POST GENERAL] hold buffer, buffer.get() = " << buffer.get() << ", dataSize = " << buffer->dataSize());

	// create completion info
	CompletionInfo* info = new CompletionInfo;
	info->buffer = buffer;

	// send the buffer
	// if the size of the buffer can be fitted into receiver's pre-posted buffer, just send it
	if(buffer->dataSize() <= IB_DEFAULT_RECEIVE_BUFFER_SIZE)
	{
		// post the SEND WR
		result = postSend(buffer.get(), type, info);
	}
	// otherwise we have to do our own large buffer send algorithm
	else
	{
		// send the large buffer send control message
		result = postControlLargeBufferSend(buffer.get(), type);
	}

	// if something goes wrong, unhold the buffer
	if(!result)
	{
		IB_ERROR("[ERROR] [POST GENERAL] fail to post send with IMM, unhold the buffer");

		SAFE_DELETE(info);

		return false;
	}

	return true;
}

bool IBConnection::sendControl(shared_ptr<Buffer> buffer)
{
	if(requestSend())
	{
		postControl(buffer);
	}
	else
	{
		SendRequest request;
		request.req = SendRequest::CONTROL;
		request.buffer = buffer;
		return mSendRequestQueue.try_push(request);
	}
}

bool IBConnection::postControl(shared_ptr<Buffer> buffer)
{
	bool result = true;

	IB_DEBUG("[POST CONTROL] hold buffer, buffer.get() = " << buffer.get());

	// create completion info
	CompletionInfo* info = new CompletionInfo;
	info->buffer = buffer;

	// post send without imm directly
	result = postSend(buffer.get(), info);

	// if something goes wrong, unhold the buffer
	if(!result)
	{
		IB_ERROR("[ERROR] [POST CONTROL] fail to post send, unhold the buffer");

		SAFE_DELETE(info);

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool IBConnection::postRecv(Buffer* buffer, CompletionInfo* info)
{
	tbb::queuing_mutex::scoped_lock lock(mVerbs.recv_lock);

	IB_DEBUG("post RECV for buffer = " << buffer << ", free size = " << buffer->freeSize());

    ibv_recv_wr rwr = {};
    ibv_sge sge;

    sge.addr   = reinterpret_cast<uint64_t>(buffer->rptr());
    sge.length = buffer->freeSize();
    sge.lkey   = mDeviceResource->getGlobalMemoryRegion()->lkey;

    rwr.wr_id   = reinterpret_cast<uint64_t>(info);
    rwr.sg_list = &sge;
    rwr.num_sge = 1;

    ibv_recv_wr* bad_recv_wr = NULL;
    int err = ibv_post_recv(mVerbs.id->qp, &rwr, &bad_recv_wr);

    if(err)
    {
    	IB_ERROR("[ERROR] error while posting RECV, err=" << err << ", addr=" << sge.addr << ", length=" << sge.length << ", lkey=" << sge.lkey << ", wr_id=" << rwr.wr_id);
    	return false;
    }

    if(bad_recv_wr)
    {
    	IB_ERROR("[ERROR] got bad_send_wr when post receive, err=" << err << ", addr=" << sge.addr << ", length=" << sge.length << ", lkey=" << sge.lkey << ", wr_id=" << rwr.wr_id);
    	return false;
    }

#ifdef IB_ENABLE_ORDERING_CHECK
    mPostRecvOrderQueue.push(rwr.wr_id);
#endif

#ifdef IB_ENABLE_DEBUG
    ++mDEBUG_TotalPostRecv;
#endif

	return true;
}

bool IBConnection::postSend(Buffer* buffer, CompletionInfo* info)
{
	++mFlowControl.outstandingSend;

	tbb::queuing_mutex::scoped_lock lock(mVerbs.send_lock);

	IB_DEBUG("post SEND for buffer = " << buffer << ", data size = " << buffer->dataSize());

    ibv_send_wr swr = {};
    ibv_sge sge;

    sge.addr   = reinterpret_cast<uint64_t>(buffer->rptr());
    sge.length = buffer->dataSize();
    sge.lkey   = mDeviceResource->getGlobalMemoryRegion()->lkey;

    swr.wr_id      = reinterpret_cast<uint64_t>(info);
    swr.opcode     = IBV_WR_SEND;
    swr.send_flags = IBV_SEND_SIGNALED;
    swr.sg_list    = &sge;
    swr.num_sge    = 1;

    ibv_send_wr* bad_send_wr = 0;
    int err = ibv_post_send(mVerbs.id->qp, &swr, &bad_send_wr);

    if(err)
    {
    	IB_ERROR("[ERROR] error while posting SEND, err=" << err << ", addr=" << sge.addr << ", length=" << sge.length << ", lkey=" << sge.lkey << ", wr_id=" << swr.wr_id);
    	return false;
    }

    if(bad_send_wr)
    {
    	IB_ERROR("[ERROR] got bad_send_wr when post SEND, err = " << err << ", addr = " << sge.addr << ", length = " << sge.length << ", lkey = " << sge.lkey << ", wr_id = " << swr.wr_id);
    	return false;
    }

#ifdef IB_ENABLE_ORDERING_CHECK
    mPostSendOrderQueue.push(swr.wr_id);
#endif

#ifdef IB_ENABLE_DEBUG
    ++mDEBUG_TotalPostSend;
#endif

    return true;
}

bool IBConnection::postSend(Buffer* buffer, uint32 imm, CompletionInfo* info)
{
	++mFlowControl.outstandingSend;

	tbb::queuing_mutex::scoped_lock lock(mVerbs.send_lock);

	IB_DEBUG("post SEND with IMM for buffer = " << buffer << ", data size = " << buffer->dataSize() << ", imm = " << imm);

    ibv_send_wr swr = {};
    ibv_sge sge;

    sge.addr   = reinterpret_cast<uint64_t>(buffer->rptr());
    sge.length = buffer->dataSize();
    sge.lkey   = mDeviceResource->getGlobalMemoryRegion()->lkey;

    swr.wr_id      = reinterpret_cast<uint64_t>(buffer);
    swr.opcode     = IBV_WR_SEND_WITH_IMM;
    swr.send_flags = IBV_SEND_SIGNALED;
    swr.sg_list    = &sge;
    swr.num_sge    = 1;
    swr.imm_data   = imm;

    ibv_send_wr* bad_send_wr = 0;
    int err = ibv_post_send(mVerbs.id->qp, &swr, &bad_send_wr);

    if(err)
    {
    	IB_ERROR("[ERROR] error while posting SEND with IMM, err = " << err << ", addr = " << sge.addr << ", length = " << sge.length << ", lkey = " << sge.lkey << ", wr_id = " << swr.wr_id << ", imm = " << swr.imm_data);
    	return false;
    }

    if(bad_send_wr)
    {
    	IB_ERROR("[ERROR] got bad_send_wr when post SEND WITH IMM, err = " << err << ", addr = " << sge.addr << ", length = " << sge.length << ", lkey = " << sge.lkey << ", wr_id = " << swr.wr_id << ", imm = " << swr.imm_data);
    	return false;
    }

#ifdef IB_ENABLE_ORDERING_CHECK
    mPostSendOrderQueue.push(swr.wr_id);
#endif

#ifdef IB_ENABLE_DEBUG
    ++mDEBUG_TotalPostSend;
#endif

    return true;
}

bool IBConnection::postRead(uint64 address, uint32 rkey, uint32 length, Buffer* buffer, CompletionInfo* info)
{
	++mFlowControl.outstandingSend;

	tbb::queuing_mutex::scoped_lock lock(mVerbs.send_lock);

	IB_DEBUG("post RDMA READ for buffer = " << buffer << ", data size = " << length);

    ibv_send_wr swr = {};
    ibv_sge sge;

    BOOST_ASSERT(length <= buffer->freeSize());

    sge.addr   = reinterpret_cast<uint64_t>(buffer->rptr());
    sge.length = length;
    sge.lkey   = mDeviceResource->getGlobalMemoryRegion()->lkey;

    swr.wr_id      = reinterpret_cast<uint64_t>(info);
    swr.opcode     = IBV_WR_RDMA_READ;
    swr.send_flags = IBV_SEND_SIGNALED;
    swr.sg_list    = &sge;
    swr.num_sge    = 1;

    swr.wr.rdma.remote_addr = address;
    swr.wr.rdma.rkey = rkey;

    ibv_send_wr* bad_send_wr = 0;
    int err = ibv_post_send(mVerbs.id->qp, &swr, &bad_send_wr);

    if(err)
    {
    	IB_ERROR("[ERROR] error while posting RDMA READ, err = " << err << ", addr = " << sge.addr << ", length = " << sge.length << ", lkey = " << sge.lkey << ", wr_id = " << swr.wr_id << ", remote_addr = " << swr.wr.rdma.remote_addr << ", rkey = " << swr.wr.rdma.rkey);
    	return false;
    }

    if(bad_send_wr)
    {
    	IB_ERROR("[ERROR] got bad_send_wr when post RDMA READ, err = " << err << ", addr = " << sge.addr << ", length = " << sge.length << ", lkey = " << sge.lkey << ", wr_id = " << swr.wr_id << ", remote_addr = " << swr.wr.rdma.remote_addr << ", rkey = " << swr.wr.rdma.rkey);
    	return false;
    }

#ifdef IB_ENABLE_ORDERING_CHECK
    mPostSendOrderQueue.push(swr.wr_id);
#endif

#ifdef IB_ENABLE_DEBUG
    ++mDEBUG_TotalPostSend;
#endif

    return true;
}

bool IBConnection::postWrite(uint64 address, uint32 rkey, uint32 length, Buffer* buffer, CompletionInfo* info)
{
	// TODO check if we need to use signal and remove source_id
	++mFlowControl.outstandingSend;

	tbb::queuing_mutex::scoped_lock lock(mVerbs.send_lock);

	IB_DEBUG("post RDMA WRITE for buffer = " << buffer << ", data size = " << length);

    ibv_send_wr swr = {};
    ibv_sge sge;

    BOOST_ASSERT(length <= buffer->dataSize());

    sge.addr   = reinterpret_cast<uint64_t>(buffer->rptr());
    sge.length = length;
    sge.lkey   = mDeviceResource->getGlobalMemoryRegion()->lkey;

    swr.wr_id      = reinterpret_cast<uint64_t>(info);
    swr.opcode     = IBV_WR_RDMA_WRITE;
    swr.send_flags = IBV_SEND_SIGNALED;
    swr.sg_list    = &sge;
    swr.num_sge    = 1;

    swr.wr.rdma.remote_addr = address;
    swr.wr.rdma.rkey = rkey;

    ibv_send_wr* bad_send_wr = 0;
    int err = ibv_post_send(mVerbs.id->qp, &swr, &bad_send_wr);

    if(err)
    {
    	IB_ERROR("[ERROR] error while posting RDMA WRITE, err = " << err << ", addr = " << sge.addr << ", length = " << sge.length << ", lkey = " << sge.lkey << ", wr_id = " << swr.wr_id << ", remote_addr = " << swr.wr.rdma.remote_addr << ", rkey = " << swr.wr.rdma.rkey);
    	return false;
    }

    if(bad_send_wr)
    {
    	IB_ERROR("[ERROR] got bad_send_wr when post RDMA WRITE, err = " << err << ", addr = " << sge.addr << ", length = " << sge.length << ", lkey = " << sge.lkey << ", wr_id = " << swr.wr_id  << ", remote_addr = " << swr.wr.rdma.remote_addr << ", rkey = " << swr.wr.rdma.rkey);
    	return false;
    }

#ifdef IB_ENABLE_ORDERING_CHECK
    mPostSendOrderQueue.push(swr.wr_id);
#endif

#ifdef IB_ENABLE_DEBUG
    ++mDEBUG_TotalPostSend;
#endif

    return true;
}

//////////////////////////////////////////////////////////////////////////
void IBConnection::notifyRecv()
{
	int err = ibv_req_notify_cq(mVerbs.rcq.get(), 0);

	if(err)
	{
		IB_ERROR("[ERROR] fail to notify receive completion queue, err = " << err);
	}
}

void IBConnection::notifySend()
{
	int err = ibv_req_notify_cq(mVerbs.scq.get(), 0);

	if(err)
	{
		IB_ERROR("[ERROR] fail to notify send completion queue, err = " << err);
	}
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<Buffer> IBConnection::getControlBuffer(bool blocking)
{
	if(blocking)
	{
		shared_ptr<Buffer> b;
		IB_DEBUG("[GET CONTROL BUFFER] get buffer (blocking), current buffer container size = " << mControlBufferQueue.size());
		mControlBufferQueue.pop(b);
		return b;
	}
	else
	{
		shared_ptr<Buffer> b;
		IB_DEBUG("[GET CONTROL BUFFER] get buffer, current buffer container size = " << mControlBufferQueue.size());
		bool bufferAvailable = mControlBufferQueue.try_pop(b);
		if(!bufferAvailable)
		{
			if(mMaxSendInFlight < 0)
			{
				// if the mMaxSendInFlight < 0, which means we have infinite send request queue,
				// we also need infinite control buffer queue, so create a new buffer here
				LOG4CXX_DEBUG(mLogger, "[GET CONTROL BUFFER] CREATE NEW CONTROL BUFFER");

				b = createBuffer(IB_DEFAULT_CONTROL_BUFFER_SIZE);
				if(!b)
				{
					LOG4CXX_FATAL(mLogger, "[GET CONTROL BUFFER] OUT OF MEMORY");
				}
			}
			else
			{
				LOG4CXX_FATAL(mLogger, "[GET CONTROL BUFFER] OUT OF BUFFER");
			}
		}
		return b;
	}
}

void IBConnection::returnControlBuffer(shared_ptr<Buffer> buffer)
{
	mControlBufferQueue.try_push(buffer);
}

//////////////////////////////////////////////////////////////////////////
void IBConnection::forceAccessExchange()
{
	shared_ptr<ibv_mr> gmr = mDeviceResource->getGlobalMemoryRegion();
	sendControlAccessExchange(reinterpret_cast<uint64>(gmr->addr), gmr->length, gmr->rkey);
}

bool IBConnection::sendControlAccessExchange(uint64 address, uint32 length, uint32 rkey)
{
	shared_ptr<Buffer> b = getControlBuffer();
	if(!b.get())
	{
		IB_ERROR("[ERROR] [SEND CONTROL] [ACCESS EXCHANGE] => FAILED, BUFFER NOT AVAILABLE");
		return false;
	}

	ControlAccessExchange accessExchange;
	accessExchange.address = address;
	accessExchange.length  = length;
	accessExchange.rkey    = rkey;

	b->clear();
	b->write(CONTROL_ACCESS_EXCHANGE);
	b->writeAny(accessExchange);

	IB_DEBUG("[SEND CONTROL] [ACCESS EXCHANGE] => access exchange info: address = " << accessExchange.address << ", length = " << accessExchange.length << ", rkey = " << accessExchange.rkey);

	return sendControl(b);
}

void IBConnection::handleControlAccessExchange(Buffer* buffer)
{
	ControlAccessExchange accessExchange;
	buffer->readAny(accessExchange);

	IB_DEBUG("[HANDLE CONTROL] [ACCESS EXCHANGE] => access exchange info: address = " << accessExchange.address << ", length = " << accessExchange.length << ", rkey = " << accessExchange.rkey);

	mRemoteAccess.address = accessExchange.address;
	mRemoteAccess.length = accessExchange.length;
	mRemoteAccess.rkey = accessExchange.rkey;
}

//////////////////////////////////////////////////////////////////////////
bool IBConnection::postControlLargeBufferSend(Buffer* largeBuffer, uint32 type)
{
	shared_ptr<Buffer> b = getControlBuffer();
	if(!b.get())
	{
		IB_ERROR("[ERROR] [SEND CONTROL] [LARGE BUFFER SEND] => FAILED, BUFFER NOT AVAILABLE");
		return false;
	}

	ControlLargeBufferSend send;
	send.id = reinterpret_cast<uint64_t>(largeBuffer);
	send.address = reinterpret_cast<uint64_t>(largeBuffer->rptr());
	send.rkey = mDeviceResource->getGlobalMemoryRegion()->rkey;
	send.length = largeBuffer->dataSize();
	send.type = type;

	b->clear();
	b->write(CONTROL_LARGE_BUFFER_SEND);
	b->writeAny(send);

	IB_DEBUG("[SEND CONTROL] [LARGE BUFFER SEND] => send info: id = " << send.id << ", address = " << send.address << ", rkey = " << send.rkey << ", length = " << send.length << ", type = " << send.type);

	return postControl(b);
}

void IBConnection::handleControlLargeBufferSend(Buffer* buffer)
{
	bool result = true;

	ControlLargeBufferSend send;
	buffer->readAny(send);

	IB_DEBUG("[HANDLE CONTROL] [LARGE BUFFER SEND] => large buffer send: id = " << send.id << ", address = " << send.address << ", rkey = " << send.rkey << ", length = " << send.length);

	CompletionInfo* info = new CompletionInfo;
	info->buffer = createBuffer(send.length);

	if(!info->buffer)
	{
		IB_ERROR("[ERROR] [HANDLE CONTROL] [LARGE BUFFER SEND] => fail to create large buffer for RDMA READ!");
		SAFE_DELETE(info);
		return;
	}

	// post RDMA read to poll buffer from remote
	result = postRead(send.address, send.rkey, send.length, info->buffer.get(), info);

	// check if anything goes wrong
	if(!result)
	{
		IB_ERROR("[ERROR] [HANDLE CONTROL] [LARGE BUFFER SEND] => fail to post RDMA READ!");

		SAFE_DELETE(info);
	}
}

bool IBConnection::sendControlLargeBufferAck(uint64 id, int32 result)
{
	shared_ptr<Buffer> b = getControlBuffer();
	if(!b)
	{
		IB_ERROR("[ERROR] [SEND CONTROL] [LARGE BUFFER ACK] => FAILED, BUFFER NOT AVAILABLE");
		return false;
	}

	ControlLargeBufferAck ack;
	ack.id = id; ack.result = result;

	b->clear();
	b->write(CONTROL_LARGE_BUFFER_ACK);
	b->writeAny(ack);

	IB_DEBUG("[SEND CONTROL] [LARGE BUFFER ACK] => ack info: id = " << ack.id << ", result = " << ack.result);

	return sendControl(b);
}

void IBConnection::handleControlLargeBufferAck(Buffer* buffer)
{
	ControlLargeBufferAck ack;
	buffer->readAny(ack);

	IB_DEBUG("[HANDLE CONTROL] [LARGE BUFFER SEND] => ack info: id = " << ack.id << ", result = " << ack.result);

	Buffer* b = reinterpret_cast<Buffer*>(ack.id);

	// unhold the previous created large buffer
	_UNHOLD_BUFFER(mSendBufferHolder, b);
}

//////////////////////////////////////////////////////////////////////////
bool IBConnection::sendControlRegDirectBuffer(uint64 sink_id, uint64 address)
{
	shared_ptr<Buffer> b = getControlBuffer();
	if(!b.get())
	{
		IB_ERROR("[SEND CONTROL] [REG DIRECT BUFFER] => FAILED, BUFFER NOT AVAILABLE");
		return false;
	}

	ControlRegDirectBuffer reg;
	reg.sink_id = sink_id;
	reg.address = address;

	b->clear();
	b->write(CONTROL_REG_DIRECT_BUFFER);
	b->writeAny(reg);

	IB_DEBUG("[SEND CONTROL] [REG DIRECT BUFFER] => direct buffer info: id = " << reg.sink_id << ", address = " << reg.address);

	return sendControl(b);
}

void IBConnection::handleControlRegDirectBuffer(Buffer* buffer)
{
	ControlRegDirectBuffer reg;
	buffer->readAny(reg);

	IB_DEBUG("[HANDLE CONTROL] [REG DIRECT BUFFER] => direct buffer info: sink_id = " << reg.sink_id << ", address = " << reg.address);

	// insert the direct buffer reference id
	_HOLD_REF(mRemoteDirectBufferRefHolder, reg.sink_id, reg.address);
}

bool IBConnection::sendControlUnregDirectBuffer(uint64 sink_id)
{
	IB_DEBUG("[SEND CONTROL] [UNREG DIRECT BUFFER]");

	shared_ptr<Buffer> b = getControlBuffer();
	if(!b.get())
	{
		IB_ERROR("[ERROR] [SEND CONTROL] [UNREG DIRECT BUFFER] => FAILED, BUFFER NOT AVAILABLE");
		return false;
	}

	IB_DEBUG("[HANDLE CONTROL] [CREDIT UPDATE] => BEGIN process SendRequestQueue, xmitCredit = " << mFlowControl.xmitCredit);
	while(!mSendRequestQueue.empty())
	{
		if(requestSend())
		{
			SendRequest request;
			mSendRequestQueue.pop(request);

			switch(request.req)
			{
				case SendRequest::GENERAL:
				{
					IB_DEBUG("[HANDLE CONTROL] [CREDIT UPDATE] => Process GENERAL request, xmitCredit = " << mFlowControl.xmitCredit << ", type = " << request.type << ", buffer = " << request.buffer.get());
					postGeneral(request.type, request.buffer);
					break;
				}
				/*
				case SendRequest::DIRECT:
				{
					IB_DEBUG("[HANDLE CONTROL] [CREDIT UPDATE] => Process DIRECT request, xmitCredit = " << mFlowControl.xmitCredit << ", type = " << request.type << ", buffer = " << request.buffer.get() << ", sink_id = " << request.sink_id);
					postDirect(request.type, request.buffer, request.sink_id);
					break;
				}
				*/
				case SendRequest::CONTROL:
				{
					IB_DEBUG("[HANDLE CONTROL] [CREDIT UPDATE] => Process CONTROL request, xmitCredit = " << mFlowControl.xmitCredit << ", buffer = " << request.buffer.get());
					postControl(request.buffer);
					break;
				}
			}
		}
		else
		{
			break;
		}
	}
	IB_DEBUG("[HANDLE CONTROL] [CREDIT UPDATE] => END process SendRequestQueue, xmitCredit = " << mFlowControl.xmitCredit);
	ControlUnregDirectBuffer unreg;
	unreg.sink_id = sink_id;

	b->clear();
	b->write(CONTROL_UNREG_DIRECT_BUFFER);
	b->writeAny(unreg);

	IB_DEBUG("[SEND CONTROL] [UNREG DIRECT BUFFER] => direct buffer info: id = " << unreg.sink_id);

	return sendControl(b);
}

void IBConnection::handleControlUnregDirectBuffer(Buffer* buffer)
{
	ControlUnregDirectBuffer unreg;
	buffer->readAny(unreg);

	IB_DEBUG("[HANDLE CONTROL] [UNREG DIRECT BUFFER] direct buffer info: sink_id = " << unreg.sink_id);

	// remove the direct buffer reference id
	_UNHOLD_REF(mRemoteDirectBufferRefHolder, unreg.sink_id);
}

bool IBConnection::sendControlSendDirectSignal(uint64 source_id, uint64 sink_id, uint32 length, uint32 type)
{
	bool result = true;

	// send control message to signal remote about the completion (by id)
	shared_ptr<Buffer> b = getControlBuffer();
	if(!b.get())
	{
		IB_ERROR("[ERROR] [SendDirectImpl] => FAILED, CONTROL BUFFER NOT AVAILABLE");
		return false;
	}
	ControlSendDirectSignal signal;
	signal.source_id = source_id;
	signal.sink_id = sink_id;
	signal.length = length;
	signal.type = type;

	b->clear();
	b->write(CONTROL_SEND_DIRECT_SIGNAL);
	b->writeAny(signal);

	IB_DEBUG("[SendDirectImpl] => signal info: source_id = " << signal.source_id << ", sink_id = " << signal.sink_id << ", length = " << signal.length << ", type = " << signal.type);

	return postControl(b);
}

void IBConnection::handleControlSendDirectSignal(Buffer* buffer)
{
	ControlSendDirectSignal signal;
	buffer->readAny(signal);

	IB_DEBUG("[HANDLE CONTROL] [SEND DIRECT SIGNAL] => signal info: source_id = " << signal.source_id << ", sink_id = " << signal.sink_id << ", length = " << signal.length << ", type = " << signal.type);

	// the sink_id is actually the pre-registered direct buffer pointer
	Buffer* b = reinterpret_cast<Buffer*>(signal.sink_id);
	b->rpos(0); b->wpos(signal.length);

	// dispatch to upper application
	shared_ptr<IBConnection> shared_from_this(mWeakThis);

	shared_ptr<Buffer> sb;
	_GET_BUFFER(mRemoteDirectBufferHolder, b, sb);

	mEngine->getDispatcher()->dispatchDataEvent(signal.type, sb, shared_from_this);
}

//////////////////////////////////////////////////////////////////////////
void IBConnection::flushSendRequestQueue()
{
	IB_DEBUG("[FLUSH SEND REQUEST QUEUE] => BEGIN process SendRequestQueue, xmitCredit = " << mFlowControl.xmitCredit);

	while(!mSendRequestQueue.empty())
	{
		if(requestSend())
		{
			SendRequest request;
			mSendRequestQueue.pop(request);

			switch(request.req)
			{
				case SendRequest::GENERAL:
				{
					IB_DEBUG("[FLUSH SEND REQUEST QUEUE] => Process GENERAL request, xmitCredit = " << mFlowControl.xmitCredit << ", type = " << request.type << ", buffer = " << request.buffer.get());
					postGeneral(request.type, request.buffer);
					break;
				}
				/*
				case SendRequest::DIRECT:
				{
					IB_DEBUG("[FLUSH SEND REQUEST QUEUE] => Process DIRECT request, xmitCredit = " << mFlowControl.xmitCredit << ", type = " << request.type << ", buffer = " << request.buffer.get() << ", sink_id = " << request.sink_id);
					postDirect(request.type, request.buffer, request.sink_id);
					break;
				}
				*/
				case SendRequest::CONTROL:
				{
					IB_DEBUG("[FLUSH SEND REQUEST QUEUE] => Process CONTROL request, xmitCredit = " << mFlowControl.xmitCredit << ", buffer = " << request.buffer.get());
					postControl(request.buffer);
					break;
				}
			}
		}
		else
		{
			break;
		}
	}

	IB_DEBUG("[FLUSH SEND REQUEST QUEUE] => END process SendRequestQueue, xmitCredit = " << mFlowControl.xmitCredit);
}

//////////////////////////////////////////////////////////////////////////
void IBConnection::sendCreditIfNecessary()
{
	if(mFlowControl.recvCredit >= IB_DEFAULT_THRESHOLD_BUFFER_ACK)
	{
		if(requestAck())
		{
			postControlCreditUpdate();
			mFlowControl.recvCredit = 0;
		}
		else
		{
			IB_ERROR("[ERROR] fail to request ack");
		}
	}
}

bool IBConnection::postControlCreditUpdate()
{
	IB_DEBUG("[SEND CONTROL] [CREDIT UPDATE]");

	shared_ptr<Buffer> b = getControlBuffer();
	if(!b.get())
	{
		IB_DEBUG("[SEND CONTROL] [CREDIT UPDATE] => FAILED, BUFFER NOT AVAILABLE");
		return false;
	}

	ControlCreditUpdate update;
#ifdef IB_ENABLE_DEBUG
	update.dbg_seq = mCreditUpdateSeq++;
#endif
	update.credit = mFlowControl.recvCredit;

	b->clear();
	b->write(CONTROL_CREDIT_UPDATE);
	b->writeAny(update);

	//IB_DEBUG("[SEND CONTROL] [CREDIT UPDATE] => credit info: xmitCredit(data) = " << update.dataCredit << ", xmitCredit(control) = " << update.controlCredit);
	IB_DEBUG("[SEND CONTROL] [CREDIT UPDATE] => credit info: recvCredit = " << update.credit << ", local xmitCredit = " << (uint32)mFlowControl.xmitCredit);

	return postControl(b);
}


void IBConnection::handleControlCreditUpdate(Buffer* buffer)
{
	tbb::queuing_mutex::scoped_lock lock(mSendLock);

	ControlCreditUpdate update;
	buffer->readAny(update);

	// update xmitCredit
	{
		tbb::queuing_mutex::scoped_lock lock(mXmitLock);
		mFlowControl.xmitCredit += update.credit;
		IB_DEBUG("[HANDLE CONTROL] [CREDIT UPDATE] => credit update: credit = " <<  update.credit << ", and xmitCredit after update = " << mFlowControl.xmitCredit);
	}

	flushSendRequestQueue();
}

#ifdef IB_ENABLE_DEBUG
void IBConnection::debugFlowControl()
{
	IB_DEBUG("[FLOW CONTROL] recvCredit = " << (uint32)mFlowControl.recvCredit << ", xmitCredit = " << (uint32)mFlowControl.xmitCredit << ", outstandingSend = " << (uint32)mFlowControl.outstandingSend << ", batchSendAck = " << mFlowControl.batchSendAck << ", batchRecvAck = " << mFlowControl.batchRecvAck);
}
#endif

//////////////////////////////////////////////////////////////////////////
void IBConnection::ensureQueuePair()
{
	// initialize protection domain
	mVerbs.pd = mDeviceResource->getGlobalProtectionDomain();

	// initialize RDMA completion channel
	mVerbs.rchannel = IBFactory::createEventChannel();

	// initialize completion channel
	mVerbs.cchannel = IBFactory::createCompletionChannel(mVerbs.id->verbs);

	// initialize send/receive completion queue
	mVerbs.scq = IBFactory::createCompletionQueue(mVerbs.id->verbs, IB_DEFAULT_CQ_ENTRIES, this, mVerbs.cchannel.get());
	mVerbs.rcq = IBFactory::createCompletionQueue(mVerbs.id->verbs, IB_DEFAULT_CQ_ENTRIES, this, mVerbs.cchannel.get());

	// initialize queue pair
    ibv_qp_init_attr qp_attr = {};

    qp_attr.cap.max_send_wr  = IB_DEFAULT_WR_ENTRIES;
    qp_attr.cap.max_send_sge = IB_DEFAULT_MAX_POST_SEGMENTS;
    qp_attr.cap.max_recv_wr  = IB_DEFAULT_WR_ENTRIES;
    qp_attr.cap.max_recv_sge = IB_DEFAULT_MAX_POST_SEGMENTS;

    qp_attr.send_cq = mVerbs.scq.get();
    qp_attr.recv_cq = mVerbs.rcq.get();
    qp_attr.qp_type = IBV_QPT_RC;

    IBFactory::createQueuePair(mVerbs.id.get(), mVerbs.pd.get(), &qp_attr);
}

} } }

#undef _HOLD_BUFFER
#undef _GET_BUFFER
#undef _UNHOLD_BUFFER

#undef _HOLD_REF
#undef _GET_REF
#undef _UNHOLD_REF

