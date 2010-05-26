/**
 * Zillians MMO
 * Copyright (C) 2007-2009 Zillians.com, Inc.
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

#ifndef ZILLIANS_SHAREDPTR_H_
#define ZILLIANS_SHAREDPTR_H_

#include "core-api/Common.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

using boost::static_pointer_cast;
using boost::const_pointer_cast;
using boost::dynamic_pointer_cast;
using boost::shared_ptr;
using boost::weak_ptr;

/**
 * @brief Allow direct comparison of shared_ptr and its wrapping type (for "equal to" operator)
 */
template<class T>
inline bool operator == (const shared_ptr<T> &a, const T* b)
{
    return (a.get() == b);
}

/**
 * @brief Allow direct comparison of shared_ptr and its wrapping type (for "not equal to" operator)
 */
template<class T>
inline bool operator != (const shared_ptr<T> &a, const T* b)
{
    return (a.get() != b);
}

#endif/*ZILLIANS_SHAREDPTR_H_*/
