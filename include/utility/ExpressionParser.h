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
 * @date Feb 10, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_EXPRESSIONPARSER_H_
#define ZILLIANS_EXPRESSIONPARSER_H_

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_attribute.hpp>
#include <iostream>
#include <string>

////////////////////////////////////////////////////////////////////////////
using namespace boost::spirit::classic;
using namespace phoenix;

namespace zillians {

namespace {

template<typename T>
struct CalculatorClosure : boost::spirit::classic::closure<CalculatorClosure<T>, T>
{
	typename boost::spirit::classic::closure<CalculatorClosure<T>, T>::member1 val;
};

template<typename T>
struct Calculator : public grammar<Calculator<T>, typename CalculatorClosure<T>::context_t>
{
    template <typename ScannerT>
    struct definition
    {
        definition(Calculator const& self)
        {
            top = expression[self.val = arg1];

            expression
                =   term[expression.val = arg1]
                    >> *(   ('+' >> term[expression.val += arg1])
                        |   ('-' >> term[expression.val -= arg1])
                        )
                ;

            term
                =   factor[term.val = arg1]
                    >> *(   ('*' >> factor[term.val *= arg1])
                        |   ('/' >> factor[term.val /= arg1])
                        )
                ;

            factor
                =   ureal_p[factor.val = arg1]
                |   '(' >> expression[factor.val = arg1] >> ')'
                |   ('-' >> factor[factor.val = -arg1])
                |   ('+' >> factor[factor.val = arg1])
                ;
        }

        typedef rule<ScannerT, typename CalculatorClosure<T>::context_t> rule_t;
        rule_t expression, term, factor;
        rule<ScannerT> top;

        rule<ScannerT> const&
        start() const { return top; }
    };
};

}

struct ExpressionParser
{
	template<typename T>
	static bool evaluate(const std::string& expression, T& value)
	{
		Calculator<T> calc;
		parse_info<> info = parse(expression.c_str(), calc[var(value) = arg1], space_p);
		if (info.full)
			return true;
		else
			return false;
	}
};

}

#endif /* ZILLIANS_EXPRESSIONPARSER_H_ */
