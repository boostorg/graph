#ifndef BOOST_MAXIMUM_FLOW_HPP
#define BOOST_MAXIMUM_FLOW_HPP

// UNDER CONSTRUCTION

namespace boost {

  enum edge_sister_t { edge_sister };
  enum edge_rcap_t { edge_sister };

  namespace detail {
    
    /*
      Implementation Based on Goldberg's 
      "On Implementing Push-Relabel Method for the Maximum Flow Problem"
      by B.V. Cherkassky and A.V. Goldberg, IPCO '95, 157--171.

      This implements the highest-label version of the push-relabel method
      with the global relabeling and gap relabelling heuristics.

      need to write a DIMACS graph format reader...
     */

    template <class Vertex>
    struct preflow_layer {
      typedef std::list<Vertex> list_type;
      typedef typename layer_list_type::iterator list_iterator;
      LayerList push_list;		// nodes with positive excess
      LayerList trans_list;		// nodes with zero excess
    };

    // Graph must have edge_sister and edge_rcap properties
    template <class Graph, 
              class CapacityMap,	// integer value type
              class VertexIndexMap,
              class FlowValue>
    class preflow_push {
    public:
      typedef graph_traits<Graph> Traits;
      typedef typename Traits::vertex_descriptor vertex_descriptor;
      typedef typename Traits::edge_descriptor edge_descriptor;
      typedef typename Traits::vertex_iterator vertex_iterator;
      typedef typename Traits::out_edge_iterator out_edge_iterator;
      typedef typename Traits::vertices_size_type vertices_size_type;

      typedef typename property_map<Graph, edge_sister_t>::type SisterMap;
      typedef typename property_map<Graph, edge_rcap_t>::type ResidualCapacityMap;

      typedef preflow_layer<vertex_descriptor> Layer;
      typedef std::vector< Layer > LayerArray;
      typedef typename LayerArray::iterator layer_iterator;
      typedef typename LayerArray::size_type rank_size_type;

      //=======================================================================
      preflow_push(Graph& g_, 
		   CapacityMap cap_,
		   vertex_descriptor src_, 
		   vertex_descriptor sink_,
		   VertexIndexMap idx_)
	: g(g_), n(num_vertices(g_)), capacity(cap_), src(src_), sink(sink_), 
	  index(idx_),
	  sister(get(edge_sister, g_)), 
	  residual_capacity(get(edge_rcap, g_))
      {
	vertex_iterator i_iter, end;
	for (tie(i_iter, end) = vertices(g); i_iter != end; ++i_iter)
	  excess_flow[*i_iter] = FlowValue();
	
	excess_flow[src] = std::numeric_limits<FlowValue>::max();
	lmax = num_vertices(g) - 1;
      }

      //=======================================================================
      void global_rank_update()
      {
	vertex_iterator i_iter, i_end;
	for (tie(i_iter,i_end) = vertices(g); i_iter != i_end; ++i_iter) {
	  rank[*i_iter] = n;
	rank[sink] = 0;

	Q.push(sink);
	lmax = lmax_push = 0;
	lmin_push = n;

	while (! Q.empty()) {
	  vertex_descriptor i = Q.top();
	  Q.pop();
	  rank_size_type j_rank = rank[i] + 1;

	  out_edge_iterator ai, a_end;
	  for (tie(ai, a_end) = out_edges(i, g); ai != a_end; ++ai) {
	    edge_descriptor a = *ai;
	    vertex_descriptor j = target(a, g);
	    if ( rank[j] == n && residual_capacity[sister[a]] > 0 ) {
	      rank[j] = j_rank;
	      current[j] = out_edges(j, g).first;
	      Layer& layer = layers[j_rank];
	      lmax = std::max(j_rank, lmax);

	      if (excess_flow[j] > 0) {
		layer.push_list.push_front(j);
		layer_list_iter[j] = layer.push_list.begin();
		lmax_push = std::max(j_rank, lmax_push);
		lmin_push = std::min(j_rank, lmin_push);
	      } else {
		layer.transit_list.push_front(j);
		layer_list_iter[j] = layer.transit_list.begin();
	      }
	      Q.push(j);
	    } // if (rank[j] == n && ...)
	  } // for out_edges(i)
	} // while (! Q.empty())
      } // global_rank_update()

      //=======================================================================
      bool push(vertex_descriptor i)
      {
	rank_size_type next_layer_rank = rank[i] - 1;

	out_edge_iterator ai, a_end;
	ai = current[i]; a_end = out_edges(i, g).second;
	for (; ai != a_end; ++ai) {
	  edge_descriptor a = *ai;
	  vertex_descriptor j = target(a, g);

	  if (rank[j] == next_layer_rank) {

	    flow_delta = std::min(excess_flow[i], residual_capacity[a]);

	    residual_capacity[a] -= flow_delta;
	    residual_capacity[sister[a]] += flow_delta;

	    if (next_layer_rank > 0) {
	      next_layer = layers[next_layer_rank];
	      if (excess_flow[j] == 0) {
		next_layer.transit_list.erase( layer_list_iter[j] );
		next_layer.push_list.push_front(j);
		lmin_push = std::min(next_layer_rank, lmin_push);
	      } // if (excess_flow[j] == 0)
	    } // if (next_layer_rank > 0)

	    excess_flow[j] += flow_delta;
	    excess_flow[i] -= flow_delta;

	    if (excess_flow[i] == 0) 
	      break;

	  } // if (rank[j] == next_layer_rank)
	} // for out_edges of i from current

	current[i] = ai;
	return ai == out_edges(i,g).second ? true : false;
      } // push()

      //=======================================================================
      rank_size_type relabel(vertex_descriptor i)
      {
	rank_size_type j_rank = num_vertices(g);
	rank[i] = j_rank;

	out_edge_iterator ai, a_end, a_j;
	for (tie(ai, a_end) = out_edges(i, g); ai != a_end; ++ai) {
	  edge_descriptor a = *ai;
	  if (residual_capacity[a] > 0) {
	    vertex_descriptor j = target(a, g);
	    if (rank[j] < j_rank) {
	      j_rank = rank[j];
	      a_j = ai;
	    }
	  }
	} // for all out edges of i

	++j_rank;
	if (j_rank < n) {
	  rank[i] = j_rank;
	  current[i] = a_j;
	  Layer& layer = layers[j_rank];
	  if (excess_flow[i] > 0) {
	    layer.push_list.push_front(, i);
	    layer_list_iter[i] = layer.push_list.begin();
	    lmax_push = std::max(j_rank, lmax_push);
	    lmin_mush = std::min(j_rank, lmin_push);
	  } else {
	    layer.transit_list.push_front(i);
	    layer_list_iter[i] = layer.transit_list.begin();
	  } // if (excess_flow[i] > 0)
	  lmax = std::max(j_rank, lmax);
	} // (j_rank < n)

	return j_rank;
      } // relabel()

      //=======================================================================
      // cleanup beyond the gap
      void gap(layer_iterator empty_layer)
      {
	rank_size_type r; // rank of layer before the current layer
	r = (empty_layer - layers.begin()) - 1;

	for (layer_iterator l = empty_layer + 1;
	     l != layers.begin() + lmax; ++l) {
	  for (typename Layer::list_iterator i = l->push_list.begin();
	       i != l->push_list.end(); ++i)
	    rank[*i] = n;

	  for (typename Layer::list_iterator i = l->trans_list.begin();
	       i != l->push_list.end(); ++i)
	    rank[*i] = n;

	  l->push_list.clear();
	  l->trans_list.clear();
	}
	lmax = r;
	lmax_push = r;
      }

      //=======================================================================
      void convert_preflow_to_flow()
      {
	vertex_iterator i_iter, i_end;
	out_edge_iterator ai, a_end;

	// handle self-loops
	for (tie(i_iter, i_end) = vertices(g); i_iter != i_end; ++i_iter)
	  for (tie(ai, a_end) = out_edge(*i, g); ai != a_end; ++ai)
	    if (target(*ai, g) == *i_iter)
	      residual_capacity[*ai] = capacity[*ai];

	// initialize
	for (tie(i_iter, i_end) = vertices(g); i_iter != i_end; ++i_iter) {
	  i = *i_iter;
	  rank[i] = white(rank[i]);
	  current[i] = out_edges(i,g).first;
	}

	for (tie(i_iter,i_end) = vertices(g); i_iter != i_end; ++i) {
	  i = *i_iter;
	  if ( rank[i] == white() && excess_flow[i] > 0
	       && i != src && i != sink ) {
	    r = i;
	    rank[r] = gray();
	    do {
	      for (; current[i] != out_edges(i,g).second; ++current[i]) {
		a = current[i];
		if ( capacity[a] == 0 && residual_capacity[a] > 0
		     && target(a,g) != src && target(a,g) != sink ) {
		  j = target(a, g);
		  if (rank[j] == white()) {
		    rank[j] = grey();
		    layer_list_iter[j] = i_iter;
		    i = j; // ??
		    break;
		  } else if (rank[j] == grey()) {
		    delta = residual_capacity[a];
		    while (1) {
		      delta = std::min(delta, residual_capacity[*current[j]]);
		      if (j == i)
			break;
		      else
			j = target(*current[j], g);
		    }
		    // back-out DFS to the first zeroed edge
		    restart = i;
		    for (j = target(*current[i], g); j != i; j = target(a,g)) {
		      a = current[j];
		      if (rank[j] == white() || residual_capacity[a] == 0) {
			rank[target(*current[j],g)] = white();
			if (rank[k] != white())
			  restart = j;
		      }
		    }

		    if (restart != i) {
		      i = restart;
		      ++current[i];
		      break;
		    }
		  }

		}
	      } // for

	      if (current[i] == out_edges(i,g).second) {
		rank[i] = black();
		if (i != src) {
		  if (bos == NULL) { 
		    bos = i_iter;
		    tos = i_iter;
		  } else {
		    layer_list_iter[i] = tos;
		    tos = i_iter;
		  }
		}
		if (i != r)
		  layer.push_or_transit_list.erase(layer_list_iter[i]); // ??
		else
		  break;
	      }
	    } while ( 1 );
	  } // if
	} // for 

	if (bos != NULL) {
	  i_iter = tos;
	  do {
	    i = *i_iter;
	    ai = out_edges(i, g).first;
	    a = *ai;
	    while (excess_flow[i] > 0) {
	      if (capacity[a] == 0 && residual_capacity[a] > 0) {
		delta = std::min(excess_flow[i], residual_capacity[a]);
		residual_capacity[a] -= delta;
		residual_capacity[sister[a]] += delta;
		excess_flow[i] -= delta;
		excess_flow[target(a, g)] += delta;
	      }
	      ++ai;
	    } // while (excess_flow[i] > 0)
	    if (i == bos)
	      break;
	    else
	      ++i_iter;
	  } while ( 1 );
	}

      } // convert_preflow_to_flow()

      //=======================================================================

      Graph& g;
      const vertices_size_type n;
      CapacityMap capacity;
      Vertex src;
      Vertex sink;
      VertexIndexMap index;

      // need to use random_access_property_map with these
      std::vector< FlowValue > excess_flow;
      std::vector< typename Layer::list_iterator > layer_list_iter;
      std::vector< out_edge_iterator > current;
      std::vector< size_type > rank;

      SisterMap sister;
      ResidualCapacityMap residual_capacity;

      LayerArray layers;
      size_type lmax;			// maximal layer
      size_type lmax_push;		// maximal layer with excess node
      size_type lmin_push;		// minimal layer with excess node
      std::queue<vertex_descriptor> Q;

      enum { global_update_frequency = 1 };
    };

  } // namespace detail

  template <class Graph, 
            class CapacityMap, class VertexIndexMap, 
            class FlowValue>
  void maximum_flow(Graph& g, 
		    typename graph_traits<Graph>::vertex_descriptor src,
		    typename graph_traits<Graph>::vertex_descriptor sink,
		    CapacityMap cap, VertexIndexMap index_map,
		    FlowValue& flow)
  {
    detail::preflow_push<Graph, CapacityMap, VertexIndexMap, FlowValue> 
      algo(g, cap, src, sink, index_map);

    algo.global_rank_update();
    num_relabels = 0;

    while (algo.lmax_push >= algo.lmin_push) {

      Layer& layer = algo.layers[algo.lmax_push];
      
      typename List::list_iterator i_iter = layer.push_list.begin();
      if (i_iter == layer.push_list.end())
	--algo.lmax_push;
      else {
	layer.push_list.pop_front();
	if (algo.push(*i_iter)) { // i must be relabelled
	  algo.relabel(*i_iter);
	  ++num_relabels;
	  if (layer.push_list.empty() && layer.transit_list.empty())
	    algo.gap(layer);
	  
	  if (n_r > global_update_frequency * n) {
	    algo.global_rank_update();
	    num_relabels = 0;
	  }
	} else {
	  layer.transit_list.erase(i_iter);
	}
      }
      
    } // while (algo.lmax_push >= algo.lmin_push)

    flow += algo.excess_flow[sink];
    algo.convert_preflow_to_flow();
    
  } // maximum_flow()

} // namespace boost

#endif // BOOST_MAXIMUM_FLOW_HPP


/*
  Max-Flow(V, E, s, t, c)
    <<initialization>>
      <<intialize preflow>>
	 for all (v,w) in (V - {s}) x (V - {s}) 
	   f(v,w) <- 0
	   f(w,v) <- 0
	 for all v in V
	   f(s,v) <- c(s,v)
	   f(v,s) <- -c(s,v)
      <<initialize labels and excesses>>
	d(s) <- n
	for all v in V - {s}
	  d(v) <- 0
	  e(v) <- f(s,v)
    <<loop>>
      while there exists a basic operation that applies
	select a basic op and apply it
    return f;

    Basic operations are push and relabel

    Push(v, w)
       applicable if v is active r_f(v,w) > 0 and d(v) = d(w) + 1
       f(v,w) <- f(v,w) + delta
       f(w,v) <- f(w,v) - delta
       e(v) <- e(v) - delta
       e(w) <- e(w) + delta

    Relabel(v)
       applicable if v is active and 
         for all w in V, r_f(v,w) > 0 => d(v) <= d(w)
       d(v) <- min{ d(w) + 1 | (v,w) in E_f }
        or d(v) <- infty if E_f is empty



    Use an undirected graph
      each undirected edge {v,w} has three stored properties
        c(v,w)
        c(w,v)
        f(v,w)

      each vertex v has the following stored properties
        current edge {v,w} which is the candidate for a pushing operation
           use an out_edge_iterator

    Use a Q to store all active vertices
      initially Q = { v in V - {s,t} | c(s,v) > 0 }


   choose node with highest-label to discharge

*/

    // Vertex Properties:
    //
    // layer_list_iter[v]  points into push_list or trans_list
    // current[v]  (value_type == out_edge_iterator)
    // rank[v]    also known as label
    // excess_flow[v]

    // Edge Properties:
    //
    // sister[(u,v)] => (v,u)
    // residual_capacity[(u,v)]


#if 0
typedef std::list<Vertex> RankSet;
typedef std::vector<LabelSet> ArrayOfRankSets;

ArrayOfLabelSets B;
ArrayOfLabelSets::size_type b;


void initialization(s)
{
  B.push_back(LabelSet());
  B[0].push_back(s);
  b = 0;
}

void maximum_flow()
{

  // stage 1
  while (! B[0].empty()) {
    v = B[b].front(); B[b].pop_front();
    discharge(v);
    b = ?;
  }
  // stage 2
  // converts f into flow
  //   compute the decomposition of f
  //   and reduce f on paths from s to nodes with flow excess
}


// global relabeling updates distance function by computing shortest
// path distance in the residual graph from all nodes to the sink


// gap relabeling: 
//
// Let g be an integer between 0 and n. Suppose at some stage
// there are no nodes with distance label g, but there are nodes
// v with g < d(v) < n. Then the sink is not reachable from
// any of these nodes. The labels of these nodes may be increased.

typedef std::list<Vertex> DistanceSet;
typedef std::vector<DistanceSet> ArrayOfDistanceSet;


    
//    Push/Relabel(v)
//      applicable when v is active
void push_relabel(v) {
  tie(v,w) = incident(*current_edge_iter[v], g);
  if (push_is_applicable(v,w))
    push(v,w);
  else {
    if (current_edge_iter[v] != out_edges(v).second)
      ++current_edge_iter[v];
    else {
      current_edge_iter[v] = out_edges(v).first;
      relabel(v);
    }       
  }
}

// Discharge
// Applicable when !Q.empty()
void discharge(v) {
  v = Q.top(); 
  Q.pop();
  do {
    push_relabel(v);
    if (w becomes active during push/relabel)
      Q.push(w);
  } while (e(v) == 0 || d(v) increases);
  if (v is still active)
    Q.push(v);
}
#endif
