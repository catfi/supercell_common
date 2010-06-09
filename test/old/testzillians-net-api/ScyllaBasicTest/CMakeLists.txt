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

INCLUDE_DIRECTORIES(${zillians-common_SOURCE_DIR}/include/)

ADD_EXECUTABLE(ScyllaBasicTest ScyllaBasicTest.cpp) 

TARGET_LINK_LIBRARIES(ScyllaBasicTest 
    zillians-common-core 
    zillians-common-networking
    )

    
ADD_CUSTOM_COMMAND(
	TARGET ScyllaBasicTest 
	POST_BUILD COMMAND cmake -E copy_if_different
	${CMAKE_CURRENT_SOURCE_DIR}/ScyllaBasicTestConfig.xml
	${EXECUTABLE_OUTPUT_PATH}/ScyllaBasicTestConfig.xml
	)

#ADD_CUSTOM_COMMAND(
#	TARGET ScyllaBasicTest 
#	POST_BUILD COMMAND cmake -E copy_if_different
#	${CMAKE_CURRENT_SOURCE_DIR}/ChannelEngineBasicTest.run
#	${EXECUTABLE_OUTPUT_PATH}/ChannelEngineBasicTest.run
#	)
	
#ADD_CUSTOM_COMMAND(
#	TARGET ScyllaBasicTest 
#	POST_BUILD COMMAND chmod +x ${EXECUTABLE_OUTPUT_PATH}/ChannelEngineBasicTest.run
#	)
	
#ADD_TEST(ChannelEngineBasicTest ${EXECUTABLE_OUTPUT_PATH}/ChannelEngineBasicTest.run ${EXECUTABLE_OUTPUT_PATH} s0 g1)
    