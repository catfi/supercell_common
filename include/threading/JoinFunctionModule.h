#include <tuple>
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <assert.h>
#include <boost/function.hpp>
#include "tbb/flow_graph.h"

namespace zillians {

//////////////////////////////////////////////////////////////////////////////
// declaration
//////////////////////////////////////////////////////////////////////////////

class JoinFunctionModule
{
private:
    typedef int value_type;
    const static size_t MAX_IN_PORT = 2;
    typedef std::tuple<value_type,value_type> TP2;
    typedef tbb::flow::join_node<TP2> JoinNodeType;
    typedef tbb::flow::function_node<TP2, value_type> FoldNodeType;
    typedef tbb::flow::function_node<value_type, value_type> DummyFuncType;
    static value_type dummyTP2Func(const TP2& t) {return 0;}
    static value_type dummyFunc(const value_type& t) {return 0;}

public:
    JoinFunctionModule(tbb::flow::graph& g,
                       boost::function<int(int)> functor_,
                       const size_t numInputPort = 1)
        : f(new DummyFuncType(g, 1, functor_))
        , size(numInputPort)
        , inputPorts(size)
        , connectingInputPort(0)
    {
        using tbb::flow::make_edge;
        using tbb::flow::input_port;
        assert(numInputPort > 0);
        createJoinLevels(g, size);
        linkJoinLevels();
        for(size_t i = 0; i != size; ++i)
        {
            inputPorts[i] = std::make_shared<DummyFuncType>(g, 1, dummyFunc);
            JoinNodeType& b = *(levels[0].joins[i/MAX_IN_PORT].joinNode);
            switch(i % MAX_IN_PORT)
            {
            case 0:
                make_edge(*inputPorts[i], input_port<0>(b));
                break;
            case 1:
                make_edge(*inputPorts[i], input_port<1>(b));
                break;
            }
        }
        if(size % MAX_IN_PORT != 0)
        {
            size_t i = size - 1 ;
            DummyFuncType& a = *inputPorts[i];
            JoinNodeType& b = *(levels[0].joins[i/MAX_IN_PORT].joinNode);
            make_edge(a, tbb::flow::input_port<1>(b));
        }
    }

    tbb::flow::function_node<value_type, value_type>& getOutputPort()
    {
        return *f;
    }

    tbb::flow::function_node<value_type, value_type>& getNextInputPort()
    {
        return *inputPorts[connectingInputPort++];
    }

    bool verifyInput()
    {
        return connectingInputPort == size;
    }

private:
    struct Join
    {
        Join(tbb::flow::graph& g, const size_t n)
          : size(n)
          , joinNode(new JoinNodeType(g))
          , foldNode(new FoldNodeType(g, 1, dummyTP2Func))
        {
            tbb::flow::make_edge(*joinNode, *foldNode);
        }

        size_t size;
        std::shared_ptr<JoinNodeType> joinNode;
        std::shared_ptr<FoldNodeType> foldNode;
    };

    struct JoinLevel
    {
        JoinLevel(tbb::flow::graph& g, const size_t n) : size(n)
        {
            for(size_t i = 0; i != n / MAX_IN_PORT; ++i)
            {
                joins.push_back(Join(g, MAX_IN_PORT));
            }
            if(n % MAX_IN_PORT != 0)
            {
                joins.push_back(Join(g, n % MAX_IN_PORT));
            }
        }
        size_t size;
        std::vector<Join> joins;
    };

private:
    void createJoinLevels(tbb::flow::graph& g, size_t n)
    {
        do {
            levels.push_back(JoinLevel(g, n));
            n = (n - 1) / 2 + 1;
        } while(n > 1);
    }

    void linkTwoJoinLevels(JoinLevel& in, JoinLevel& next)
    {
        assert(in.joins.size() == next.size);
        for(size_t i = 0; i != in.joins.size(); ++i)
        {
            FoldNodeType& a = *(in.joins[i].foldNode);
            JoinNodeType& b = *(next.joins[i/MAX_IN_PORT].joinNode);
            switch(i % MAX_IN_PORT)
            {
            case 0:
                tbb::flow::make_edge(a, tbb::flow::input_port<0>(b));
                break;
            case 1:
                tbb::flow::make_edge(a, tbb::flow::input_port<1>(b));
                break;
            }
        }
        if(in.joins.size() % MAX_IN_PORT != 0)
        {
            size_t i = in.joins.size() - 1 ;
            FoldNodeType& a = *(in.joins[i].foldNode);
            JoinNodeType& b = *(next.joins[i/MAX_IN_PORT].joinNode);
            tbb::flow::make_edge(a, tbb::flow::input_port<1>(b));
        }
    }

    void linkJoinLevels()
    {
        for(size_t l = 0; l < levels.size() - 1; ++l)
        {
            linkTwoJoinLevels(levels[l], levels[l + 1]);
        }
        FoldNodeType& b = *levels[levels.size()-1].joins[0].foldNode;
        tbb::flow::make_edge(b, *f);
    }

private:
    std::shared_ptr<DummyFuncType> f;
    size_t size;
    std::vector<std::shared_ptr<DummyFuncType>> inputPorts;
    std::vector<JoinLevel> levels;
    size_t connectingInputPort;
};

} // namespace ziilians
