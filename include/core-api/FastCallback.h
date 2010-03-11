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

#ifndef ZILLIANS_FASTCALLBACK_H_
#define ZILLIANS_FASTCALLBACK_H_

#include "core-api/Common.h"

namespace zillians {

/**
 * \brief FastCallback provides fast callback mechanism which is useful in various asynchronous application
 */
class FastCallback
{
public:
	FastCallback() { }
	FastCallback(const FastCallback &obj) { }
	virtual ~FastCallback() { }

public:
	virtual void invoke() = 0;
};

/**
 * \brief FastCallback0 is a templated callback which takes no parameter
 */
class FastCallback0 : public FastCallback
{
public:
	FastCallback0() { clear(); }
	FastCallback0(const FastCallback0 &obj)
	{
		mFastCallback = obj.mFastCallback;
		mReference = obj.mReference;
	}
	virtual ~FastCallback0() { }

public:
	template <class T, void (T::*method)()>
	inline void bind(T* obj)
	{
		mReference = (void*)obj;
		mFastCallback = method_body<T, method>;
	}

	template <class T, void (T::*method)()>
	inline static void method_body(FastCallback0* ref)
	{
		T* obj = static_cast<T*>(ref->mReference);
		(obj->*method)();
	}

public:
	template <class T, void (T::*method)() const>
	inline void bind(T* obj)
	{
		mReference = (void*)obj;
		mFastCallback = const_method_body<T, method>;
	}

	template <class T, void (T::*method)() const>
	inline static void const_method_body(FastCallback0* ref)
	{
		T* obj = static_cast<T*>(ref->mReference);
		(obj->*method)();
	}

public:
	inline virtual void invoke()
	{
		if(mFastCallback) mFastCallback(this);
	}

	inline void clear()
	{
		mFastCallback = NULL; mReference = NULL;
	}

private:
	void (*mFastCallback)(FastCallback0* ref);
	void *mReference;
};

/**
 * \brief FastCallback01is a templated callback which takes single parameter
 */
class FastCallback1 : public FastCallback
{
public:
	FastCallback1() { clear(); }
	FastCallback1(const FastCallback1 &obj)
	{
		mFastCallback = obj.mFastCallback;
		mReference = obj.mReference;
		mArg0 = obj.mArg0;
	}
	virtual ~FastCallback1() { }

public:
	template <class T, class A0, void (T::*method)(A0)>
	inline void bind(T* obj, A0 arg0)
	{
		mReference = (void*)obj;
		mArg0 = reinterpret_cast<int*>(arg0);
		mFastCallback = method_body<T, A0, method>;
	}

	template <class T, class A0, void (T::*method)(A0)>
	inline static void method_body(FastCallback1* ref)
	{
		T* obj = static_cast<T*>(ref->mReference);
		A0 arg0 = reinterpret_cast<A0>(ref->mArg0);
		(obj->*method)(arg0);
	}

public:
	template <class T, class A0, void (T::*method)(A0) const>
	inline void bind(T* obj, A0 arg0)
	{
		mReference = (void*)obj;
		mArg0 = reinterpret_cast<int*>(arg0);
		mFastCallback = const_method_body<T, method>;
	}

	template <class T, class A0, void (T::*method)(A0) const>
	inline static void const_method_body(FastCallback1* ref)
	{
		T* obj = static_cast<T*>(ref->mReference);
		A0 arg0 = reinterpret_cast<A0>(ref->mArg0);
		(obj->*method)(arg0);
	}

public:
	inline virtual void invoke()
	{
		if(mFastCallback) mFastCallback(this);
	}

	inline void clear()
	{
		mFastCallback = NULL; mReference = NULL; mArg0 = NULL;
	}

private:
	void (*mFastCallback)(FastCallback1* ref);
	void *mReference;
	int *mArg0;
};

/**
 * \brief FastCallback01is a templated callback which takes parameter list with two elements
 */
class FastCallback2 : public FastCallback
{
public:
	FastCallback2() { clear(); }
	FastCallback2(const FastCallback2 &obj)
	{
		mFastCallback = obj.mFastCallback;
		mReference = obj.mReference;
		mArg0 = obj.mArg0;
		mArg1 = obj.mArg1;
	}
	virtual ~FastCallback2() { }

public:
	template <class T, class A0, class A1, void (T::*method)(A0, A1)>
	inline void bind(T* obj, A0 arg0, A1 arg1)
	{
		mReference = (void*)obj;
		mArg0 = reinterpret_cast<int*>(arg0);
		mArg1 = reinterpret_cast<int*>(arg1);
		mFastCallback = method_body<T, A0, A1, method>;
	}

	template <class T, class A0, class A1, void (T::*method)(A0, A1)>
	inline static void method_body(FastCallback2* ref)
	{
		T* obj = static_cast<T*>(ref->mReference);
		A0 arg0 = reinterpret_cast<A0>(ref->mArg0);
		A1 arg1 = reinterpret_cast<A1>(ref->mArg1);
		(obj->*method)(arg0, arg1);
	}

public:
	template <class T, class A0, class A1, void (T::*method)(A0, A1) const>
	inline void bind(T* obj, A0 arg0, A1 arg1)
	{
		mReference = (void*)obj;
		mArg0 = reinterpret_cast<int*>(arg0);
		mArg1 = reinterpret_cast<int*>(arg1);
		mFastCallback = const_method_body<T, method>;
	}

	template <class T, class A0, class A1, void (T::*method)(A0, A1) const>
	inline static void const_method_body(FastCallback2* ref)
	{
		T* obj = static_cast<T*>(ref->mReference);
		A0 arg0 = reinterpret_cast<A0>(ref->mArg0);
		A1 arg1 = reinterpret_cast<A1>(ref->mArg1);
		(obj->*method)(arg0, arg1);
	}

public:
	inline virtual void invoke()
	{
		if(mFastCallback) mFastCallback(this);
	}

	inline void clear()
	{
		mFastCallback = NULL; mReference = NULL; mArg0 = NULL; mArg1 = NULL;
	}

private:
	void (*mFastCallback)(FastCallback2* ref);
	void *mReference;
	int *mArg0;
	int *mArg1;
};

/**
 * \brief FastCallback01is a templated callback which takes parameter list with three elements
 */
class FastCallback3 : public FastCallback
{
public:
	FastCallback3() { clear(); }
	FastCallback3(const FastCallback3 &obj)
	{
		mFastCallback = obj.mFastCallback;
		mReference = obj.mReference;
		mArg0 = obj.mArg0;
		mArg1 = obj.mArg1;
		mArg2 = obj.mArg2;
	}
	virtual ~FastCallback3() { }

public:
	template <class T, class A0, class A1, class A2, void (T::*method)(A0, A1, A2)>
	inline void bind(T* obj, A0 arg0, A1 arg1, A2 arg2)
	{
		mReference = (void*)obj;
		mArg0 = reinterpret_cast<int*>(arg0);
		mArg1 = reinterpret_cast<int*>(arg1);
		mArg2 = reinterpret_cast<int*>(arg2);
		mFastCallback = method_body<T, A0, A1, A2, method>;
	}

	template <class T, class A0, class A1, class A2, void (T::*method)(A0, A1, A2)>
	inline static void method_body(FastCallback3* ref)
	{
		T* obj = static_cast<T*>(ref->mReference);
		A0 arg0 = reinterpret_cast<A0>(ref->mArg0);
		A1 arg1 = reinterpret_cast<A1>(ref->mArg1);
		A2 arg2 = reinterpret_cast<A2>(ref->mArg2);
		(obj->*method)(arg0, arg1, arg2);
	}

public:
	template <class T, class A0, class A1, class A2, void (T::*method)(A0, A1, A2) const>
	inline void bind(T* obj, A0 arg0, A1 arg1, A2 arg2)
	{
		mReference = (void*)obj;
		mArg0 = reinterpret_cast<int*>(arg0);
		mArg1 = reinterpret_cast<int*>(arg1);
		mArg2 = reinterpret_cast<int*>(arg2);
		mFastCallback = const_method_body<T, method>;
	}

	template <class T, class A0, class A1, class A2, void (T::*method)(A0, A1, A2) const>
	inline static void const_method_body(FastCallback3* ref)
	{
		T* obj = static_cast<T*>(ref->mReference);
		A0 arg0 = reinterpret_cast<A0>(ref->mArg0);
		A1 arg1 = reinterpret_cast<A1>(ref->mArg1);
		A2 arg2 = reinterpret_cast<A2>(ref->mArg2);
		(obj->*method)(arg0, arg1, arg2);
	}

public:
	inline virtual void invoke()
	{
		if(mFastCallback) mFastCallback(this);
	}

	inline void clear()
	{
		mFastCallback = NULL; mReference = NULL; mArg0 = NULL; mArg1 = NULL; mArg2 = NULL;
	}

private:
	void (*mFastCallback)(FastCallback3* ref);
	void *mReference;
	int *mArg0;
	int *mArg1;
	int *mArg2;
};

}

#endif/*ZILLIANS_FASTCALLBACK_H_*/
