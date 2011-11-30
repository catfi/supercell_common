/**
 * Zillians MMO
 * Copyright (C) 2007-2011 Zillians.com, Inc.
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

#ifndef ZILLIANS_CONTAINERS_H_
#define ZILLIANS_CONTAINERS_H_

#include "core/Common.h"

#if defined _WIN32
	#include <unordered_set>
	#include <unordered_map>
	using std::unordered_set;
	using std::unordered_map;
#else
	#ifndef __GXX_EXPERIMENTAL_CXX0X__
		#include <boost/unordered_set.hpp>
		#include <boost/unordered_map.hpp>
		using boost::unordered_set;
		using boost::unordered_map;
	#else
		#include <unordered_set>
		#include <unordered_map>
		using std::unordered_set;
		using std::unordered_map;
	#endif
#endif

#endif /* ZILLIANS_CONTAINERS_H_ */
