// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
template<typename Graph>
class BGL_PYTHON_VISITOR
{
  class wrap 
    : public BGL_PYTHON_VISITOR<Graph>, 
      public boost::python::wrapper<BGL_PYTHON_VISITOR<Graph> >
  {
  public:
    typedef typename BGL_PYTHON_VISITOR<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename BGL_PYTHON_VISITOR<Graph>::edge_descriptor   edge_descriptor;

#define BGL_PYTHON_EVENT(Name,Descriptor)                               \
    void Name(Descriptor x, const Graph& g) const                       \
    {                                                                   \
      if (boost::python::override f = this->get_override(#Name))        \
        f(x, boost::cref(g));                                           \
      else BGL_PYTHON_VISITOR<Graph>::Name(x, g);                       \
    }                                                                   \
                                                                        \
    void default_##Name(Descriptor x, const Graph& g)  const            \
    { this->BGL_PYTHON_VISITOR<Graph>::Name(x, g); }    
#  include BGL_PYTHON_EVENTS_HEADER
#undef BGL_PYTHON_EVENT
  };

 public:
  class default_arg : public BGL_PYTHON_VISITOR<Graph> { };

  struct ref 
  {
    typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename graph_traits<Graph>::edge_descriptor   edge_descriptor;

    ref(const BGL_PYTHON_VISITOR<Graph>& v) : v(v) { }

#define BGL_PYTHON_EVENT(Name, Descriptor)                            \
    void Name(Descriptor x, const Graph& g) const { v.Name(x, g); }
#  include BGL_PYTHON_EVENTS_HEADER
#undef BGL_PYTHON_EVENT

  private:
    const BGL_PYTHON_VISITOR<Graph>& v;
  };

  typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef typename graph_traits<Graph>::edge_descriptor   edge_descriptor;

  virtual ~BGL_PYTHON_VISITOR() {}

#define BGL_PYTHON_EVENT(Name, Descriptor)                            \
  virtual void Name(Descriptor x, const Graph& g) const {}
#  include BGL_PYTHON_EVENTS_HEADER
#undef BGL_PYTHON_EVENT

  static void declare(const char* name, const char* default_name)
  {
    using boost::python::class_;
    using boost::python::bases;
    using boost::python::no_init;
    using boost::python::objects::registered_class_object;
    using boost::python::type_id;
    
    if (registered_class_object(type_id<wrap>()).get() != 0)
      return;

#define BGL_PYTHON_EVENT(Name, Descriptor)                              \
      .def(#Name, &BGL_PYTHON_VISITOR<Graph>::Name, &wrap::default_##Name)
    class_<wrap, boost::noncopyable>(name)
#  include BGL_PYTHON_EVENTS_HEADER
#undef BGL_PYTHON_EVENT
      ;

    class_<default_arg, bases<BGL_PYTHON_VISITOR<Graph> > >(default_name, 
                                                            no_init);
  }
};

