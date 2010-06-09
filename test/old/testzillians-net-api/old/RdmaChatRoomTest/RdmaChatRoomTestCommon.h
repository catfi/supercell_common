#include "core/Prerequisite.h"
#include "networking/sys/rdma/RdmaDeviceResourceManager.h"
#include "networking/sys/rdma/RdmaNetEngine.h"
#include "networking/sys/rdma/RdmaDataHandler.h"
#include "networking/sys/rdma/RdmaConnectionHandler.h"
#include "networking/sys/rdma/RdmaConnection.h"
#include "networking/sys/buffer_manager/RdmaBufferManager.h"
#include "networking/sys/Poller.h"
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
