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
 * @date Aug 6, 2010 sdk - Initial version created.
 */

#ifndef ZILLIANS_TEMPLATETRICKS_H_
#define ZILLIANS_TEMPLATETRICKS_H_

#include <boost/mpl/assert.hpp>
#include <boost/mpl/has_xxx.hpp>

// Introspection for member templates.
#define HAS_MEMBER_TEMPLATE_ACCESS( \
            args, class_type, param \
        ) \
    class_type::template BOOST_PP_ARRAY_ELEM(1, args) \
/**/
#define HAS_MEMBER_TEMPLATE_SUBSTITUTE_PARAMETER( \
            args, param \
        ) \
    template< \
        BOOST_PP_ENUM_PARAMS( \
            BOOST_PP_ARRAY_ELEM(2, args) \
          , typename param \
        ) \
> \
    class param

#define HAS_MEMBER_TEMPLATE(name, n) \
    BOOST_MPL_HAS_MEMBER_IMPLEMENTATION( \
        ( 4, ( BOOST_PP_CAT(has_, name), name, n, false ) ) \
      , BOOST_MPL_HAS_MEMBER_INTROSPECT \
      , HAS_MEMBER_TEMPLATE_SUBSTITUTE_PARAMETER \
      , HAS_MEMBER_TEMPLATE_ACCESS \
    )

// Introspection for member functions.
#define HAS_MEMBER_FUNCTION_ACCESS( \
            args, class_type, param \
        ) \
    &class_type::BOOST_PP_ARRAY_ELEM(1, args)

#define HAS_MEMBER_FUNCTION_SUBSTITUTE_PARAMETER( \
            args, param \
        ) \
    BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_ARRAY_ELEM(4, args)) \
    (U::*) \
    BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_ARRAY_ELEM(4, args))

#define HAS_MEMBER_FUNCTION(name, sig) \
    BOOST_MPL_HAS_MEMBER_IMPLEMENTATION( \
        ( 5, ( BOOST_PP_CAT(has_, name), name, 0, false, sig ) ) \
      , BOOST_MPL_HAS_MEMBER_INTROSPECT \
      , HAS_MEMBER_FUNCTION_SUBSTITUTE_PARAMETER \
      , HAS_MEMBER_FUNCTION_ACCESS \
    )

// Introspection for member static functions.
#define HAS_MEMBER_STATIC_FUNCTION_SUBSTITUTE_PARAMETER( \
            args, param \
        ) \
    BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_ARRAY_ELEM(4, args)) \
    (*) \
    BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_ARRAY_ELEM(4, args))


#define HAS_MEMBER_STATIC_FUNCTION(name, sig) \
    BOOST_MPL_HAS_MEMBER_IMPLEMENTATION( \
        ( 5, ( BOOST_PP_CAT(has_, name), name, 0, false, sig ) ) \
      , BOOST_MPL_HAS_MEMBER_INTROSPECT \
      , HAS_MEMBER_STATIC_FUNCTION_SUBSTITUTE_PARAMETER \
      , HAS_MEMBER_FUNCTION_ACCESS \
    )

#endif /* ZILLIANS_TEMPLATETRICKS_H_ */
