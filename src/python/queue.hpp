// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_PYTHON_QUEUE_HPP
#define BOOST_GRAPH_PYTHON_QUEUE_HPP

#include <boost/python.hpp>

namespace boost { namespace graph { namespace python {

template<typename T>
class python_queue
{
  class wrap 
    : public python_queue<T>, public boost::python::wrapper<python_queue<T> >
  {
  public:
    bool empty() const    { return this->get_override("empty")(); }
    T top() const         { return this->get_override("top")(); }
    void pop()            { this->get_override("pop")(); }
    void push(const T& x) { this->get_override("push")(x); }
  };

 public:
  class default_queue : public python_queue<T> 
  {
    bool empty() const { return true; }
    T top() const { return T(); }
    void pop() {}
    void push(const T&) {}
  };

  virtual ~python_queue() {}
  virtual bool empty() const = 0;
  virtual T top() const = 0;
  virtual void pop() = 0;
  virtual void push(const T&) = 0;

  static void declare(const char* name, const char* default_name)
  {
    using boost::python::objects::registered_class_object;
    using boost::python::type_id;
    using boost::python::class_;
    using boost::python::bases;
    using boost::python::no_init;
    using boost::python::pure_virtual;

    if (registered_class_object(type_id<wrap>()).get() == 0) {
      class_<wrap, boost::noncopyable>(name)
        .def("empty", pure_virtual(&python_queue<T>::empty))
        .def("top", pure_virtual(&python_queue<T>::top))
        .def("pop", pure_virtual(&python_queue<T>::pop))
        .def("push", pure_virtual(&python_queue<T>::push))
        ;
     }

    if (registered_class_object(type_id<default_queue>()).get() == 0)
    {
      class_<default_queue, bases<python_queue> >(default_name, no_init);
    }
  }
};

} } } // end namespace boost::graph::python

#endif // BOOST_GRAPH_PYTHON_QUEUE_HPP
