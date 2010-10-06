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
 * @date Oct 6, 2010 Jerry - Initial version created.
 */

#include <cstdio>
#include <utility>
#include "tbb/task_scheduler_init.h"
#include "tbb/enumerable_thread_specific.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

#include "core/Types.h"                               // zillians::uint32
#include "utility/UUIDUtil.h"                         // UUID
#include <tbb/concurrent_hash_map.h>                  // tbb::concurrent_hash_map
#include <map>                                        // std::map
#include <ext/hash_map>                               // __gnu_cxx::hash_map
#include "vw/processors/cuda/kernel/api/RuntimeApi.h" // vw::processors::cuda::RuntimeApi
#include "core/Singleton.h"                           // zillians::Singleton

#define MAX_UUID_COUNT        100000
#define DEFAULT_UUID_COUNT_0  10000
#define DEFAULT_UUID_COUNT_1  20000
#define DEFAULT_UUID_COUNT_2  40000
#define DEFAULT_QUERY_COUNT_0 10000
#define DEFAULT_QUERY_COUNT_1 20000
#define DEFAULT_QUERY_COUNT_2 40000
#define DEFAULT_REPEAT_ITERS  1000

namespace zillians
{

namespace zvpc = vw::processors::cuda;

typedef tbb::concurrent_hash_map<UUID, uint32, UUIDHasher> tbb_uuid2int_map_t;
typedef tbb::concurrent_hash_map<uint32, UUID>             tbb_int2uuid_map_t;
typedef std::map<UUID, uint32>                             stl_uuid2int_map_t;
typedef std::map<uint32, UUID>                             stl_int2uuid_map_t;
typedef __gnu_cxx::hash_map<UUID, uint32>                  gnu_uuid2int_map_t;
typedef __gnu_cxx::hash_map<uint32, UUID>                  gnu_int2uuid_map_t;
typedef zvpc::RuntimeApi                                   KernelApi;

#if 0
    typedef tbb::enumerable_thread_specific< std::pair<int,int> > CounterType;
    CounterType MyCounters(std::make_pair(0,0));
    struct Body
    {
        void operator()(const tbb::blocked_range<int> &r) const
        {
            CounterType::reference my_counter = MyCounters.local();
            ++my_counter.first;
            for(int i = r.begin(); i != r.end(); ++i)
                ++my_counter.second;
        }
    };
#endif

tbb_uuid2int_map_t tbb_uuid2int_map;
tbb_int2uuid_map_t tbb_int2uuid_map;
stl_uuid2int_map_t stl_uuid2int_map;
stl_int2uuid_map_t stl_int2uuid_map;
gnu_uuid2int_map_t gnu_uuid2int_map;
gnu_int2uuid_map_t gnu_int2uuid_map;

class uuid_mgr_t : public Singleton<uuid_mgr_t, true>
{
public:
    uuid_mgr_t(uint32 Count = MAX_UUID_COUNT)
    {
        uuid_value_arr = new UUID[Count];
        for(int i = 0; i<Count; i++)
            uuid_value_arr[i].random();
    }
    virtual ~uuid_mgr_t()
    {
        delete []uuid_value_arr;
    }
    const UUID &at(uint32 Index) const
    {
        return uuid_value_arr[Index];
    }
private:
    UUID *uuid_value_arr;
};

//=============================================================================
// TBB Parallel-For Functors
//=============================================================================

// TBB
struct query_tbb_t
{
    void operator()(const tbb::blocked_range<int> &r) const
    {
        for(int i = r.begin(); i != r.end(); i++)
        {
            const UUID &UUID_key = uuid_mgr_t::instance()->at(i);
            uint32 uint32_value = i;
            tbb_uuid2int_map_t::accessor a;
            BOOST_ASSERT(tbb_uuid2int_map.find(a, UUID_key) && "fail to find");
            BOOST_ASSERT(a->second == uint32_value          && "found result incorrect");
        }
    }
};
struct query_rev_tbb_t
{
    void operator()(const tbb::blocked_range<int> &r) const
    {
        for(int i = r.begin(); i != r.end(); i++)
        {
            uint32 uint32_key = i;
            const UUID &UUID_value = uuid_mgr_t::instance()->at(i);
            tbb_int2uuid_map_t::accessor a;
            BOOST_ASSERT(tbb_int2uuid_map.find(a, uint32_key) && "fail to find");
            BOOST_ASSERT(a->second == UUID_value              && "found result incorrect");
        }
    }
};

// STL
struct query_stl_t
{
    void operator()(const tbb::blocked_range<int> &r) const
    {
        for(int i = r.begin(); i != r.end(); i++)
        {
            const UUID &UUID_key = uuid_mgr_t::instance()->at(i);
            uint32 uint32_value = i;
            stl_uuid2int_map_t::iterator p = stl_uuid2int_map.find(UUID_key);
            BOOST_ASSERT(p != stl_uuid2int_map.end() && "fail to find");
            BOOST_ASSERT((*p).second == uint32_value && "found result incorrect");
        }
    }
};
struct query_rev_stl_t
{
    void operator()(const tbb::blocked_range<int> &r) const
    {
        for(int i = r.begin(); i != r.end(); i++)
        {
            uint32 uint32_key = i;
            const UUID &UUID_value = uuid_mgr_t::instance()->at(i);
            stl_int2uuid_map_t::iterator p = stl_int2uuid_map.find(uint32_key);
            BOOST_ASSERT(p != stl_int2uuid_map.end() && "fail to find");
            BOOST_ASSERT((*p).second == UUID_value   && "found result incorrect");
        }
    }
};

// GNU
struct query_gnu_t
{
    void operator()(const tbb::blocked_range<int> &r) const
    {
        for(int i = r.begin(); i != r.end(); i++)
        {
            const UUID &UUID_key = uuid_mgr_t::instance()->at(i);
            uint32 uint32_value = i;
            gnu_uuid2int_map_t::iterator p = gnu_uuid2int_map.find(UUID_key);
            BOOST_ASSERT(p != gnu_uuid2int_map.end() && "fail to find");
            BOOST_ASSERT((*p).second == uint32_value && "found result incorrect");
        }
    }
};
struct query_rev_gnu_t
{
    void operator()(const tbb::blocked_range<int> &r) const
    {
        for(int i = r.begin(); i != r.end(); i++)
        {
            uint32 uint32_key = i;
            const UUID &UUID_value = uuid_mgr_t::instance()->at(i);
            gnu_int2uuid_map_t::iterator p = gnu_int2uuid_map.find(uint32_key);
            BOOST_ASSERT(p != gnu_int2uuid_map.end() && "fail to find");
            BOOST_ASSERT((*p).second == UUID_value   && "found result incorrect");
        }
    }
};

// Fixture: the RuntimeApi instance
KernelApi Api;

//=============================================================================
// Container Tests
//=============================================================================

// TBB
static void test_query_tbb(uint32 UUIDCount, uint32 QueryCount, uint32 RepeatIters)
{
    //=====================================
    // Prepare Input
    //=====================================

    {
        tbb_uuid2int_map.clear();
        for(int i = 0; i<UUIDCount; i++)
        {
            const UUID &UUID_key = uuid_mgr_t::instance()->at(i);
            uint32 uint32_value = i;
            tbb_uuid2int_map.insert(std::make_pair(UUID_key, uint32_value));
        }
    }

    //=====================================
    // Enter Kernel
    //=====================================

    {
        // initialize timer
        KernelApi::event_type StartEvent;
        KernelApi::event_type StopEvent;
        Api.event.create(&StartEvent);
        Api.event.create(&StopEvent);
        tbb::tick_count StartTick = tbb::tick_count::now();
        Api.event.record(StartEvent);

        for(int i = 0; i<RepeatIters; i++)
            tbb::parallel_for(tbb::blocked_range<int>(0, QueryCount), query_tbb_t());

        // finalize timer
        Api.event.record(StopEvent);
        Api.event.synchronize(StopEvent);
        std::cout << "run time (reported by CUDA): " <<
            Api.event.queryElapsed(StartEvent, StopEvent)/RepeatIters << " ms" << std::endl;
        tbb::tick_count StopTick = tbb::tick_count::now();
        std::cout << "run time (reported by TBB): " <<
            (StopTick-StartTick).seconds()*1000.0/RepeatIters << " ms" << std::endl;
    }

    //=====================================
    // Check Result
    //=====================================

    {
    }
}
static void test_query_rev_tbb(uint32 UUIDCount, uint32 QueryCount, uint32 RepeatIters)
{
    //=====================================
    // Prepare Input
    //=====================================

    {
        tbb_uuid2int_map.clear();
        for(int i = 0; i<UUIDCount; i++)
        {
            uint32 uint32_key = i;
            const UUID &UUID_value = uuid_mgr_t::instance()->at(i);
            tbb_int2uuid_map.insert(std::make_pair(uint32_key, UUID_value));
        }
    }

    //=====================================
    // Enter Kernel
    //=====================================

    {
        // initialize timer
        KernelApi::event_type StartEvent;
        KernelApi::event_type StopEvent;
        Api.event.create(&StartEvent);
        Api.event.create(&StopEvent);
        tbb::tick_count StartTick = tbb::tick_count::now();
        Api.event.record(StartEvent);

        for(int i = 0; i<RepeatIters; i++)
            tbb::parallel_for(tbb::blocked_range<int>(0, QueryCount), query_rev_tbb_t());

        // finalize timer
        Api.event.record(StopEvent);
        Api.event.synchronize(StopEvent);
        std::cout << "run time (reported by CUDA): " <<
            Api.event.queryElapsed(StartEvent, StopEvent)/RepeatIters << " ms" << std::endl;
        tbb::tick_count StopTick = tbb::tick_count::now();
        std::cout << "run time (reported by TBB): " <<
            (StopTick-StartTick).seconds()*1000.0/RepeatIters << " ms" << std::endl;
    }

    //=====================================
    // Check Result
    //=====================================

    {
    }
}

// STL
static void test_query_stl(uint32 UUIDCount, uint32 QueryCount, uint32 RepeatIters)
{
    //=====================================
    // Prepare Input
    //=====================================

    {
        stl_uuid2int_map.clear();
        for(int i = 0; i<UUIDCount; i++)
        {
            const UUID &UUID_key = uuid_mgr_t::instance()->at(i);
            uint32 uint32_value = i;
            stl_uuid2int_map.insert(std::make_pair(UUID_key, uint32_value));
        }
    }

    //=====================================
    // Enter Kernel
    //=====================================

    {
        // initialize timer
        KernelApi::event_type StartEvent;
        KernelApi::event_type StopEvent;
        Api.event.create(&StartEvent);
        Api.event.create(&StopEvent);
        tbb::tick_count StartTick = tbb::tick_count::now();
        Api.event.record(StartEvent);

        for(int i = 0; i<RepeatIters; i++)
            tbb::parallel_for(tbb::blocked_range<int>(0, QueryCount), query_stl_t());

        // finalize timer
        Api.event.record(StopEvent);
        Api.event.synchronize(StopEvent);
        std::cout << "run time (reported by CUDA): " <<
            Api.event.queryElapsed(StartEvent, StopEvent)/RepeatIters << " ms" << std::endl;
        tbb::tick_count StopTick = tbb::tick_count::now();
        std::cout << "run time (reported by TBB): " <<
            (StopTick-StartTick).seconds()*1000.0/RepeatIters << " ms" << std::endl;
    }

    //=====================================
    // Check Result
    //=====================================

    {
    }
}
static void test_query_rev_stl(uint32 UUIDCount, uint32 QueryCount, uint32 RepeatIters)
{
    //=====================================
    // Prepare Input
    //=====================================

    {
        stl_uuid2int_map.clear();
        for(int i = 0; i<UUIDCount; i++)
        {
            uint32 uint32_key = i;
            const UUID &UUID_value = uuid_mgr_t::instance()->at(i);
            stl_int2uuid_map.insert(std::make_pair(uint32_key, UUID_value));
        }
    }

    //=====================================
    // Enter Kernel
    //=====================================

    {
        // initialize timer
        KernelApi::event_type StartEvent;
        KernelApi::event_type StopEvent;
        Api.event.create(&StartEvent);
        Api.event.create(&StopEvent);
        tbb::tick_count StartTick = tbb::tick_count::now();
        Api.event.record(StartEvent);

        for(int i = 0; i<RepeatIters; i++)
            tbb::parallel_for(tbb::blocked_range<int>(0, QueryCount), query_rev_stl_t());

        // finalize timer
        Api.event.record(StopEvent);
        Api.event.synchronize(StopEvent);
        std::cout << "run time (reported by CUDA): " <<
            Api.event.queryElapsed(StartEvent, StopEvent)/RepeatIters << " ms" << std::endl;
        tbb::tick_count StopTick = tbb::tick_count::now();
        std::cout << "run time (reported by TBB): " <<
            (StopTick-StartTick).seconds()*1000.0/RepeatIters << " ms" << std::endl;
    }

    //=====================================
    // Check Result
    //=====================================

    {
    }
}

// GNU
static void test_query_gnu(uint32 UUIDCount, uint32 QueryCount, uint32 RepeatIters)
{
    //=====================================
    // Prepare Input
    //=====================================

    {
        gnu_uuid2int_map.clear();
        for(int i = 0; i<UUIDCount; i++)
        {
            const UUID &UUID_key = uuid_mgr_t::instance()->at(i);
            uint32 uint32_value = i;
            gnu_uuid2int_map.insert(std::make_pair(UUID_key, uint32_value));
        }
    }

    //=====================================
    // Enter Kernel
    //=====================================

    {
        // initialize timer
        KernelApi::event_type StartEvent;
        KernelApi::event_type StopEvent;
        Api.event.create(&StartEvent);
        Api.event.create(&StopEvent);
        tbb::tick_count StartTick = tbb::tick_count::now();
        Api.event.record(StartEvent);

        for(int i = 0; i<RepeatIters; i++)
            tbb::parallel_for(tbb::blocked_range<int>(0, QueryCount), query_gnu_t());

        // finalize timer
        Api.event.record(StopEvent);
        Api.event.synchronize(StopEvent);
        std::cout << "run time (reported by CUDA): " <<
            Api.event.queryElapsed(StartEvent, StopEvent)/RepeatIters << " ms" << std::endl;
        tbb::tick_count StopTick = tbb::tick_count::now();
        std::cout << "run time (reported by TBB): " <<
            (StopTick-StartTick).seconds()*1000.0/RepeatIters << " ms" << std::endl;
    }

    //=====================================
    // Check Result
    //=====================================

    {
    }
}
static void test_query_rev_gnu(uint32 UUIDCount, uint32 QueryCount, uint32 RepeatIters)
{
    //=====================================
    // Prepare Input
    //=====================================

    {
        gnu_uuid2int_map.clear();
        for(int i = 0; i<UUIDCount; i++)
        {
            uint32 uint32_key = i;
            const UUID &UUID_value = uuid_mgr_t::instance()->at(i);
            gnu_int2uuid_map.insert(std::make_pair(uint32_key, UUID_value));
        }
    }

    //=====================================
    // Enter Kernel
    //=====================================

    {
        // initialize timer
        KernelApi::event_type StartEvent;
        KernelApi::event_type StopEvent;
        Api.event.create(&StartEvent);
        Api.event.create(&StopEvent);
        tbb::tick_count StartTick = tbb::tick_count::now();
        Api.event.record(StartEvent);

        for(int i = 0; i<RepeatIters; i++)
            tbb::parallel_for(tbb::blocked_range<int>(0, QueryCount), query_rev_gnu_t());

        // finalize timer
        Api.event.record(StopEvent);
        Api.event.synchronize(StopEvent);
        std::cout << "run time (reported by CUDA): " <<
            Api.event.queryElapsed(StartEvent, StopEvent)/RepeatIters << " ms" << std::endl;
        tbb::tick_count StopTick = tbb::tick_count::now();
        std::cout << "run time (reported by TBB): " <<
            (StopTick-StartTick).seconds()*1000.0/RepeatIters << " ms" << std::endl;
    }

    //=====================================
    // Check Result
    //=====================================

    {
    }
}

//=============================================================================
// All Tests
//=============================================================================

static void test_query_all(uint32 UUIDCount, uint32 QueryCount)
{
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Testing data (" <<
        "UUIDCount="  << UUIDCount  << ", " <<
        "QueryCount=" << QueryCount << ")"  << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << std::endl;

    std::cout << "[TBB -- QUERY (uuid --> int)]" << std::endl;
    test_query_tbb(UUIDCount, QueryCount, DEFAULT_REPEAT_ITERS);
    std::cout << std::endl;
    std::cout << "[TBB -- QUERY (int --> uuid)]" << std::endl;
    test_query_rev_tbb(UUIDCount, QueryCount, DEFAULT_REPEAT_ITERS);
    std::cout << std::endl;

    std::cout << "[STL -- QUERY (uuid --> int)]" << std::endl;
    test_query_stl(UUIDCount, QueryCount, DEFAULT_REPEAT_ITERS);
    std::cout << std::endl;
    std::cout << "[STL -- QUERY (int --> uuid)]" << std::endl;
    test_query_rev_stl(UUIDCount, QueryCount, DEFAULT_REPEAT_ITERS);
    std::cout << std::endl;

    std::cout << "[GNU -- QUERY (uuid --> int)]" << std::endl;
    test_query_gnu(UUIDCount, QueryCount, DEFAULT_REPEAT_ITERS);
    std::cout << std::endl;
    std::cout << "[GNU -- QUERY (int --> uuid)]" << std::endl;
    test_query_rev_gnu(UUIDCount, QueryCount, DEFAULT_REPEAT_ITERS);
    std::cout << std::endl;
}

}

//=============================================================================
// Main
//=============================================================================

int main()
{
    #if 0
        tbb::parallel_for(tbb::blocked_range<int>(0, 100000000), Body());
        for(CounterType::const_iterator i = MyCounters.begin(); i != MyCounters.end(); ++i)
        {
            printf("Thread stats:\n");
            printf(" calls to operator(): %d", i->first);
            printf(" total # of iterations executed: %d\n\n", i->second);
        }
    #endif

    test_query_all(DEFAULT_UUID_COUNT_0, DEFAULT_QUERY_COUNT_0);

    test_query_all(DEFAULT_UUID_COUNT_1, DEFAULT_QUERY_COUNT_0);
    test_query_all(DEFAULT_UUID_COUNT_1, DEFAULT_QUERY_COUNT_1);

    test_query_all(DEFAULT_UUID_COUNT_2, DEFAULT_QUERY_COUNT_0);
    test_query_all(DEFAULT_UUID_COUNT_2, DEFAULT_QUERY_COUNT_1);
    test_query_all(DEFAULT_UUID_COUNT_2, DEFAULT_QUERY_COUNT_2);
}
