//=======================================================================
// Copyright (c) 2019 Yi Ji
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//=======================================================================

#define BOOST_TEST_MODULE link_cut_tree_test

#include <boost/graph/link_cut_tree.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <map>
#include <deque>
#include <numeric>
#include <unordered_map>

using namespace boost;

std::size_t lowest_common_ancestor(std::size_t N, std::size_t u, std::size_t w)
{
    const std::size_t size = std::max(u, w) + 1;
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
void test_link_cut_tree(LinkCutTree lct, const std::vector<Element> &elements)
{
    BOOST_FOREACH(const Element &ele, elements)
    {
        lct.make_tree(ele);
        BOOST_CHECK(lct.find_root(ele) == ele);
    }
    
    for (int i = 0; i < elements.size() - 1; ++i)
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
    
    for (int N = 2; N < 7; ++N)
    {
        for (unsigned long i = elements.size() - 1; i > 0; --i)
        {
            unsigned long i_parent = (i - 1) / N;
            lct.link(elements[i], elements[i_parent]);
            BOOST_CHECK(lct.lowest_common_ancestor(elements[i], elements[i_parent]) ==
                        elements[i_parent]);
            std::deque<unsigned long> queue;
            queue.push_back(i);
            while (!queue.empty())
            {
                unsigned long idx = queue.front();
                queue.pop_front();
                BOOST_CHECK(lct.find_root(elements[idx]) == elements[i_parent]);
                unsigned long idx_child = (idx + 1) * N;
                if (idx_child < elements.size())
                {
                    for (int i = 0; i < N; ++i)
                    {
                        queue.push_back(idx_child-i);
                    }
                }
            }
        }
        for (int i = 0; i < elements.size(); ++i)
        {
            BOOST_CHECK(lct.find_root(elements[i]) == elements[0]);
            for (int j = 0; j < elements.size(); ++j)
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

BOOST_AUTO_TEST_CASE(link_cut_tree_test1)
{
    typedef associative_property_map< std::map<int, int> > map_t;
    typedef link_cut_tree<map_t> link_cut_tree_t;
    std::vector<int> elements(100);
    std::iota(std::begin(elements), std::end(elements), -49);
    std::map<int, int> parent_map, left_map, right_map;
    map_t parent(parent_map), left(left_map), right(right_map);
    link_cut_tree_t lct(parent, left, right);
    test_link_cut_tree<link_cut_tree_t, int>(lct, elements);
}

BOOST_AUTO_TEST_CASE(link_cut_tree_test2)
{
    typedef associative_property_map< std::map<char, char> > map_t;
    typedef associative_property_map< std::unordered_map<char, char> > unordered_map_t;
    typedef link_cut_tree<map_t, unordered_map_t> link_cut_tree_t;
    std::vector<char> elements(20);
    std::iota(std::begin(elements), std::end(elements), 'a');
    std::map<char, char> parent_map;
    std::unordered_map<char, char> left_map, right_map;
    map_t parent(parent_map);
    unordered_map_t left(left_map), right(right_map);
    link_cut_tree_t lct(parent, left, right);
    test_link_cut_tree<link_cut_tree_t, char>(lct, elements);
}

BOOST_AUTO_TEST_CASE(link_cut_tree_with_storage_test1)
{
    typedef typed_identity_property_map<std::string> map_t;
    typedef link_cut_tree_with_storage<map_t> link_cut_tree_t;
    std::vector<std::string> elements;
    std::vector<int> numbers(100);
    std::iota(std::begin(numbers), std::end(numbers), -49);
    std::transform(std::begin(numbers),
                   std::end(numbers),
                   std::back_inserter(elements),
                   boost::bind(lexical_cast<std::string, int>, _1));
    link_cut_tree_t lct;
    test_link_cut_tree<link_cut_tree_t, std::string>(lct, elements);
}

BOOST_AUTO_TEST_CASE(link_cut_tree_with_storage_test2)
{
    typedef associative_property_map< std::map<std::string, int> > map_t;
    typedef link_cut_tree_with_storage<map_t> link_cut_tree_t;
    std::vector<std::string> elements;
    std::vector<int> numbers(100);
    std::map<std::string, int> id_map;
    map_t id(id_map);
    std::iota(std::begin(numbers), std::end(numbers), -49);
    BOOST_FOREACH(int i, numbers)
    {
        std::string i_str = std::to_string(i);
        elements.push_back(i_str);
        put(id, i_str, i);
    }
    link_cut_tree_t lct(id);
    test_link_cut_tree<link_cut_tree_t, std::string>(lct, elements);
}
