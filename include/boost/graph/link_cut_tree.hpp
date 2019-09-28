//
//=======================================================================
// Copyright 2019
// Author: Yi Ji
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
#ifndef BOOST_LINK_CUT_TREE_HPP
#define BOOST_LINK_CUT_TREE_HPP

#include <boost/graph/properties.hpp>
#include <unordered_map>

namespace boost
{
    template<class ElementParentMap, class ElementChildMap = ElementParentMap>
    class link_cut_tree
    {
    public:
        inline link_cut_tree(ElementParentMap p, ElementChildMap l, ElementChildMap r) : parent(p), left(l), right(r) {}
        
        inline link_cut_tree(const link_cut_tree<ElementParentMap, ElementChildMap> &c)
            : parent(c.parent), left(c.left), right(c.right) {}

        template <class Element>
        inline void make_tree(Element x)
        {
            make_path(x);
        }

        template <class Element>
        inline Element find_root(Element x)
        {
            return find_tail(expose(x));
        }

        template <class Element>
        inline void link(Element x, Element y) // Element x must be a tree root
        {
            Element r = expose(x);
            r = join(r, r, expose(y));
            put_successor(r, r);
        }

        template <class Element>
        inline void cut(Element x)
        {
            expose(x);
            std::pair<Element, Element> uv = split(x);
            put_successor(x, x);
            put_successor(uv.second, uv.second);
        }

        template <class Element>
        inline Element lowest_common_ancestor(Element x, Element y) // Elements x and y must have same root
        {
            expose(x);
            return expose(y);
        }

    private:
        ElementParentMap parent;
        ElementChildMap left, right;

        template <class Element>
        inline Element get_parent(Element x) const
        {
            Element x_parent = get(parent, x);
            if (get(left, x_parent) == x || get(right, x_parent) == x)
                return x_parent;
            return x; // x_parent is actually x_successor when x has no parent
        }

        template <class Element>
        inline Element get_successor(Element x) const
        {
            return get(parent, x);
        }

        template <class Element>
        inline void put_successor(Element x, Element x_successor)
        {
            put(parent, x, x_successor);
        }

        template <class Element>
        inline void rotate(const Element x, const ElementChildMap &side)
        {
            const ElementChildMap &opposite = (&side == &left) ? right : left;
            const Element pivot = get(side, x);
            const Element x_parent = get_parent(x);
            const Element pivot_side = get(opposite, pivot);

            if (x_parent != x)
            {
                put(parent, pivot, x_parent);
                put(get_side(x), x_parent, pivot);
            }
            else
            {
                Element x_successor = get_successor(x);
                put_successor(pivot, x == x_successor ? pivot : x_successor);
            }

            if (pivot_side != pivot)
            {
                put(side, x, pivot_side);
                put(parent, pivot_side, x);
            }
            else
                put(side, x, x);
            
            put(opposite, pivot, x);
            put(parent, x, pivot);
        }

        template <class Element>
        inline ElementChildMap& get_side(Element x)
        {
            Element x_parent = get_parent(x);
            if (get(left, x_parent) == x)
                return left;
            return right;
        }

        template <class Element>
        inline void splay(Element x)
        {
            for (Element x_parent = get_parent(x); x != x_parent; x_parent = get_parent(x))
            {
                const Element x_grandparent = get_parent(x_parent);
                const ElementChildMap &x_side = get_side(x);
                const ElementChildMap &x_parent_side = get_side(x_parent);

                if (x_grandparent == x_parent)
                    rotate(x_parent, x_side);
                else if (&x_side == &x_parent_side)
                {
                    rotate(x_grandparent, x_parent_side);
                    rotate(x_parent, x_side);
                }
                else
                {
                    rotate(x_parent, x_side);
                    rotate(x_grandparent, x_parent_side);
                }
            }
        }

        template <class Element>
        inline Element expose(Element x)
        {
            Element r = x;
            while (true)
            {
                const Element x_successor = get_successor(find_path(x));
                const std::pair<Element, Element> uv = split(x);
                if (x != uv.first)
                    put_successor(uv.first, x);
                r = join(r, x, uv.second);
                if (x == x_successor)
                    break;
                x = x_successor;
            }
            put_successor(r, r);
            return r;
        }

        template <class Element>
        inline void make_path(Element x)
        {
            put(parent, x, x);
            put(right, x, x);
            put(left, x, x);
        }

        template <class Element>
        inline Element find_path(Element x)
        {
            splay(x);
            return x;
        }

        template <class Element>
        inline Element find_tail(Element x)
        {
            while (get(right, x) != x)
                x = get(right, x);
            splay(x);
            return x;
        }

        template <class Element>
        inline Element join(Element u, Element v, Element w)
        {
            if (u != v)
                put(parent, u, v);
            if (w != v)
                put(parent, w, v);
            put(left, v, u);
            put(right, v, w);
            return v;
        }

        template <class Element>
        inline std::pair<Element, Element> split(Element x)
        {
            splay(x);
            Element x_left = get(left, x);
            Element x_right = get(right, x);
            if (x_left != x)
                put(parent, x_left, x_left);
            if (x_right != x)
                put(parent, x_right, x_right);
            put(left, x, x);
            put(right, x, x);
            return std::make_pair<>(x_left, x_right);
        }
    };
    
    
    template <class ID = identity_property_map>
    class link_cut_tree_with_storage :
    public link_cut_tree<associative_property_map<std::unordered_map<typename property_traits<ID>::value_type, typename property_traits<ID>::value_type> > >
    {
    public:
        typedef typename property_traits<ID>::key_type Vertex;
        typedef typename property_traits<ID>::value_type Index;
        typedef std::unordered_map<Index, Index> IndexMapContainer;
        typedef associative_property_map<IndexMapContainer> IndexMap;
        typedef link_cut_tree<IndexMap> LCT;
        
        link_cut_tree_with_storage(ID id_ = ID()) :
        LCT(IndexMap(parent_map), IndexMap(left_map), IndexMap(right_map)),
        id(id_) {}
        
        template <class Vertex>
        inline void make_tree(Vertex x)
        {
            const Index x_id = get(id, x);
            LCT::make_tree(x_id);
            id_to_vertex[x_id] = x;
        }
        
        template <class Vertex>
        inline Vertex find_root(Vertex x)
        {
            return id_to_vertex[LCT::find_root(get(id, x))];
        }
        
        template <class Vertex>
        inline void link(Vertex x, Vertex y)
        {
            LCT::link(get(id, x), get(id, y));
        }
        
        template <class Vertex>
        inline void cut(Vertex x)
        {
            LCT::cut(get(id, x));
        }
        
        template <class Vertex>
        inline Vertex lowest_common_ancestor(Vertex x, Vertex y)
        {
            return id_to_vertex[LCT::lowest_common_ancestor(get(id, x), get(id, y))];
        }
        
    private:
        ID id;
        std::unordered_map<Index, Vertex> id_to_vertex;
        IndexMapContainer parent_map, left_map, right_map;
    };
}

#endif // BOOST_LINK_CUT_TREE_HPP
