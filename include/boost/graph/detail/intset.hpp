

// (C) Copyright Jeremy Siek 2001. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#include <boost/config.hpp>

// UNDER CONSTRUCTION

namespace boost {

  namespace detail {
    
    template <typename SizeType, 
              typename Allocator = std::allocator<SizeType> >
    class intset {
    public:
      intset(SizeType n) : m_index(n), m_value(n) {
	
      }
      
    private:
      std::vector<SizeType, Allocator> m_index;
      std::vector<SizeType, Allocator> m_value;
      size_type m_next;
    };
    
    

  } // namespace detail 

} // namespace boost
