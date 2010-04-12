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

#ifndef ZILLIANS_NET_RDMA_IBCONNECTION_H_
#define ZILLIANS_NET_RDMA_IBCONNECTION_H_

#include "core-api/Prerequisite.h"
#include "core-api/Buffer.h"
#include "core-api/HashMap.h"
#include "core-api/ContextHub.h"
#include "net-api/rdma/Poller.h"
#include "net-api/rdma/infiniband/IBCommon.h"
#include "net-api/rdma/infiniband/IBDeviceResource.h"
#include "net-api/rdma/infiniband/IBFactory.h"

#include "tbb/atomic.h"
#include "tbb/spin_mutex.h"
#include "tbb/queuing_mutex.h"

using namespace zillians;

namespace zillians { namespace net { namespace rdma {

class IBNetEngine;

/**
 * @brief IBConnection represents a tunneled Infiniband RDMA connection
 *
 * IBConnection encapsulate both Infiniband RDMA Send/Receive and Read/Write schematics and employs flow control over the connection
 * IBConnection can register itself to Poller (by calling IBConnection::start()) to start processing incoming messages
 *
 * @see NetEngine, IBConnector, IBAcceptor
 */
class IBConnection : public ContextHub<true>
{
	friend class IBConnector;
	friend class IBAcceptor;
public:
	typedef rdma_cm_id* HandleType;
	typedef boost::function< void (int error) > CompletionHandler;

public:
	IBConnection(IBNetEngine* engine, SharedPtr<rdma_cm_id> id);
	virtual ~IBConnection();

	//////////////////////////////////////////////////////////////////////
	//// Public Interfaces
	//////////////////////////////////////////////////////////////////////
	inline static SharedPtr<IBConnection> create(IBNetEngine* engine, SharedPtr<rdma_cm_id> id)
	{
		SharedPtr<IBConnection> p(new IBConnection(engine, id));
		p->mWeakThis = p;
		return p;
	}

	inline HandleType getHandle() {	return mVerbs.id.get(); }

	SharedPtr<Buffer> createBuffer(size_t size);

	bool sendThrottled(uint32 type, SharedPtr<Buffer> buffer);
	bool send(uint32 type, SharedPtr<Buffer> buffer);

//	uint64 registrerDirect(SharedPtr<Buffer> buffer);
//	void unregisterDirect(uint64 sink_id);
//	bool sendDirect(uint32 type, SharedPtr<Buffer> buffer, uint64 sink_id);

	void close();

	//////////////////////////////////////////////////////////////////////
	//// RDMA Read/Write Semantics
	//////////////////////////////////////////////////////////////////////
	uint64 registerDirect(SharedPtr<Buffer> buffer);
	void unregisterDirect(uint64 sink);

	bool write(uint64 sink, std::size_t offset, SharedPtr<Buffer> buffer, std::size_t size);
	bool writeAsync(uint64 sink, std::size_t offset, SharedPtr<Buffer> buffer, std::size_t size, CompletionHandler handler);

	bool read(SharedPtr<Buffer> buffer, uint64 sink, std::size_t offset, std::size_t size);
	bool readAsync(SharedPtr<Buffer> buffer, uint64 sink, std::size_t offset, std::size_t size, CompletionHandler handler);

	//////////////////////////////////////////////////////////////////////
	//// RDMA Send/Receive Semantics
	//////////////////////////////////////////////////////////////////////
	bool sendAsync(uint32 type, SharedPtr<Buffer>, CompletionHandler handler);
	bool receiveAsync(SharedPtr<Buffer>, CompletionHandler handler);

	//////////////////////////////////////////////////////////////////////
	//// Parameter Adjust
	//////////////////////////////////////////////////////////////////////
	int32 mMaxSendInFlight;
	void setMaxSendInFlight(int32 maxSendInFlight);

	//////////////////////////////////////////////////////////////////////
	//// Types & Containers
	//////////////////////////////////////////////////////////////////////
    typedef tbb::concurrent_hash_map<uint64, uint64> RefHolder;
    typedef tbb::concurrent_hash_map<uint64, SharedPtr<Buffer> > BufferRefHolder;
	typedef tbb::concurrent_bounded_queue< SharedPtr<Buffer> > BufferQueue;

	//////////////////////////////////////////////////////////////////////
	//// Poller Handlers & Related
	//////////////////////////////////////////////////////////////////////
public:
	void handleDataEvent(ev::io& w, int revent);
	void handleChannelEvent(ev::io& w, int revent);

private:
	void handleSendCompletionRdmaWrite(const ibv_wc &wc);
	void handleSendCompletionRdmaRead(const ibv_wc &wc);
	void handleSendCompletionControlBuffer(const ibv_wc &wc);
	void handleSendCompletionGeneralBuffer(const ibv_wc &wc);

	void handleRecvCompletionControlBuffer(const ibv_wc &wc);
	void handleRecvCompletionGeneralBuffer(const ibv_wc &wc);

	void start(SharedPtr<Poller> poller);
	void stop();


	//////////////////////////////////////////////////////////////////////
	//// Internal Send Routines
	//////////////////////////////////////////////////////////////////////
	IBConnection::BufferRefHolder mRecvBufferHolder;
	IBConnection::BufferRefHolder mSendBufferHolder;

	bool postGeneral(uint32 type, SharedPtr<Buffer> buffer);
	bool sendControl(SharedPtr<Buffer> buffer);
	bool postControl(SharedPtr<Buffer> buffer);
	//bool postDirect(uint32 type, SharedPtr<Buffer> buffer, uint64 sink_id);

	//////////////////////////////////////////////////////////////////////
	//// Internal Control - Completion Info
	//////////////////////////////////////////////////////////////////////
	struct CompletionInfo : zillians::ConcurrentObjectPool<CompletionInfo>
	{
		CompletionInfo() { }
		~CompletionInfo() { buffer.reset(); handler.clear(); }
		SharedPtr<Buffer> buffer;
		CompletionHandler handler;
	};

	//////////////////////////////////////////////////////////////////////
	//// RDMA Send/Recv Read/Write
	//////////////////////////////////////////////////////////////////////
#ifdef IB_ENABLE_ORDERING_CHECK
	tbb::concurrent_bounded_queue<uint64> mPostSendOrderQueue;
	tbb::concurrent_bounded_queue<uint64> mPostRecvOrderQueue;
#endif

	bool postRecv(Buffer* buffer, CompletionInfo* info);
	bool postSend(Buffer* buffer, CompletionInfo* info);
	bool postSend(Buffer* buffer, uint32 imm, CompletionInfo* info);

	bool postRead(uint64 address, uint32 rkey, uint32 length, Buffer* buffer, CompletionInfo* info);
	bool postWrite(uint64 address, uint32 rkey, uint32 length, Buffer* buffer, CompletionInfo* info);

	void notifyRecv();
	void notifySend();


    //////////////////////////////////////////////////////////////////////
	//// Control Buffers
	//////////////////////////////////////////////////////////////////////
	IBConnection::BufferQueue mControlBufferQueue;
	SharedPtr<Buffer> getControlBuffer(bool blocking = false);
	void returnControlBuffer(SharedPtr<Buffer> buffer);

	//////////////////////////////////////////////////////////////////////
	//// Internal Control - Remote Access Exchange
	//////////////////////////////////////////////////////////////////////
    struct ControlAccessExchange
    {
    	uint64 address;
    	uint32 length;
    	uint32 rkey;
    };

    struct
    {
		uint64 address;
		uint32 length;
		uint32 rkey;
    } mRemoteAccess;

    // hold the local large buffer => remote buffer id
    IBConnection::RefHolder mLargeBufferAckRefHolder;
    IBConnection::RefHolder mLargeBufferTypeHolder;

	void forceAccessExchange();
    bool sendControlAccessExchange(uint64 address, uint32 length, uint32 rkey);
    void handleControlAccessExchange(Buffer* buffer);


	//////////////////////////////////////////////////////////////////////
	//// Special Control - Large Buffer Send (via RDMA READ)
	//////////////////////////////////////////////////////////////////////
    struct ControlLargeBufferSend
    {
    	uint64 id;
		uint64 address;
		uint32 length;
		uint32 rkey;
		uint32 type;
    };

    struct ControlLargeBufferAck
    {
		uint64 id;
		int32 result;
    };

    bool postControlLargeBufferSend(Buffer* buffer, uint32 type);
    void handleControlLargeBufferSend(Buffer* buffer);

    bool sendControlLargeBufferAck(uint64 id, int32 result);
    void handleControlLargeBufferAck(Buffer* buffer);


    //////////////////////////////////////////////////////////////////////
    //// Internal Control - Direct Send (via RDMA WRITE)
    //////////////////////////////////////////////////////////////////////
    struct ControlRegDirectBuffer
    {
    	uint64 sink_id;
    	uint64 address;
    };

    struct ControlUnregDirectBuffer
    {
    	uint64 sink_id;
    };

    struct ControlSendDirectSignal
    {
    	uint64 source_id;
    	uint64 sink_id;
    	uint32 length;
    	uint32 type;
    };

    // hold the remote buffer id => local buffer id reference
    IBConnection::RefHolder mLocalDirectBufferRefHolder;
    // hold the remote buffer id => remote buffer address reference
    IBConnection::RefHolder mRemoteDirectBufferRefHolder;
    // hold buffers which are being sent to remote
    IBConnection::BufferRefHolder mLocalDirectBufferHolder;
    // hold buffers which are registered locally
    IBConnection::BufferRefHolder mRemoteDirectBufferHolder;

    bool sendControlRegDirectBuffer(uint64 sink_id, uint64 address);
    void handleControlRegDirectBuffer(Buffer* buffer);

    bool sendControlUnregDirectBuffer(uint64 sink_id);
    void handleControlUnregDirectBuffer(Buffer* buffer);

    bool sendControlSendDirectSignal(uint64 source_id, uint64 sink_id, uint32 length, uint32 type);
    void handleControlSendDirectSignal(Buffer* buffer);


	//////////////////////////////////////////////////////////////////////
	//// Flow Control (xmitCredit & recvCredit)
    //////////////////////////////////////////////////////////////////////
    struct
    {
    	uint32 recvCredit;
    	uint32 xmitCredit;
    	tbb::atomic<uint32> outstandingSend;

        uint32 batchSendAck;
        uint32 batchRecvAck;
    } mFlowControl;

    tbb::queuing_mutex mXmitLock;
    tbb::queuing_mutex mSendLock;

    inline bool requestSend()
    {
    	if(!mConnected) return false;

    	if(mFlowControl.outstandingSend >= IB_DEFAULT_WR_ENTRIES / 2)
    	{
    		return false;
    	}

    	tbb::queuing_mutex::scoped_lock lock(mXmitLock);
    	if(mFlowControl.xmitCredit > IB_DEFAULT_ACK_BUFFER_COUNT)
    	{
    		--mFlowControl.xmitCredit;
    		return true;
    	}
    	return false;
    }

    inline bool requestAck()
    {
    	if(!mConnected) return false;

    	if(mFlowControl.outstandingSend >= IB_DEFAULT_WR_ENTRIES)
    	{
    		return false;
    	}

    	tbb::queuing_mutex::scoped_lock lock(mXmitLock);
    	if(mFlowControl.xmitCredit > 0)
    	{
    		--mFlowControl.xmitCredit;
    		return true;
    	}
    	return false;
    }

    struct SendThrottle
    {
    	SendThrottle()
    	{
    		slow_down = false;
    		slow_factor = 0.0005;
    		observation_time = 1.0;
    		slow_step = 0.0005;
    		ok_count = 0;
    		ok_threshold = 100000;
    	}

		bool slow_down;
		double slow_factor;
		double observation_time;
		double slow_step;
		int ok_count;
		int ok_threshold;
    } mSendThrottle;

	//////////////////////////////////////////////////////////////////////
	//// Flow Control (Send Request Queue)
    //////////////////////////////////////////////////////////////////////
    struct SendRequest
    {
    	enum { GENERAL, /*DIRECT, */CONTROL } req;
    	uint32 type;
    	SharedPtr<Buffer> buffer;
    	uint64 sink_id;
    };

    typedef tbb::concurrent_bounded_queue<SendRequest> RequestQueue;
    IBConnection::RequestQueue mSendRequestQueue;

    void flushSendRequestQueue();


	//////////////////////////////////////////////////////////////////////
	//// Flow Control (Credit Update)
    //////////////////////////////////////////////////////////////////////
    struct ControlCreditUpdate
    {
#ifdef IB_ENABLE_DEBUG
    	uint32 dbg_seq;
#endif
    	uint32 credit;
    };

    void sendCreditIfNecessary();

    bool postControlCreditUpdate();
    void handleControlCreditUpdate(Buffer* buffer);

#ifdef IB_ENABLE_DEBUG
    tbb::atomic<uint32> mDEBUG_TotalEvents;
    tbb::atomic<uint32> mDEBUG_TotalPostSend;
    tbb::atomic<uint32> mDEBUG_TotalPostRecv;
    tbb::atomic<uint32> mDEBUG_HoldBuffer;
    tbb::atomic<uint32> mDEBUG_HoldRef;

    uint32 mCreditUpdateSeq;

    void debugFlowControl();
#endif


    //////////////////////////////////////////////////////////////////////
    //// Buffer Reference Management
    //////////////////////////////////////////////////////////////////////
    // TODO maybe we don't need to use concurrent_hash_map but use concurrent_queue instead...need more test on the behavior of RDMA transport
    // see this: http://www.mail-archive.com/general@lists.openfabrics.org/msg09776.html
    // which reports some bogus receive completion



	//////////////////////////////////////////////////////////////////////
	//// Verbs
    //////////////////////////////////////////////////////////////////////
	struct
	{
		SharedPtr<rdma_event_channel>	rchannel;
		SharedPtr<rdma_cm_id>			id;
		SharedPtr<ibv_pd> 				pd;
		SharedPtr<ibv_comp_channel> 	cchannel;
		SharedPtr<ibv_cq>				scq;
		SharedPtr<ibv_cq> 				rcq;
		tbb::queuing_mutex				send_lock;
		tbb::queuing_mutex				recv_lock;
	} mVerbs;


	//////////////////////////////////////////////////////////////////////
	//// IO Watchers
	//////////////////////////////////////////////////////////////////////
    struct
    {
    	ev::io dataRead;
    	ev::io channelRead;
    } mWatchers;

    bool mWatchersStarted;


    //////////////////////////////////////////////////////////////////////
    //// Control Message Types
    //////////////////////////////////////////////////////////////////////
    const static uint32 CONTROL_CREDIT_UPDATE;// 		= 1;//0x10000000;
    const static uint32 CONTROL_LARGE_BUFFER_SEND;//	= 2;//0x20000000;
    const static uint32 CONTROL_LARGE_BUFFER_ACK;//	= 3;//0x30000000;
    const static uint32 CONTROL_ACCESS_EXCHANGE;//		= 4;//0x40000000;
    const static uint32 CONTROL_SEND_DIRECT_SIGNAL;// 	= 5;//0x50000000;
    const static uint32 CONTROL_REG_DIRECT_BUFFER;//	= 6;//0x60000000;
    const static uint32 CONTROL_UNREG_DIRECT_BUFFER;//	= 7;//0x70000000;


	//////////////////////////////////////////////////////////////////////
	//// Miscellaneous
	//////////////////////////////////////////////////////////////////////
	static log4cxx::LoggerPtr mLogger;

	IBNetEngine* mEngine;
    WeakPtr<IBConnection> mWeakThis;

	SharedPtr<IBDeviceResource> mDeviceResource;
    SharedPtr<Poller> mPoller;
    bool mConnected;

	void ensureQueuePair();
};

} } }

#endif/*ZILLIANS_NET_RDMA_IBCONNECTION_H_*/
