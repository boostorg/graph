/* tarjan_offline_lca.cpp source file
 *
 * Copyright 2004 Cromwell D. Enage.  Covered by the Boost Software License,
 * Version 1.0.  (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

/*
 * Defines the std::ios class and std::cout, its global output instance.
 */
#include <iostream>

/*
 * Defines the boost::numeric::ublas::matrix class template.
 */
#include <boost/numeric/ublas/matrix.hpp>

/*
 * Defines the boost::print_graph function template.
 */
#include <boost/graph/graph_utility.hpp>

/*
 * Defines the boost::adjacency_list class template and its associated nonmember
 * function templates.
 */
#include <boost/graph/adjacency_list.hpp>

/*
 * Defines the boost::tarjan_offline_lca function template.
 */
#include <boost/graph/tarjan_offline_lca.hpp>

int main()
{
    typedef boost::adjacency_list<> Graph;

    const int vertex_count = 15;

    Graph g(vertex_count);

    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 3, g);
    boost::add_edge(1, 4, g);
    boost::add_edge(2, 5, g);
    boost::add_edge(2, 6, g);
    boost::add_edge(3, 7, g);
    boost::add_edge(3, 8, g);
    boost::add_edge(4, 9, g);
    boost::add_edge(4, 10, g);
    boost::add_edge(5, 11, g);
    boost::add_edge(5, 12, g);
    boost::add_edge(6, 13, g);
    boost::add_edge(6, 14, g);

    std::cout << "Input graph:" << std::endl;
    boost::print_graph(g);
    std::cout << std::endl;

    boost::numeric::ublas::matrix<unsigned int>
        ancestor_matrix(vertex_count, vertex_count);

    boost::tarjan_offline_lca(g, 0, ancestor_matrix);

    for (int i = 0; i < vertex_count; ++i)
    {
        for (int j = 0; j < vertex_count; ++j)
        {
            std::cout << "Least common ancestor of " << i << " and " << j;
            std::cout << " is " << ancestor_matrix(i, j) << std::endl;
        }
    }

    return 0;
}

