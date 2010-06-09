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
/**
 * @date Mar 3, 2009 sdk - Initial version created.
 */

#include "core/Prerequisite.h"
#include "core/FragmentFreeAllocator.h"
#include <iostream>
#include <string>
#include <limits>

#define BOOST_TEST_MODULE FragmentFreeAllocatorTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace zillians;
using namespace std;

BOOST_AUTO_TEST_SUITE( FragmentFreeAllocatorTest )

BOOST_AUTO_TEST_CASE( FragmentFreeAllocatorTestCase1 )
{
	const std::size_t size = 20*1024*1024;
	char* raw = new char[size];

	// reserve 20MB at the beginning
	FragmentAllocator allocator(raw, size);

	BOOST_CHECK(allocator.available() == size);

	// first allocation 7MB, free 13MB
	MutablePointer* ptr1 = NULL;
	std::size_t size1 = 7*1024*1024;
	{
		allocator.allocate(&ptr1, size1);
		BOOST_CHECK(allocator.available() == size - size1);
	}

	// second allocation 8MB, free 5MB
	MutablePointer* ptr2 = NULL;
	std::size_t size2 = 8*1024*1024;
	{
		allocator.allocate(&ptr2, size2);
		BOOST_CHECK(allocator.available() == size - size1 - size2);
	}

	// free the first allocation, free 12MB
	allocator.deallocate(ptr1);
	BOOST_CHECK(allocator.available() == size - size2);

	// free the second allocation, free 20MB
	allocator.deallocate(ptr2);
	BOOST_CHECK(allocator.available() == size);

	delete[] raw; raw = NULL;
}

BOOST_AUTO_TEST_CASE( FragmentFreeAllocatorTestCase2 )
{
	const std::size_t size = 20*1024*1024;
	char* raw = new char[size];

	// reserve 20MB at the beginning
	FragmentAllocator allocator(raw, size);

	BOOST_CHECK(allocator.available() == size);

	// first allocation 7MB, free 13MB
	MutablePointer* ptr1 = NULL;
	std::size_t size1 = 7*1024*1024;
	{
		allocator.allocate(&ptr1, size1);
		BOOST_CHECK(allocator.available() == size - size1);
	}

	// second allocation 8MB, free 5MB
	MutablePointer* ptr2 = NULL;
	std::size_t size2 = 8*1024*1024;
	{
		allocator.allocate(&ptr2, size2);
		BOOST_CHECK(allocator.available() == size - size1 - size2);
	}

	// free the second allocation, free 13MB
	allocator.deallocate(ptr2);
	BOOST_CHECK(allocator.available() == size - size1);

	// free the first allocation, free 20MB
	allocator.deallocate(ptr1);
	BOOST_CHECK(allocator.available() == size);

	delete[] raw; raw = NULL;
}

BOOST_AUTO_TEST_CASE( FragmentFreeAllocatorTestCase3 )
{
	const std::size_t size = 20*1024*1024;
	char* raw = new char[size];

	// reserve 20MB at the beginning
	FragmentAllocator allocator(raw, size);

	BOOST_CHECK(allocator.available() == size);

	// first allocation 10MB, free 10MB
	MutablePointer* ptr1 = NULL;
	std::size_t size1 = 10*1024*1024;
	{
		allocator.allocate(&ptr1, size1);
		BOOST_CHECK(allocator.available() == size - size1);
	}

	// second allocation 10MB, free 0MB
	MutablePointer* ptr2 = NULL;
	std::size_t size2 = 10*1024*1024;
	{
		allocator.allocate(&ptr2, size2);
		BOOST_CHECK(allocator.available() == size - size1 - size2);
	}

	// free the second allocation, free 10MB
	allocator.deallocate(ptr1);
	BOOST_CHECK(allocator.available() == size - size2);

	// free the first allocation, free 20MB
	allocator.deallocate(ptr2);
	BOOST_CHECK(allocator.available() == size);

	delete[] raw; raw = NULL;
}

BOOST_AUTO_TEST_CASE( FragmentFreeAllocatorTestCase4 )
{
	const std::size_t size = 20*1024*1024;
	char* raw = new char[size];

	// reserve 20MB at the beginning
	FragmentAllocator allocator(raw, size);

	BOOST_CHECK(allocator.available() == size);

	// 1st allocation 1MB, free 19MB
	MutablePointer* ptr1 = NULL;
	std::size_t size1 = 1*1024*1024;
	{
		allocator.allocate(&ptr1, size1);
		BOOST_CHECK(allocator.available() == size - size1);
	}

	// 2nd allocation 2MB, free 17MB
	MutablePointer* ptr2 = NULL;
	std::size_t size2 = 2*1024*1024;
	{
		allocator.allocate(&ptr2, size2);
		BOOST_CHECK(allocator.available() == size - size1 - size2);
	}

	// 3rd allocation 3MB, free 14MB
	MutablePointer* ptr3 = NULL;
	std::size_t size3 = 3*1024*1024;
	{
		allocator.allocate(&ptr3, size3);
		BOOST_CHECK(allocator.available() == size - size1 - size2 - size3);
	}

	// 4th allocation 4MB, free 10MB
	MutablePointer* ptr4 = NULL;
	std::size_t size4 = 4*1024*1024;
	{
		allocator.allocate(&ptr4, size4);
		BOOST_CHECK(allocator.available() == size - size1 - size2 - size3 - size4);
	}

	// 5th allocation 5MB, free 5MB
	MutablePointer* ptr5 = NULL;
	std::size_t size5 = 5*1024*1024;
	{
		allocator.allocate(&ptr5, size5);
		BOOST_CHECK(allocator.available() == size - size1 - size2 - size3 - size4 - size5);
	}

	// free the 1st allocation, free 6MB
	allocator.deallocate(ptr1);
	BOOST_CHECK(allocator.available() == size - size2 - size3 - size4 - size5);

	// free the 2nd allocation, free 8MB
	allocator.deallocate(ptr2);
	BOOST_CHECK(allocator.available() == size - size3 - size4 - size5);

	// free the 3rd allocation, free 11MB
	allocator.deallocate(ptr3);
	BOOST_CHECK(allocator.available() == size - size4 - size5);

	// free the 4th allocation, free 15MB
	allocator.deallocate(ptr4);
	BOOST_CHECK(allocator.available() == size - size5);

	// free the 5th allocation, free 20MB
	allocator.deallocate(ptr5);
	BOOST_CHECK(allocator.available() == size);

	delete[] raw; raw = NULL;
}

BOOST_AUTO_TEST_CASE( FragmentFreeAllocatorTestCase5 )
{
	const std::size_t size = 20*1024*1024;
	char* raw = new char[size];

	// reserve 20MB at the beginning
	FragmentAllocator allocator(raw, size);

	BOOST_CHECK(allocator.available() == size);

	// 1st allocation 1MB, free 19MB
	MutablePointer* ptr1 = NULL;
	std::size_t size1 = 1*1024*1024;
	{
		BOOST_CHECK(allocator.allocate(&ptr1, size1));
		BOOST_CHECK(ptr1);
		BOOST_CHECK(allocator.available() == size - size1);
	}

	// 2nd allocation 2MB, free 17MB
	MutablePointer* ptr2 = NULL;
	std::size_t size2 = 2*1024*1024;
	{
		BOOST_CHECK(allocator.allocate(&ptr2, size2));
		BOOST_CHECK(ptr2);
		BOOST_CHECK(allocator.available() == size - size1 - size2);
	}

	// 3rd allocation 3MB, free 14MB
	MutablePointer* ptr3 = NULL;
	std::size_t size3 = 3*1024*1024;
	{
		BOOST_CHECK(allocator.allocate(&ptr3, size3));
		BOOST_CHECK(ptr3);
		BOOST_CHECK(allocator.available() == size - size1 - size2 - size3);
	}

	// 4th allocation 4MB, free 10MB
	MutablePointer* ptr4 = NULL;
	std::size_t size4 = 4*1024*1024;
	{
		BOOST_CHECK(allocator.allocate(&ptr4, size4));
		BOOST_CHECK(ptr4);
		BOOST_CHECK(allocator.available() == size - size1 - size2 - size3 - size4);
	}

	// 5th allocation 5MB, free 5MB
	MutablePointer* ptr5 = NULL;
	std::size_t size5 = 5*1024*1024;
	{
		BOOST_CHECK(allocator.allocate(&ptr5, size5));
		BOOST_CHECK(ptr5);
		BOOST_CHECK(allocator.available() == size - size1 - size2 - size3 - size4 - size5);
	}

	// 20MB = | 1MB | 2MB | 3MB | 4MB | 5MB | 5MB(Free) |

	// free the 1st allocation, free 6MB
	BOOST_CHECK(allocator.deallocate(ptr1));
	BOOST_CHECK(allocator.available() == size - size2 - size3 - size4 - size5);

	// 20MB = | 1MB(Free) | 2MB | 3MB | 4MB | 5MB | 5MB(Free) |

	// free the 3rd allocation, free 9MB
	BOOST_CHECK(allocator.deallocate(ptr3));
	BOOST_CHECK(allocator.available() == size - size2 - size4 - size5);

	// 20MB = | 1MB(Free) | 2MB | 3MB(Free) | 4MB | 5MB | 5MB(Free) |

	// free the 4th allocation, free 13MB
	BOOST_CHECK(allocator.deallocate(ptr4));
	BOOST_CHECK(allocator.available() == size - size2 - size5);

	// 20MB = | 1MB(Free) | 2MB | 3MB(Free) | 4MB(Free) | 5MB | 5MB(Free) |
	// 20MB = | 1MB(Free) | 2MB | 7MB(Free)(Merged) | 5MB | 5MB(Free) |

	// 6th allocation 8MB, failed
	MutablePointer* ptr6 = NULL;
	std::size_t size6 = 8*1024*1024;
	{
		BOOST_CHECK(!allocator.allocate(&ptr6, size6));
		BOOST_CHECK(!ptr6);
		BOOST_CHECK(allocator.available() == size - size2 - size5);
	}

	// defrag the allocator
	{
		FragmentFreeOperator defrag(
				boost::bind(memcpy,
						FragmentFreeOperator::placeholders::dst,
						FragmentFreeOperator::placeholders::src,
						FragmentFreeOperator::placeholders::size));
		defrag(allocator);
	}

	// 20MB = | 2MB | 5MB | 13MB(Free) |

	// 6th allocation 8MB, free 5MB
	{
		BOOST_CHECK(allocator.allocate(&ptr6, size6));
		BOOST_CHECK(ptr6);
		BOOST_CHECK(allocator.available() == size - size2 - size5 - size6);
	}

	// 20MB = | 2MB | 5MB | 8MB | 5MB(Free) |

	// free the 5th allocation, free 11MB
	allocator.deallocate(ptr5);
	BOOST_CHECK(allocator.available() == size - size2 - size6);

	// 20MB = | 2MB | 5MB(Free) | 8MB | 5MB(Free) |

	// free the 6th allocation, free 14MB
	allocator.deallocate(ptr6);
	BOOST_CHECK(allocator.available() == size - size2);

	// 20MB = | 2MB | 5MB(Free) | 8MB(Free) | 5MB(Free) |
	// 20MB = | 2MB | 18MB(Free)(Merged) |

	// 7th allocation 18MB, free 0MB
	MutablePointer* ptr7 = NULL;
	std::size_t size7 = 18*1024*1024;
	{
		BOOST_CHECK(allocator.allocate(&ptr7, size7));
		BOOST_CHECK(ptr7);
		BOOST_CHECK(allocator.available() == size - size2 - size7);
	}

	// free the 2nd allocation, free 2MB
	allocator.deallocate(ptr2);
	BOOST_CHECK(allocator.available() == size - size7);

	// free the 7th allocation, free 20MB
	allocator.deallocate(ptr7);
	BOOST_CHECK(allocator.available() == size);

	delete[] raw; raw = NULL;
}

BOOST_AUTO_TEST_CASE( FragmentFreeAllocatorTestCase6 )
{
	const std::size_t size = 20*1024*1024;
	char* raw = new char[size];

	// reserve 20MB at the beginning
	FragmentAllocator allocator(raw, size);

	delete[] raw; raw = NULL;
}

BOOST_AUTO_TEST_SUITE_END()
