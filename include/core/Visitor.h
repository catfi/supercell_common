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

namespace zillians {

namespace detail {

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

} // detail

template<typename Base>
struct VisitableBase
{
	template<typename Visitable>
	size_t _get_tag_helper(const Visitable* v) const
	{
		std::size_t t = detail::get_tag<Visitable, Base>();
		printf("tag for %s = %ld\n", typeid(v).name(), t);
		return t;
	}
};

#define DEFINE_VISITABLE()					\
		virtual size_t tag() const			\
		{									\
			return _get_tag_helper(this);	\
		}

template<typename Base, typename Function>
struct VTable {
	std::vector<Function> mTable;

	template<typename Visitable>
	void add(Function f)
	{
		size_t index = detail::get_tag<Visitable, Base>();

		printf("adding %s to vtable, index = %ld\n", typeid(Visitable).name(), index);

		if(index >= mTable.size())
		{
			const size_t base_tag = detail::get_tag<Base, Base>();
			Function default_function = (base_tag >= mTable.size()) ? 0 : mTable[base_tag];
			printf("resize, base_tag = %ld, default function = %p\n", base_tag, default_function);
			mTable.resize(index + 1, default_function);
		}

		mTable[index] = f;
		printf("register type = %s index = %d to function %p\n", typeid(Visitable).name(), index, f);
	}

	Function operator[] (size_t index) const
	{
		if(index > mTable.size())
		{
			printf("getting index larger then mTable, index = %ld, mTable.size() = %ld\n", index, mTable.size());
			index = detail::get_tag<Base, Base>();
		}

		Function f = mTable[index];
		printf("getting function for index = %ld, function = %p\n", index, f);
		return f;
	}
};

template<typename Visitable, typename Base>
struct GetVisitMethodArgumentType {
  typedef Visitable Type;
};

// specialize for const Base
template<typename Visitable, typename Base >
struct GetVisitMethodArgumentType< Visitable, const Base> {
  typedef const Visitable Type;
};

template<typename Visitor, typename VisitedList, typename Invoker>
struct CreateVtable
{
	struct AppendHelper {
		AppendHelper(CreateVtable<Visitor, VisitedList, Invoker>* _cv) : cv(_cv) { }

		template< typename Visitable >
		void operator()(Visitable v)
		{
			printf("register %s\n", typeid(v).name());
			cv->mVTable.template add<Visitable>(
					&Visitor::template thunk<Visitor, Visitable, Invoker>
			);
		}

		CreateVtable<Visitor, VisitedList, Invoker>* cv;
	};

	typename Visitor::VTableType mVTable;

	CreateVtable()
	{
		//(*this)(static_cast<typename Visitor::BaseT*>(0));
		AppendHelper helper(this);

		helper(typename Visitor::BaseT());
		boost::mpl::for_each<VisitedList>(helper); // TODO problem here
	}
};

template<typename Visitor, typename VisitedList, typename Invoker>
struct GetStaticVtable {
	// declare static instanceof vtable
	static CreateVtable<Visitor, VisitedList, Invoker> s_table;

	// provide conversion operator
	operator const typename Visitor::VTableType*() const
	{
		return &s_table.mVTable;
	}
};

template< typename Visitor, typename VisitedList, typename Invoker>
CreateVtable<Visitor, VisitedList, Invoker> GetStaticVtable<Visitor, VisitedList, Invoker>::s_table;

// global helper function
template<typename Visitor, typename VisitedList, typename Invoker>
void visits(Visitor& visitor, const VisitedList&, const Invoker&)
{
	// instantiate the static vtable and set the vtable pointer
	visitor.mVTable = GetStaticVtable<Visitor, VisitedList, Invoker>();
}

template< typename Base, typename ReturnType>
struct Visitor {
	typedef Base BaseT;
	typedef ReturnType ReturnTypeT;
	template<typename VisitorImpl, typename Visitable, typename Invoker>
	ReturnType thunk(Base& b)
	{
		typedef typename GetVisitMethodArgumentType<Visitable, Base>::Type VisitableType;
		VisitorImpl& visitor = static_cast<VisitorImpl&>(*this);
		VisitableType& visitable = static_cast<VisitableType&>(b);
		return Invoker::invoke(visitor, visitable);
	}

	typedef ReturnType (Visitor::*Function)(Base&);

	typedef VTable<const Base, Function> VTableType;
	const VTableType* mVTable;

	ReturnType operator()(Base& b)
	{
		Function f = (*mVTable)[b.tag()];
		printf("invoke Visitor::%p\n", f);
		return (this->*f)(b);
	}
};

#define VISIT_INVOKER( name )                    \
		struct { \
			template<typename VisitorImpl, typename Visitable> \
			static ReturnTypeT invoke(VisitorImpl& visitor, Visitable& visitable) \
			{ \
				return visitor.name(visitable); \
			} \
		}

}

#endif /* ZILLIANS_VISITOR_H_ */
