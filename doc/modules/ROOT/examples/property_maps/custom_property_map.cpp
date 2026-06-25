#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <iterator>
#include <vector>

// A simplified iterator_property_map to show how custom property maps work.
// Wraps a random access iterator and an ID map (descriptor -> integer offset).
template <class Iterator, class IDMap>
class iterator_map {
public:
    using key_type   = typename boost::property_traits<IDMap>::key_type;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using reference  = typename std::iterator_traits<Iterator>::reference;
    using category   = boost::lvalue_property_map_tag;

    iterator_map(Iterator i = Iterator(), const IDMap& id = IDMap())
        : m_iter(i), m_id(id) {}

    Iterator m_iter;
    IDMap    m_id;
};

// get(): read a value
template <class Iter, class ID>
typename std::iterator_traits<Iter>::value_type
get(const iterator_map<Iter, ID>& m,
    typename boost::property_traits<ID>::key_type key)
{
    return m.m_iter[get(m.m_id, key)];
}

// put(): write a value
template <class Iter, class ID>
void put(const iterator_map<Iter, ID>& m,
         typename boost::property_traits<ID>::key_type key,
         const typename std::iterator_traits<Iter>::value_type& value)
{
    m.m_iter[get(m.m_id, key)] = value;
}

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS>;

    Graph g(4);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);

    // Use our custom property map to store labels
    std::vector<std::string> labels(num_vertices(g));
    auto index = get(vertex_index, g);
    iterator_map<std::vector<std::string>::iterator,
                 decltype(index)> label_map(labels.begin(), index);

    // Write through the property map
    put(label_map, vertex(0, g), "A");
    put(label_map, vertex(1, g), "B");
    put(label_map, vertex(2, g), "C");
    put(label_map, vertex(3, g), "D");

    // Read through the property map
    for (auto vi = vertices(g).first; vi != vertices(g).second; ++vi) {
        std::cout << "vertex " << *vi << " = " << get(label_map, *vi) << "\n";
    }
}
