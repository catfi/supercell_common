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
 * @date Aug 3, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_VISITOR_H_
#define ZILLIANS_VISITOR_H_

#include "core/Prerequisite.h"
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#define ENABLE_DEBUG_VISITOR	0

namespace zillians { namespace visitor { namespace detail {

template<typename Base>
struct tag_counter
{
  static size_t s_counter;
};

template<typename Base>
size_t tag_counter<Base>::s_counter = 0;

template<typename Visitable, typename Base>
struct tag_holder
{
	static size_t s_tag;
};

template<typename Visitable, typename Base>
size_t get_tag()
{
	size_t& tag = tag_holder<const Visitable, const Base>::s_tag;
	if( tag == 0 )
	tag = ++tag_counter<const Base>::s_counter;
	return tag;
}

template<typename Visitable, typename Base>
size_t tag_holder<Visitable, Base>::s_tag = get_tag<Visitable, Base>();


template<typename Base, typename Function>
struct vtable {
	std::vector<Function> mTable;

	template<typename Visitable>
	void add(Function f)
	{
		size_t index = get_tag<Visitable, Base>();

#if ENABLE_DEBUG_VISITOR
		printf("adding %s to vtable, index = %ld\n", typeid(Visitable).name(), index);
#endif

		if(index >= mTable.size())
		{
			const size_t base_tag = get_tag<Base, Base>();
			Function default_function = (base_tag >= mTable.size()) ? 0 : mTable[base_tag];
#if ENABLE_DEBUG_VISITOR
			printf("resize, base_tag = %ld, default function = %p\n", base_tag, default_function);
#endif
			mTable.resize(index + 1, default_function);
		}

		mTable[index] = f;
#if ENABLE_DEBUG_VISITOR
		printf("register type = %s index = %d to function %p\n", typeid(Visitable).name(), index, f);
#endif
	}

	Function operator[] (size_t index) const
	{
		if(index > mTable.size())
		{
#if ENABLE_DEBUG_VISITOR
			printf("getting index larger then mTable, index = %ld, mTable.size() = %ld\n", index, mTable.size());
#endif
			index = get_tag<Base, Base>();
		}

		Function f = mTable[index];
#if ENABLE_DEBUG_VISITOR
		printf("getting function for index = %ld, function = %p\n", index, f);
#endif
		return f;
	}
};


template<typename Visitable, typename Base>
struct get_visit_method_argument_type {
  typedef Visitable Type;
};

// specialize for const Base
template<typename Visitable, typename Base >
struct get_visit_method_argument_type< Visitable, const Base> {
  typedef const Visitable Type;
};

template<typename Visitor, typename VisitedList, typename Invoker>
struct create_vtable
{
	struct vtable_append_helper {
		vtable_append_helper(create_vtable<Visitor, VisitedList, Invoker>* _cvbl) : cvbl(_cvbl) { }

		template< typename Visitable >
		void operator()(Visitable v)
		{
			cvbl->vtable.template add<Visitable>(
					&Visitor::template thunk<Visitor, Visitable, Invoker>
			);
		}

		create_vtable<Visitor, VisitedList, Invoker>* cvbl;
	};

	typename Visitor::VTableT vtable;

	create_vtable()
	{
		vtable_append_helper helper(this);
		helper(typename Visitor::BaseT());
		boost::mpl::for_each<VisitedList>(helper);
	}
};

template<typename Visitor, typename VisitedList, typename Invoker>
struct get_static_vtable {
	static create_vtable<Visitor, VisitedList, Invoker> s_table;
	operator const typename Visitor::VTableT*() const
	{
		return &s_table.vtable;
	}
};

template< typename Visitor, typename VisitedList, typename Invoker>
create_vtable<Visitor, VisitedList, Invoker> get_static_vtable<Visitor, VisitedList, Invoker>::s_table;

} } // detail::visitor

template<typename Base>
struct VisitableBase
{
	template<typename Visitable>
	size_t _get_tag_helper(const Visitable* v) const
	{
		std::size_t t = visitor::detail::get_tag<Visitable, Base>();
#if ENABLE_DEBUG_VISITOR
		printf("tag for %s = %ld\n", typeid(v).name(), t);
#endif
		return t;
	}
};

#define DEFINE_VISITABLE()					\
		virtual size_t tag() const			\
		{									\
			return _get_tag_helper(this);	\
		}

template< typename Base, typename ReturnType>
struct Visitor {
	typedef Base BaseT;
	typedef ReturnType ReturnT;
	typedef ReturnType (Visitor::*FunctionT)(Base&);
	typedef visitor::detail::vtable<const Base, FunctionT> VTableT;

	template<typename VisitorImpl, typename Visitable, typename Invoker>
	ReturnType thunk(Base& b)
	{
		typedef typename visitor::detail::get_visit_method_argument_type<Visitable, Base>::Type VisitableType;
		VisitorImpl& visitor = static_cast<VisitorImpl&>(*this);
		VisitableType& visitable = static_cast<VisitableType&>(b);
		return Invoker::invoke(visitor, visitable);
	}

	const VTableT* mVTable;

	ReturnType operator()(Base& b)
	{
		FunctionT f = (*mVTable)[b.tag()];
#if ENABLE_DEBUG_VISITOR
		printf("invoke Visitor::%p\n", f);
#endif
		return (this->*f)(b);
	}

	// global helper function
	template<typename Visitor, typename VisitedList, typename Invoker>
	static void _register_visitable(Visitor& visitor, const VisitedList&, const Invoker&)
	{
		visitor.mVTable = visitor::detail::get_static_vtable<Visitor, VisitedList, Invoker>();
	}

};

#define REGISTER_VISITABLE(invoker, ...)		\
		_register_visitable(*this, boost::mpl::vector<__VA_ARGS__>(), invoker());

#define CREATE_INVOKER(invoker, function_name)                    \
		typedef struct { \
			template<typename VisitorImpl, typename Visitable> \
			static ReturnT invoke(VisitorImpl& visitor, Visitable& visitable) \
			{ \
				return visitor.function_name(visitable); \
			} \
		} invoker;

}

#endif /* ZILLIANS_VISITOR_H_ */
