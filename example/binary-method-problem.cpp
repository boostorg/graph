//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, Indiana University,
// Bloomington, IN 47405.
//
// Permission to modify the code and to distribute the code is
// granted, provided the text of this NOTICE is retained, a notice if
// the code was modified is included with the above COPYRIGHT NOTICE
// and with the COPYRIGHT NOTICE in the LICENSE file, and that the
// LICENSE file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================
#include <string>
#include <iostream>

class Point
{
public:
  virtual bool equal(const Point * p) const = 0;
};

class ColorPoint:public Point
{
public:
  ColorPoint(float x, float y, std::string c):x(x), y(y), color(c)
  {
  }
  virtual bool equal(const ColorPoint * p) const
  {
    return color == p->color && x == p->x && y == p->y;
  }
protected:
  float x, y;
  std::string color;
};

class ColorPoint2:public Point
{
public:
  ColorPoint2(float x, float y, std::string s):x(x), y(y), color(s)
  {
  }
  virtual bool equal(const Point * p) const
  {
    const ColorPoint2 *cp = dynamic_cast < const ColorPoint2 * >(p);
      return color == cp->color && x == cp->x && y == cp->y;
  }
protected:
  float x, y;
  std::string color;
};

void
print_equal(const Point * a, const Point * b)
{
  std::cout << std::boolalpha << a->equal(b) << std::endl;
}

template < typename PointType > void
print_equal2(const PointType * a, const PointType * b)
{
  std::cout << std::boolalpha << a->equal(b) << std::endl;
}


int
main()
{
  Point *p = new ColorPoint2(0.0, 0.0, "blue");
  Point *q = new ColorPoint2(0.0, 0.0, "green");
  print_equal(p, q);
  ColorPoint *a = new ColorPoint(0.0, 0.0, "blue");
  ColorPoint *b = new ColorPoint(0.0, 0.0, "green");
  print_equal2(a, b);


  return 0;
}
