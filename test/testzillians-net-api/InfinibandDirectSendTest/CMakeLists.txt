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

ADD_EXECUTABLE(InfinibandDirectSendTest InfinibandDirectSendTest.cpp) 

TARGET_LINK_LIBRARIES(InfinibandDirectSendTest 
    zillians-common-core-api 
    zillians-common-net-api)

ADD_CUSTOM_COMMAND(
	TARGET InfinibandDirectSendTest 
	POST_BUILD COMMAND cmake -E copy_if_different
	${CMAKE_CURRENT_SOURCE_DIR}/InfinibandDirectSendTest.run
	${EXECUTABLE_OUTPUT_PATH}/InfinibandDirectSendTest.run
	)
	
ADD_CUSTOM_COMMAND(
	TARGET InfinibandDirectSendTest 
	POST_BUILD COMMAND chmod +x ${EXECUTABLE_OUTPUT_PATH}/InfinibandDirectSendTest.run
	)
	
ADD_TEST(InfinibandDirectSendTest ${EXECUTABLE_OUTPUT_PATH}/InfinibandDirectSendTest.run ${EXECUTABLE_OUTPUT_PATH} g0 g1)
