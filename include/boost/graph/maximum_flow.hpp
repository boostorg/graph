#ifndef BOOST_MAXIMUM_FLOW_HPP
#define BOOST_MAXIMUM_FLOW_HPP

// UNDER CONSTRUCTION

#include <boost/graph/detail/list_base.hpp>

namespace boost {

  enum edge_reverse_t { edge_reverse };
  enum edge_residual_capacity_t { edge_residual_capacity };

  namespace detail {
    
    /*
      Implementation Based on Goldberg's 
      "On Implementing Push-Relabel Method for the Maximum Flow Problem"
      by B.V. Cherkassky and A.V. Goldberg, IPCO '95, 157--171.

      Also based on the h_prf.c and hi_pr.c code written by the above authors.

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
        : active_vertices(new list_node<Vertex>()),
          inactive_vertices(new list_node<Vertex>()) 
        { }
      ~preflow_layer() { 
        dlist_clear(active_vertices, delete_node<Vertex>());
        delete active_vertices; 
        dlist_clear(inactive_vertices, delete_node<Vertex>()); 
        delete inactive_vertices;
      }
      typedef list_node<Vertex> list_node;
      list_node<Vertex>* active_vertices;    // nodes with positive excess
      list_node<Vertex>* inactive_vertices;  // nodes with zero excess
      
      // active_vertices and inactive_vertices point to the dummy node
      // of the dlist.  next(active_vertices) is the first node of the
      // push list.  next(inactive_vertices) is the first node of the
      // transit list.
    };

    // Graph must have edge_reverse and edge_rcap properties
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

      typedef typename property_map<Graph, edge_reverse_t>::type SisterMap;
      typedef typename property_map<Graph, edge_residual_capacity_t>::type
        ResidualCapacityMap;

      typedef preflow_layer<vertex_descriptor> Layer;
      typedef std::vector< Layer > LayerArray;
      typedef typename LayerArray::iterator layer_iterator;
      typedef typename LayerArray::size_type distance_size_type;

      typedef color_traits<default_color_type> ColorTraits;

      typedef list_node<vertex_descriptor> list_node;


      //=======================================================================
      // Layer List Management Functions

      void add_active(vertex_descriptor u, Layer& layer) {
        list_node* u_node = new list_node(u);
        dlist_insert_before(next(layer.active_vertices), u_node, next, prev);
        max_active = std::max(distance[j], max_active);
        min_active = std::min(distance[j], min_active);
        layer_list_ptr[u] = u_node;
      }
      void remove_active(vertex_descriptor u) {
        dlist_remove(layer_list_ptr[u], next, prev);    
        delete layer_list_ptr[u];
      }

      void add_inactive(vertex_descriptor u, Layer& layer) {
        list_node* u_node = new list_node(u);
        dlist_insert_before(next(layer.inactive_vertices), u_node, next, prev);
        layer_list_ptr[u] = u_node;
      }
      void remove_inactive(vertex_descriptor u) {
        dlist_remove(layer_list_ptr[u], next, prev);    
        delete layer_list_ptr[u];
      }

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
          distance(num_vertices(g_)),
          color(num_vertices(g_)),
          reverse_edge(get(edge_sister, g_)),
          residual_capacity(get(edge_residual_capacity, g_))
      {
        vertex_iterator i_iter, end;
        for (tie(i_iter, end) = vertices(g); i_iter != end; ++i_iter)
          excess_flow[*i_iter] = 0;
        
        excess_flow[src] = std::numeric_limits<FlowValue>::max();
        max_distance = num_vertices(g) - 1;
      }

      //=======================================================================
      // This is a breadth-first search over the residual graph
      // (well, actually the reverse of the residual graph).
      // Would be cool to have a graph view adaptor for hiding certain
      // edges, like the saturated (non-residual) edges in this case.
      // Goldberg's implementation abused "distance" for the coloring.
      void global_distance_update()
      {
        vertex_iterator i_iter, i_end;
        for (tie(i_iter,i_end) = vertices(g); i_iter != i_end; ++i_iter) {
          color[*i_iter] = ColorTraits::white();
          distance[*i_iter] = n;
        }
        color[sink] = ColorTraits::gray();
        distance[sink] = 0;
        
        for (distance_size_type l = 0; l != max_distance; ++l) {
          dlist_clear(layers[l].active_vertices);
          dlist_clear(layers[l].inactive_vertices);
        }
        
        max_distance = max_active = 0;
        min_active = n;

        Q.push(sink);
        while (! Q.empty()) {
          vertex_descriptor i = Q.top();
          Q.pop();
          distance_size_type j_distance = distance[i] + 1;

          out_edge_iterator ai, a_end;
          for (tie(ai, a_end) = out_edges(i, g); ai != a_end; ++ai) {
            edge_descriptor a = *ai;
            vertex_descriptor j = target(a, g);
            if ( color[j] == ColorTraits::white()
                 && residual_capacity[reverse_edge[a]] > 0 ) {
              distance[j] = j_distance;
              color[j] = ColorTraits::gray();
              current[j] = out_edges(j, g).first;
              max_distance = std::max(j_distance, max_distance);

              if (excess_flow[j] > 0)
                add_active(j, layers[j_distance]);
              else
                add_inactive(j, layers[j_distance]);

              Q.push(j);
            }
          }
        }
      } // global_distance_update()

      //=======================================================================
      // This function is called "push" in Goldberg's h_prf implementation,
      // but it is called "discharge" in the paper and in hi_pr.c.
      void discharge(vertex_descriptor i)
      {
        assert(excess_flow[i] > 0);
        while (1) {
          distance_size_type next_layer_distance = distance[i] - 1;
          out_edge_iterator ai, ai_end;
          for (ai = current[i], ai_end = out_edges(i, g).second;
               ai != ai_end; ++ai) {
            edge_descriptor a = *ai;
            if (residual_capacity[a] > 0) {
              vertex_descriptor j = target(a, g);
              if (distance[j] == next_layer_distance) {
                if (j != sink && excess_flow[j] == 0) {
                  remove_inactive(j);
                  add_active(j, layers[next_layer_distance]);
                }
                push_flow(a);
                if (excess_flow[i] == 0)
                  break;
              }
            }
          } // for out_edges of i starting from current

          Layer& layer = layers[distance[i]];

          if (ai == ai_end) {   // i must be relabeled
            relabel(i);
            if (dlist_empty(layer.active_vertices)
                && dlist_empty(layer.inactive_vertices))
              gap(distance[i]);
            if (distance[i] == n)
              break;
          } else {              // i is no longer active
            current[i] = ai;
            add_inactive(i, layer);
            break;
          }

        } // while (1)
      } // discharge()

      //=======================================================================
      // This corresponds to the "push" update operation of the paper,
      // not the "push" function in Goldberg's implementation.
      void push_flow(edge_descriptor u_v)
      {
        vertex_descriptor
          u = source(u_v, g),
          v = target(u_v, g);
        
        FlowValue flow_delta
          = std::min(excess_flow[u], residual_capacity[u_v]);

        residual_capacity[u_v] -= flow_delta;
        residual_capacity[reverse_edge[u_v]] += flow_delta;

        excess_flow[u] -= flow_delta;
        excess_flow[v] += flow_delta;
      } // push_flow()

      //=======================================================================
      distance_size_type relabel(vertex_descriptor i)
      {
        distance_size_type min_distance = num_vertices(g);
        distance[i] = min_distance;

        // Examine the residual out-edges of vertex i, choosing the
        // edge whose target vertex has the minimal distance.
        // This is the highest-label or "HL" optimization.
        out_edge_iterator ai, a_end, min_edge_iter;
        for (tie(ai, a_end) = out_edges(i, g); ai != a_end; ++ai) {
          edge_descriptor a = *ai;
          vertex_descriptor j = target(a, g);
          if (residual_capacity[a] > 0 && distance[j] < min_distance) {
              min_distance = distance[j];
              min_edge_iter = ai;
          }
        } // for all out edges of i

        ++min_distance;
        if (min_distance < n) {
          distance[i] = min_distance;     // this is the main action
          current[i] = min_edge_iter;
          max_distance = std::max(min_distance, max_distance);
        } // (min_distance < n)

        return min_distance;
      } // relabel()

      //=======================================================================
      // cleanup beyond the gap
      void gap(distance_size_type empty_distance)
      {
        distance_size_type r; // distance of layer before the current layer
        r = empty_distance - 1;

        for (layer_iterator l = layers.begin() + empty_distance + 1;
             l != layers.begin() + max_distance; ++l) {
          list_node* i;
          for (i = next(l->inactive_vertices);  
               i != l->inactive_vertices; i = next(i))
            distance[i->m_data] = n;
          dlist_clear(l->inactive_vertices);
        }
        max_distance = r;
        max_active = r;
      }

      //=======================================================================
      // remove excessive flow, the "second phase"
      // Basically, this does a DFS on the reverse flow graph of nodes
      //   with excess flow.
      // If a cycle is found, cancel it.
      // Return the nodes with excess flow in topological order.
      //
      // Unlike the prefl_to_flow() implementation, we use
      //   "color" instead of "distance" for the DFS labels
      //   "parent" instead of nl_prev for the DFS tree
      //   "topo_next" instead of nl_next for the topological ordering
      void convert_preflow_to_flow()
      {
        vertex_iterator i_iter, i_end;
        out_edge_iterator ai, a_end;

        vertex_descriptor r, restart, i;

        vertex_descriptor tos, bos;
	bool bos_null = true;

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
	// eliminate flow cycles and topologically order the vertices
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
                if ( capacity[a] == 0 && residual_capacity[a]) {
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
                      residual_capacity[reverse_edge[a]] += delta;
                      j = target(a, g);
                      if (j == i)
                        break;
                    }

                    // back-out of DFS to the first saturated edge
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
                  if (bos_null) {
                    bos = i;
		    bos_null = false;
                    tos = i;
                  } else {
                    topo_next[i] = tos;
                    tos = i;
                  }
                }
                if (i != r) {
                  i = topo_next[i];
                  ++current[i];
                } else
                  break;
              }
            } // while (1)
	    // In case i was changed, adjust i_iter to match
	    i_iter = vertices(g).first + get(index, i);
          } // if (color[i] == white && excess_flow[i] > 0 & ...)
        } // for all vertices in g

	// return excesses
	// note that sink is not on the stack
        if (! bos_null) {
	  for (i = tos; i != bos; i = topo_next[i]) {
            ai = out_edges(i, g).first;
            a = *ai;
            while (excess_flow[i] > 0) {
              if (capacity[a] == 0 && residual_capacity[a] > 0)
                push_flow(a);
              ++ai;
            }
          }
	  // now at the bottom
	  i = bos;
	  a = out_edge(i, g).first;
	  while (excess_flow[i] > 0) {
	    if (capacity[a] == 0 && residual_capacity[a] > 0)
	      push_flow(a);
	    ++ai;
	  }
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
      std::vector< distance_size_type > distance;
      std::vector<default_color_type> color;

      // Edge Property Maps that must be interior to the graph
      SisterMap reverse_edge;
      ResidualCapacityMap residual_capacity;

      LayerArray layers;
      distance_size_type max_distance;	// maximal distance
      distance_size_type max_active;    // maximal distance with active node
      distance_size_type min_active;	// minimal distance with active node
      std::queue<vertex_descriptor> Q;

      inline double global_update_frequency() { return 0.5; }

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

    algo.global_distance_update();
    typename graph_traits<Graph>::vertices_size_type 
      num_relabels = 0, n = num_vertices(g);

    while (algo.max_active >= algo.min_active) {

      Layer& layer = algo.layers[algo.max_active];
      
      list_node* i_node = next(layer.active_vertices);
      if (i_node == layer.active_vertices)
        --algo.max_active;
      else {
        dlist_remove(next(layer.active_vertices), next, prev);
        if (algo.discharge(*i_node)) { // returns true if done with out-edges
          algo.relabel(*i_node);
          ++num_relabels;

          // Gap relabelling heuristic
          if (dlist_empty(layer.active_vertices) && dlist_empty(layer.inactive_vertices))
            algo.gap(layer);

          // Global relabelling heuristic
          if (num_relabels > algo.global_update_frequency() * n) {
            algo.global_distance_update();
            num_relabels = 0;
          }

        } else {
          dlist_remove(i_node, next, prev);
          delete i_node;
        }
      }
      
    } // while (algo.max_active >= algo.min_active)

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
    // layer_list_ptr[v]  points into active_vertices or inactive_vertices
    // current[v]  (value_type == out_edge_iterator)
    // distance[v]    also known as label
    // excess_flow[v]

    // Edge Properties:
    //
    // reverse_edge[(u,v)] => (v,u)
    // residual_capacity[(u,v)]


#if 0
typedef std::list<Vertex> DistanceSet;
typedef std::vector<LabelSet> ArrayOfDistanceSets;

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
