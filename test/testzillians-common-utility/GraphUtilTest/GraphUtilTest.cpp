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

#include "core/Prerequisite.h"
#include "utility/GraphUtil.h"
#include "utility/UUIDUtil.h"

#define BOOST_TEST_MODULE GraphUtilTest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/topological_sort.hpp>

using namespace std;
using namespace zillians;

BOOST_AUTO_TEST_SUITE( GraphUtilTestSuite )

BOOST_AUTO_TEST_CASE( IndirectMapTestCase1 )
{
	using namespace boost;

	typedef indirect_graph_traits<int,int> IndirectGraphTraits;

	typedef property<vertex_name_t, std::string, IndirectGraphTraits::vertex_property > VertexProperty;
	typedef property<edge_index_t, int, IndirectGraphTraits::edge_property > EdgeProperty;

	// Note that when using indirect_graph_mapping with adjacency_list, the vertex/edge storage must be listS, otherwise the vertex/edge descriptor might not be consistent with the internal storage in indirect_graph_mapping after removing some vertex/edge.
	// However, if you plan not to use edge reference but only vertex, it's OK to use vecS on edge storage while defining the adjacency_list
	typedef adjacency_list<listS, listS, bidirectionalS, VertexProperty, EdgeProperty> Graph;
	typedef graph_traits<Graph>::edge_descriptor EdgeDescriptor;
	typedef graph_traits<Graph>::vertex_descriptor VertexDescriptor;

	typedef indirect_graph_mapping<IndirectGraphTraits, VertexDescriptor, EdgeDescriptor> IndirectGraphMapping;

	Graph g;
	IndirectGraphMapping mapping;

	BOOST_CHECK_NO_THROW(add_vertex(10, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(20, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(30, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(40, g, mapping));

	//add_edge(vertex(10, g, mapping), vertex(20, g, mapping), g);
	//remove_edge(vertex(10, g, mapping), vertex(20, g, mapping), g);

	BOOST_CHECK_NO_THROW(add_edge(1, 10, 20, g, mapping));
	BOOST_CHECK_NO_THROW(add_edge(2, 20, 30, g, mapping));
	BOOST_CHECK_NO_THROW(add_edge(3, 30, 40, g, mapping));
	BOOST_CHECK_NO_THROW(add_edge(4, 20, 30, g, mapping)); // allow parallel edge for listS edge type as long as the edge identifier is unique
	BOOST_REQUIRE_THROW(add_edge(2, 20, 30, g, mapping), std::invalid_argument); // the edge identifier is not unique, throw exception

	VertexDescriptor vv = vertex(10, g, mapping);
	property_map<Graph, vertex_name_t>::type name_map;
	name_map[vv] = "";

	// check vertex reference conversion
	BOOST_CHECK(vertex_ref(vertex(10, g, mapping), g, mapping) == 10);

	// check edge reference conversion
	BOOST_CHECK(edge_ref(edge(1, g, mapping), g, mapping) == 1);

	// look up invalid vertex reference will throw std::invalid_argument exception
	BOOST_REQUIRE_THROW(vertex(50, g, mapping), std::invalid_argument);

	// look up invalid vertex reference will throw std::invalid_argument exception
	BOOST_REQUIRE_THROW(edge(50, 20, g, mapping), std::invalid_argument);

	// remove edge by source and target vertex reference
	BOOST_CHECK_NO_THROW(remove_edge(10, 20, g, mapping));

	// remove edge by edge reference
	BOOST_CHECK_NO_THROW(remove_edge(3, g, mapping));

	// remove edge by edge reference
	Graph::edge_descriptor result;
	BOOST_CHECK_NO_THROW(result = edge(20, 30, g, mapping););
	BOOST_CHECK_NO_THROW(remove_edge(result, g, mapping));

	BOOST_CHECK_NO_THROW(remove_vertex(10, g, mapping));
	BOOST_CHECK_NO_THROW(remove_vertex(20, g, mapping));
	BOOST_CHECK_NO_THROW(remove_vertex(30, g, mapping));
	BOOST_CHECK_NO_THROW(remove_vertex(40, g, mapping));

	// clear the graph
	clear(g, mapping);
}

BOOST_AUTO_TEST_CASE( IndirectMapTestCase2 )
{
	using namespace boost;

	typedef indirect_graph_traits<int,int> IndirectGraphTraits;

	typedef property<vertex_name_t, std::string, IndirectGraphTraits::vertex_property > VertexProperty;
	typedef property<edge_index_t, int, IndirectGraphTraits::edge_property > EdgeProperty;

	// Here we use vecS for edge storage, so you shouldn't use any add_edge/remove_edge in our GraphUtil, but use the built-in add_edge/remove_edge instead
	typedef adjacency_list<listS, vecS, bidirectionalS, VertexProperty, EdgeProperty> Graph;
	typedef graph_traits<Graph>::edge_descriptor EdgeDescriptor;
	typedef graph_traits<Graph>::vertex_descriptor VertexDescriptor;

	typedef indirect_graph_mapping<IndirectGraphTraits, VertexDescriptor, EdgeDescriptor> IndirectGraphMapping;

	Graph g;
	IndirectGraphMapping mapping;

	BOOST_CHECK_NO_THROW(add_vertex(10, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(20, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(30, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(40, g, mapping));

	std::pair<EdgeDescriptor, bool> result;

	result = add_edge(vertex(10, g, mapping), vertex(20, g, mapping), g);
	BOOST_CHECK(result.second == true);
	result = add_edge(vertex(20, g, mapping), vertex(30, g, mapping), g);
	BOOST_CHECK(result.second == true);
	result = add_edge(vertex(30, g, mapping), vertex(40, g, mapping), g);
	BOOST_CHECK(result.second == true);

	// One case is that some algorithm requires vecS storage for edge, such as topological_sort, then it's possible to use vecS instead of listS
	std::list<VertexDescriptor> ordering;
	topological_sort(g, front_inserter(ordering));

	std::list<VertexDescriptor>::iterator it;
	it = ordering.begin();	BOOST_CHECK(vertex_ref(*it, g, mapping) == 10);
	++it; 					BOOST_CHECK(vertex_ref(*it, g, mapping) == 20);
	++it; 					BOOST_CHECK(vertex_ref(*it, g, mapping) == 30);
	++it; 					BOOST_CHECK(vertex_ref(*it, g, mapping) == 40);

	BOOST_CHECK_NO_THROW(remove_edge(vertex(10, g, mapping), vertex(20, g, mapping), g));

	// clear the graph
	clear(g, mapping);
}

BOOST_AUTO_TEST_CASE( IndirectMapTestCase3 )
{
	using namespace boost;

	typedef indirect_graph_traits<UUID,int> IndirectGraphTraits;

	typedef property<vertex_name_t, std::string, IndirectGraphTraits::vertex_property > VertexProperty;
	typedef property<edge_index_t, int, IndirectGraphTraits::edge_property > EdgeProperty;

	// Here we use vecS for edge storage, so you shouldn't use any add_edge/remove_edge in our GraphUtil, but use the built-in add_edge/remove_edge instead
	typedef adjacency_list<listS, vecS, bidirectionalS, VertexProperty, EdgeProperty> Graph;
	typedef graph_traits<Graph>::edge_descriptor EdgeDescriptor;
	typedef graph_traits<Graph>::vertex_descriptor VertexDescriptor;

	typedef indirect_graph_mapping<IndirectGraphTraits, VertexDescriptor, EdgeDescriptor> IndirectGraphMapping;

	Graph g;
	IndirectGraphMapping mapping;

	UUID a = "de01b9dc-37b8-11de-b3b7-001d92648305";
	UUID b = "eefe4c0a-37b8-11de-aa06-001d92648305";
	UUID c = "ef222b34-37b8-11de-ab53-001d92648305";
	UUID d = "ef5eebfa-37b8-11de-9d60-001d92648305";
	UUID e = "ff7840fe-37b8-11de-bf52-001d92648305";

	VertexDescriptor a_vd;
	VertexDescriptor b_vd;
	VertexDescriptor c_vd;
	VertexDescriptor d_vd;
	VertexDescriptor e_vd;

	BOOST_CHECK_NO_THROW((a_vd = add_vertex(a, g, mapping)));
	BOOST_CHECK_NO_THROW((b_vd = add_vertex(b, g, mapping)));
	BOOST_CHECK_NO_THROW((d_vd = add_vertex(d, g, mapping)));

	BOOST_CHECK(a_vd != b_vd);
	BOOST_CHECK(b_vd != d_vd);
	BOOST_CHECK(a_vd != d_vd);

	BOOST_CHECK_NO_THROW(vertex(a, g, mapping));
	BOOST_CHECK(a_vd == vertex(a, g, mapping));

	BOOST_CHECK_NO_THROW(vertex(b, g, mapping));
	BOOST_CHECK(b_vd == vertex(b, g, mapping));

	BOOST_CHECK_NO_THROW(vertex(d, g, mapping));
	BOOST_CHECK(d_vd == vertex(d, g, mapping));

	// clear the graph
	clear(g, mapping);
}

BOOST_AUTO_TEST_CASE( IndirectMapTestCase4 )
{
	using namespace boost;

	typedef indirect_graph_traits<int,int> IndirectGraphTraits;

	typedef property<vertex_name_t, std::string, IndirectGraphTraits::vertex_property > VertexProperty;
	typedef property<edge_index_t, int, IndirectGraphTraits::edge_property > EdgeProperty;

	// Note that when using indirect_graph_mapping with adjacency_list, the vertex/edge storage must be listS, otherwise the vertex/edge descriptor might not be consistent with the internal storage in indirect_graph_mapping after removing some vertex/edge.
	// However, if you plan not to use edge reference but only vertex, it's OK to use vecS on edge storage while defining the adjacency_list
	typedef adjacency_list<listS, listS, bidirectionalS, VertexProperty, EdgeProperty> Graph;
	typedef graph_traits<Graph>::edge_descriptor EdgeDescriptor;
	typedef graph_traits<Graph>::vertex_descriptor VertexDescriptor;

	typedef indirect_graph_mapping<IndirectGraphTraits, VertexDescriptor, EdgeDescriptor> IndirectGraphMapping;

	Graph g;
	IndirectGraphMapping mapping;

	BOOST_CHECK_NO_THROW(add_vertex(10, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(20, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(30, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(40, g, mapping));

	//add_edge(vertex(10, g, mapping), vertex(20, g, mapping), g);
	//remove_edge(vertex(10, g, mapping), vertex(20, g, mapping), g);

	BOOST_CHECK_NO_THROW(add_edge(1, 10, 20, g, mapping));
	BOOST_CHECK_NO_THROW(add_edge(2, 20, 30, g, mapping));
	BOOST_CHECK_NO_THROW(add_edge(3, 30, 40, g, mapping));

	// clear vertex 20, so edge 1 and 2 will be removed
	BOOST_CHECK_NO_THROW(clear_vertex(20, g, mapping));

	BOOST_REQUIRE_THROW(remove_edge(1, g, mapping), std::invalid_argument);
	BOOST_REQUIRE_THROW(remove_edge(2, g, mapping), std::invalid_argument);

	BOOST_CHECK_NO_THROW(remove_vertex(20, g, mapping));

	// clear the graph
	clear(g, mapping);
}

BOOST_AUTO_TEST_CASE( IndirectMapTestCase5 )
{
	using namespace boost;

	typedef indirect_graph_traits<int,int> IndirectGraphTraits;

	typedef property<vertex_name_t, std::string, IndirectGraphTraits::vertex_property > VertexProperty;
	typedef property<edge_index_t, int, IndirectGraphTraits::edge_property > EdgeProperty;

	// Note that when using indirect_graph_mapping with adjacency_list, the vertex/edge storage must be listS, otherwise the vertex/edge descriptor might not be consistent with the internal storage in indirect_graph_mapping after removing some vertex/edge.
	// However, if you plan not to use edge reference but only vertex, it's OK to use vecS on edge storage while defining the adjacency_list
	typedef adjacency_list<listS, listS, bidirectionalS, VertexProperty, EdgeProperty> Graph;
	typedef graph_traits<Graph>::edge_descriptor EdgeDescriptor;
	typedef graph_traits<Graph>::vertex_descriptor VertexDescriptor;

	typedef indirect_graph_mapping<IndirectGraphTraits, VertexDescriptor, EdgeDescriptor> IndirectGraphMapping;

	Graph g;
	IndirectGraphMapping mapping;

#define NUM_KEYS 500
	std::vector<int> keys;
	for(int i=0;i<NUM_KEYS;++i)
	{
		keys.push_back(i);
		BOOST_CHECK_NO_THROW(add_vertex(i, g, mapping));
	}

	std::random_shuffle(keys.begin(), keys.end());
	for(std::vector<int>::iterator it = keys.begin(); it != keys.end(); ++it)
	{
		//printf("removing %d\n", *it);
		BOOST_CHECK_NO_THROW(remove_vertex(*it, g, mapping));
	}
#undef NUM_KEYS

	// clear the graph
	clear(g, mapping);
}

BOOST_AUTO_TEST_CASE( IndirectMapTestCase6 )
{
	using namespace boost;

	typedef indirect_graph_traits<int,int> IndirectGraphTraits;

	typedef property<vertex_name_t, std::string, IndirectGraphTraits::vertex_property > VertexProperty;
	typedef property<edge_index_t, int, IndirectGraphTraits::edge_property > EdgeProperty;

	// Note that when using indirect_graph_mapping with adjacency_list, the vertex/edge storage must be listS, otherwise the vertex/edge descriptor might not be consistent with the internal storage in indirect_graph_mapping after removing some vertex/edge.
	// However, if you plan not to use edge reference but only vertex, it's OK to use vecS on edge storage while defining the adjacency_list
	typedef adjacency_list<vecS, vecS, bidirectionalS, VertexProperty, EdgeProperty> Graph;
	typedef graph_traits<Graph>::edge_descriptor EdgeDescriptor;
	typedef graph_traits<Graph>::vertex_descriptor VertexDescriptor;

	typedef indirect_graph_mapping<IndirectGraphTraits, VertexDescriptor, EdgeDescriptor> IndirectGraphMapping;

	Graph g;
	IndirectGraphMapping mapping;

#define NUM_KEYS 5000
	std::vector<int> keys;
	for(int i=0;i<NUM_KEYS;++i)
	{
		keys.push_back(i);
		BOOST_CHECK_NO_THROW(add_vertex(i, g, mapping));
	}

	// for the first time we create NUM_KEYS vertices and erase half of them
	int erase_count = 0;
	std::random_shuffle(keys.begin(), keys.end());
	for(std::vector<int>::iterator it = keys.begin(); erase_count < NUM_KEYS/2; ++it)
	{
		//printf("removing %d\n", *it);
		BOOST_CHECK_NO_THROW(remove_vertex(*it, g, mapping));
		it = keys.erase(it);
		++erase_count;
	}

	// try a second time and create another set of NUM_KEY vertices and remove all of them
	for(int i=NUM_KEYS;i<NUM_KEYS+NUM_KEYS;++i)
	{
		keys.push_back(i);
		BOOST_CHECK_NO_THROW(add_vertex(i, g, mapping));
	}

	std::random_shuffle(keys.begin(), keys.end());
	for(std::vector<int>::iterator it = keys.begin(); it != keys.end(); ++it)
	{
		//printf("removing %d\n", *it);
		BOOST_CHECK_NO_THROW(remove_vertex(*it, g, mapping));
	}
	keys.clear();
#undef NUM_KEYS

	// clear the graph
	clear(g, mapping);
}

BOOST_AUTO_TEST_CASE( IndirectMapTestCase7 )
{
	using namespace boost;

	typedef indirect_graph_traits<int,int> IndirectGraphTraits;

	typedef property<vertex_name_t, std::string, IndirectGraphTraits::vertex_property > VertexProperty;
	typedef property<edge_index_t, int, IndirectGraphTraits::edge_property > EdgeProperty;

	// Note that when using indirect_graph_mapping with adjacency_list, the vertex/edge storage must be listS, otherwise the vertex/edge descriptor might not be consistent with the internal storage in indirect_graph_mapping after removing some vertex/edge.
	// However, if you plan not to use edge reference but only vertex, it's OK to use vecS on edge storage while defining the adjacency_list
	typedef adjacency_list<listS, listS, bidirectionalS, VertexProperty, EdgeProperty> Graph;
	typedef graph_traits<Graph>::edge_descriptor EdgeDescriptor;
	typedef graph_traits<Graph>::vertex_descriptor VertexDescriptor;

	typedef indirect_graph_mapping<IndirectGraphTraits, VertexDescriptor, EdgeDescriptor> IndirectGraphMapping;

	Graph g;
	IndirectGraphMapping mapping;

	BOOST_CHECK_NO_THROW(add_vertex(10, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(20, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(30, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(40, g, mapping));

	BOOST_CHECK_NO_THROW(add_edge(1, 10, 20, g, mapping));
	BOOST_CHECK_NO_THROW(add_edge(2, 20, 30, g, mapping));
	BOOST_CHECK_NO_THROW(add_edge(3, 30, 40, g, mapping));

	Graph g_new;
	IndirectGraphMapping mapping_new;

	// copy the graph
	BOOST_CHECK_NO_THROW(copy(g_new, mapping_new, g, mapping));

	// clear old graph
	clear(g, mapping);

	// clear vertex 20, so edge 1 and 2 will be removed
	BOOST_REQUIRE_THROW(add_vertex(10, g_new, mapping_new), std::invalid_argument);
	BOOST_REQUIRE_THROW(add_vertex(20, g_new, mapping_new), std::invalid_argument);
	BOOST_REQUIRE_THROW(add_vertex(30, g_new, mapping_new), std::invalid_argument);
	BOOST_REQUIRE_THROW(add_vertex(40, g_new, mapping_new), std::invalid_argument);

	BOOST_REQUIRE_THROW(add_edge(1, 10, 20, g_new, mapping_new), std::invalid_argument);
	BOOST_REQUIRE_THROW(add_edge(2, 20, 30, g_new, mapping_new), std::invalid_argument);
	BOOST_REQUIRE_THROW(add_edge(3, 30, 40, g_new, mapping_new), std::invalid_argument);

	BOOST_CHECK_NO_THROW(vertex(10, g_new, mapping_new));

	// clear the graph
	clear(g_new, mapping_new);
}

BOOST_AUTO_TEST_CASE( IndirectMapTestCase8 )
{
	using namespace boost;

	typedef indirect_graph_traits<int,int> IndirectGraphTraits;

	typedef property<vertex_name_t, std::string, IndirectGraphTraits::vertex_property > VertexProperty;
	typedef property<edge_index_t, int, IndirectGraphTraits::edge_property > EdgeProperty;

	// Note that when using indirect_graph_mapping with adjacency_list, the vertex/edge storage must be listS, otherwise the vertex/edge descriptor might not be consistent with the internal storage in indirect_graph_mapping after removing some vertex/edge.
	// However, if you plan not to use edge reference but only vertex, it's OK to use vecS on edge storage while defining the adjacency_list
	typedef adjacency_list<setS, setS, bidirectionalS, VertexProperty, EdgeProperty> Graph;
	typedef graph_traits<Graph>::edge_descriptor EdgeDescriptor;
	typedef graph_traits<Graph>::vertex_descriptor VertexDescriptor;

	typedef indirect_graph_mapping<IndirectGraphTraits, VertexDescriptor, EdgeDescriptor> IndirectGraphMapping;

	Graph g;
	IndirectGraphMapping mapping;

	BOOST_CHECK_NO_THROW(add_vertex(10, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(20, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(30, g, mapping));
	BOOST_CHECK_NO_THROW(add_vertex(40, g, mapping));

	BOOST_CHECK_NO_THROW(add_edge(1, 10, 20, g, mapping));
	BOOST_CHECK_NO_THROW(add_edge(2, 20, 30, g, mapping));
	BOOST_CHECK_NO_THROW(add_edge(3, 30, 40, g, mapping));

	BOOST_REQUIRE_THROW(add_edge(1, 10, 20, g, mapping), std::invalid_argument);
	BOOST_REQUIRE_THROW(add_edge(4, 10, 20, g, mapping), std::invalid_argument);

	BOOST_REQUIRE_THROW(add_edge(2, 20, 30, g, mapping), std::invalid_argument);
	BOOST_REQUIRE_THROW(add_edge(5, 20, 30, g, mapping), std::invalid_argument);

}

BOOST_AUTO_TEST_SUITE_END()
