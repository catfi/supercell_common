# 
# Zillians MMO
# Copyright (C) 2007-2009 Zillians.com, Inc.
# For more information see http:#www.zillians.com
#
# Zillians MMO is the library and runtime for massive multiplayer online game
# development in utility computing model, which runs as a service for every 
# developer to build their virtual world running on our GPU-assisted machines
#
# This is a close source library intended to be used solely within Zillians.com
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
#
# Contact Information: info@zillians.com
#

IF(RDMA_FOUND)
    INCLUDE_DIRECTORIES(${ibverbs_custom_build_SOURCE_DIR}/include/)
    INCLUDE_DIRECTORIES(${rdmacm_custom_build_SOURCE_DIR}/include/)
ENDIF()

INCLUDE_DIRECTORIES(${zillians-common_SOURCE_DIR}/include/)

SET(ZILLIANS_TEMPORARY_BUILD_LIST "")

LIST(APPEND TEMPORARY_BUILD_LIST
    Channel.cpp
	sys/tcp/TcpSessionImpl.cpp
	)

IF(COROSYNC_FOUND)
    LIST(APPEND TEMPORARY_BUILD_LIST
    	group/CloseProcessGroup.cpp
    	group/CloseProcessGroupManager.cpp
    	group/ExtendedVirtualSynchrony.cpp
	)
ELSE()
    MESSAGE(STATUS "Corosync not available, disabling build for CPG part in libzillians-common-net-api")
ENDIF()

IF(TICPP_FOUND)
    LIST(APPEND TEMPORARY_BUILD_LIST
    	scylla/ScyllaChannelEngine.cpp
    	scylla/ScyllaNodeDB.cpp
    	scylla/ScyllaChannel.cpp
	)
ELSE()
    MESSAGE(STATUS "TICPP not available, disabling build for Scylla part in libzillians-common-net-api")
ENDIF()
	
IF(RDMA_FOUND)
	LIST(APPEND TEMPORARY_BUILD_LIST
		rdma/address/InetAddress.cpp
		rdma/address/InetSocketAddress.cpp
		rdma/buffer_manager/IBBufferManager.cpp
		rdma/infiniband/IBAcceptor.cpp
		rdma/infiniband/IBCommon.cpp
		rdma/infiniband/IBConnection.cpp
		rdma/infiniband/IBConnector.cpp
		rdma/infiniband/IBCxxCompatibleWrapper.c
		rdma/infiniband/IBDeviceResource.cpp
		rdma/infiniband/IBDeviceResourceManager.cpp
		rdma/infiniband/IBFactory.cpp
		rdma/resolver/InetSocketResolver.cpp
		rdma/Poller.cpp
		)
ELSE()
    MESSAGE(STATUS "RDMA not available, disabling build for RDMA part in libzillians-common-net-api")
ENDIF(RDMA_FOUND)
	 
ADD_LIBRARY(zillians-common-net-api
	${TEMPORARY_BUILD_LIST}
    )

UNSET(TEMPORARY_BUILD_LIST CACHE)

IF(RDMA_FOUND)
    TARGET_LINK_LIBRARIES(zillians-common-net-api 
        zillians-common-core-api
        zillians-common-util-api
#    	rdmacm_custom_build ibverbs_custom_build mthca_custom_build
        rdmacm ibverbs
    	${ZILLIANS_DEP_LIBS}
    	${EV_LIBRARIES}
    	${TICPP_LIBRARIES}
    	${COROSYNC_LIBRARIES}
        )
ELSE()
    TARGET_LINK_LIBRARIES(zillians-common-net-api 
        zillians-common-core-api
        zillians-common-util-api
    	${ZILLIANS_DEP_LIBS}
    	${TICPP_LIBRARIES}
    	${COROSYNC_LIBRARIES}
        )
ENDIF()
