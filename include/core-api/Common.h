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
#include "core-api/Platform.h"
#include "core-api/Types.h"
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
// GNU GCC branch prediction optimization
// TODO: find cooresponding pragma in MSVC and other compiler
//#ifdef __GNUC__
//	#define LIKELY(x)   __builtin_expect(!!(x), 1)
//	#define UNLIKELY(x) __builtin_expect(!!(x), 0)
//#else
	#define LIKELY(x) (x)
	#define UNLIKELY(x) (x)
//#endif

//////////////////////////////////////////////////////////////////////////
/// Enable/Disable template separation model compiliation
/// NOTE: currently there's no GNU compiler support for separation compiliation model
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
