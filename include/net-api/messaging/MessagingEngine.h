/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
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
 * @date Mar 19, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_MESSAGING_MESSAGINGENGINE_H_
#define ZILLIANS_MESSAGING_MESSAGINGENGINE_H_

#include "core-api/Prerequisite.h"
#include "net-api/rdma/infiniband/IBDeviceResourceManager.h"
#include "net-api/rdma/infiniband/IBNetEngine.h"
#include "net-api/rdma/infiniband/IBDispatcher.h"
#include "net-api/rdma/buffer_manager/IBBufferManager.h"
#include "net-api/rdma/Poller.h"
#include "net-api/messaging/ControlHandler.h"
#include <ext/hash_map>

using namespace zillians::net::rdma;
using namespace __gnu_cxx;

namespace zillians { namespace messaging {

class MessagingEngine
{
public:
	struct Remote
	{
		friend class MessagingEngine;
	private:
		Remote();
	public:
		bool write(shared_ptr<Buffer> buffer);
		bool sendControl(int32 type, shared_ptr<Buffer> buffer);

	private:
		std::string name;
		uint64 rposKey;
		uint64 wposKey;
		uint64 bufferKey;
		shared_ptr<Buffer> rposCache;
		shared_ptr<Buffer> wposCache;
		std::size_t bufferSize;
		shared_ptr<IBConnection> connection;
	};

	struct Local
	{
		friend class MessagingEngine;
	private:
		Local();
	public:
		std::size_t read(shared_ptr<Buffer> buffer);
		std::size_t peek(shared_ptr<Buffer> buffer);
		bool skip(std::size_t size);

	public:
		bool checkAvailable();
		void getAvailableBufferRanges(std::vector<std::pair<void*, std::size_t> >& ranges);

	private:
		std::string name;
		shared_ptr<Buffer> rpos;
		shared_ptr<Buffer> wpos;
		shared_ptr<Buffer> buffer;
		uint64 bufferKey;
		shared_ptr<IBConnection> connection;
	};

public:
	MessagingEngine(const std::string& localEndPoint);
	~MessagingEngine();

public:
	bool findRemote(const std::string& name, /*OUT*/ shared_ptr<Remote>& remote);
	bool findLocal(const std::string& name, /*OUT*/ shared_ptr<Local>& local);

	bool createRemote(const std::string& name, /*OUT*/ shared_ptr<Remote>& remote);
	bool createLocal(const std::string& name, shared_ptr<Buffer> buffer, /*OUT*/ shared_ptr<Local>& local);

public:
	void registerDefaultControlHandler(shared_ptr<ControlHandler> Handler);
	void unregisterDefaultControlHandler();
	void registerControlHandler(int32 type, shared_ptr<ControlHandler> Handler);
	void unregisterControlHandler(int32 type);

private:
	void handleAcceptorCompleted(shared_ptr<IBConnection> connection, int err);
	void handleConnectorCompleted(shared_ptr<IBConnection> connection, int err, ConnectionContext* context);
	void handleConnected(shared_ptr<IBConnection> connection);
	void handleDisconnected(shared_ptr<IBConnection> connection);

private:
	void handleQueueInfoExchangeRequest(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<IBConnection> connection);
	void handleQueueInfoExchangeResponse(uint32 type, shared_ptr<Buffer> buffer, shared_ptr<IBConnection> connection);

private:
	hash_map<std::string, shared_ptr<Local> > mLocalInfos;
	hash_map<std::string, shared_ptr<Remote> > mRemoteInfos;

	typedef std::map<int32, shared_ptr<DataHandler> > tControlHandlerMap;

	struct
	{
		shared_ptr<ControlHandler> def;
		tControlHandlerMap map;
	} mControlHandler;

private:
	struct ConnectionContext
	{
		std::string name;
		ConditionVariable<bool> condVariable;
	};

private:
	// Sender-initiated
	struct QueueInfoExchangeMsg
	{
		struct Request
		{
			static const int32 TYPE = 1001;

			template <typename Archive>
			void serialize(Archive& ar, const unsigned int version)
			{
				ar & queueName;
			}

			std::string queueName;
		};

		struct Response
		{
			static const int32 TYPE = 1002;

			template <typename Archive>
			void serialize(Archive& ar, const unsigned int version)
			{
				ar & success;
				ar & bufferKey;
			}

			bool success;
			uint64 bufferKey;
		};
	};

private:
	shared_ptr<Poller> mPoller;
	shared_ptr<IBDispatcher> mDispatcher;
	shared_ptr<IBNetEngine> mEngine;
	tbb::tbb_thread mPollerThread;

};

} }

#endif /* ZILLIANS_MESSAGING_MESSAGINGENGINE_H_ */
