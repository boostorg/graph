// Copyright Michael Drexl 2005, 2006.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://boost.org/LICENSE_1_0.txt)

#include <boost/config.hpp>

#ifdef BOOST_MSVC
#pragma warning(disable : 4267)
#endif

#include <boost/graph/adjacency_list.hpp>

#include <boost/graph/r_c_shortest_paths.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <boost/core/lightweight_test.hpp>

namespace
{
///
/// @brief Structure representig the vertices, containing the id exposition and name
/// and x,y for representation
///
struct vertex_t
{
    size_t id;
    size_t x;
    size_t y;
    size_t movedReq;
    double expo;
    std::string name;
};

///
/// @brief Structure for edge with an id and two vertex_t extremities
///
struct edge_t
{
    size_t id;
    std::vector<vertex_t> extremities;

};

///
/// @brief Definition of some typedef.
///
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, vertex_t, edge_t> graph_t;
typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_desc;
typedef boost::graph_traits<graph_t>::edge_descriptor edge_desc;
typedef std::unordered_map<size_t, std::unordered_map<size_t, vertex_desc> > vertex_map_t;

///
/// @brief Function creating the vertices.
///
vertex_map_t create_vertices(graph_t &g, std::vector<std::vector<size_t> > const &pos_p)
{
    vertex_map_t vertex_map;

    size_t idx_l = 0;
    // Read all positions
    for(size_t i = 0 ; i < pos_p.size() ; ++ i)
    {
        for(size_t j = 0 ; j < pos_p[i].size() ; ++ j)
        {
            // Create a vertex based on the index in the vectors
            vertex_t cur_vertex;
            cur_vertex.id = idx_l++;
            cur_vertex.x = i;
            cur_vertex.y = j;
            // required move is given by the value in the vectors
            cur_vertex.movedReq = pos_p[i][j];
            // exposition is always one
            cur_vertex.expo = 1;

            cur_vertex.name = boost::lexical_cast<std::string>(i)+","+boost::lexical_cast<std::string>(j);

            vertex_desc desc = add_vertex(cur_vertex, g);
            vertex_map[i][j] = desc;
        }
    }

    return vertex_map;
}

struct not_found{};

template<typename Map>
typename Map::mapped_type map_get(typename Map::key_type k, const Map &m)
{
    typename Map::const_iterator search = m.find(k);
    if (search == m.end()) throw not_found();
    return search->second;
}

///
/// @brief Function creating the edges.
///
void create_edges(const vertex_map_t &vertex_map, graph_t &g)
{
    size_t index_l = 0;
    // We iterate on every vertex of the map
    for(size_t i = 0 ; i < vertex_map.size() ; ++ i )
    {
        for(size_t j = 0 ; j < map_get(i,vertex_map).size() ; ++ j )
        {
            // If not on first row we link every vertex
            // with the one directly above it (same column i and row j-1)
            if ( j > 0 )
            {
                edge_t cur_edge;
                cur_edge.id = index_l++;
                vertex_desc v1 = map_get(j-1,map_get(i, vertex_map));
                vertex_desc v2 = map_get(j,map_get(i, vertex_map));
                cur_edge.extremities = {g[v1], g[v2]};
                add_edge(v1, v2, cur_edge, g);
            }
            // If not on first column we link every vertex
            // with the one directly on the left (column i-1 and same row j)
            if ( i > 0 )
            {
                edge_t cur_edge;
                cur_edge.id = index_l++;
                vertex_desc v1 = map_get(j,map_get(i-1, vertex_map));
                vertex_desc v2 = map_get(j,map_get(i, vertex_map));
                cur_edge.extremities = {g[v1], g[v2]};
                add_edge(v1, v2, cur_edge, g);
            }
        }
    }
}

///
/// @brief A simple label wtih movement required and exposition (plus the current vertex)
///
struct Label {
    Label() {}

    vertex_t v;
    size_t id;
    size_t movedReq = 0;
    double expo = 0.;

    // Required for r_c_shortest_paths
    bool operator<(Label const & other_p ) const
    {
        // Compare required move first
        if(movedReq != other_p.movedReq)
        {
            return movedReq < other_p.movedReq;
        }
        // Then exposition
        if(std::abs(expo - other_p.expo) < 1e-5)
        {
            return expo < other_p.expo;
        }
        // Finaly only id
        return id < other_p.id;
    }
};

///
/// @brief Dominance function
/// Compare movement required
/// Then exposition
///
struct DominanceFunction {
    /// @brief
    inline bool operator()(Label const & a, Label const & b) const
    {
        if(a.movedReq < b.movedReq)
        {
            return true;
        } else if (a.movedReq > b.movedReq)
        {
            return false;
        }
        return a.expo < b.expo;
    }
};

///
/// @brief A function that update a label with the data of an edge.
///
struct ResourceExtensionFunction
{
    ResourceExtensionFunction(size_t maxMove_p) : maxMove(maxMove_p) {};

    size_t maxMove;

    inline bool operator()( graph_t const & g,
        Label & new_cont,
        Label const & old_cont,
        edge_desc ed ) const
    {
        return extends(old_cont, new_cont, g[ed]);
    }

    inline bool extends(Label const & oldLabel_p, Label & label_p, edge_t const & property_p) const
    {
        // Get opposite vertex from the edge
        vertex_t v = property_p.extremities[0].id == oldLabel_p.id ? property_p.extremities[1] : property_p.extremities[0];
        // uodate label value
        label_p.v = v;
        label_p.id = v.id;
        label_p.movedReq = oldLabel_p.movedReq + v.movedReq;
        label_p.expo = oldLabel_p.expo + v.expo;
        // check max move allowed
        return label_p.movedReq <= maxMove;
    }
};

/// @brief Path object for result
struct Path
{
    Label label;
    std::vector<edge_t> edges;

};

/// @brief Streaming Path to a string
std::ostream &operator<<(std::ostream &os_p, Path const &path_p)
{
    os_p << "Path[target = "<<path_p.label.v.name<<", path={";
    std::string lastId_l = path_p.label.v.name;
    os_p << "\"" <<lastId_l<< "\"" ;
    for(edge_t const &e_l : path_p.edges)
    {
        os_p<<", ";
        // If extremities[0] is last vertex walked on then use extremities[1]
        std::string id_l = e_l.extremities[0].name == lastId_l ? e_l.extremities[1].name : e_l.extremities[0].name;
        os_p << "\"" << id_l << "\"" ;
        lastId_l = id_l;
    }
    os_p<<"}";
    os_p<<", movedReq = "<<path_p.label.movedReq;
    os_p<<", expo = "<<path_p.label.expo<<"]";
    return os_p;
}

/// @brief get all paths from a source to all reachable destination
/// @param map_p map of moverequired to gain access to a position, must be squared
/// @param orig_p coordinate of the origin
/// @param maxMove_p the max number of move allowed (move required to get to the origin are not counted since we start on it)
std::vector<Path> getAllPathsToN(std::vector<std::vector<size_t> > const &map_p, std::pair<size_t, size_t> const &orig_p, size_t maxMove_p)
{
    // creating a graph g from vertices and edges of files
    graph_t g;
    vertex_map_t vertex_map = create_vertices(g, map_p);
    vertex_desc source = vertex_map[orig_p.first][orig_p.second];
    create_edges(vertex_map, g);

    // Shortest path

    std::vector< std::vector< edge_desc > > shortestPaths_l; // vector of vector to store all the optimal paths
    std::vector< Label > bestLabels_l; // vector of label to store the value of all optimal paths

    Label label_l;
    label_l.v = g[source];
    label_l.id = label_l.v.id;
    // Call of the shortest path
    r_c_shortest_paths_to_all
    (    g, // graph
        get(&vertex_t::id,g), // map of vertices id
        get(&edge_t::id,g), // map of edges id
        source, // starting vertex
        shortestPaths_l, // optimal paths storage
        bestLabels_l, // optimal values storage
        label_l, // initial label
        ResourceExtensionFunction(maxMove_p), //REF model
        DominanceFunction() // dominance function
    );

    std::vector<Path> vectPath_l;

    // Create Paths from boost objects
    for(size_t i = 0 ; i < shortestPaths_l.size() ; ++ i)
    {
        Path newPath_l;
        auto path_l = shortestPaths_l[i];
        auto label_l = bestLabels_l[i];
        newPath_l.label = label_l;
        for(auto edge_desc_l : path_l)
        {
            newPath_l.edges.push_back(g[edge_desc_l]);
        }
        vectPath_l.push_back(newPath_l);
    }

    return vectPath_l;
}

/// @brief construct the string representation of a handmade path
/// based on the operator<< of Path
/// used to compare handmade path with real Path
std::string buildString(std::vector<std::string> const &path_p, size_t moveReq_p, double expo_p, bool reverse_p)
{
    if(path_p.empty())
    {
        return std::string();
    }
    std::stringstream ss_l;
    // Get correct iterator based on reverse
    std::string firstId_l = reverse_p ? *path_p.rbegin() : * path_p.begin();
    ss_l << "Path[target = "<<firstId_l<<", path={";
    ss_l << "\"" << firstId_l<<"\"";
    if(reverse_p)
    {
        std::for_each(++path_p.rbegin(), path_p.rend(), [&](std::string const str_p)
        {
            ss_l<<", \""<<str_p<<"\"";
        });
    } else
    {
        std::for_each(++path_p.begin(), path_p.end(), [&](std::string const str_p)
        {
            ss_l<<", \""<<str_p<<"\"";
        });
    }
    ss_l<<"}";
    ss_l<<", movedReq = "<<moveReq_p;
    ss_l<<", expo = "<<expo_p<<"]";
    return ss_l.str();
}

/// @brief manual path build using % operator
struct manual_path
{
    manual_path &operator%(std::string const &vertex_p)
    {
        internal_path.push_back(vertex_p);
        return *this;
    }
    std::vector<std::string> internal_path;
};

/// @brief test that the given path exists in the solution
bool testPathExistence(std::vector<Path> const &paths_p, manual_path const &manual_path_p, size_t moveReq_p, double expo_p)
{
    std::string refStringOneWay_l = buildString(manual_path_p.internal_path, moveReq_p, expo_p, false);
    for(Path const &path_l : paths_p)
    {
        if(boost::lexical_cast<std::string, Path>(path_l) == refStringOneWay_l)
        {
            return true;
        }
    }
    // fail if not found with the given message
    return false;
}

/// @brief helpers struct to construct a row of position
struct manual_row
{
    manual_row &operator%(size_t moveReq_p)
    {
        row.push_back(moveReq_p);
        return *this;
    }

    std::vector<size_t> row;
};

/// @brief helpers struct to construct a map of position
struct manual_map
{

    manual_map &operator%(manual_row const &row_p)
    {
        map.push_back(row_p.row);
        return *this;
    }

    std::vector<std::vector<size_t> > map;
};

}

///
/// All tests are based on a map giving the number of required moves to access each position
///

void test_shortest_paths_to_all_no_move_allowed()
{
    //    0  1  2  3  4
    //   --------------
    //0 | 1, 1, 1, 1, 1
    //1 | 1, 0, 1, 0, 1
    //2 | 0, 0, 0, 0, 0
    //3 | 1, 0, 1, 0, 1
    //4 | 1, 1, 1, 0, 1
    //
    // we read vertical index first
    // 2,0 is line 2 and colum 0
    //
    manual_map map_l;
    map_l % (manual_row() % 1 % 1 % 1 % 1 % 1);
    map_l % (manual_row() % 1 % 0 % 1 % 0 % 1);
    map_l % (manual_row() % 0 % 0 % 0 % 0 % 0);
    map_l % (manual_row() % 1 % 0 % 1 % 0 % 1);
    map_l % (manual_row() % 1 % 1 % 1 % 0 % 1);

    std::vector<Path> vectPath_l = getAllPathsToN(map_l.map, {2,0}, 0);

    // Check all paths to all accessible tiles (no move are allowed meaning that all tile with 1 value are forbidden)
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "1,1" % "2,1" % "2,0", 0, 2));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "1,3" % "2,3" % "2,2" % "2,1" % "2,0", 0, 4));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,0", 0, 0));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,1" % "2,0", 0, 1));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,2" % "2,1" % "2,0", 0, 2));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,3" % "2,2" % "2,1" % "2,0", 0, 3));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,4" % "2,3" % "2,2" % "2,1" % "2,0", 0, 4));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "3,1" % "2,1" % "2,0", 0, 2));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "3,3" % "2,3" % "2,2" % "2,1" % "2,0", 0, 4));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "4,3" % "3,3" % "2,3" % "2,2" % "2,1" % "2,0", 0, 5));

}

void test_shortest_paths_to_all_one_move_allowed()
{
    //    0  1  2  3  4
    //   --------------
    //0 | 1, 1, 1, 1, 1
    //1 | 1, 1, 1, 0, 1
    //2 | 0, 0, 1, 0, 1
    //
    // we read vertical index first
    // 2,0 is line 2 and colum 0
    //
    manual_map map_l;
    map_l % (manual_row() % 1 % 1 % 1 % 1 % 1);
    map_l % (manual_row() % 1 % 1 % 1 % 0 % 1);
    map_l % (manual_row() % 0 % 0 % 1 % 0 % 1);

    std::vector<Path> vectPath_l = getAllPathsToN(map_l.map, {2,0}, 1);

    // Check all paths to all accessible tiles (no move are allowed meaning that all tile with 1 value are forbidden)
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "1,0" % "2,0", 1, 1));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "1,1" % "2,1" % "2,0", 1, 2));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "1,3" % "2,3" % "2,2" % "2,1" % "2,0", 1, 4));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,0", 0, 0));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,1" % "2,0", 0, 1));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,2" % "2,1" % "2,0", 1, 2));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,3" % "2,2" % "2,1" % "2,0", 1, 3));
}

void test_shortest_paths_to_all_multiple_paths()
{
    //    0  1  2  3  4
    //   --------------
    //0 | 1, 1, 1, 1, 1
    //1 | 0, 0, 1, 1, 1
    //2 | 0, 0, 1, 0, 1
    //
    // we read vertical index first
    // 2,0 is line 2 and colum 0
    //
    manual_map map_l;
    map_l % (manual_row() % 1 % 1 % 1 % 1 % 1);
    map_l % (manual_row() % 0 % 0 % 1 % 0 % 1);
    map_l % (manual_row() % 0 % 0 % 1 % 0 % 0);

    std::vector<Path> vectPath_l = getAllPathsToN(map_l.map, {2,0}, 0);

    // Check all paths to all accessible tiles (no move are allowed meaning that all tile with 1 value are forbidden)
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "1,0" % "2,0", 0, 1));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "1,1" % "1,0" % "2,0", 0, 2));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "1,1" % "2,1" % "2,0", 0, 2));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,0", 0, 0));
    BOOST_TEST(testPathExistence(vectPath_l, manual_path() % "2,1" % "2,0", 0, 1));
}

int main(int, char*[])
{
    test_shortest_paths_to_all_no_move_allowed();
    test_shortest_paths_to_all_one_move_allowed();
    test_shortest_paths_to_all_multiple_paths();
    return boost::report_errors();
}
