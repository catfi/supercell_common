#include "core-api/Prerequisite.h"
#include "net-api/sys/rdma/RdmaDeviceResourceManager.h"
#include "net-api/sys/rdma/RdmaNetEngine.h"
#include "net-api/sys/rdma/RdmaDataHandler.h"
#include "net-api/sys/rdma/RdmaConnectionHandler.h"
#include "net-api/sys/rdma/RdmaConnection.h"
#include "net-api/sys/buffer_manager/RdmaBufferManager.h"
#include "net-api/sys/Poller.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <boost/bind.hpp>
#include <tbb/tbb_thread.h>
#include <iostream>
#include <map>
#include <string>

#define POOL_SIZE 256*1024*1024
#define BUFFER_SIZE 256*1024
#define DEFAULT_TIMEOUT	 1000

typedef int32 ChatRoomMsgType;
typedef int32 ChatRoomClientStatus;
