/**
 * Zillians MMO
 * Copyright (C) 2007-2008 Zillians.com, Inc.
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
 * @date 2009/02/09 Nothing - Added BufferManager.h include
 */

#ifndef ZILLIANS_PREREQUISITE_H_
#define ZILLIANS_PREREQUISITE_H_

#include "core-api/Common.h"
#include "core-api/FastCallback.h"
#include "core-api/ObjectPool.h"
#include "core-api/SharedPtr.h"
#include "core-api/Buffer.h"

#if BUILD_WITH_BOOST
#include <boost/assert.hpp>
#include <boost/algorithm/minmax.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/if.hpp>
#include <boost/lambda/loops.hpp>
#include <boost/lambda/switch.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/exceptions.hpp>
#include <boost/lambda/algorithm.hpp>
#endif

#if BUILD_WITH_LOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#endif

#endif/*ZILLIANS_PREREQUISITE_H_*/
