
#ifndef RDMAPERFORMACETESTSERVER_H_
#define RDMAPERFORMACETESTSERVER_H_

#include "core/Prerequisite.h"
#include "networking/sys/rdma/RdmaNetEngine.h"
#include "networking/sys/rdma/RdmaConnectionHandler.h"
#include <tbb/tick_count.h>
#include <tbb/tbb_thread.h>

#define SENDITERATION 10000
#define BIGSENDITERATION 20
#define DIRECTSENDITERATION 20

using namespace zillians;
using namespace zillians::networking;

struct BufferInfo
{
	uint64 id;
	uint32 length;
};

class RdmaPerformanceTestServer : public RdmaConnectionHandler, public RdmaDataHandler
{
public:
	RdmaPerformanceTestServer();
	virtual ~RdmaPerformanceTestServer();


public:
	virtual void onDisconnected(shared_ptr<RdmaConnection> connection);
	virtual void onConnected(shared_ptr<RdmaConnection> connection);
	virtual void onError(shared_ptr<RdmaConnection> connection, int code);

public:
	virtual void handle(uint32 type, shared_ptr<Buffer> b, shared_ptr<RdmaConnection> connection);


public:
	void run();

public:
	size_t mSize;
	tbb::tick_count mStartTime, mEndTime;
private:
	static log4cxx::LoggerPtr mLogger;
	shared_ptr<RdmaConnection> mConnection;
	shared_ptr<Buffer> mBuffer;

};
log4cxx::LoggerPtr RdmaPerformanceTestServer::mLogger(log4cxx::Logger::getLogger("RdmaPerformanceTestServer"));



class RdmaPerformanceTestClient : public RdmaConnectionHandler, public RdmaDataHandler
{
	public:
		RdmaPerformanceTestClient();
		virtual ~RdmaPerformanceTestClient();


	public:
		virtual void onDisconnected(shared_ptr<RdmaConnection> connection);
		virtual void onConnected(shared_ptr<RdmaConnection> connection);
		virtual void onError(shared_ptr<RdmaConnection> connection, int code);

	public:
		virtual void handle(uint32 type, shared_ptr<Buffer> b, shared_ptr<RdmaConnection> connection);

	public:
		void run();

		void doSendTest();
		void doBigSendTest();
		void doDirectSendTest();

	public:
		size_t mSize;
		tbb::tick_count mSendstartTime, mEndTime;
		tbb::tick_count mBigSendstartTime;
		tbb::tick_count mDirectSendstartTime;

	private:
		static log4cxx::LoggerPtr mLogger;
		shared_ptr<RdmaConnection> mConnection;
		shared_ptr<Buffer> mBuffer;
		bool mLock;


};
log4cxx::LoggerPtr RdmaPerformanceTestClient::mLogger(log4cxx::Logger::getLogger("RdmaPerformanceTestClient"));



#endif //define RDMAPERFORMACETESTSERVER_H_
