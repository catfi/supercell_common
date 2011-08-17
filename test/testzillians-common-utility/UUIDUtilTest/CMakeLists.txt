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

include_directories(${zillians-common_SOURCE_DIR}/include/)

add_executable(UUIDUtilTest UUIDUtilTest.cpp)

target_link_libraries(UUIDUtilTest 
    zillians-common-core
    zillians-common-utility
    )

zillians_add_simple_test(TARGET UUIDUtilTest)
zillians_add_test_subject(SUBJECT common-utility-critical TARGET UUIDUtilTest)
#zillians_add_compare_output_test(UUIDUtilTest ${CMAKE_CURRENT_SOURCE_DIR}/expected_output.txt)
#zillians_add_regex_match_output_test(UUIDUtilTest "*.Error" "MATCH_FOR_FAILURE")