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

INCLUDE_DIRECTORIES(${PROJECT_COMMON_SOURCE_DIR}/include/)

ADD_LIBRARY(zillians-common-utility
    utility/crypto/base64.cpp
    utility/crypto/Crypto.cpp
    utility/crypto/machine_info.cpp
	utility/BitTrickUtil.cpp
	utility/DemanglingUtil.cpp
	utility/StringUtil.cpp
    utility/TimerUtil.cpp
	utility/UUIDUtil.cpp
	utility/DependencySolver.cpp
	utility/UnicodeUtil.cpp
	utility/sha1.cpp
    )

TARGET_LINK_LIBRARIES(zillians-common-utility
    boost_system
    ${OPENSSL_LIBRARIES}
    )

ADD_SUBDIRECTORY(utility/archive)