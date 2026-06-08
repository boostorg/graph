//=======================================================================
// Copyright (c) 2019 Yi Ji
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//=======================================================================

#define BOOST_TEST_MODULE link_cut_trees_test

#include <boost/graph/link_cut_trees.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <deque>
#include <algorithm>
#include <iterator>

using namespace boost;

std::size_t lowest_common_ancestor(std::size_t N, std::size_t u, std::size_t w)
{
    const std::size_t size = std::max<std::size_t>(u, w) + 1;
    std::vector<bool> ancester_of_u(size, false), ancester_of_w(size, false);
    while (u > 0 || w > 0)
    {
        ancester_of_u[u] = ancester_of_w[w] = true;
        if (ancester_of_u[w])
        {
            return w;
        }
        if (ancester_of_w[u])
        {
            return u;
        }
        u = u > 0 ? (u - 1) / N : 0;
        w = w > 0 ? (w - 1) / N : 0;
    }
    return 0;
}

template<typename LinkCutTree, typename Element>
void test_link_cut_trees(LinkCutTree lct, const std::vector<Element> &elements)
{
    BOOST_FOREACH(const Element &ele, elements)
    {
        lct.make_tree(ele);
        BOOST_CHECK(lct.find_root(ele) == ele);
    }
    
    for (std::size_t i = 0; i < elements.size() - 1; ++i)
    {
        lct.link(elements[i+1], elements[i]);
        BOOST_CHECK(lct.find_root(elements[i]) == elements[0]);
        BOOST_CHECK(lct.find_root(elements[i+1]) == elements[0]);
        BOOST_CHECK(lct.lowest_common_ancestor(elements[i+1], elements[i]) == elements[i]);
    }
    
    BOOST_FOREACH(const Element &ele, elements)
    {
        lct.cut(ele);
        BOOST_CHECK(lct.find_root(ele) == ele);
    }
    
    for (std::size_t N = 2; N < 7; ++N)
    {
        for (std::size_t i = elements.size() - 1; i > 0; --i)
        {
            std::size_t i_parent = (i - 1) / N;
            lct.link(elements[i], elements[i_parent]);
            BOOST_CHECK(lct.lowest_common_ancestor(elements[i], elements[i_parent]) ==
                        elements[i_parent]);
            std::deque<std::size_t> queue;
            queue.push_back(i);
            while (!queue.empty())
            {
                std::size_t idx = queue.front();
                queue.pop_front();
                BOOST_CHECK(lct.find_root(elements[idx]) == elements[i_parent]);
                std::size_t idx_child = (idx + 1) * N;
                if (idx_child < elements.size())
                {
                    for (std::size_t i = 0; i < N; ++i)
                    {
                        queue.push_back(idx_child-i);
                    }
                }
            }
        }
        for (std::size_t i = 0; i < elements.size(); ++i)
        {
            BOOST_CHECK(lct.find_root(elements[i]) == elements[0]);
            for (std::size_t j = 0; j < elements.size(); ++j)
            {
                BOOST_CHECK(lct.lowest_common_ancestor(elements[i], elements[j]) ==
                            elements[lowest_common_ancestor(N, i, j)]);
            }
        }
        BOOST_FOREACH(const Element &ele, adaptors::reverse(elements))
        {
            lct.cut(ele);
            BOOST_CHECK(lct.find_root(ele) == ele);
        }
    }
}

BOOST_AUTO_TEST_CASE(link_cut_trees_test1)
{
    typedef associative_property_map< std::map<int, int> > map_t;
    typedef link_cut_trees<map_t> link_cut_trees_t;
    std::vector<int> elements(100);
    boost::range::iota(elements, -49);
    std::map<int, int> parent_map, left_map, right_map;
    map_t parent(parent_map), left(left_map), right(right_map);
    link_cut_trees_t lct(parent, left, right);
    test_link_cut_trees<link_cut_trees_t, int>(lct, elements);
}

BOOST_AUTO_TEST_CASE(link_cut_trees_test2)
{
    typedef associative_property_map< std::map<char, char> > map_t;
    typedef associative_property_map< boost::unordered_map<char, char> > unordered_map_t;
    typedef link_cut_trees<map_t, unordered_map_t> link_cut_trees_t;
    std::vector<char> elements(20);
    boost::range::iota(elements, 'a');
    std::map<char, char> parent_map;
    boost::unordered_map<char, char> left_map, right_map;
    map_t parent(parent_map);
    unordered_map_t left(left_map), right(right_map);
    link_cut_trees_t lct(parent, left, right);
    test_link_cut_trees<link_cut_trees_t, char>(lct, elements);
}

BOOST_AUTO_TEST_CASE(link_cut_trees_test3)
{
    typedef typed_identity_property_map<std::string> map_t;
    typedef link_cut_trees_with_storage<map_t> link_cut_trees_t;
    std::vector<std::string> elements;
    std::vector<int> numbers(100);
    boost::range::iota(numbers, -49);
    boost::range::transform(numbers, std::back_inserter(elements), boost::bind(lexical_cast<std::string, int>, _1));
    link_cut_trees_t lct;
    test_link_cut_trees<link_cut_trees_t, std::string>(lct, elements);
}

BOOST_AUTO_TEST_CASE(link_cut_trees_test4)
{
    typedef associative_property_map< std::map<std::string, int> > map_t;
    typedef link_cut_trees_with_storage<map_t> link_cut_trees_t;
    std::vector<std::string> elements;
    std::vector<int> numbers(100);
    std::map<std::string, int> id_map;
    map_t id(id_map);
    boost::range::iota(numbers, -49);
    BOOST_FOREACH(int i, numbers)
    {
        std::string i_str = lexical_cast<std::string, int>(i);
        elements.push_back(i_str);
        put(id, i_str, i);
    }
    link_cut_trees_t lct(id);
    test_link_cut_trees<link_cut_trees_t, std::string>(lct, elements);
}

BOOST_AUTO_TEST_CASE(link_cut_trees_test5)
{
    typedef associative_property_map< std::map<std::string, int> > id_map_t;
    typedef vector_property_map<std::string> inverse_id_map_t;
    typedef link_cut_trees_with_storage<id_map_t, inverse_id_map_t> link_cut_trees_t;
    std::vector<std::string> elements;
    std::vector<int> numbers(100);
    std::map<std::string, int> id_map;
    id_map_t id(id_map);
    inverse_id_map_t inverse_id;
    boost::range::iota(numbers, -49);
    BOOST_FOREACH(int i, numbers)
    {
        std::string i_str = lexical_cast<std::string, int>(i);
        elements.push_back(i_str);
        put(id, i_str, i+49);
        put(inverse_id, i+49, i_str);
    }
    link_cut_trees_t lct(id, inverse_id);
    test_link_cut_trees<link_cut_trees_t, std::string>(lct, elements);
}

BOOST_AUTO_TEST_CASE(link_cut_trees_test6)
{
    typedef associative_property_map< std::map<std::string, int> > id_map_t;
    typedef vector_property_map<std::string> inverse_id_map_t;
    typedef std::map<int, int> index_map_container_t;
    typedef link_cut_trees_with_storage<id_map_t, inverse_id_map_t, index_map_container_t> link_cut_trees_t;
    std::vector<std::string> elements;
    std::vector<int> numbers(100);
    std::map<std::string, int> id_map;
    id_map_t id(id_map);
    inverse_id_map_t inverse_id;
    boost::range::iota(numbers, -49);
    BOOST_FOREACH(int i, numbers)
    {
        std::string i_str = lexical_cast<std::string, int>(i);
        elements.push_back(i_str);
        put(id, i_str, i+49);
        put(inverse_id, i+49, i_str);
    }
    link_cut_trees_t lct(id, inverse_id);
    test_link_cut_trees<link_cut_trees_t, std::string>(lct, elements);
}