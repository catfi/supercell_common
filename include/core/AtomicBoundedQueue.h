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
 * @date Apr 13, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_ATOMICBOUNDEDQUEUE_H_
#define ZILLIANS_ATOMICBOUNDEDQUEUE_H_

#include "core/Common.h"
#include "core/JustThread.h"

namespace zillians {

template<typename T>
class AtomicBoundedQueue
{
public:
	AtomicBoundedQueue(std::size_t buffer_size) : buffer(new cell_t[buffer_size]), buffer_mask(buffer_size - 1)
	{
		BOOST_ASSERT((buffer_size >= 2) && ((buffer_size & (buffer_size - 1)) == 0) && "the buffer size must be greater than 2 and is power of 2");
        for (size_t i = 0; i != buffer_size; i += 1)
            buffer[i].sequence.store(i, std::memory_order_relaxed);
        enqueue_pos.store(0, std::memory_order_relaxed);
        dequeue_pos.store(0, std::memory_order_relaxed);
	}

	~AtomicBoundedQueue()
	{
        delete [] buffer;
	}

public:
    bool push(T const& data)
    {
        cell_t* cell;
        size_t pos = enqueue_pos.load(std::memory_order_relaxed);
        for (;;)
        {
            cell = &buffer[pos & buffer_mask];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif == 0)
            {
                if (enqueue_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                    break;
            }
            else if (dif < 0)
                return false;
            else
                pos = enqueue_pos.load(std::memory_order_relaxed);
        }

        cell->data = data;
        cell->sequence.store(pos + 1, std::memory_order_release);

        return true;
    }

    bool pop(T& data)
    {
        cell_t* cell;
        size_t pos = dequeue_pos.load(std::memory_order_relaxed);
        for (;;)
        {
            cell = &buffer[pos & buffer_mask];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
            if (dif == 0)
            {
                if (dequeue_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                    break;
            }
            else if (dif < 0)
                return false;
            else
                pos = dequeue_pos.load(std::memory_order_relaxed);
        }

        data = cell->data;
        cell->sequence.store(pos + buffer_mask + 1, std::memory_order_release);

        return true;
    }

private:
    struct cell_t
    {
        std::atomic<size_t>     sequence;
        T                       data;
    };

    static size_t const         cacheline_size = 64;
    typedef char                cacheline_pad_t[cacheline_size];

    cacheline_pad_t             pad0;
    cell_t* const               buffer;
    size_t const                buffer_mask;
    cacheline_pad_t             pad1;
    std::atomic<size_t>         enqueue_pos;
    cacheline_pad_t             pad2;
    std::atomic<size_t>         dequeue_pos;
    cacheline_pad_t             pad3;

    AtomicBoundedQueue(AtomicBoundedQueue const&);
    void operator= (AtomicBoundedQueue const&);
};

}
#endif /* ZILLIANS_ATOMICBOUNDEDQUEUE_H_ */
