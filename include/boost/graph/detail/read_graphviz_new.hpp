// Copyright 2004-9 Trustees of Indiana University

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//
// read_graphviz_new.hpp - 
//   Initialize a model of the BGL's MutableGraph concept and an associated
//  collection of property maps using a graph expressed in the GraphViz
// DOT Language.  
//
//   Based on the grammar found at:
//   http://www.graphviz.org/cvs/doc/info/lang.html
//
//   Jeremiah rewrite used grammar found at:
//   http://www.graphviz.org/doc/info/lang.html
//   and page 34 or http://www.graphviz.org/pdf/dotguide.pdf
//
//   See documentation for this code at: 
//     http://www.boost.org/libs/graph/doc/read-graphviz.html
//

// Author: Jeremiah Willcock
//         Ronald Garcia
//

#ifndef BOOST_READ_GRAPHVIZ_NEW_HPP
#define BOOST_READ_GRAPHVIZ_NEW_HPP

#include <boost/ref.hpp>
#include <boost/function/function2.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <algorithm>
#include <exception> // for std::exception
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <map>
#include <iostream>
#include <cstdlib>
#include <boost/graph/graphviz.hpp>
#include <boost/throw_exception.hpp>
#include <boost/regex.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace boost {

namespace read_graphviz_detail {
  struct token {
    enum token_type {
      kw_strict,
      kw_graph,
      kw_digraph,
      kw_node,
      kw_edge,
      kw_subgraph,
      left_brace,
      right_brace,
      semicolon,
      equal,
      left_bracket,
      right_bracket,
      comma,
      colon,
      dash_greater,
      dash_dash,
      plus,
      left_paren,
      right_paren,
      at,
      identifier,
      quoted_string, // Only used internally in tokenizer
      eof,
      invalid
    } type;
    std::string normalized_value; // May have double-quotes removed and/or some escapes replaced
    token(token_type type, const std::string& normalized_value)
      : type(type), normalized_value(normalized_value) {}
    token(): type(invalid), normalized_value("") {}
    friend std::ostream& operator<<(std::ostream& o, const token& t) {
      switch (t.type) {
        case token::kw_strict: o << "<strict>"; break;
        case token::kw_graph: o << "<graph>"; break;
        case token::kw_digraph: o << "<digraph>"; break;
        case token::kw_node: o << "<node>"; break;
        case token::kw_edge: o << "<edge>"; break;
        case token::kw_subgraph: o << "<subgraph>"; break;
        case token::left_brace: o << "<left_brace>"; break;
        case token::right_brace: o << "<right_brace>"; break;
        case token::semicolon: o << "<semicolon>"; break;
        case token::equal: o << "<equal>"; break;
        case token::left_bracket: o << "<left_bracket>"; break;
        case token::right_bracket: o << "<right_bracket>"; break;
        case token::comma: o << "<comma>"; break;
        case token::colon: o << "<colon>"; break;
        case token::dash_greater: o << "<dash-greater>"; break;
        case token::dash_dash: o << "<dash-dash>"; break;
        case token::plus: o << "<plus>"; break;
        case token::left_paren: o << "<left_paren>"; break;
        case token::right_paren: o << "<right_paren>"; break;
        case token::at: o << "<at>"; break;
        case token::identifier: o << "<identifier>"; break;
        case token::quoted_string: o << "<quoted_string>"; break;
        case token::eof: o << "<eof>"; break;
        default: o << "<invalid type>"; break;
      }
      o << " '" << t.normalized_value << "'";
      return o;
    }
  };

  struct lex_error: public std::exception {
    std::string errmsg;
    char bad_char;
    lex_error(const std::string& errmsg, char bad_char): errmsg(errmsg), bad_char(bad_char) {
      this->errmsg += std::string(" (char is '") + bad_char + "')";
    }
    const char* what() const throw () {return errmsg.c_str();}
    ~lex_error() throw () {};
  };

  struct parse_error: public std::exception {
    std::string errmsg;
    token bad_token;
    parse_error(const std::string& errmsg, const token& bad_token): errmsg(errmsg), bad_token(bad_token) {
      this->errmsg += std::string(" (token is \"") + boost::lexical_cast<std::string>(bad_token) + "\")";
    }
    const char* what() const throw () {return errmsg.c_str();}
    ~parse_error() throw () {};
  };

  template <typename BidirectionalIterator>
  struct tokenizer {
    BidirectionalIterator begin, end;
    std::vector<token> lookahead;
    // Precomputed regexes
    boost::regex stuff_to_skip;
    boost::regex basic_id_token;
    boost::regex punctuation_token;
    boost::regex number_token;
    boost::regex quoted_string_token;

    tokenizer(BidirectionalIterator begin, BidirectionalIterator end)
      : begin(begin), end(end)
    {
      std::string end_of_token = "(?=(?:\\W))";
      std::string whitespace = "(?:\\s+)";
      std::string slash_slash_comment = "(?://.*$)";
      std::string slash_star_comment = "(?:/\\*.*?\\*/)";
      std::string hash_comment = "(?:^#.*?$)";
      stuff_to_skip = "\\A(?:" + whitespace + "|" + slash_slash_comment + "|" + slash_star_comment + "|" + hash_comment + ")*";
      basic_id_token = "\\A([[:alpha:]_](?:\\w*))";
      punctuation_token = "\\A([][{};=,:+()@]|[-][>-])";
      number_token = "\\A([-]?(?:(?:\\.\\d+)|(?:\\d+(?:\\.\\d*)?)))";
      quoted_string_token = "\\A(\"(?:[^\"\\\\]|(?:[\\\\].))*\")";
    }

    void skip() {
      boost::match_results<BidirectionalIterator> results;
      bool found = boost::regex_search(begin, end, results, stuff_to_skip);
      assert (found);
      boost::sub_match<BidirectionalIterator> sm1 = results.suffix();
      assert (sm1.second == end);
      begin = sm1.first;
    }

    token get_token_raw() {
      if (!lookahead.empty()) {
        token t = lookahead.front();
        lookahead.erase(lookahead.begin());
        return t;
      }
      skip();
      if (begin == end) return token(token::eof, "");
      // Look for keywords first
      bool found;
      boost::match_results<BidirectionalIterator> results;
      found = boost::regex_search(begin, end, results, basic_id_token);
      if (found) {
        std::string str = results[1].str();
        std::string str_lower = boost::algorithm::to_lower_copy(str);
        begin = results.suffix().first;
        if (str_lower == "strict") {
          return token(token::kw_strict, str);
        } else if (str_lower == "graph") {
          return token(token::kw_graph, str);
        } else if (str_lower == "digraph") {
          return token(token::kw_digraph, str);
        } else if (str_lower == "node") {
          return token(token::kw_node, str);
        } else if (str_lower == "edge") {
          return token(token::kw_edge, str);
        } else if (str_lower == "subgraph") {
          return token(token::kw_subgraph, str);
        } else {
          return token(token::identifier, str);
        }
      }
      found = boost::regex_search(begin, end, results, punctuation_token);
      if (found) {
        std::string str = results[1].str();
        begin = results.suffix().first;
        switch (str[0]) {
          case '[': return token(token::left_bracket, str);
          case ']': return token(token::right_bracket, str);
          case '{': return token(token::left_brace, str);
          case '}': return token(token::right_brace, str);
          case ';': return token(token::semicolon, str);
          case '=': return token(token::equal, str);
          case ',': return token(token::comma, str);
          case ':': return token(token::colon, str);
          case '+': return token(token::plus, str);
          case '(': return token(token::left_paren, str);
          case ')': return token(token::right_paren, str);
          case '@': return token(token::at, str);
          case '-': {
            switch (str[1]) {
              case '-': return token(token::dash_dash, str);
              case '>': return token(token::dash_greater, str);
              default: assert (!"Definition of punctuation_token does not match switch statement");
            }
          }
          default: assert (!"Definition of punctuation_token does not match switch statement"); std::abort();
        }
      }
      found = boost::regex_search(begin, end, results, number_token);
      if (found) {
        std::string str = results[1].str();
        begin = results.suffix().first;
        return token(token::identifier, str);
      }
      found = boost::regex_search(begin, end, results, quoted_string_token);
      if (found) {
        std::string str = results[1].str();
        begin = results.suffix().first;
        // Remove the beginning and ending quotes
        assert (str.size() >= 2);
        str.erase(str.begin());
        str.erase(str.end() - 1);
        // Unescape quotes in the middle, but nothing else (see format spec)
        for (size_t i = 0; i + 1 < str.size() /* May change */; ++i) {
          if (str[i] == '\\' && str[i + 1] == '"') {
            str.erase(str.begin() + i);
            // Don't need to adjust i
          } else if (str[i] == '\\' && str[i + 1] == '\n') {
            str.erase(str.begin() + i);
            str.erase(str.begin() + i);
            --i; // Invert ++ that will be applied
          }
        }
        return token(token::quoted_string, str);
      }
      if (*begin == '<') {
        throw_lex_error("HTML strings not supported");
        return token();
      } else {
        throw_lex_error("Invalid character");
        return token();
      }
    }

    token peek_token_raw() {
      if (lookahead.empty()) {
        token t = get_token_raw();
        lookahead.push_back(t);
      }
      return lookahead.front();
    }

    token get_token() { // Handle string concatenation
      token t = get_token_raw();
      if (t.type != token::quoted_string) return t;
      std::string str = t.normalized_value;
      while (peek_token_raw().type == token::plus) {
        get_token_raw();
        token t2 = get_token_raw();
        if (t2.type != token::quoted_string) {
          throw_lex_error("Must have quoted string after string concatenation");
        }
        str += t2.normalized_value;
      }
      return token(token::identifier, str); // Note that quoted_string does not get passed to the parser
    }

    void throw_lex_error(const std::string& errmsg) {
      boost::throw_exception(lex_error(errmsg, *begin));
    }
  };

  typedef std::map<std::string, std::string> properties;

  struct node_id {
    std::string name;
    std::string angle; // Or empty if no angle
    std::vector<std::string> location; // Up to two identifiers
  };

  // Parser policy object should have the following methods:
  // struct parser_policy {
  //   void do_node(const node_id& n, const properties& default_props, const properties& custom_props);
  //   void do_edge(const node_id& a, const node_id& b, const properties& props);
  //   void do_begin_graph(bool is_strict, bool is_directed, const std::string& name);
  //   void do_end_graph(const properties& props);
  //   void do_begin_subgraph(const std::string& name);
  //   void do_end_subgraph(const properties& props);
  // };

  template <typename BidirectionalIterator, typename Policy>
  struct parser {
    tokenizer<BidirectionalIterator> the_tokenizer;
    std::vector<token> lookahead;
    bool graph_is_directed;
    properties graph_props;
    properties node_props;
    properties edge_props;
    Policy hooks;

    parser(BidirectionalIterator begin, BidirectionalIterator end, Policy hooks)
      : the_tokenizer(begin, end), lookahead(), hooks(hooks) {}

    token get() {
      if (lookahead.empty()) {
        token t = the_tokenizer.get_token();
        return t;
      } else {
        token t = lookahead.front();
        lookahead.erase(lookahead.begin());
        return t;
      }
    }

    token peek() {
      if (lookahead.empty()) {
        lookahead.push_back(the_tokenizer.get_token());
      }
      return lookahead.front();
    }

    void error(const std::string& str) {
      boost::throw_exception(parse_error(str, peek()));
    }

    void parse_graph(bool want_directed) {
      bool is_strict = false;
      bool is_directed;
      std::string name;
      if (peek().type == token::kw_strict) {get(); is_strict = true;}
      switch (peek().type) {
        case token::kw_graph: is_directed = false; break;
        case token::kw_digraph: is_directed = true; break;
        default: error("Wanted \"graph\" or \"digraph\"");
      }
      graph_is_directed = is_directed; // Used to check edges
      if (want_directed != graph_is_directed) {
        if (want_directed) {
          boost::throw_exception(boost::undirected_graph_error());
        } else {
          boost::throw_exception(boost::directed_graph_error());
        }
      }
      hooks.do_begin_graph(is_strict, is_directed, name);
      get();
      switch (peek().type) {
        case token::identifier: name = peek().normalized_value; get(); break;
        case token::left_brace: break;
        default: error("Wanted a graph name or left brace");
      }
      if (peek().type == token::left_brace) get(); else error("Wanted a left brace to start the graph");
      parse_stmt_list();
      if (peek().type == token::right_brace) get(); else error("Wanted a right brace to end the graph");
      hooks.do_end_graph(graph_props);
      if (peek().type == token::eof) {} else error("Wanted end of file");
    }

    void parse_stmt_list() {
      while (true) {
        if (peek().type == token::right_brace) return;
        parse_stmt();
        if (peek().type == token::semicolon) get();
      }
    }

    void parse_stmt() {
      switch (peek().type) {
        case token::kw_node:
        case token::kw_edge:
        case token::kw_graph: parse_attr_stmt(); break;
        case token::kw_subgraph:
        case token::left_brace: parse_subgraph(); break;
        case token::identifier: {
          token id = get();
          switch (peek().type) {
            case token::dash_dash:
            case token::dash_greater: {
              node_id n = parse_node_id_rest(id);
              parse_edge_stmt(n);
              break;
            }
            case token::equal: {
              get();
              if (peek().type != token::identifier) error("Wanted identifier as right side of =");
              token id2 = get();
              graph_props[id.normalized_value] = id2.normalized_value;
              break;
            }
            default: {
              node_id n = parse_node_id_rest(id);
              parse_node_stmt(n);
              break;
            }
          }
          break;
        }
        default: error("Invalid start token for statement");
      }
    }

    void parse_attr_stmt() {
      switch (get().type) {
        case token::kw_graph: parse_attr_list(graph_props); break;
        case token::kw_node: parse_attr_list(node_props); break;
        case token::kw_edge: parse_attr_list(edge_props); break;
        default: assert (!"Bad attr_stmt case"); std::abort();
      }
    }

    void parse_subgraph() {
      std::string name;
      if (peek().type == token::kw_subgraph) {
        get();
        if (peek().type == token::identifier) {
          name = get().normalized_value;
        }
      }
      properties saved_node_props = node_props;
      properties saved_edge_props = edge_props;
      properties saved_graph_props = graph_props;
      if (peek().type != token::left_brace) {
        if (name.empty()) error("Subgraph reference needs a name");
        hooks.do_subgraph_reference(name);
        return;
      }
      hooks.do_begin_subgraph(name);
      if (peek().type == token::left_brace) get(); else error("Wanted left brace to start subgraph");
      parse_stmt_list();
      if (peek().type == token::right_brace) get(); else error("Wanted right brace to end subgraph");
      hooks.do_end_subgraph(graph_props);
      node_props = saved_node_props;
      edge_props = saved_edge_props;
      graph_props = saved_graph_props;
      if (peek().type == token::dash_greater ||
          peek().type == token::dash_dash) { // FIXME
        std::cerr << "FIXME: Subgraphs in edges are not supported" << std::endl;
        error("Unsupported subgraph edge case");
      }
    }

    node_id parse_node_id_rest(const token& name) {
      // A node ID is a node name, followed optionally by a port angle and a
      // port location (in either order); a port location is either :id,
      // :id:id, or :(id,id); the last two forms are treated as equivalent
      // although I am not sure about that.
      node_id id;
      id.name = name.normalized_value;
      parse_more:
      switch (peek().type) {
        case token::at: {
          get();
          if (peek().type != token::identifier) error("Wanted identifier as port angle");
          if (id.angle != "") error("Duplicate port angle");
          id.angle = get().normalized_value;
          goto parse_more;
        }
        case token::colon: {
          get();
          if (!id.location.empty()) error("Duplicate port location");
          switch (peek().type) {
            case token::identifier: {
              id.location.push_back(get().normalized_value);
              switch (peek().type) {
                case token::colon: {
                  get();
                  if (peek().type != token::identifier) error("Wanted identifier as port location");
                  id.location.push_back(get().normalized_value);
                  goto parse_more;
                }
                default: goto parse_more;
              }
            }
            case token::left_paren: {
              get();
              if (peek().type != token::identifier) error("Wanted identifier as first element of port location");
              id.location.push_back(get().normalized_value);
              if (peek().type != token::comma) error("Wanted comma between parts of port location");
              get();
              if (peek().type != token::identifier) error("Wanted identifier as second element of port location");
              id.location.push_back(get().normalized_value);
              if (peek().type != token::right_paren) error("Wanted right parenthesis to close port location");
              get();
              goto parse_more;
            }
            default: error("Wanted identifier or left parenthesis as start of port location");
          }
        }
        default: break;
      }
      return id;
    }

    node_id parse_node_id() {
      if (peek().type != token::identifier) error("Wanted identifier as node name (subgraphs not supported yet)");
      token name = get();
      return parse_node_id_rest(name);
    }

    void parse_node_stmt(const node_id& n) {
      properties this_node_props;
      if (peek().type == token::left_bracket) parse_attr_list(this_node_props);
      hooks.do_node(n, node_props, this_node_props);
    }

    void parse_edge_stmt(const node_id& lhs) {
      std::vector<node_id> nodes_in_chain(1, lhs);
      while (true) {
        bool leave_loop = true;
        switch (peek().type) {
          case token::dash_dash: {
            if (graph_is_directed) error("Using -- in directed graph");
            get();
            nodes_in_chain.push_back(parse_node_id());
            leave_loop = false;
            break;
          }
          case token::dash_greater: {
            if (!graph_is_directed) error("Using -> in undirected graph");
            get();
            nodes_in_chain.push_back(parse_node_id());
            leave_loop = false;
            break;
          }
          default: leave_loop = true; break;
        }
        if (leave_loop) break;
      }
      properties this_edge_props = edge_props;
      if (peek().type == token::left_bracket) parse_attr_list(this_edge_props);
      assert (nodes_in_chain.size() >= 2); // Should be in node parser otherwise
      for (size_t i = 0; i + 1 < nodes_in_chain.size(); ++i) {
        hooks.do_edge(nodes_in_chain[i], nodes_in_chain[i + 1], this_edge_props);
      }
    }

    void parse_attr_list(properties& props) {
      while (true) {
        if (peek().type == token::left_bracket) get(); else error("Wanted left bracket to start attribute list");
        while (true) {
          switch (peek().type) {
            case token::right_bracket: break;
            case token::identifier: {
              std::string lhs = get().normalized_value;
              std::string rhs = "true";
              if (peek().type == token::equal) {
                get();
                if (peek().type != token::identifier) error("Wanted identifier as value of attributed");
                rhs = get().normalized_value;
              }
              props[lhs] = rhs;
              break;
            }
            default: error("Wanted identifier as name of attribute");
          }
          if (peek().type == token::comma) {get(); continue;}
          break;
        }
        if (peek().type == token::right_bracket) get(); else error("Wanted right bracket to end attribute list");
        if (peek().type != token::left_bracket) break;
      }
    }
  };

  struct graph_parser_policy {
    ::boost::detail::graph::mutate_graph* mg;

    typedef boost::detail::graph::node_t vertex;
    typedef boost::detail::graph::edge_t edge;

    std::map<std::string, vertex> vertex_map;
    std::map<std::string, edge> edge_map;

    graph_parser_policy(::boost::detail::graph::mutate_graph* mg)
      : mg(mg),
        vertex_map(),
        edge_map()
      {}

    bool node_exists(const node_id& n) const {
      std::map<std::string, vertex>::const_iterator i = vertex_map.find(n.name);
      return (i != vertex_map.end());
    }

    vertex get_or_build_node(const node_id& n) {
      // FIXME: use ports
      std::map<std::string, vertex>::const_iterator i = vertex_map.find(n.name);
      if (i == vertex_map.end()) {
        vertex v = n.name;
        mg->do_add_vertex(v);
        vertex_map.insert(std::make_pair(v, v));
        return v;
      } else {
        return i->second;
      }
    }

    static std::string node_id_to_string(const node_id& n) {
      std::string result = n.name;
      for (size_t i = 0; i < n.location.size(); ++i) {
        result += ":" + n.location[i];
      }
      if (!n.angle.empty()) result += "@" + n.angle;
      return result;
    }

    static std::string props_to_string(const properties& props) {
      std::string result = "[";
      for (properties::const_iterator i = props.begin(); i != props.end(); ++i) {
        if (i != props.begin()) result += ", ";
        result += i->first + "=" + i->second;
      }
      result += "]";
      return result;
    }

    void do_node(const node_id& n, const properties& def_props, const properties& new_props) {
      std::cerr << node_id_to_string(n) << " " << props_to_string(def_props) << " " << props_to_string(new_props) << std::endl;
      properties props_to_set = new_props;
      if (!node_exists(n)) { // Only use defaults if node is new
        props_to_set.insert(def_props.begin(), def_props.end()); // This keeps old properties in preference to ones from def_props
      }
      vertex v = get_or_build_node(n);
      for (properties::const_iterator i = props_to_set.begin(); i != props_to_set.end(); ++i) {
        mg->set_node_property(i->first, v, i->second);
      }
    }

    void do_edge(const node_id& a, const node_id& b, const properties& props) {
      std::cerr << node_id_to_string(a) << " -> " << node_id_to_string(b) << " " << props_to_string(props) << std::endl;
      edge e = edge::new_edge();
      mg->do_add_edge(e, get_or_build_node(a), get_or_build_node(b));
      for (properties::const_iterator i = props.begin(); i != props.end(); ++i) {
        mg->set_edge_property(i->first, e, i->second);
      }
    }

    void do_begin_graph(bool is_strict, bool is_directed, const std::string& name) {
      // std::cerr << "starting" << (is_strict ? " strict" : "") << " " << (is_directed ? "directed" : "undirected") << " graph named " << name << std::endl;
    }

    void do_end_graph(const properties& props) {
      std::cerr << "ending graph " << props_to_string(props) << std::endl;
      for (properties::const_iterator i = props.begin(); i != props.end(); ++i) {
        mg->set_graph_property(i->first, i->second);
      }
    }
      
    void do_subgraph_reference(const std::string& name) {
      // std::cerr << "subgraph reference to " << name << std::endl;
    }

    void do_begin_subgraph(const std::string& name) {
      // std::cerr << "starting subgraph named " << name << std::endl;
    }

    void do_end_subgraph(const properties& props) {
      // std::cerr << "ending subgraph " << props_to_string(props) << std::endl;
    }
  };

} // namespace read_graphviz_detail

template <typename BidirectionalIterator, typename MutableGraph>
bool read_graphviz(BidirectionalIterator begin, BidirectionalIterator end,
                   MutableGraph& graph, boost::dynamic_properties& dp,
                   std::string const& node_id = "node_id") {
  boost::detail::graph::mutate_graph_impl<MutableGraph> mg(graph, dp, node_id);
  read_graphviz_detail::graph_parser_policy policy(&mg);
  read_graphviz_detail::parser<BidirectionalIterator, read_graphviz_detail::graph_parser_policy> p(begin, end, policy);
  p.parse_graph(mg.is_directed());
  return true;
}

} // namespace boost

#endif // BOOST_READ_GRAPHVIZ_NEW_HPP
