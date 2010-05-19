/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
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
 * @date May 16, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_JUSTTHREAD_H_
#define ZILLIANS_JUSTTHREAD_H_

#include "core-api/Common.h"

// C++0x threading provided by Just Software Solution (UK)
#if BUILD_WITH_JUSTTHREAD
#include <chrono>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <mutex>
#include <ratio>
#include <system_error>
#include <future>
#include <exception_ptr>
#endif

#endif /* ZILLIANS_JUSTTHREAD_H_ */
