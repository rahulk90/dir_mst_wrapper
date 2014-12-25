#include <iostream>
#include <vector>
#include <fstream>
#include <utility>
#include <iterator>
#include <cstdlib>
#include <string>
#include <limits>
#include <cerrno>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/concept_check.hpp>
#include <boost/operators.hpp>
#include <boost/iterator.hpp>
#include <boost/multi_array.hpp>
#include <boost/random.hpp>
#include <sys/time.h>
#include "../src/edmonds_optimum_branching.hpp"
#include "boost/tuple/tuple.hpp"
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/io.hpp>
using boost::tuple;
using namespace std;
using namespace boost;

#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

// definitions of a complete graph that implements the EdgeListGraph
// concept of Boost's graph library.
namespace boost {
	struct complete_graph {
		complete_graph(int n_vertices) : n_vertices(n_vertices) {}
		int n_vertices;
		
		struct edge_iterator : public input_iterator_helper<edge_iterator, int, std::ptrdiff_t, int const *, int>
		{
			int edge_idx, n_vertices;
			edge_iterator() : edge_idx(0), n_vertices(-1) {}
			edge_iterator(int n_vertices, int edge_idx) : edge_idx(edge_idx), n_vertices(n_vertices) {}
			edge_iterator &operator++()
			{
				if (edge_idx >= n_vertices * n_vertices)
					return *this;
				++edge_idx;
				if (edge_idx / n_vertices == edge_idx % n_vertices)
					++edge_idx;
				return *this;
			}
			int operator*() const {return edge_idx;}
			bool operator==(const edge_iterator &iter) const
			{
				return edge_idx == iter.edge_idx;
			}
		};
	};
	
	template<>
	struct graph_traits<complete_graph> {
		typedef int                             vertex_descriptor;
		typedef int                             edge_descriptor;
		typedef directed_tag                    directed_category;
		typedef disallow_parallel_edge_tag      edge_parallel_category;
		typedef edge_list_graph_tag             traversal_category;
		typedef complete_graph::edge_iterator   edge_iterator;
		typedef unsigned                        edges_size_type;
		
		static vertex_descriptor null_vertex() {return -1;}
	};

	pair<complete_graph::edge_iterator, complete_graph::edge_iterator>
	edges(const complete_graph &g)
	{
		return make_pair(complete_graph::edge_iterator(g.n_vertices, 1),
						 complete_graph::edge_iterator(g.n_vertices, g.n_vertices*g.n_vertices));
	}
	
	unsigned
	num_edges(const complete_graph &g)
	{
		return (g.n_vertices - 1) * (g.n_vertices - 1);
	}
	
	int
	source(int edge, const complete_graph &g)
	{
		return edge / g.n_vertices;
	}
	
	int
	target(int edge, const complete_graph &g)
	{
		return edge % g.n_vertices;
	}
}

typedef graph_traits<complete_graph>::edge_descriptor Edge;
typedef graph_traits<complete_graph>::vertex_descriptor Vertex;


int dirMST(string fin,string fout,string mode)
{
	int n_vertices;
	vector< tuple<int,int,float> > edge_list;
	tuple<int,int,float> g_edge;

	string line;
	int v1,v2,ctr = 0;
	float weight;
	ifstream inputf (fin.c_str());
	if(inputf.is_open())
	{
		getline(inputf,line);
		sscanf(line.c_str(),"%d",&n_vertices);
		while(getline(inputf,line))
		{
			sscanf (line.c_str(),"%d,%d,%f",&v1,&v2,&weight);
			//Assumes vertices start from 0...N-1
			edge_list.push_back(make_tuple(v1,v2,weight));
		}
		inputf.close();
	}
	else
	{
		cerr<<"Input file not found. Cannot be opened\n";
		exit(1);
	}
	
	//Read in the edgelist and set the weights to their values, all other weights will
	//be -Inf
	//2 dimensional array of integer weights -Converting to float, make sure ok

	//Build a complete graph on n vertices (hack)
	//TODO: Fix to build it based on the graph structure
	complete_graph g(n_vertices);
	multi_array<float, 2> weights(extents[n_vertices][n_vertices]);
	vector<Vertex> parent(n_vertices);
	Vertex roots[] = {0, 1};
	vector<Edge> branching;
	//Initialize all weights to 0
	for(int i=0;i<n_vertices;i++)
	{
		for(int j=0;j<n_vertices;j++)
		{   
			//If using Minimum Spanning Tree
			if(mode.compare("min")==0)
				weights[i][j] = numeric_limits<int>::max();
			else//Maximum spanning tree
				weights[i][j] = numeric_limits<int>::min();
		}
	}

	//Write out the indices of the edge set that form the directed spanning tree
	numeric::ublas::mapped_matrix<int> index_map (n_vertices,n_vertices);
	DEBUG_MSG("---Graph Read---\nN: "<<n_vertices);
	BOOST_FOREACH(g_edge,edge_list)
	{
		v1 = get<0>(g_edge);
		v2 = get<1>(g_edge);
		weight=get<2>(g_edge);
		DEBUG_MSG(v1<<" | "<<v2<<" | "<<weight);
		weights[v1][v2] = weight;
		index_map(v1,v2) = ctr++;
	}

	if(mode.compare("min")==0)
	{
		//TOptimum is Maximum set to false -> MinST
		edmonds_optimum_branching<false, true, true>
		(g, identity_property_map(), weights.origin(),
		 roots, roots + 0, back_inserter(branching));
	}
	else
	{
		//TOptimum is Maximum set to true -> MaxST
		edmonds_optimum_branching<true, true, true>
		(g, identity_property_map(), weights.origin(),
		 roots, roots + 0, back_inserter(branching));
	}
		
	//Write result to file
	DEBUG_MSG("--Optimal ("<<mode<<") Branching--");
	ofstream outf(fout.c_str());
	if(outf==NULL)
	{
		cerr<<"Output file not created."<<endl;
		exit(1);
	}
	BOOST_FOREACH(Edge e ,branching)
	{
		v1 =source(e, g);
		v2 =target(e, g);
		ctr = index_map(v1,v2);

		DEBUG_MSG(v1<<"->"<<v2<<" idx:"<<ctr);
		outf<<ctr<<endl;
	} 
	
	return EXIT_SUCCESS;

}
string PNAME = "mstwrapper";
int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		cerr << "Usage: " << PNAME
			 << " <input file name> <output file name> <min|max>\n";
		exit(1);
	}
	string mode = string(argv[3]);
	if(mode.compare("min")==0 || mode.compare("max")==0)//strcmp(argv[3],"min")==0 || strcmp(argv[3],"max"){
	{
		return dirMST(argv[1],argv[2],mode);
	}
	else
	{
		cerr<<"Final argument not <min> or <max>"<<endl;
		exit(1);
	}
	
}
