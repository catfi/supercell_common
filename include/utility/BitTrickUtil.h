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
 * @date May 19, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_BITTRICKUTIL_H_
#define ZILLIANS_BITTRICKUTIL_H_

#include "core/Types.h"

namespace zillians {

/**
 * Round-up helper function to calculate round-up value
 * @param v the value to round-up
 * @param m the multiple of rounding
 * @return the rounded value (upward)
 */
template<typename Value, typename Multiple>
inline static Value round_up_to_nearest_power(const Value& v, const Multiple& m)
{
	if(m == 0 || v % m == 0)
		return v;
	else
		return (v / m + 1) * m;
}

template<typename T>
struct round_up_to_nearest_power_of_two;

template<>
struct round_up_to_nearest_power_of_two<uint32>
{
	static uint32 apply(uint32 v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}
};

template<>
struct round_up_to_nearest_power_of_two<uint64>
{
	static uint64 apply(uint64 v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		v++;
		return v;
	}
};

}

#endif /* ZILLIANS_BITTRICKUTIL_H_ */
