#include "done.hpp"
#include <boost/python.hpp>

namespace boost { namespace graph { namespace python {

using namespace boost::python;

class done_wrap : public done, public wrapper<done>
{
public:
  bool operator()() const
  {
    return this->get_override("__call__")();
  }
};

void export_done()
{
  class_<done_wrap, boost::noncopyable>("Done")
    .def("__call__", pure_virtual(&done::operator()))
    ;
}

} } } // end namespace boost::graph::python
