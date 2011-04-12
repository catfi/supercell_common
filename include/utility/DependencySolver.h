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
 * @date Apr 11, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_DEPENDENCYSOLVER_H_
#define ZILLIANS_DEPENDENCYSOLVER_H_

#include "core/Prerequisite.h"
#include "core/Singleton.h"
#include "utility/GraphUtil.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/pending/property.hpp>

namespace zillians {

class DependencySolver
{
public:
	DependencySolver();
	virtual ~DependencySolver();

public:
	typedef indirect_graph_traits<std::string, int > IndirectGraphTraits;

	typedef boost::property<boost::vertex_index_t, std::size_t,
			boost::property<boost::vertex_color_t, bool,
			boost::property<boost::vertex_name_t, std::string,
			IndirectGraphTraits::vertex_property> > > ModuleProperty;
	typedef boost::property<boost::edge_name_t, std::string,
			IndirectGraphTraits::edge_property> EdgeProperty;

	typedef boost::adjacency_list<boost::setS, boost::listS, boost::bidirectionalS, ModuleProperty, EdgeProperty> Graph;
	typedef boost::graph_traits<Graph> Traits;

	typedef Traits::edge_descriptor EdgeDescriptor;
	typedef Traits::vertex_descriptor VertexDescriptor;
	typedef Traits::vertex_iterator vertex_iter;
	typedef Traits::in_edge_iterator in_edge_iter;
	typedef Traits::out_edge_iterator out_edge_iter;

public:
	bool addNode(const std::string& id);
	bool addDependency(const std::string& id, const std::string& require_id);
	bool removeNode(const std::string& id);
	bool removeDependency(const std::string& id, const std::string& require_id);
	bool isNodeExist(const std::string& id);
	bool isDependencyExist(const std::string& id, const std::string& require_id);
	void clear();

public:
	 bool compileTopologicalOrder(std::list<std::string>& result);
	 bool compileReversedTopologicalOrder(std::list<std::string>& result);
	 bool compileRequireNodes(/*IN*/ const std::string& id, /*OUT*/ std::list<std::string>& result);
	 bool compileDependentNodes(/*IN*/ const std::string& id, /*OUT*/ std::list<std::string>& result);

private:
	 void findRequireNode(/*IN*/ const VertexDescriptor& v);
	 void findDependentNode(/*IN*/ const VertexDescriptor& v);

private:
	Graph mDependencyGraph;
	indirect_graph_mapping<IndirectGraphTraits, VertexDescriptor, EdgeDescriptor>  mMapping;
	boost::property_map<Graph, boost::vertex_index_t>::type mIndex; ///< the vertex index for topological sort(for listS)
	boost::property_map<Graph, boost::vertex_color_t>::type mGroupFlag;///< the flag to mark traveled vertices in recursive work

private:
	static log4cxx::LoggerPtr mLogger;
};

}

#endif /* ZILLIANS_DEPENDENCYSOLVER_H_ */
