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
/**
 * @date May 5, 2009 sdk - Initial version created.
 */


#ifndef ZILLIANS_GRAPHUTIL_H_
#define ZILLIANS_GRAPHUTIL_H_

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

using namespace boost;

enum edge_indirect_reference_t { edge_indirect_reference };
enum vertex_indirect_reference_t { vertex_indirect_reference };

namespace boost {
  BOOST_INSTALL_PROPERTY(edge, indirect_reference);
  BOOST_INSTALL_PROPERTY(vertex, indirect_reference);
}

namespace zillians {

/**
 * Indirect Graph Traits is a helper class to provide additional vertex/edge property definition when constructing an adjacency_list.
 *
 * @code
 * typedef indirect_graph_traits<UUID,UUID> IndirectGraphTraits;
 * typedef property<vertex_index_t, int, IndirectGraphTraits::vertex_property > VertexProperty;
 * typedef property<edge_index_t, int, IndirectGraphTraits::edge_property > EdgeProperty;
 * typedef adjacency_list<listS, listS, bidirectionalS, VertexProperty, EdgeProperty> Graph;
 * @endcode
 *
 * @see indirect_graph_mapping
 */
template <class IndirectVertexReference, class IndirectEdgeReference>
struct indirect_graph_traits
{
	typedef IndirectVertexReference vertex_reference_type;
	typedef IndirectEdgeReference edge_reference_type;
	typedef boost::property<vertex_indirect_reference_t, IndirectVertexReference> vertex_property;
	typedef boost::property<edge_indirect_reference_t, IndirectEdgeReference> edge_property;
};

/**
 * Indirect Graph Mapping is an utility designed for Boost Graph Library (BGL) to support additional vertex/edge indexing functionalities.
 * You may define any comparable type as a reference index/key to underlying vertex/edge descriptor, for example, int, string, or even UUID...
 * We provide additional helper function to maintain the relationship between the reference to descriptor when adding/removing vertex/edge, so
 * you don't need to call any method of this class directly; they're called implicitly inside those helper functions.
 *
 * @see add_vertex, remove_vertex, add_edge, remove_edge
 */
template <class IndirectGraphTraits, class VertexDescriptor, class EdgeDescriptor>
class indirect_graph_mapping
{
public:
	typedef typename IndirectGraphTraits::vertex_reference_type IndirectVertexReference;
	typedef typename IndirectGraphTraits::edge_reference_type IndirectEdgeReference;
	typedef std::map<IndirectVertexReference, VertexDescriptor> VertexReferenceMap;
	typedef std::map<IndirectEdgeReference, EdgeDescriptor> EdgeReferenceMap;

public:
	VertexReferenceMap mVertexReferenceMap;
	EdgeReferenceMap mEdgeReferenceMap;

public:
	void clear()
	{
		mVertexReferenceMap.clear();
		mEdgeReferenceMap.clear();
	}

public:
	bool findVertexReference(const IndirectVertexReference& vertex_reference, /*OUT*/ VertexDescriptor& vertex_descriptor)
	{
		typename VertexReferenceMap::iterator it = mVertexReferenceMap.find(vertex_reference);
		if(it == mVertexReferenceMap.end()) return false;

		vertex_descriptor = it->second;

		return true;
	}

	bool findEdgeReference(const IndirectEdgeReference& edge_reference, /*OUT*/ EdgeDescriptor& edge_descriptor)
	{
		typename EdgeReferenceMap::iterator it = mEdgeReferenceMap.find(edge_reference);
		if(it == mEdgeReferenceMap.end()) return false;

		edge_descriptor = it->second;

		return true;
	}

public:
	bool addVertexReference(const IndirectVertexReference& vertex_reference, /*IN*/ VertexDescriptor& vertex_descriptor)
	{
		typename VertexReferenceMap::iterator it = mVertexReferenceMap.find(vertex_reference);
		if(it != mVertexReferenceMap.end()) return false;

		mVertexReferenceMap[vertex_reference] = vertex_descriptor;

		return true;
	}

	bool removeVertexReference(const IndirectVertexReference& vertex_reference, /*OUT*/ VertexDescriptor& vertex_descriptor)
	{
		typename VertexReferenceMap::iterator it = mVertexReferenceMap.find(vertex_reference);
		if(it == mVertexReferenceMap.end()) return false;

		vertex_descriptor = it->second;
		mVertexReferenceMap.erase(it);

		return true;
	}

	bool addEdgeReference(const IndirectEdgeReference& edge_reference, /*IN*/ EdgeDescriptor& edge_descriptor)
	{
		typename EdgeReferenceMap::iterator it = mEdgeReferenceMap.find(edge_reference);
		if(it != mEdgeReferenceMap.end()) return false;

		mEdgeReferenceMap[edge_reference] = edge_descriptor;

		return true;
	}

	bool removeEdgeReference(const IndirectEdgeReference& edge_reference, /*OUT*/ EdgeDescriptor& edge_descriptor)
	{
		typename EdgeReferenceMap::iterator it = mEdgeReferenceMap.find(edge_reference);
		if(it == mEdgeReferenceMap.end()) return false;

		edge_descriptor = it->second;
		mEdgeReferenceMap.erase(it);

		return true;
	}
};

//////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline void copy(
		Graph& g_new_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_new_,
		const Graph& g_,
		const indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_
		)
{
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;

	// copy the graph
	g_new_ = g_;

	// rebuild the indirect mapping
	m_new_.clear();

	// rebuild the vertex mapping
	indirect_vertex_property_map prop_map_v = get(vertex_indirect_reference, g_new_);
	typename graph_traits<Graph>::vertex_iterator vi, vend;
	for(tie(vi,vend) = vertices(g_new_); vi != vend; ++vi)
	{
		m_new_.addVertexReference(prop_map_v[*vi], *vi);
	}

	// rebuild the edge mapping
	indirect_edge_property_map prop_map_e = get(edge_indirect_reference, g_new_);
	typename graph_traits<Graph>::edge_iterator ei, eend;
	for(tie(ei,eend) = edges(g_new_); ei != eend; ++ei)
	{
		const typename IndirectGraphTraits::edge_reference_type er = prop_map_e[*ei];
		typename Graph::edge_descriptor ed = *ei;
		m_new_.addEdgeReference(er, ed);
	}
}

//////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline void clear(
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	g_.clear();
	m_.clear();
}

///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline typename Graph::vertex_descriptor add_vertex(
		const typename IndirectGraphTraits::vertex_reference_type& ru_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	typename Graph::vertex_descriptor u = boost::add_vertex(g_);
	if(!m_.addVertexReference(ru_, u))
	{
		boost::remove_vertex(u, g_);
		throw std::invalid_argument("duplicated vertex reference value while adding vertex");
	}

	indirect_vertex_property_map prop_map = get(vertex_indirect_reference, g_);

	prop_map[u] = ru_;

	return u;
}

template <class IndirectGraphTraits, class Graph>
inline typename Graph::vertex_descriptor add_vertex(
		const typename IndirectGraphTraits::vertex_reference_type& ru_,
		const typename Graph::vertex_property_type& p_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	typename Graph::vertex_descriptor u = boost::add_vertex(p_, g_);
	if(!m_.addVertexReference(ru_, u))
	{
		boost::remove_vertex(u, g_);
		throw std::invalid_argument("duplicated vertex reference value while adding vertex");
	}

	indirect_vertex_property_map prop_map = get(vertex_indirect_reference, g_);

	prop_map[u] = ru_;

	return u;
}

///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline void remove_vertex(
		const typename IndirectGraphTraits::vertex_reference_type& ru_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	typename Graph::vertex_descriptor u;

	if(!m_.removeVertexReference(ru_, u))
	{
		throw std::invalid_argument("invalid vertex reference value while removing vertex");
	}

	boost::remove_vertex(u, g_);
}

template <class IndirectGraphTraits, class Graph>
inline void remove_vertex(
		typename Graph::vertex_descriptor u_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	typename Graph::vertex_descriptor dummy;

	indirect_vertex_property_map prop_map = get(vertex_indirect_reference, g_);
	if(!m_.removeVertexReference(prop_map[u_], dummy))
	{
		throw std::invalid_argument("invalid vertex reference value while removing vertex");
	}

	BOOST_ASSERT(dummy == u_);

	boost::remove_vertex(u_, g_);
}

///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline const typename IndirectGraphTraits::vertex_reference_type& vertex_ref(
		typename Graph::vertex_descriptor u_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	indirect_vertex_property_map prop_map = get(vertex_indirect_reference, g_);
	return prop_map[u_];
}

template <class IndirectGraphTraits, class Graph>
inline typename Graph::vertex_descriptor vertex(
		const typename IndirectGraphTraits::vertex_reference_type& ru_,
		const Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	typename Graph::vertex_descriptor u;
	if(!m_.findVertexReference(ru_, u))
	{
		throw std::invalid_argument("invalid vertex reference value while finding vertex");
	}
	return u;
}

///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline std::pair<typename Graph::edge_descriptor, bool> add_edge(
		const typename IndirectGraphTraits::edge_reference_type& re_,
		const typename IndirectGraphTraits::vertex_reference_type ru_,
		const typename IndirectGraphTraits::vertex_reference_type rv_,
		const typename Graph::edge_property_type& p_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	// check if the edge reference type in our graph mapping is equal to the edge reference property type defined in graph
	typedef typename IndirectGraphTraits::edge_reference_type indirect_edge_reference_type;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_edge_reference_type, typename indirect_edge_property_map::value_type>::value));

    typename Graph::vertex_descriptor u, v;
	if(!m_.findVertexReference(ru_, u) || !m_.findVertexReference(rv_, v))
	{
		throw std::invalid_argument("invalid source/target vertex reference value while adding edge");
	}

	return add_edge(re_, u, v, p_, g_, m_);
}

template <class IndirectGraphTraits, class Graph>
inline std::pair<typename Graph::edge_descriptor, bool> add_edge(
		const typename IndirectGraphTraits::edge_reference_type& re_,
		typename Graph::vertex_descriptor u_,
		typename Graph::vertex_descriptor v_,
		const typename Graph::edge_property_type& p_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the edge reference type in our graph mapping is equal to the edge reference property type defined in graph
	typedef typename IndirectGraphTraits::edge_reference_type indirect_edge_reference_type;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_edge_reference_type, typename indirect_edge_property_map::value_type>::value));

	std::pair<typename Graph::edge_descriptor, bool> result = boost::add_edge(u_, v_, p_, g_);
	if(!result.second)
	{
		throw std::invalid_argument("fail to add edge into graph");
	}

	if(!m_.addEdgeReference(re_, result.first))
	{
		boost::remove_edge(result.first, g_);
		throw std::invalid_argument("duplicated reference value while adding edge");
	}

	indirect_edge_property_map prop_map = get(edge_indirect_reference, g_);

	prop_map[result.first] = re_;

	return result;
}

template <class IndirectGraphTraits, class Graph>
inline std::pair<typename Graph::edge_descriptor, bool> add_edge(
		const typename IndirectGraphTraits::edge_reference_type& re_,
		const typename IndirectGraphTraits::vertex_reference_type ru_,
		const typename IndirectGraphTraits::vertex_reference_type rv_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	// check if the edge reference type in our graph mapping is equal to the edge reference property type defined in graph
	typedef typename IndirectGraphTraits::edge_reference_type indirect_edge_reference_type;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_edge_reference_type, typename indirect_edge_property_map::value_type>::value));

	typename Graph::edge_property_type p;
	return add_edge(re_, ru_, rv_, p, g_, m_);
}

template <class IndirectGraphTraits, class Graph>
inline std::pair<typename Graph::edge_descriptor, bool> add_edge(
		const typename IndirectGraphTraits::edge_reference_type& re_,
		typename Graph::vertex_descriptor u_,
		typename Graph::vertex_descriptor v_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the edge reference type in our graph mapping is equal to the edge reference property type defined in graph
	typedef typename IndirectGraphTraits::edge_reference_type indirect_edge_reference_type;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_edge_reference_type, typename indirect_edge_property_map::value_type>::value));

	typename Graph::edge_property_type p;
	return add_edge(re_, u_, v_, p, g_, m_);
}

///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline void remove_edge(
		const typename IndirectGraphTraits::vertex_reference_type& ru_,
		const typename IndirectGraphTraits::vertex_reference_type& rv_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

    typename Graph::vertex_descriptor u, v;
	if(!m_.findVertexReference(ru_, u) || !m_.findVertexReference(rv_, v))
	{
		throw std::invalid_argument("invalid vertex reference value while removing edge");
	}

	remove_edge(u, v, g_, m_);
}

template <class IndirectGraphTraits, class Graph>
inline void remove_edge(
		typename Graph::vertex_descriptor u_,
		typename Graph::vertex_descriptor& v_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the edge reference type in our graph mapping is equal to the edge reference property type defined in graph
	typedef typename IndirectGraphTraits::edge_reference_type indirect_edge_reference_type;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_edge_reference_type, typename indirect_edge_property_map::value_type>::value));

	// find the edge descriptor from given source and target vertex descriptors
	bool found = false;
	typename Graph::edge_descriptor e;
	tie(e, found) = edge(u_, v_, g_);
	if(!found)
	{
		throw std::invalid_argument("cannot find edge descriptor from the given source and target vertex descriptors");
	}

	indirect_edge_property_map prop_map = get(edge_indirect_reference, g_);
	if(!m_.removeEdgeReference(prop_map[e], e))
	{
		throw std::invalid_argument("invalid edge reference value while removing edge reference");
	}

	boost::remove_edge(u_, v_, g_);
}

template <class IndirectGraphTraits, class Graph>
inline void remove_edge(
		const typename IndirectGraphTraits::edge_reference_type& re_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the edge reference type in our graph mapping is equal to the edge reference property type defined in graph
	typedef typename IndirectGraphTraits::edge_reference_type indirect_edge_reference_type;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_edge_reference_type, typename indirect_edge_property_map::value_type>::value));

	typename Graph::edge_descriptor e;

	if(!m_.removeEdgeReference(re_, e))
	{
		throw std::invalid_argument("invalid edge reference value while removing edge");
	}

	boost::remove_edge(e, g_);
}

template <class IndirectGraphTraits, class Graph>
inline void remove_edge(
		typename Graph::edge_descriptor e_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the edge reference type in our graph mapping is equal to the edge reference property type defined in graph
	typedef typename IndirectGraphTraits::edge_reference_type indirect_edge_reference_type;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_edge_reference_type, typename indirect_edge_property_map::value_type>::value));

	typename Graph::edge_descriptor dummy;

	indirect_edge_property_map prop_map = get(edge_indirect_reference, g_);
	if(!m_.removeEdgeReference(prop_map[e_], dummy))
	{
		throw std::invalid_argument("invalid edge reference value while removing edge");
	}

	BOOST_ASSERT(dummy == e_);

	boost::remove_edge(e_, g_);
}

///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph, class Predicate>
inline void remove_out_edge_if(
		const typename IndirectGraphTraits::vertex_reference_type& v_,
		Predicate pred,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	throw std::logic_error("not yet implemented");
}

template <class IndirectGraphTraits, class Graph, class Predicate>
inline void remove_out_edge_if(
		typename Graph::vertex_descriptor v_,
		Predicate pred,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	throw std::logic_error("not yet implemented");
}

template <class IndirectGraphTraits, class Graph, class Predicate>
inline void remove_in_edge_if(
		const typename IndirectGraphTraits::vertex_reference_type& v_,
		Predicate pred,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	throw std::logic_error("not yet implemented");
}

template <class IndirectGraphTraits, class Graph, class Predicate>
inline void remove_in_edge_if(
		typename Graph::vertex_descriptor v_,
		Predicate pred,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	throw std::logic_error("not yet implemented");
}

///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline void clear_vertex(
		const typename IndirectGraphTraits::vertex_reference_type& u_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	typename Graph::vertex_descriptor u = vertex(u_, g_, m_);
	clear_vertex(u, g_, m_);
}

template <class IndirectGraphTraits, class Graph>
inline void clear_vertex(
		typename Graph::vertex_descriptor u_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	clear_out_edges(u_, g_, m_);
	clear_in_edges(u_, g_, m_);
}

///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline void clear_out_edges(
		const typename IndirectGraphTraits::vertex_reference_type& u_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	typename Graph::vertex_descriptor u = vertex(u_, g_, m_);
	clear_out_edges(u, g_, m_);
}

template <class IndirectGraphTraits, class Graph>
inline void clear_out_edges(
		typename Graph::vertex_descriptor u_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	typedef typename Graph::edge_descriptor ed;

	typename Graph::out_edge_iterator it_oe, it_oe_end;
	tie(it_oe, it_oe_end) = out_edges(u_, g_);
	int count = out_degree(u_, g_);
	ed* ed_array = new ed[count];
	for(int c = 0; it_oe != it_oe_end; ++it_oe, ++c)
	{
		ed_array[c] = *it_oe;
	}
	for(int c = 0; c < count; ++c)
	{
		// this is a workaround for adjacency_list whose edge storage is vecS; here we remove_edge and ignore the possible exception
		// TODO we should add some more traits to enable reference mapping only on vertex
		try	{ remove_edge(ed_array[c], g_, m_);	}
		catch(std::invalid_argument& ia) { }
	}
	delete[] ed_array;
}

///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline void clear_in_edges(
		const typename IndirectGraphTraits::vertex_reference_type& u_,
        Graph& g_,
        indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	typename Graph::vertex_descriptor u = vertex(u_, g_, m_);
	clear_in_edges(u, g_, m_);
}

template <class IndirectGraphTraits, class Graph>
inline void clear_in_edges(
		typename Graph::vertex_descriptor u_,
        Graph& g_,
        indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	typedef typename Graph::edge_descriptor ed;

	typename Graph::in_edge_iterator it_ie, it_ie_end;
	tie(it_ie, it_ie_end) = in_edges(u_, g_);
	int count = in_degree(u_, g_);
	ed* ed_array = new ed[count];
	for(int c = 0; it_ie != it_ie_end; ++it_ie, ++c)
	{
		ed_array[c] = *it_ie;
	}
	for(int c = 0; c < count; ++c)
	{
		// this is a workaround for adjacency_list whose edge storage is vecS; here we remove_edge and ignore the possible exception
		// TODO we should add some more traits to enable reference mapping only on vertex
		try	{ remove_edge(ed_array[c], g_, m_);	}
		catch(std::invalid_argument& ia) { }
	}
	delete[] ed_array;
}


///////////////////////////////////////////////////////////////////////////
template <class IndirectGraphTraits, class Graph>
inline const typename IndirectGraphTraits::edge_reference_type edge_ref(
		typename Graph::edge_descriptor e_,
		Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the edge reference type in our graph mapping is equal to the edge reference property type defined in graph
	typedef typename IndirectGraphTraits::edge_reference_type indirect_edge_reference_type;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_edge_reference_type, typename indirect_edge_property_map::value_type>::value));

	indirect_edge_property_map prop_map = get(edge_indirect_reference, g_);

	return prop_map[e_];
}

template <class IndirectGraphTraits, class Graph>
inline typename Graph::edge_descriptor edge(
		const typename IndirectGraphTraits::edge_reference_type& re_,
		const Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	// check if the edge reference type in our graph mapping is equal to the edge reference property type defined in graph
	typedef typename IndirectGraphTraits::edge_reference_type indirect_edge_reference_type;
	typedef typename property_map< Graph, edge_indirect_reference_t>::type indirect_edge_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_edge_reference_type, typename indirect_edge_property_map::value_type>::value));

	typename Graph::edge_descriptor e;
	if(!m_.findEdgeReference(re_, e))
	{
		throw std::invalid_argument("invalid edge reference value while finding edge");
	}

	return e;
}

template <class IndirectGraphTraits, class Graph>
inline std::pair<typename Graph::edge_descriptor, bool> edge(
		const typename IndirectGraphTraits::vertex_reference_type& ru_,
		const typename IndirectGraphTraits::vertex_reference_type& rv_,
		const Graph& g_,
		indirect_graph_mapping< IndirectGraphTraits, typename Graph::vertex_descriptor, typename Graph::edge_descriptor>& m_)
{
	// check if the vertex reference type in our graph mapping is equal to the vertex reference property type defined in graph
	typedef typename IndirectGraphTraits::vertex_reference_type indirect_vertex_reference_type;
	typedef typename property_map< Graph, vertex_indirect_reference_t>::type indirect_vertex_property_map;
	BOOST_STATIC_ASSERT((!!is_same<indirect_vertex_reference_type, typename indirect_vertex_property_map::value_type>::value));

	typename Graph::vertex_descriptor u, v;
	if(!m_.findVertexReference(ru_, u) || !m_.findVertexReference(rv_, v))
	{
		throw std::invalid_argument("invalid vertex reference value while finding edge");
	}

	return boost::edge(u, v, g_);
}

}

#endif/*ZILLIANS_GRAPHUTIL_H_*/
