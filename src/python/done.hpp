#ifndef BOOST_GRAPH_PYTHON_DONE_HPP
#define BOOST_GRAPH_PYTHON_DONE_HPP

namespace boost { namespace graph { namespace python {

class done
{
 public:
  virtual ~done() { }
  virtual bool operator()() const = 0;
};

} } } // end namespace boost::graph::python

#endif // BOOST_GRAPH_PYTHON_DONE_HPP
