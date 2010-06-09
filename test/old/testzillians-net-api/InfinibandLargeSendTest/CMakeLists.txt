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

ADD_EXECUTABLE(InfinibandLargeSendTest InfinibandLargeSendTest.cpp) 

TARGET_LINK_LIBRARIES(InfinibandLargeSendTest 
    zillians-common-core 
    zillians-common-networking)

ADD_CUSTOM_COMMAND(
	TARGET InfinibandLargeSendTest 
	POST_BUILD COMMAND cmake -E copy 
	${CMAKE_CURRENT_SOURCE_DIR}/InfinibandLargeSendTest.run
	${EXECUTABLE_OUTPUT_PATH}/InfinibandLargeSendTest.run
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/InfinibandLargeSendTest.run
	)

ADD_CUSTOM_COMMAND(
	TARGET InfinibandLargeSendTest 
	POST_BUILD COMMAND chmod +x ${EXECUTABLE_OUTPUT_PATH}/InfinibandLargeSendTest.run
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/InfinibandLargeSendTest.run
	)
	
ADD_TEST(InfinibandLargeSendTest ${EXECUTABLE_OUTPUT_PATH}/InfinibandLargeSendTest.run ${EXECUTABLE_OUTPUT_PATH} g0 g1)
