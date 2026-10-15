// Copyright (C) 2006  Tiago de Paula Peixoto <tiago@forked.de>
// Copyright (C) 2004,2009  The Trustees of Indiana University.
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Jeremiah Willcock
//           Andrew Lumsdaine
//           Tiago de Paula Peixoto

#define BOOST_GRAPH_SOURCE
#include <boost/throw_exception.hpp>
#include <boost/graph/graphml.hpp>
#include <boost/graph/dll_import_export.hpp>
#include <boost/graph/detail/rapidxml.hpp>
#include <iterator>
#include <map>
#include <string>
#include <vector>

using namespace boost;

namespace
{

namespace rapidxml = boost::graph::detail::rapidxml;
using xml_node = rapidxml::xml_node< char >;
using xml_attribute = rapidxml::xml_attribute< char >;
using xml_document = rapidxml::xml_document< char >;

// Value of an attribute, or a fallback when it is absent.
std::string attr_value(const xml_node* node, const char* name, const std::string& fallback)
{
    if (const xml_attribute* a = node->first_attribute(name))
        return std::string(a->value(), a->value_size());
    return fallback;
}

// Value of an attribute that must be present; throws parse_error otherwise.
std::string require_attr(const xml_node* node, const char* name)
{
    if (const xml_attribute* a = node->first_attribute(name))
        return std::string(a->value(), a->value_size());
    BOOST_THROW_EXCEPTION(parse_error(std::string("missing required attribute: ") + name));
}

// Text content of an element.
std::string node_text(const xml_node* node)
{
    return std::string(node->value(), node->value_size());
}

class graphml_reader
{
public:
    graphml_reader(mutate_graph& g) : m_g(g) {}

    void get_graphs(const xml_node* top,
        size_t desired_idx /* or -1 for all */, bool is_root,
        std::vector< const xml_node* >& result)
    {
        size_t current_idx = 0;
        bool is_first = is_root;
        for (const xml_node* n = top->first_node("graph"); n; n = n->next_sibling("graph"))
        {
            if (current_idx == desired_idx || desired_idx == (size_t)(-1))
            {
                result.push_back(n);
                if (is_first)
                {
                    is_first = false;
                    for (const xml_node* data = n->first_node("data"); data; data = data->next_sibling("data"))
                    {
                        std::string key = require_attr(data, "key");
                        std::string value = node_text(data);
                        handle_graph_property(key, value);
                    }
                }

                get_graphs(n, (size_t)(-1), false, result);
                if (desired_idx != (size_t)(-1))
                    break;
            }
            ++current_idx;
        }
    }

    void run(std::istream& in, size_t desired_idx)
    {
        // rapidxml parses in place, so load the whole stream into a buffer
        // that outlives the document, and zero-terminate it.
        std::vector< char > buffer(
            (std::istreambuf_iterator< char >(in)),
            std::istreambuf_iterator< char >());
        buffer.push_back('\0');

        xml_document doc;
        try
        {
            // no_comments | trim_whitespace, as the old property_tree call used.
            constexpr int flags = rapidxml::parse_normalize_whitespace | rapidxml::parse_trim_whitespace;
            doc.parse< flags >(&buffer[0]);
        }
        catch (const rapidxml::parse_error& e)
        {
            BOOST_THROW_EXCEPTION(parse_error(e.what()));
        }

        const xml_node* gml = doc.first_node("graphml");
        if (!gml)
            BOOST_THROW_EXCEPTION(parse_error("no graphml element found"));

        // Search for attributes
        for (const xml_node* child = gml->first_node("key"); child; child = child->next_sibling("key"))
        {
            std::string id = attr_value(child, "id", "");
            std::string for_ = attr_value(child, "for", "");
            std::string name = attr_value(child, "attr.name", "");
            std::string type = attr_value(child, "attr.type", "");
            key_kind kind = all_key;
            if (for_ == "graph")
                kind = graph_key;
            else if (for_ == "node")
                kind = node_key;
            else if (for_ == "edge")
                kind = edge_key;
            else if (for_ == "hyperedge")
                kind = hyperedge_key;
            else if (for_ == "port")
                kind = port_key;
            else if (for_ == "endpoint")
                kind = endpoint_key;
            else if (for_ == "all")
                kind = all_key;
            else if (for_ == "graphml")
                kind = graphml_key;
            else
            {
                BOOST_THROW_EXCEPTION(
                    parse_error("Attribute for is not valid: " + for_));
            }
            m_keys[id] = kind;
            m_key_name[id] = name;
            m_key_type[id] = type;
            if (const xml_node* default_ = child->first_node("default"))
                m_key_default[id] = node_text(default_);
        }
        // Search for graphs
        std::vector< const xml_node* > graphs;
        handle_graph();
        get_graphs(gml, desired_idx, true, graphs);
        for (const xml_node* gr : graphs)
        {
            // Search for nodes
            for (const xml_node* node = gr->first_node("node"); node; node = node->next_sibling("node"))
            {
                std::string id = require_attr(node, "id");
                handle_vertex(id);
                for (const xml_node* data = node->first_node("data"); data; data = data->next_sibling("data"))
                {
                    std::string key = require_attr(data, "key");
                    std::string value = node_text(data);
                    handle_node_property(key, id, value);
                }
            }
        }
        for (const xml_node* gr : graphs)
        {
            bool default_directed
                = require_attr(gr, "edgedefault") == "directed";
            // Search for edges
            for (const xml_node* edge = gr->first_node("edge"); edge; edge = edge->next_sibling("edge"))
            {
                std::string source = require_attr(edge, "source");
                std::string target = require_attr(edge, "target");
                std::string local_directed = attr_value(edge, "directed", "");
                bool is_directed
                    = (local_directed.empty() ? default_directed
                                              : local_directed == "true");
                if (is_directed != m_g.is_directed())
                {
                    if (is_directed)
                    {
                        BOOST_THROW_EXCEPTION(directed_graph_error());
                    }
                    else
                    {
                        BOOST_THROW_EXCEPTION(undirected_graph_error());
                    }
                }
                size_t old_edges_size = m_edge.size();
                handle_edge(source, target);
                for (const xml_node* data = edge->first_node("data"); data; data = data->next_sibling("data"))
                {
                    std::string key = require_attr(data, "key");
                    std::string value = node_text(data);
                    handle_edge_property(key, old_edges_size, value);
                }
            }
        }
    }

private:
    /// The kinds of keys. Not all of these are supported
    enum key_kind
    {
        graph_key,
        node_key,
        edge_key,
        hyperedge_key,
        port_key,
        endpoint_key,
        all_key,
        graphml_key
    };

    void handle_vertex(const std::string& v)
    {
        bool is_new = false;

        if (m_vertex.find(v) == m_vertex.end())
        {
            m_vertex[v] = m_g.do_add_vertex();
            is_new = true;
        }

        if (is_new)
        {
            std::map< std::string, std::string >::iterator iter;
            for (iter = m_key_default.begin(); iter != m_key_default.end();
                 ++iter)
            {
                if (m_keys[iter->first] == node_key)
                    handle_node_property(iter->first, v, iter->second);
            }
        }
    }

    any get_vertex_descriptor(const std::string& v) { return m_vertex[v]; }

    void handle_edge(const std::string& u, const std::string& v)
    {
        handle_vertex(u);
        handle_vertex(v);

        any source, target;
        source = get_vertex_descriptor(u);
        target = get_vertex_descriptor(v);

        any edge;
        bool added;
        boost::tie(edge, added) = m_g.do_add_edge(source, target);
        if (!added)
        {
            BOOST_THROW_EXCEPTION(bad_parallel_edge(u, v));
        }

        size_t e = m_edge.size();
        m_edge.push_back(edge);

        std::map< std::string, std::string >::iterator iter;
        for (iter = m_key_default.begin(); iter != m_key_default.end(); ++iter)
        {
            if (m_keys[iter->first] == edge_key)
                handle_edge_property(iter->first, e, iter->second);
        }
    }

    void handle_graph()
    {
        std::map< std::string, std::string >::iterator iter;
        for (iter = m_key_default.begin(); iter != m_key_default.end(); ++iter)
        {
            if (m_keys[iter->first] == graph_key)
                handle_graph_property(iter->first, iter->second);
        }
    }

    void handle_graph_property(
        const std::string& key_id, const std::string& value)
    {
        m_g.set_graph_property(m_key_name[key_id], value, m_key_type[key_id]);
    }

    void handle_node_property(const std::string& key_id,
        const std::string& descriptor, const std::string& value)
    {
        m_g.set_vertex_property(m_key_name[key_id], m_vertex[descriptor], value,
            m_key_type[key_id]);
    }

    void handle_edge_property(
        const std::string& key_id, size_t descriptor, const std::string& value)
    {
        m_g.set_edge_property(
            m_key_name[key_id], m_edge[descriptor], value, m_key_type[key_id]);
    }

    mutate_graph& m_g;
    std::map< std::string, key_kind > m_keys;
    std::map< std::string, std::string > m_key_name;
    std::map< std::string, std::string > m_key_type;
    std::map< std::string, std::string > m_key_default;
    std::map< std::string, any > m_vertex;
    std::vector< any > m_edge;
};

}

namespace boost
{
void BOOST_GRAPH_DECL read_graphml(
    std::istream& in, mutate_graph& g, size_t desired_idx)
{
    graphml_reader reader(g);
    reader.run(in, desired_idx);
}
}
