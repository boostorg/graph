#ifndef BOOST_MAXIMUM_FLOW_HPP
#define BOOST_MAXIMUM_FLOW_HPP

// UNDER CONSTRUCTION

#include <boost/graph/detail/list_base.hpp>

namespace boost {

  enum edge_sister_t { edge_sister };
  enum edge_residual_capacity_t { edge_residual_capacity };

  namespace detail {
    
    /*
      Implementation Based on Goldberg's 
      "On Implementing Push-Relabel Method for the Maximum Flow Problem"
      by B.V. Cherkassky and A.V. Goldberg, IPCO '95, 157--171.

      Also based on the h_prf.c code written by the above authors.

      This implements the highest-label version of the push-relabel method
      with the global relabeling and gap relabeling heuristics.

      need to write a DIMACS graph format reader...

      The "rank" or layers used here and in Goldberg's implementation
      is the reverse of the distance discussed in the paper.

      Each layer consists of all active nodes with the same rank.
        An active node has positive excess flow and its
        distance is less than n (it is not blocked).
     */

    template <class T>
    struct list_node {
      explicit list_node(const T& x)
        : m_data(x), m_next(*this), m_prev(*this) { }
      T m_data;
      list_node<T>* m_next;
      list_node<T>* m_prev;
    };
    template <class T>
    struct list_node_next {
      T& operator()(list_node<T>* x) { return x->m_next; }
      const T& operator()(const list_node<T>* x) { return x->m_next; }
    };
    template <class T>
    struct list_node_prev {
      T& operator()(list_node<T>* x) { return x->m_prev; }
      const T& operator()(const list_node<T>* x) { return x->m_prev; }
    };
    template <class T>
    struct delete_node {
      void operator()(const list_node<T>* x) { delete x; }
    };

    template <class Vertex>
    struct preflow_layer {
      preflow_layer()
        : push_list(new list_node<Vertex>()),
          transit_list(new list_node<Vertex>()) 
        { }
      ~preflow_layer() { 
        dlist_clear(push_list, delete_node<Vertex>());
        delete push_list; 
        dlist_clear(transit_list, delete_node<Vertex>()); 
        delete transit_list;
      }
      typedef list_node<Vertex> list_node;
      list_node<Vertex>* push_list;     // nodes with positive excess
      list_node<Vertex>* transit_list;  // nodes with zero excess

      // push_list and transit_list point to the dummy node of the dlist.
      // next(push_list) is the first node of the push list.
      // next(transit_list) is the first node of the transit list.
    };

    // Graph must have edge_sister and edge_rcap properties
    template <class Graph, 
              class EdgeCapacityMap,    // integer value type
              class VertexIndexMap,     // vertex_descriptor -> integer
              class FlowValue>
    class push_relabel
    {
    public:
      typedef graph_traits<Graph> Traits;
      typedef typename Traits::vertex_descriptor vertex_descriptor;
      typedef typename Traits::edge_descriptor edge_descriptor;
      typedef typename Traits::vertex_iterator vertex_iterator;
      typedef typename Traits::out_edge_iterator out_edge_iterator;
      typedef typename Traits::vertices_size_type vertices_size_type;

      typedef typename property_map<Graph, edge_sister_t>::type SisterMap;
      typedef typename property_map<Graph, edge_rcap_t>::type
        ResidualCapacityMap;

      typedef preflow_layer<vertex_descriptor> Layer;
      typedef std::vector< Layer > LayerArray;
      typedef typename LayerArray::iterator layer_iterator;
      typedef typename LayerArray::size_type rank_size_type;

      typedef color_traits<default_color_type> ColorTraits;

      typedef list_node<vertex_descriptor> list_node;

      //=======================================================================
      push_relabel(Graph& g_, 
                   EdgeCapacityMap cap_,
                   vertex_descriptor src_, 
                   vertex_descriptor sink_,
                   VertexIndexMap idx_)
        : g(g_), n(num_vertices(g_)), capacity(cap_), src(src_), sink(sink_), 
          index(idx_),
          excess_flow(num_vertices(g_)),
          layer_list_ptr(num_vertices(g_)),
          current(num_vertices(g_)),
          rank(num_vertices(g_)),
          color(num_vertices(g_)),
          sister(get(edge_sister, g_)),
          residual_capacity(get(edge_residual_capacity, g_))
      {
        vertex_iterator i_iter, end;
        for (tie(i_iter, end) = vertices(g); i_iter != end; ++i_iter)
          excess_flow[*i_iter] = 0;
        
        excess_flow[src] = std::numeric_limits<FlowValue>::max();
        lmax = num_vertices(g) - 1;
      }

      //=======================================================================
      // This is a breadth-first search over the residual graph
      // (well, actually the reverse of the residual graph).
      // Would be cool to have a graph view adaptor for hiding certain
      // edges, like the saturated (non-residual) edges in this case.
      // Goldberg's implementation abused "rank" for the coloring.
      void global_rank_update()
      {
        vertex_iterator i_iter, i_end;
        for (tie(i_iter,i_end) = vertices(g); i_iter != i_end; ++i_iter) {
          color[*i_iter] = ColorTraits::white();
        color[sink] = ColorTraits::gray();

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
            if ( color[j] == ColorTraits::white()
                 && residual_capacity[sister[a]] > 0 ) {
              rank[j] = j_rank;
              color[j] = ColorTraits::gray();
              current[j] = out_edges(j, g).first;
              Layer& layer = layers[j_rank];
              lmax = std::max(j_rank, lmax);

              if (excess_flow[j] > 0) {
                list_node* pj = new list_node(j);
                dlist_insert_before(next(layer.push_list), pj, next, prev);
                layer_list_ptr[j] = pj;
                lmax_push = std::max(j_rank, lmax_push);
                lmin_push = std::min(j_rank, lmin_push);
              } else {
                list_node* pj = new list_node(j);
                dlist_insert_before(next(layer.transit_list), pj, next, prev);
                layer_list_ptr[j] = pj;
              }
              Q.push(j);
            } // if (rank[j] == n && ...)
          } // for out_edges(i)
        } // while (! Q.empty())
      } // global_rank_update()

      //=======================================================================
      // This function is called "push" in Goldberg's implementation,
      // but it is called "discharge" in the paper.
      bool discharge(vertex_descriptor i)
      {
        rank_size_type next_layer_rank = rank[i] - 1;

        out_edge_iterator ai, a_end;
        ai = current[i]; a_end = out_edges(i, g).second;
        for (; ai != a_end; ++ai) {
          edge_descriptor a = *ai;
          vertex_descriptor j = target(a, g);

          if (rank[j] == next_layer_rank) { // if j belongs to the next layer

           if (next_layer_rank > 0) {
              next_layer = layers[next_layer_rank];

              if (excess_flow[j] == 0) { // j already had zero excess flow
                dlist_remove(layer_list_ptr[j], next, prev);
                dlist_insert_before(next(next_layer.push_list), 
                                    layer_list_ptr[j], next, prev);
                lmin_push = std::min(next_layer_rank, lmin_push);
              } // if (excess_flow[j] == 0)

            } // if (next_layer_rank > 0)

            push(a);

            if (excess_flow[i] == 0)
              break;

          } // if (i,j) is admissible
        } // for out_edges of i from current

        current[i] = ai;
        return ai == out_edges(i,g).second ? true : false; // end of list?
      } // discharge()

      //=======================================================================
      // This corresponds to the "push" update operation of the paper,
      // not the "push" function in Goldberg's implementation.
      void push(edge_descriptor u_v)
      {
        vertex_descriptor
          u = source(u_v, g),
          v = target(u_v, g);
        
        FlowValue flow_delta
          = std::min(excess_flow[u], residual_capacity[u_v]);

        residual_capacity[u_v] -= flow_delta;
        residual_capacity[sister[u_v]] += flow_delta;

        excess_flow[u] -= flow_delta;
        excess_flow[v] += flow_delta;
      }

      //=======================================================================
      rank_size_type relabel(vertex_descriptor i)
      {
        rank_size_type j_rank = num_vertices(g);
        rank[i] = j_rank;

        // Examine the residual out-edges of vertex i, choosing the
        // edge whose target vertex has the minimal rank.
        // This is the highest-label or "HL" optimization.
        out_edge_iterator ai, a_end, a_j;
        for (tie(ai, a_end) = out_edges(i, g); ai != a_end; ++ai) {
          edge_descriptor a = *ai;
          vertex_descriptor j = target(a, g);
          if (residual_capacity[a] > 0 && rank[j] < j_rank) {
              j_rank = rank[j];
              a_j = ai;
          }
        } // for all out edges of i

        ++j_rank;
        if (j_rank < n) {
          rank[i] = j_rank;     // this is the main action
          current[i] = a_j;
          Layer& layer = layers[j_rank];
          if (excess_flow[i] > 0) {
            list_node* pi = new list_node(i);
            dlist_insert_before(next(layer.push_list), pi, next, prev);
            layer_list_ptr[i] = pi;
            lmax_push = std::max(j_rank, lmax_push);
            lmin_mush = std::min(j_rank, lmin_push);
          } else {
            list_node* pi = new list_node(i);
            dlist_insert_before(next(layer.transit_list), pi, next, prev);
            layer_list_ptr[i] = pi;
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
          list_node* i;
          for (i = next(l->push_list); i != l->push_list; i = next(i))
            rank[i->m_data] = n;

          for (i = next(l->transit_list);  i != l->transit_list; i = next(i))
            rank[i->m_data] = n;

          dlist_clear(l->push_list);
          dlist_clear(l->trans_list);
        }
        lmax = r;
        lmax_push = r;
      }

      //=======================================================================
      // remove excessive flow, the "second phase"
      // Basically, this does a DFS on the reverse flow graph of nodes
      //   with excess flow.
      // If a cycle is found, cancel it.
      // Return the nodes with excess flow in topological order.
      //
      // Unlike the prefl_to_flow() implementation, we use
      //   "color" instead of "rank" for the DFS labels
      //   "parent" instead of nl_prev for the DFS tree
      //   "topo_next" instead of nl_next for the topological ordering
      void convert_preflow_to_flow()
      {
        vertex_iterator i_iter, i_end;
        out_edge_iterator ai, a_end;

        vertex_descriptor r, restart;
        vertex_iterator tos, bos;

        std::vector<vertex_descriptor> parent(n);
        std::vector<vertex_descriptor> topo_next(n);

        // handle self-loops
        for (tie(i_iter, i_end) = vertices(g); i_iter != i_end; ++i_iter)
          for (tie(ai, a_end) = out_edge(*i, g); ai != a_end; ++ai)
            if (target(*ai, g) == *i_iter)
              residual_capacity[*ai] = capacity[*ai];

        // initialize
        for (tie(i_iter, i_end) = vertices(g); i_iter != i_end; ++i_iter) {
          i = *i_iter;
          color[i] = ColorTraits::white();
          parent[i] = i;
          current[i] = out_edges(i, g).first;
        }

        for (tie(i_iter, i_end) = vertices(g); i_iter != i_end; ++i_iter) {
          i = *i_iter;
          if (color[i] == ColorTraits::white() 
              && excess_flow[i] > 0
              && i != src && i != sink ) {
            r = i;
            color[r] = ColorTraits::gray();
            while (1) {
              for (; current[i] != out_edges(i, g).second; ++current[i]) {
                a = current[i];
                if ( capacity[a] == 0 && residual_capacity[a] > 0
                     && target(a, g) != src && target(a, g) != sink ) {
                  j = target(a, g);
                  if (color[j] == ColorTraits::white()) {
                    color[j] = ColorTraits::gray();
                    parent[j] = i;
                    i = j;
                    break;
                  } else if (color[j] == ColorTraits::gray()) {
                    // find minimum flow on the cycle
                    delta = residual_capacity[a];
                    while (1) {
                      delta = std::min(delta, residual_capacity[*current[j]]);
                      if (j == i)
                        break;
                      else
                        j = target(*current[j], g);
                    }
                    // remove delta flow units
                    j = i;
                    while (1) {
                      a = *current[j];
                      residual_capacity[a] -= delta;
                      residual_capacity[sister[a]] += delta;
                      j = target(a, g);
                      if (j == i)
                        break;
                    }

                    // back-out DFS to the first zeroed edge
                    restart = i;
                    for (j = target(*current[i], g); j != i; j = target(a, g)){
                      a = current[j];
                      if (color[j] == ColorTraits::white() || 
                          residual_capacity[a] == 0) {
                        color[target(*current[j], g)] = ColorTraits::white();
                        if (color[j] != ColorTraits::white())
                          restart = j;
                      }
                    } // for
                    if (restart != i) {
                      i = restart;
                      ++current[i];
                      break;
                    }
                  } // else if (color[j] == ColorTraits::gray())
                } // if (capacity[a] == 0 ...
              } // for out_edges(i, g)  (though "i" changes during loop)

              if (current[i] == out_edges(i, g).second) {
                // scan of i is complete
                color[i] = ColorTraits::black();
                if (i != src) {
                  if (bos == NULL) { 
                    bos = i_iter;
                    tos = i_iter;
                  } else {
                    topo_next[i] = *tos;
                    tos = i_iter;
                  }
                }
                if (i != r) {
		  i = topo_next[i];
		  ++current[i];
                } else
                  break;
              }
            } // while (1)
          } // if color == white ...

          i_iter = vertices(g).first + i;
        } // for all vertices in g

        if (bos != vertices(g).second) {
          i_iter = tos;
          do {
            i = *i_iter;
            ai = out_edges(i, g).first;
            a = *ai;
            while (excess_flow[i] > 0 && ai != out_edges(i, g).second) {
              if (capacity[a] == 0 && residual_capacity[a] > 0)
		push(a);
              ++ai;
            }
            if (i == *bos)
              break;
            else
              i = topo_next[i];
          } while ( 1 );
        }
       	
      } // convert_preflow_to_flow()

      //=======================================================================

      Graph& g;
      const vertices_size_type n;
      EdgeCapacityMap capacity;
      Vertex src;
      Vertex sink;
      VertexIndexMap index;

      // need to use random_access_property_map with these
      std::vector< FlowValue > excess_flow;
      std::vector< list_node* > layer_list_ptr;
      std::vector< out_edge_iterator > current;
      std::vector< rank_size_type > rank;
      std::vector<default_color_type> color;

      // Edge Property Maps that must be interior to the graph
      SisterMap sister;
      ResidualCapacityMap residual_capacity;

      LayerArray layers;
      rank_size_type lmax;              // maximal layer
      rank_size_type lmax_push;         // maximal layer with excess node
      rank_size_type lmin_push;         // minimal layer with excess node
      std::queue<vertex_descriptor> Q;

      inline double global_update_frequency() { return 1.0; }

      list_node_next<T> next;
      list_node_prev<T> prev;
    };

  } // namespace detail

  template <class Graph, 
            class EdgeCapacityMap, class VertexIndexMap, 
            class FlowValue>
  void maximum_flow(Graph& g, 
                    typename graph_traits<Graph>::vertex_descriptor src,
                    typename graph_traits<Graph>::vertex_descriptor sink,
                    EdgeCapacityMap cap, VertexIndexMap index_map,
                    FlowValue& flow)
  {
    detail::push_relabel<Graph, EdgeCapacityMap, VertexIndexMap, FlowValue> 
      algo(g, cap, src, sink, index_map);

    algo.global_rank_update();
    typename graph_traits<Graph>::vertices_size_type 
      num_relabels = 0, n = num_vertices(g);

    while (algo.lmax_push >= algo.lmin_push) {

      Layer& layer = algo.layers[algo.lmax_push];
      
      list_node* i_node = next(layer.push_list);
      if (i_node == layer.push_list)
        --algo.lmax_push;
      else {
        dlist_remove(next(layer.push_list), next, prev);
        if (algo.discharge(*i_node)) { // returns true if done with out-edges
          algo.relabel(*i_node);
          ++num_relabels;

          // Gap relabelling heuristic
          if (dlist_empty(layer.push_list) && dlist_empty(layer.transit_list))
            algo.gap(layer);

          // Global relabelling heuristic
          if (num_relabels > algo.global_update_frequency() * n) {
            algo.global_rank_update();
            num_relabels = 0;
          }

        } else {
          dlist_remove(i_node, next, prev);
          delete i_node;
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
    // layer_list_ptr[v]  points into push_list or trans_list
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
