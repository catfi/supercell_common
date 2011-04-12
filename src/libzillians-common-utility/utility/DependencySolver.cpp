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

#include "utility/DependencySolver.h"

namespace zillians {

//////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr DependencySolver::mLogger(log4cxx::Logger::getLogger("zillians.utility.DependencySolver"));

DependencySolver::DependencySolver()
{
	// set the property map
	mIndex = get(boost::vertex_index, mDependencyGraph);
	mGroupFlag = get(boost::vertex_color, mDependencyGraph);
}

DependencySolver::~DependencySolver()
{

}

bool DependencySolver::addNode(const std::string& id)
{
	// add given node to the dependency graph
	//LOG4CXX_DEBUG(mLogger, "add node " << id);
	try
	{
		VertexDescriptor vd;
		vd = add_vertex(id, mDependencyGraph, mMapping);
	}
	catch(std::invalid_argument& ia)
	{
		LOG4CXX_ERROR(mLogger, "fail to add node : " << ia.what());
		return false;
	}
	return true;
}

bool DependencySolver::addDependency(const std::string& id, const std::string& require_id)
{
	// add dependency between the given nodes
	//LOG4CXX_DEBUG(mLogger, "add dependency " << id << " " << require_id);
	try
	{
		static int increment_number = 0;
		increment_number++;
		Graph::edge_descriptor newEdge = add_edge(increment_number, id, require_id, mDependencyGraph, mMapping);
	}
	catch(std::invalid_argument& ia)
	{
		LOG4CXX_ERROR(mLogger, "fail to add dependency: " << ia.what());
		return false;
	}
	//LOG4CXX_DEBUG(mLogger, "successfully add dependency " << id << " " << require_id);
	return true;
}

bool DependencySolver::removeNode(const std::string& id)
{
	// remove given node from the dependency graph
	//LOG4CXX_DEBUG(mLogger, "remove node " << id);
	try
	{
		clear_vertex(id, mDependencyGraph, mMapping);
		remove_vertex(id, mDependencyGraph, mMapping);
	}
	catch(std::invalid_argument& ia)
	{
		LOG4CXX_ERROR(mLogger, "fail to remove node: " << ia.what());
		return false;
	}
	return true;
}

bool DependencySolver::removeDependency(const std::string& id, const std::string& require_id)
{
	// remove dependency between the given nodes
	//LOG4CXX_DEBUG(mLogger, "remove dependency " << id << " " << require_id);
	try
	{
		remove_edge(id, require_id, mDependencyGraph, mMapping);
	}
	catch(std::invalid_argument& ia)
	{
		LOG4CXX_ERROR(mLogger, "fail to remove dependency: " << ia.what());
		return false;
	}
	return true;
}

bool DependencySolver::isNodeExist(const std::string& id)
{
	// return if the node with specified id exist in the graph
	try
	{
		vertex(id, mDependencyGraph, mMapping);
	}
	catch(std::invalid_argument& ia)
	{
		return false;
	}
	return true;
}

bool DependencySolver::isDependencyExist(const std::string& id, const std::string& require_id)
{
	// return if the node with specified id exist in the graph
	try
	{
		edge(id, require_id, mDependencyGraph, mMapping);
	}
	catch(std::invalid_argument& ia)
	{
		return false;
	}
	return true;
}

void DependencySolver::clear()
{
	// clear the dependency graph, remove all nodes and dependencies
	//LOG4CXX_DEBUG(mLogger, "clear dependency graph");
	zillians::clear(mDependencyGraph, mMapping);
}

bool DependencySolver::compileTopologicalOrder(std::list<std::string>& result)
{
	// compile the complete list to load node (by topological sort)
	//LOG4CXX_DEBUG(mLogger, "compile topological ordering");
	std::list<VertexDescriptor> make_order;
	try
	{
		boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
		boost::graph_traits<Graph>::vertices_size_type cnt = 0;
		for(boost::tie(vi,vi_end) = vertices(mDependencyGraph); vi != vi_end; ++vi)
			put(mIndex, *vi, cnt++);

		boost::topological_sort(mDependencyGraph, std::front_inserter(make_order));
		std::list<VertexDescriptor>::iterator it, it_end = make_order.end();
		for (it = make_order.begin(); it != it_end; ++it)
		{
			result.push_front(vertex_ref(*it, mDependencyGraph, mMapping));
		}
	}
	catch(const boost::not_a_dag &nad)
	{
		LOG4CXX_ERROR(mLogger, "dependency graph is not a DAG, topological sort failed: " << nad.what());
		return false;
	}
	catch(std::invalid_argument& ia)
	{
		LOG4CXX_ERROR(mLogger, "fail to compile node load order: " << ia.what());
		return false;
	}
	return true;
}

bool DependencySolver::compileReversedTopologicalOrder(std::list<std::string>& result)
{
	// compile the complete list to unload node (by reversed topological sort)
	//LOG4CXX_DEBUG(mLogger, "compile node unload order");
	std::list<VertexDescriptor> make_order;
	try
	{
		boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
		boost::graph_traits<Graph>::vertices_size_type cnt = 0;
		for(boost::tie(vi,vi_end) = vertices(mDependencyGraph); vi != vi_end; ++vi)
			put(mIndex, *vi, cnt++);

		boost::topological_sort(mDependencyGraph, std::front_inserter(make_order));
		std::list<VertexDescriptor>::iterator it, it_end = make_order.end();
		for (it = make_order.begin(); it != it_end; ++it)
		{
			result.push_back(vertex_ref(*it, mDependencyGraph, mMapping));
		}
	}
	catch(const boost::not_a_dag &nad)
	{
		LOG4CXX_ERROR(mLogger, "dependency graph is not a DAG, topological sort failed: " << nad.what());
		return false;
	}
	catch(std::invalid_argument& ia)
	{
		LOG4CXX_ERROR(mLogger, "fail to compile node unload order: " << ia.what());
		return false;
	}
	return true;
}

bool DependencySolver::compileRequireNodes(/*IN*/ const std::string& id, /*OUT*/ std::list<std::string>& result)
{
	// find the list of node required by the given node
	// sorted by load order, exclude the given node
	//LOG4CXX_DEBUG(mLogger, "compile require node of " << id);
	vertex_iter vi, vi_end;
	for(boost::tie(vi, vi_end) = vertices(mDependencyGraph); vi != vi_end; ++vi)
	{
		mGroupFlag[*vi] = false;
	}
	findRequireNode(vertex(id, mDependencyGraph, mMapping));
	try
	{
		boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
		boost::graph_traits<Graph>::vertices_size_type cnt = 0;
		for(boost::tie(vi,vi_end) = vertices(mDependencyGraph); vi != vi_end; ++vi)
			put(mIndex, *vi, cnt++);
		std::list<VertexDescriptor> make_order;
		boost::topological_sort(mDependencyGraph, std::front_inserter(make_order));
		std::list<VertexDescriptor>::iterator it = make_order.begin(), it_end = make_order.end();
		for (; it != it_end; ++it)
		{
			if(mGroupFlag[*it])
				result.push_front(vertex_ref(*it, mDependencyGraph, mMapping));
		}
	}
	catch(const boost::not_a_dag &nad)
	{
		LOG4CXX_ERROR(mLogger, "dependency graph is not a DAG, topological sort failed : " << nad.what());
		return false;
	}
	catch(std::invalid_argument& ia)
	{
		LOG4CXX_ERROR(mLogger, "compile require nodes: " << ia.what());
		return false;
	}
	return true;
}

bool DependencySolver::compileDependentNodes(/*IN*/ const std::string& id, /*OUT*/ std::list<std::string>& result)
{
	// find the list of nodes which depend on the given node
	// sorted by unload order, exclude the given node
	//LOG4CXX_DEBUG(mLogger, "compile dependent nodes of " << id);
	vertex_iter vi, vi_end;
	for(boost::tie(vi, vi_end) = vertices(mDependencyGraph); vi != vi_end; ++vi)
	{
		mGroupFlag[*vi] = false;
	}
	findDependentNode(vertex(id, mDependencyGraph, mMapping));
	try
	{
		boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
		boost::graph_traits<Graph>::vertices_size_type cnt = 0;
		for(boost::tie(vi,vi_end) = vertices(mDependencyGraph); vi != vi_end; ++vi)
			put(mIndex, *vi, cnt++);
		std::list<VertexDescriptor> make_order;
		boost::topological_sort(mDependencyGraph, std::front_inserter(make_order));
		std::list<VertexDescriptor>::iterator it = make_order.begin(), it_end = make_order.end();
		for (; it != it_end; ++it)
		{
			if(mGroupFlag[*it])
				result.push_back(vertex_ref(*it, mDependencyGraph, mMapping));
		}
	}
	catch(const boost::not_a_dag &nad)
	{
		LOG4CXX_ERROR(mLogger, "dependency graph is not a DAG, topological sort failed : " << nad.what());
		return false;
	}
	catch(std::invalid_argument& ia)
	{
		LOG4CXX_ERROR(mLogger, "fail to compile dependent nodes: " << ia.what());
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void DependencySolver::findRequireNode(/*IN*/ const VertexDescriptor& v)
{
	// recursively find the require nodes
	out_edge_iter ei, ei_end;
	for(boost::tie(ei, ei_end) = boost::out_edges(v, mDependencyGraph); ei != ei_end; ++ei)
	{
		VertexDescriptor vd = boost::target(*ei, mDependencyGraph);
		if(!mGroupFlag[vd])
		{
			mGroupFlag[vd] = true;
			findRequireNode(vd);
		}
	}
}

void DependencySolver::findDependentNode(/*IN*/ const VertexDescriptor& v)
{
	// recursively find the dependent nodes
	in_edge_iter ei, ei_end;
	for(boost::tie(ei, ei_end) = boost::in_edges(v, mDependencyGraph); ei != ei_end; ++ei)
	{
		VertexDescriptor vd = boost::source(*ei, mDependencyGraph);
		if(!mGroupFlag[vd])
		{
			mGroupFlag[vd] = true;
			findDependentNode(vd);
		}
	}
}

}
