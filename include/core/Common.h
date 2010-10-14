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

#ifndef ZILLIANS_COMMON_H_
#define ZILLIANS_COMMON_H_

//////////////////////////////////////////////////////////////////////////
// Include common headers
#include "core/Platform.h"
#include "core/Types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <algorithm>

#ifdef _WIN32
#define MSVC_VERSION	_MSC_VER
#else
#define GCC_VERSION		(__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#ifdef _WIN32
#include <xutility>
#endif

//////////////////////////////////////////////////////////////////////////
#define ZN_OK		0
#define ZN_ERROR	-1

//////////////////////////////////////////////////////////////////////////
/// Macros to help safely free memory and nullize afterwards
#ifndef NULL
#define NULL ((void*)0)
#endif

#define SAFE_NULL(x)   { x = NULL; }
#define SAFE_FREE(x)   { if(!!(x)) ::free(x); x = NULL; }
#define SAFE_DELETE(x) { if(!!(x)) delete (x); x = NULL; }
#define SAFE_DELETE_ARRAY(x) { if(!!(x)) delete[] (x); x = NULL; }

//////////////////////////////////////////////////////////////////////////
/// Macros to support C++ visibility (supported after GCC 4.0 to reduce DSO performance)
#if defined _WIN32 || defined __CYGWIN__
	#ifdef BUILD_SHARED_LIBS
		#ifdef __GNUC__
			#define ZILLIANS_API __attribute__((dllexport))
		#else
			#define ZILLIANS_API __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
		#endif
	#elif defined __GNUC__
		#define ZILLIANS_API __attribute__((dllimport))
	#else
		#define ZILLIANS_API __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
	#endif
	#define DLL_LOCAL
#else
	#if __GNUC__ >= 4
		#define ZILLIANS_API 	__attribute__ ((visibility("default")))
		#define ZILLIANS_LOCAL  __attribute__ ((visibility("hidden")))
	#else
		#define ZILLIANS_API
		#define ZILLIANS_LOCAL
	#endif
#endif

#ifdef __GNUC__
#define pure_function __attribute__((const))
#else
#define pure_function
#endif

#define restricted_pointer __restrict

// Generic helper definitions for shared library support
//#if defined _WIN32 || defined __CYGWIN__
//	#define ZILLIANS_HELPER_DLL_IMPORT __declspec(dllimport)
//	#define ZILLIANS_HELPER_DLL_EXPORT __declspec(dllexport)
//	#define ZILLIANS_HELPER_DLL_LOCAL
//#else
//	#if __GNUC__ >= 4
//		#define ZILLIANS_HELPER_DLL_IMPORT __attribute__ ((visibility("default")))
//		#define ZILLIANS_HELPER_DLL_EXPORT __attribute__ ((visibility("default")))
//		#define ZILLIANS_HELPER_DLL_LOCAL  __attribute__ ((visibility("hidden")))
//	#else
//		#define ZILLIANS_HELPER_DLL_IMPORT
//		#define ZILLIANS_HELPER_DLL_EXPORT
//		#define ZILLIANS_HELPER_DLL_LOCAL
//	#endif
//#endif
//
//#ifdef ZILLIANS_BUILD_DLL
//  #ifdef ZILLIANS_DLL_EXPORTS
//    #define API_DECL ZILLIANS_HELPER_DLL_EXPORT
//  #else
//    #define API_DECL ZILLIANS_HELPER_DLL_IMPORT
//  #endif
//  #define API_LOCAL_DECL ZILLIANS_HELPER_DLL_LOCAL
//#else
//  #define API_DECL
//  #define API_LOCAL_DECL
//#endif

//////////////////////////////////////////////////////////////////////////
// GNU GCC branch prediction optimization
// TODO: find cooresponding pragma in MSVC and other compiler
#ifdef __GNUC__
	#define LIKELY(x)   __builtin_expect(!!(x), 1)
	#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
	#define LIKELY(x) (x)
	#define UNLIKELY(x) (x)
#endif

//////////////////////////////////////////////////////////////////////////
/// Enable/Disable template separation model compilation
/// NOTE: currently there's no GNU compiler support for separation compilation model
//#define ZN_ENABLE_TEMPLATE_SEPARATION_MODEL

//////////////////////////////////////////////////////////////////////////
// Network Parameters
#define ZN_IPV6_SUPPORT 0
#define ZN_DEFER_ACCEPT_SUPPORT 1

//////////////////////////////////////////////////////////////////////////
// MIN and MAX macro (using GNU C++ extension if applicable)
//#ifdef __GNUC__
//	#define MIN(x,y) ((x) <? (y))
//	#define MAX(x,y) ((x) >? (y))
//#else
//	#define MIN(x,y)  ((x) < (y) ? (x) : (y))
//	#define MAX(x,y)  ((x) > (y) ? (x) : (y))
//#endif


#endif/*ZILLIANS_COMMON_H_*/
