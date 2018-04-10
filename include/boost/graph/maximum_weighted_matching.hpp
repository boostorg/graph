//=======================================================================
// Copyright (c) 2018 Yi Ji
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//=======================================================================

#ifndef BOOST_GRAPH_MAXIMUM_WEIGHTED_MATCHING_HPP
#define BOOST_GRAPH_MAXIMUM_WEIGHTED_MATCHING_HPP

#include <algorithm> // for std::iter_swap
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/graph/max_cardinality_matching.hpp>

namespace boost
{
    template <typename Graph, typename MateMap, typename VertexIndexMap>
    typename property_traits<typename property_map<Graph, edge_weight_t>::type>::value_type
    matching_weight_sum(const Graph& g, MateMap mate, VertexIndexMap vm)
    {
        typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator_t;
        typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor_t;
        typedef typename property_traits<typename property_map<Graph, edge_weight_t>::type>::value_type edge_property_t;
        
        edge_property_t weight_sum = 0;
        vertex_iterator_t vi, vi_end;
        
        for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
        {
            vertex_descriptor_t v = *vi;
            if (get(mate, v) != graph_traits<Graph>::null_vertex() && get(vm, v) < get(vm, get(mate,v)))
                weight_sum += get(edge_weight, g, edge(v,mate[v],g).first);
        }
        return weight_sum;
    }
    
    
    template <typename Graph, typename MateMap>
    inline typename property_traits<typename property_map<Graph, edge_weight_t>::type>::value_type
    matching_weight_sum(const Graph& g, MateMap mate)
    {
        return matching_weight_sum(g, mate, get(vertex_index,g));
    }
    
    template <typename Graph, typename MateMap, typename VertexIndexMap>
    class weighted_augmenting_path_finder
    {
        
    public:
        
        template <typename T>
        struct map_vertex_to_
        {
            typedef boost::iterator_property_map<typename std::vector<T>::iterator, VertexIndexMap> type;
        };
        typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator_t;
        typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor_t;
        typedef typename graph_traits<Graph>::out_edge_iterator out_edge_iterator_t;
        typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor_t;
        typedef typename graph_traits<Graph>::edges_size_type e_size_t;
        typedef typename graph_traits<Graph>::vertices_size_type v_size_t;
        typedef typename graph_traits<Graph>::edge_iterator edge_iterator_t;
        typedef typename property_traits<typename property_map<Graph, edge_weight_t>::type>::value_type edge_property_t;
        typedef typename std::deque<vertex_descriptor_t> vertex_list_t;
        typedef typename std::vector<edge_descriptor_t> edge_list_t;
        typedef typename map_vertex_to_<vertex_descriptor_t>::type vertex_to_vertex_map_t;
        typedef typename map_vertex_to_<e_size_t>::type vertex_to_esize_map_t;
        typedef typename map_vertex_to_<edge_property_t>::type vertex_to_weight_map_t;
        typedef typename map_vertex_to_<bool>::type vertex_to_bool_map_t;
        typedef typename map_vertex_to_<int>::type vertex_to_int_map_t;
        typedef typename map_vertex_to_<std::pair<vertex_descriptor_t, vertex_descriptor_t> >::type vertex_to_pair_map_t;
        
        class blossom
        {
        public:
            
            typedef boost::shared_ptr<blossom> blossom_ptr_t;
            std::vector<blossom_ptr_t> sub_blossoms;
            edge_property_t dual_var = 0;
            blossom_ptr_t father = blossom_ptr_t();
            
            // get the base vertex of a blossom by recursively getting
            // its base sub-blossom, which is always the first one in
            // sub_blossoms because of how we create and maintain blossoms
            virtual vertex_descriptor_t get_base()
            {
                blossom* b = this;
                while (!b->sub_blossoms.empty())
                    b = b->sub_blossoms[0].get();
                return b->get_base();
            }
            
            // set a sub-blossom as a blossom's base by exchanging it
            // with its first sub-blossom
            void set_base(const blossom_ptr_t& sub)
            {
                for (blossom_iterator_t bi = sub_blossoms.begin(); bi != sub_blossoms.end(); ++bi)
                {
                    if (sub.get() == bi->get())
                    {
                        std::iter_swap(sub_blossoms.begin(), bi);
                        break;
                    }
                }
            }
            
            // get all vertices inside recursively
            virtual std::vector<vertex_descriptor_t> vertices()
            {
                std::vector<vertex_descriptor_t> all_vertices;
                for (blossom_iterator_t bi = sub_blossoms.begin(); bi != sub_blossoms.end(); ++bi)
                {
                    std::vector<vertex_descriptor_t> some_vertices = (*bi)->vertices();
                    all_vertices.insert(all_vertices.end(), some_vertices.begin(), some_vertices.end());
                }
                return all_vertices;
            }
        };
        
        // a trivial_blossom only has one vertex and no sub-blossom;
        // for each vertex v, in_blossom[v] is the trivial_blossom that
        // contains it directly
        class trivial_blossom : public blossom
        {
        public:
            trivial_blossom(vertex_descriptor_t v) : trivial_vertex(v) {}
            virtual vertex_descriptor_t get_base()
            {
                return trivial_vertex;
            }
            
            virtual std::vector<vertex_descriptor_t> vertices()
            {
                std::vector<vertex_descriptor_t> all_vertices;
                all_vertices.push_back(trivial_vertex);
                return all_vertices;
            }
            
        private:
            vertex_descriptor_t trivial_vertex;
        };
        
        typedef boost::shared_ptr<blossom> blossom_ptr_t;
        typedef typename std::vector<blossom_ptr_t>::iterator blossom_iterator_t;
        typedef typename map_vertex_to_<blossom_ptr_t>::type vertex_to_blossom_map_t;
        
        weighted_augmenting_path_finder(const Graph& arg_g, MateMap arg_mate, VertexIndexMap arg_vm) :
        g(arg_g),
        vm(arg_vm),
        mate_vector(num_vertices(g)),
        label_S_vector(num_vertices(g), graph_traits<Graph>::null_vertex()),
        label_T_vector(num_vertices(g), graph_traits<Graph>::null_vertex()),
        outlet_vector(num_vertices(g)),
        in_blossom_vector(num_vertices(g)),
        old_label_vector(num_vertices(g)),
        dual_var_vector(std::vector<edge_property_t>(num_vertices(g), std::numeric_limits<edge_property_t>::min())),
        dual_var(dual_var_vector.begin(), vm),
        mate(mate_vector.begin(), vm),
        label_S(label_S_vector.begin(), vm),
        label_T(label_T_vector.begin(), vm),
        outlet(outlet_vector.begin(), vm),
        in_blossom(in_blossom_vector.begin(), vm),
        old_label(old_label_vector.begin(), vm)
        {
            vertex_iterator_t vi, vi_end;
            edge_iterator_t ei, ei_end;
            
            edge_property_t max_weight = std::numeric_limits<edge_property_t>::min();
            for (boost::tie(ei,ei_end) = edges(g); ei != ei_end; ++ei)
                max_weight = std::max(max_weight, get(edge_weight, g, *ei));
            
            for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
            {
                vertex_descriptor_t u = *vi;
                mate[u] = get(arg_mate, u);
                dual_var[u] = 2*max_weight;
                in_blossom[u] = boost::make_shared<trivial_blossom>(u);
                outlet[u] = u;
            }
        }
        
        // return the top blossom where v is contained inside
        blossom_ptr_t in_top_blossom(vertex_descriptor_t v)
        {
            blossom_ptr_t b = in_blossom[v];
            while (b->father != blossom_ptr_t())
                b = b->father;
            return b;
        }
        
        // check if vertex v is in blossom b
        bool is_in_blossom(blossom_ptr_t b, vertex_descriptor_t v)
        {
            if (v == graph_traits<Graph>::null_vertex())
                return false;
            blossom_ptr_t vb = in_blossom[v]->father;
            while (vb != blossom_ptr_t())
            {
                if (vb.get() == b.get())
                    return true;
                vb = vb->father;
            }
            return false;
        }
        
        // return the base vertex of the top blossom that contains v
        inline vertex_descriptor_t base_vertex(vertex_descriptor_t v)
        {
            return in_top_blossom(v)->get_base();
        }
        
        // add an existed top blossom of base vertex v into new top
        // blossom b as its sub-blossom
        void add_sub_blossom(blossom_ptr_t b, vertex_descriptor_t v)
        {
            blossom_ptr_t sub = in_top_blossom(v);
            sub->father = b;
            b->sub_blossoms.push_back(sub);
            if (sub->sub_blossoms.empty())
                return;
            for (blossom_iterator_t bi = top_blossoms.begin(); bi != top_blossoms.end(); ++bi)
            {
                if (bi->get() == sub.get())
                {
                    top_blossoms.erase(bi);
                    break;
                }
            }
        }
        
        // when a top blossom is created or its base vertex getting an S-label,
        // add all edges incident to this blossom into even_edges
        void bloom(blossom_ptr_t b)
        {
            std::vector<vertex_descriptor_t> vertices_of_b = b->vertices();
            typename std::vector<vertex_descriptor_t>::iterator vi;
            for (vi = vertices_of_b.begin(); vi != vertices_of_b.end(); ++vi)
            {
                out_edge_iterator_t oei, oei_end;
                for (boost::tie(oei, oei_end) = out_edges(*vi, g); oei != oei_end; ++oei)
                {
                    if (target(*oei,g) != *vi)
                        even_edges.push_back(*oei);
                }
            }
        }
        
        // expand a top blossom, put all its non-trivial sub-blossoms into top_blossoms
        blossom_iterator_t expand_blossom(blossom_iterator_t bi, std::vector<blossom_ptr_t>& new_ones)
        {
            blossom_ptr_t b = *bi;
            for (blossom_iterator_t i = b->sub_blossoms.begin(); i != b->sub_blossoms.end(); ++i)
            {
                blossom_ptr_t sub_blossom = *i;
                sub_blossom->father = blossom_ptr_t();
                // new top blossoms cannot be pushed back into top_blossoms immediately,
                // because push_back() may cause reallocation and then invalid iterators
                if (!sub_blossom->sub_blossoms.empty())
                    new_ones.push_back(sub_blossom);
            }
            return top_blossoms.erase(bi);
        }
        
        // when vertices v and w are matched to each other by augmenting,
        // we must set v/w as base vertex of any blossom who contains v/w and
        // is a sub-blossom of their lowest (smallest) common blossom
        void adjust_blossom(vertex_descriptor_t v, vertex_descriptor_t w)
        {
            blossom_ptr_t vb = in_blossom[v], wb = in_blossom[w], lowest_common_blossom;
            std::vector<blossom_ptr_t> v_ancestors, w_ancestors;
            
            while (vb->father != blossom_ptr_t())
            {
                v_ancestors.push_back(vb->father);
                vb = vb->father;
            }
            while (wb->father != blossom_ptr_t())
            {
                w_ancestors.push_back(wb->father);
                wb = wb->father;
            }
            
            typename std::vector<blossom_ptr_t>::reverse_iterator i, j;
            i = v_ancestors.rbegin();
            j = w_ancestors.rbegin();
            while (i != v_ancestors.rend() && j != w_ancestors.rend() && i->get() == j->get())
            {
                lowest_common_blossom = *i;
                ++i;++j;
            }
            
            vb = in_blossom[v];
            wb = in_blossom[w];
            while (vb->father != lowest_common_blossom)
            {
                vb->father->set_base(vb);
                vb = vb->father;
            }
            while (wb->father != lowest_common_blossom)
            {
                wb->father->set_base(wb);
                wb = wb->father;
            }
        }
        
        // every edge weight is multiplied by 4 to ensure integer weights
        // throughout the algorithm if all input weights are integers
        edge_property_t slack(const edge_descriptor_t& e)
        {
            vertex_descriptor_t v, w;
            v = source(e, g);
            w = target(e, g);
            return dual_var[v] + dual_var[w] - 4*get(edge_weight, g, e);
        }
        
        // backtrace one step on vertex v along the augmenting path
        // by its labels and its vertex state;
        // boolean parameter "relabeling" means whether we are in relabeling process,
        // if we are, then we use old labels to backtrace and also we
        // don't jump to its base vertex when we reach an odd vertex
        std::pair<vertex_descriptor_t, int> parent(std::pair<vertex_descriptor_t, int> v,
                                                   bool relabeling = false)
        {
            if (v.second == graph::detail::V_EVEN)
                return std::make_pair(label_S[v.first], graph::detail::V_ODD);
            else if (v.second == graph::detail::V_ODD)
            {
                vertex_descriptor_t w = relabeling ? old_label[v.first].first : base_vertex(label_T[v.first]);
                return std::make_pair(w, graph::detail::V_EVEN);
            }
            return std::make_pair(v.first, graph::detail::V_UNREACHED);
        }
        
        // backtrace from vertices v and w to their free (unmatched) ancesters,
        // return the nearest common ancestor (null_vertex if none) of v and w
        vertex_descriptor_t nearest_common_ancestor(vertex_descriptor_t v, vertex_descriptor_t w,
                                                    vertex_descriptor_t& v_free_ancestor,
                                                    vertex_descriptor_t& w_free_ancestor)
        {
            std::pair<vertex_descriptor_t, int> v_up(v, graph::detail::V_EVEN);
            std::pair<vertex_descriptor_t, int> w_up(w, graph::detail::V_EVEN);
            vertex_descriptor_t nca;
            nca = w_free_ancestor = v_free_ancestor = graph_traits<Graph>::null_vertex();
            
            std::vector<bool> ancestor_of_w_vector(num_vertices(g), false);
            std::vector<bool> ancestor_of_v_vector(num_vertices(g), false);
            vertex_to_bool_map_t ancestor_of_w(ancestor_of_w_vector.begin(), vm);
            vertex_to_bool_map_t ancestor_of_v(ancestor_of_v_vector.begin(), vm);
            
            while (nca == graph_traits<Graph>::null_vertex() &&
                   (v_free_ancestor == graph_traits<Graph>::null_vertex() ||
                    w_free_ancestor == graph_traits<Graph>::null_vertex()
                    )
                   )
            {
                ancestor_of_w[w_up.first] = true;
                ancestor_of_v[v_up.first] = true;
                
                if (w_free_ancestor == graph_traits<Graph>::null_vertex())
                    w_up = parent(w_up);
                if (v_free_ancestor == graph_traits<Graph>::null_vertex())
                    v_up = parent(v_up);
                
                if (mate[v_up.first] == graph_traits<Graph>::null_vertex())
                    v_free_ancestor = v_up.first;
                if (mate[w_up.first] == graph_traits<Graph>::null_vertex())
                    w_free_ancestor = w_up.first;
                
                if (ancestor_of_w[v_up.first] == true)
                    nca = v_up.first;
                else if (ancestor_of_v[w_up.first] == true)
                    nca = w_up.first;
                else if (v_free_ancestor == w_free_ancestor &&
                         v_free_ancestor != graph_traits<Graph>::null_vertex())
                    nca = v_up.first;
            }
            
            return nca;
        }
        
        // when a new top blossom b is created by connecting (v, w), we add sub-blossoms into
        // b along backtracing from v_prime and w_prime to stop_vertex (the base vertex);
        // also, we set labels and outlet for each base vertex we pass by
        void make_blossom(blossom_ptr_t b, vertex_descriptor_t w_prime,
                          vertex_descriptor_t v_prime, vertex_descriptor_t stop_vertex)
        {
            std::pair<vertex_descriptor_t, int> u(v_prime, graph::detail::V_ODD);
            std::pair<vertex_descriptor_t, int> u_up(w_prime, graph::detail::V_EVEN);
            
            for (; u_up.first != stop_vertex; u = u_up, u_up = parent(u))
            {
                if (u_up.second == graph::detail::V_EVEN)
                {
                    if (!in_top_blossom(u_up.first)->sub_blossoms.empty())
                        outlet[u_up.first] = label_T[u.first];
                    label_T[u_up.first] = outlet[u.first];
                }
                else if (u_up.second == graph::detail::V_ODD)
                    label_S[u_up.first] = u.first;
                
                add_sub_blossom(b, u_up.first);
            }
        }
        
        // the design of recursively expanding augmenting path in (reversed_)retrieve_augmenting_path
        // functions is inspired by same functions in max_cardinality_matching.hpp;
        // except that in weighted matching, we use "outlet" vertices instead of "bridge" vertex pairs:
        // if blossom b is the smallest non-trivial blossom that contains its base vertex v, then
        // v and outlet[v] are where augmenting path enters and leaves b
        void retrieve_augmenting_path(vertex_descriptor_t v, vertex_descriptor_t w, int v_state)
        {
            if (v == w)
                aug_path.push_back(v);
            else if (v_state == graph::detail::V_EVEN)
            {
                aug_path.push_back(v);
                retrieve_augmenting_path(label_S[v], w, graph::detail::V_ODD);
            }
            else if (v_state == graph::detail::V_ODD)
            {
                if (outlet[v] == v)
                    aug_path.push_back(v);
                else
                    reversed_retrieve_augmenting_path(outlet[v], v, graph::detail::V_EVEN);
                retrieve_augmenting_path(label_T[v], w, graph::detail::V_EVEN);
            }
        }
        
        
        void reversed_retrieve_augmenting_path(vertex_descriptor_t v, vertex_descriptor_t w, int v_state)
        {
            if (v == w)
                aug_path.push_back(v);
            else if (v_state == graph::detail::V_EVEN)
            {
                reversed_retrieve_augmenting_path(label_S[v], w, graph::detail::V_ODD);
                aug_path.push_back(v);
            }
            else if (v_state == graph::detail::V_ODD)
            {
                reversed_retrieve_augmenting_path(label_T[v], w, graph::detail::V_EVEN);
                if (outlet[v] != v)
                    retrieve_augmenting_path(outlet[v], v, graph::detail::V_EVEN);
                else
                    aug_path.push_back(v);
            }
        }
        
        // correct labels for vertices in the augmenting path
        void relabel(vertex_descriptor_t v)
        {
            blossom_ptr_t b = in_blossom[v]->father;
            
            if (!is_in_blossom(b, mate[v]))
            { // if v is a new base vertex
                std::pair<vertex_descriptor_t, int> u(v, graph::detail::V_EVEN);
                while (label_S[u.first] != u.first && is_in_blossom(b, label_S[u.first]))
                    u = parent(u, true);
                
                vertex_descriptor_t old_base = u.first;
                if (label_S[old_base] != old_base)
                { // if old base is not exposed
                    label_T[v] = label_S[old_base];
                    outlet[v] = old_base;
                }
                else
                { // if old base is exposed then new label_T[v] is not in b,
                  // we must (i) make b2 the smallest blossom containing v but not as base vertex
                  // (ii) backtrace from b2's new base vertex to b
                    label_T[v] = graph_traits<Graph>::null_vertex();
                    for (b = b->father; b != blossom_ptr_t() && b->get_base() == v; b = b->father);
                    if (b != blossom_ptr_t())
                    {
                        u = std::make_pair(b->get_base(), graph::detail::V_ODD);
                        while (!is_in_blossom(in_blossom[v]->father, old_label[u.first].first))
                            u = parent(u, true);
                        label_T[v] = u.first;
                        outlet[v] = old_label[u.first].first;
                    }
                }
            }
            else if (label_S[v] == v || !is_in_blossom(b, label_S[v]))
            { // if v is an old base vertex
                // let u be the new base vertex; backtrace from u's old T-label
                std::pair<vertex_descriptor_t, int> u(b->get_base(), graph::detail::V_ODD);
                while (old_label[u.first].first != graph_traits<Graph>::null_vertex() &&
                       old_label[u.first].first != v)
                    u = parent(u, true);
                label_T[v] = old_label[u.first].second;
                outlet[v] = v;
            }
            else // if v is neither a new nor an old base vertex
                label_T[v] = label_S[v];
        }
        
        bool augment_matching()
        {
            even_edges.clear();
            
            vertex_iterator_t vi, vi_end;
            
            for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
            {
                vertex_descriptor_t u = *vi;
                out_edge_iterator_t ei, ei_end;
                
                if (base_vertex(u) != u)
                    continue;
                
                label_S[u] = label_T[u] = graph_traits<Graph>::null_vertex();
                outlet[u] = u;
                
                if (mate[u] == graph_traits<Graph>::null_vertex())
                {
                    label_S[u] = u;
                    bloom(in_top_blossom(u));
                }
            }
            
            delta1 = std::numeric_limits<edge_property_t>::max();
            
            // end initializations
            
            vertex_descriptor_t v, w, w_free_ancestor, v_free_ancestor;
            v = w = w_free_ancestor = v_free_ancestor = graph_traits<Graph>::null_vertex();
            bool found_alternating_path = false;
            
            // note that we only use edges of zero slack value for augmenting
            while (!even_edges.empty() && !found_alternating_path)
            {
                // search for augmenting paths depth-first
                edge_descriptor_t current_edge = even_edges.back();
                even_edges.pop_back();
                
                v = source(current_edge, g);
                w = target(current_edge, g);
                
                vertex_descriptor_t v_prime = base_vertex(v);
                vertex_descriptor_t w_prime = base_vertex(w);
                
                // a paranoid check
                if (label_S[v_prime] == graph_traits<Graph>::null_vertex())
                {
                    std::swap(v_prime, w_prime);
                    std::swap(v, w);
                }
                
                if (label_S[w_prime] == graph_traits<Graph>::null_vertex() &&
                    label_T[w_prime] == graph_traits<Graph>::null_vertex())
                {
                    if (slack(current_edge) > 0)
                    {
                        delta1 = std::min(delta1, slack(current_edge));
                        continue;
                    }
                    label_T[w_prime] = v;
                    outlet[w_prime] = w;
                    vertex_descriptor_t w_prime_mate = mate[w_prime];
                    label_S[w_prime_mate] = w_prime;
                    bloom(in_top_blossom(w_prime_mate));
                }
                
                // w_prime == v_prime implies that we get an edge that has been shrunk into a blossom
                else if (label_S[w_prime] != graph_traits<Graph>::null_vertex() && w_prime != v_prime)
                {
                    if (slack(current_edge) > 0)
                    {
                        delta1 = std::min(delta1, slack(current_edge)/2);
                        continue;
                    }
                    
                    // if nca is null_vertex then we have an augmenting path; otherwise we have
                    // a new top blossom with nca as its base vertex
                    vertex_descriptor_t nca = nearest_common_ancestor(v_prime, w_prime,
                                                                      v_free_ancestor, w_free_ancestor);
                    
                    if (nca == graph_traits<Graph>::null_vertex())
                        found_alternating_path = true; //to break out of the loop
                    else
                    {
                        // create a new blossom and set labels for vertices inside
                        
                        blossom_ptr_t b = boost::make_shared<blossom>();
                        add_sub_blossom(b, nca);
                        
                        label_T[w_prime] = v;
                        label_T[v_prime] = w;
                        outlet[w_prime] = w;
                        outlet[v_prime] = v;
                        
                        make_blossom(b, w_prime, v_prime, nca);
                        make_blossom(b, v_prime, w_prime, nca);
                        
                        label_T[nca] = graph_traits<Graph>::null_vertex();
                        outlet[nca] = nca;
                        
                        top_blossoms.push_back(b);
                        bloom(b);
                    }
                }
            }
            
            if (!found_alternating_path)
                return false;
            
            // retrieve the augmenting path and put it in aug_path
            reversed_retrieve_augmenting_path(v, v_free_ancestor, graph::detail::V_EVEN);
            retrieve_augmenting_path(w, w_free_ancestor, graph::detail::V_EVEN);
            
            // augment the matching along aug_path
            vertex_descriptor_t a, b;
            vertex_list_t reversed_aug_path;
            while (!aug_path.empty())
            {
                a = aug_path.front();
                aug_path.pop_front();
                reversed_aug_path.push_back(a);
                b = aug_path.front();
                aug_path.pop_front();
                reversed_aug_path.push_back(b);
                
                mate[a] = b;
                mate[b] = a;
                
                // reset base vertex for every blossom in augment path
                adjust_blossom(a, b);
            }
            
            for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
                old_label[*vi] = std::make_pair(label_T[*vi], outlet[*vi]);
            
            // correct labels for in-blossom vertices along aug_path
            while (!reversed_aug_path.empty())
            {
                a = reversed_aug_path.front();
                reversed_aug_path.pop_front();
                
                if (in_blossom[a]->father != blossom_ptr_t())
                    relabel(a);
            }
            
            for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
            {
                vertex_descriptor_t u = *vi;
                if (mate[u] != graph_traits<Graph>::null_vertex())
                    label_S[u] = mate[u];
            }
            
            // expand blossoms with zero dual variables
            std::vector<blossom_ptr_t> new_top_blossoms;
            for (blossom_iterator_t bi = top_blossoms.begin(); bi != top_blossoms.end();)
            {
                if ((*bi)->dual_var <= 0)
                    bi = expand_blossom(bi, new_top_blossoms);
                else
                    ++bi;
            }
            top_blossoms.insert(top_blossoms.end(), new_top_blossoms.begin(), new_top_blossoms.end());
            return true;
        }
        
        // slack the vertex and blossom dual variables when there is no augmenting path found
        // according to the primal-dual method
        bool adjust_dual_var()
        {
            delta2 = delta3 = std::numeric_limits<edge_property_t>::max();
            
            vertex_iterator_t vi, vi_end;
            for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
                delta2 = std::min(delta2, dual_var[*vi]);
            
            for (blossom_iterator_t bi = top_blossoms.begin(); bi != top_blossoms.end(); ++bi)
            {
                const blossom_ptr_t& b = *bi;
                if (label_T[b->get_base()] != graph_traits<Graph>::null_vertex())
                    delta3 = std::min(delta3, b->dual_var/2);
            }
            
            edge_property_t delta_min = std::min(std::min(delta2, delta3), delta1);
            
            for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
            {
                vertex_descriptor_t v = *vi, v_prime = base_vertex(v);
                if (label_S[v_prime] != graph_traits<Graph>::null_vertex())
                    dual_var[v] -= delta_min;
                else if (label_T[v_prime] != graph_traits<Graph>::null_vertex())
                    dual_var[v] += delta_min;
            }
            
            for (blossom_iterator_t bi = top_blossoms.begin(); bi != top_blossoms.end(); ++bi)
            {
                blossom_ptr_t b = *bi;
                vertex_descriptor_t b_base = b->get_base();
                if (label_T[b_base] != graph_traits<Graph>::null_vertex())
                    b->dual_var -= 2*delta_min;
                else if (label_S[b_base] != graph_traits<Graph>::null_vertex())
                    b->dual_var += 2*delta_min;
            }
            
            if (delta1 == delta_min)
                return true;
            
            if (delta3 == delta_min)
            {
                // expand odd blossoms with zero dual variables
                std::vector<blossom_ptr_t> new_top_blossoms;
                for (blossom_iterator_t bi = top_blossoms.begin(); bi != top_blossoms.end();)
                {
                    const blossom_ptr_t& b = *bi;
                    if (b->dual_var <= 0 && label_T[b->get_base()] != graph_traits<Graph>::null_vertex())
                        bi = expand_blossom(bi, new_top_blossoms);
                    else
                        ++bi;
                }
                top_blossoms.insert(top_blossoms.end(), new_top_blossoms.begin(), new_top_blossoms.end());
                return true;
            }
            
            return false;
        }
        
        template <typename PropertyMap>
        void get_current_matching(PropertyMap pm)
        {
            vertex_iterator_t vi, vi_end;
            for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
                put(pm, *vi, mate[*vi]);
        }
        
    private:
        
        const Graph& g;
        VertexIndexMap vm;
        
        // storage for the property maps below
        std::vector<vertex_descriptor_t> mate_vector;
        std::vector<vertex_descriptor_t> label_S_vector;
        std::vector<vertex_descriptor_t> label_T_vector;
        std::vector<vertex_descriptor_t> outlet_vector;
        std::vector<edge_property_t> dual_var_vector;
        std::vector<blossom_ptr_t> in_blossom_vector;
        std::vector<std::pair<vertex_descriptor_t, vertex_descriptor_t> > old_label_vector;
        
        // iterator property maps
        vertex_to_vertex_map_t mate;
        vertex_to_vertex_map_t label_S; // v has an S-label -> v can be an even vertex, label_S[v] is its mate
        vertex_to_vertex_map_t label_T; // v has a T-label -> v can be an odd vertex, label_T[v] is its predecessor in aug_path
        vertex_to_vertex_map_t outlet;
        vertex_to_weight_map_t dual_var;
        vertex_to_blossom_map_t in_blossom; // map any vertex v to the trivial blossom containing v
        vertex_to_pair_map_t old_label; // <old T-label, old outlet> before relabel
        
        vertex_list_t aug_path;
        edge_list_t even_edges;
        std::vector<blossom_ptr_t> top_blossoms;
        edge_property_t delta1, delta2, delta3;
        
    };
    
	template <typename Graph, typename MateMap, typename VertexIndexMap>
	void maximum_weighted_matching(const Graph& g, MateMap mate, VertexIndexMap vm)
	{
        empty_matching<Graph, MateMap>::find_matching(g, mate);
        weighted_augmenting_path_finder<Graph, MateMap, VertexIndexMap> augmentor(g, mate, vm);
        
        // can have |V| times augmenting at most
        for (int t = 0; t < num_vertices(g); ++t)
        {
            bool augmented = false;
            while (!augmented)
            {
                augmented = augmentor.augment_matching();
                if (!augmented)
                {
                    if (!augmentor.adjust_dual_var())
                        break;
                }
            }
            if (!augmented)
                break;
        }
    
        augmentor.get_current_matching(mate);
    }

	template <typename Graph, typename MateMap>
	inline void maximum_weighted_matching(const Graph& g, MateMap mate)
	{
        maximum_weighted_matching(g, mate, get(vertex_index,g));
	}
    
    // brute-force matcher searches all possible combinations of matched edges to get the maximum weighted matching
    // which can be used for testing on small graphs (within dozens vertices)
    template <typename Graph, typename MateMap, typename VertexIndexMap>
    class brute_force_matching
    {
    public:
        
        typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor_t;
        typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator_t;
        typedef typename graph_traits<Graph>::edge_iterator edge_iterator_t;
        typedef boost::iterator_property_map<typename std::vector<vertex_descriptor_t>::iterator, VertexIndexMap> vertex_to_vertex_map_t;
        
        brute_force_matching(const Graph& arg_g, MateMap arg_mate, VertexIndexMap arg_vm) :
        g(arg_g),
        vm(arg_vm),
        mate_vector(num_vertices(g)),
        best_mate_vector(num_vertices(g)),
        mate(mate_vector.begin(), vm),
        best_mate(best_mate_vector.begin(), vm)
        {
            vertex_iterator_t vi,vi_end;
            for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
                best_mate[*vi] = mate[*vi] = get(arg_mate, *vi);
        }
        
        template <typename PropertyMap>
        void find_matching(PropertyMap pm)
        {
            edge_iterator_t ei;
            boost::tie(ei, ei_end) = edges(g);
            select_edge(ei);
            
            vertex_iterator_t vi,vi_end;
            for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
                put(pm, *vi, best_mate[*vi]);
        }
        
    private:
        
        const Graph& g;
        VertexIndexMap vm;
        std::vector<vertex_descriptor_t> mate_vector, best_mate_vector;
        vertex_to_vertex_map_t mate, best_mate;
        edge_iterator_t ei_end;
        
        void select_edge(edge_iterator_t ei)
        {
            if (ei == ei_end)
            {
                if (matching_weight_sum(g, mate) > matching_weight_sum(g, best_mate))
                {
                    vertex_iterator_t vi, vi_end;
                    for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
                        best_mate[*vi] = mate[*vi];
                }
                return;
            }
            
            vertex_descriptor_t v, w;
            v = source(*ei, g);
            w = target(*ei, g);
            
            select_edge(++ei);
            
            if (mate[v] == graph_traits<Graph>::null_vertex() &&
                mate[w] == graph_traits<Graph>::null_vertex())
            {
                mate[v] = w;
                mate[w] = v;
                select_edge(ei);
                mate[v] = mate[w] = graph_traits<Graph>::null_vertex();
            }
        }
        
    };
    
    template <typename Graph, typename MateMap, typename VertexIndexMap>
    void brute_force_maximum_weighted_matching(const Graph& g, MateMap mate, VertexIndexMap vm)
    {
        empty_matching<Graph, MateMap>::find_matching(g, mate);
        brute_force_matching<Graph, MateMap, VertexIndexMap> brute_force_matcher(g, mate, vm);
        brute_force_matcher.find_matching(mate);
    }

    template <typename Graph, typename MateMap>
    inline void brute_force_maximum_weighted_matching(const Graph& g, MateMap mate)
    {
        brute_force_maximum_weighted_matching(g, mate, get(vertex_index, g));
    }
    
}

#endif
