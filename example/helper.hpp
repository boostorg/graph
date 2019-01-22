// (C) Copyright Andrew Sutton 2007
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_EXAMPLE_HELPER_HPP
#define BOOST_GRAPH_EXAMPLE_HELPER_HPP

#include <string>
#include <sstream>
#include <map>
#include <algorithm>

#include <boost/graph/properties.hpp>

template <typename Graph, typename NameMap, typename VertexMap>
typename boost::graph_traits<Graph>::vertex_descriptor
add_named_vertex(Graph& g, NameMap nm, const std::string& name, VertexMap& vm)
{
    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;

    Vertex v;
    auto [iter, inserted] = vm.insert(make_pair(name, Vertex()));
    if(inserted) {
        // The name was unique so we need to add a vertex to the graph
        v = add_vertex(g);
        iter->second = v;
        put(nm, v, name);      // store the name in the name map
    }
    else {
        // We had alread inserted this name so we can return the
        // associated vertex.
        v = iter->second;
    }
    return v;
}

template <typename Graph, typename NameMap, typename InputStream>
inline std::map<std::string, typename boost::graph_traits<Graph>::vertex_descriptor>
read_graph(Graph& g, NameMap nm, InputStream& is)
{
    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
    std::map<std::string, Vertex> verts;
    for(std::string line; std::getline(is, line); ) {
        if(line.empty()) continue;
        auto index = line.find_first_of(',');
        std::string first(line, 0, index);
        std::string second(line, index + 1);

        auto u = add_named_vertex(g, nm, first, verts);
        auto v = add_named_vertex(g, nm, second, verts);
        add_edge(u, v, g);
    }
    return verts;
}

template <typename Graph, typename InputStream>
inline std::map<std::string, typename boost::graph_traits<Graph>::vertex_descriptor>
read_graph(Graph& g, InputStream& is)
{
    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
    using NameMap = boost::null_property_map<Vertex, std::string>;
    return read_graph(g, NameMap(), is);
}

template <typename Graph, typename NameMap, typename WeightMap, typename InputStream>
inline std::map<std::string, typename boost::graph_traits<Graph>::vertex_descriptor>
read_weighted_graph(Graph& g, NameMap nm, WeightMap wm, InputStream& is)
{
    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
    std::map<std::string, Vertex> verts;
    for(std::string line; std::getline(is, line); ) {
        if(line.empty()) continue;
        auto i = line.find_first_of(',');
        auto j = line.find_first_of(',', i + 1);
        std::string first(line, 0, i);
        std::string second(line, i + 1, j - i - 1);
        std::string prob(line, j + 1);

        // convert the probability to a float
        std::stringstream ss(prob);
        float p;
        ss >> p;

        // add the vertices to the graph
        auto u = add_named_vertex(g, nm, first, verts);
        auto v = add_named_vertex(g, nm, second, verts);

        // add the edge and set the weight
        auto e = add_edge(u, v, g).first;
        put(wm, e, p);
    }
    return verts;
}


template <typename Graph, typename WeightMap, typename InputStream>
inline std::map<std::string, typename boost::graph_traits<Graph>::vertex_descriptor>
read_weighted_graph(Graph& g, WeightMap wm, InputStream& is)
{
    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
    using NameMap = boost::null_property_map<Vertex, std::string>;

    return read_weighted_graph(g, NameMap(), wm, is);
}


#endif
