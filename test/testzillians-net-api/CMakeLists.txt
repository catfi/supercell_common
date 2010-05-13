# 
# Zillians MMO
# Copyright (C) 2007-2010 Zillians.com, Inc.
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
#    ADD_SUBDIRECTORY(InfinibandDeviceResourceManagerTest)
#    ADD_SUBDIRECTORY(InfinibandBasicSendReceiveTest)
#    ADD_SUBDIRECTORY(InfinibandDirectSendTest)
#    ADD_SUBDIRECTORY(InfinibandLargeSendTest)
    ADD_SUBDIRECTORY(InfinibandReadWriteTest)
ENDIF()

IF(TICPP_FOUND)
    ADD_SUBDIRECTORY(ScyllaBasicTest)
ENDIF()

IF(COROSYNC_FOUND)
    ADD_SUBDIRECTORY(CloseProcessGroupTest)
ENDIF()

ADD_SUBDIRECTORY(SessionBasicTest)
ADD_SUBDIRECTORY(SessionDispatcherTest)
#ADD_SUBDIRECTORY(SessionSendReceiveTest)
ADD_SUBDIRECTORY(SessionSynchronousTest)
ADD_SUBDIRECTORY(SessionConcurrentTest)
ADD_SUBDIRECTORY(SessionVectorBufferTest)
#ADD_SUBDIRECTORY(TimeSynchronizerTest) # Fails with some messages in protocol removed. 20100511
