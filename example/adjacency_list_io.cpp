// François Faure, iMAGIS-GRAVIR / UJF, 2001

#include <boost/graph/adjacency_list_io.hpp>
#include <fstream>

using namespace boost;

//======== my data structure
struct MyStruct { double value; };
ostream& operator << ( ostream& out, const MyStruct& s )
{
	out << s.value << " ";
	return out;
}
istream& operator >> ( istream& in, MyStruct& s )
{
	in >> s.value;
	return in;
}

//======== vertex properties
struct n1_t { enum { num = 23063}; typedef vertex_property_tag kind; };
struct n2_t { enum { num = 23062}; typedef vertex_property_tag kind; };
struct n3_t { enum { num = 23061}; typedef vertex_property_tag kind; };
typedef property< n1_t, int,
        property< n2_t, double,
		property< n3_t, MyStruct > > > VertexProperty;


//====== edge properties
struct e1_t { enum { num = 23064}; typedef edge_property_tag kind; };
typedef property<e1_t, double> EdgeProperty;



//===== graph types

typedef 
	adjacency_list<vecS, listS, directedS, no_property, no_property> 
	Graph1;

typedef 
	adjacency_list<setS, setS, bidirectionalS, VertexProperty, EdgeProperty> 
	Graph2;



int 
main()
{
	// read Graph1
	Graph1 g1;
	ifstream readFile1("data1.txt");
	readFile1 >> read( g1 );
 	cout << "graph g1 from file data1.txt:\n" 
	     << write( g1 ) 
		 << endl;

	// read Graph2 and all internal properties
	Graph2 g2;
	ifstream readFile2("data2.txt");
	readFile2 >> read( g2 );
 	cout << "graph g2 from file data2.txt:\n" 
	     << write( g2 ) 
		 << endl;
	
	// read Graph2, no property given. Write no property.
	Graph2 g21;
	ifstream readFile21("data1.txt");
	readFile21 >> read( g21, no_property(), no_property() );
 	cout << "graph g21 from file data1.txt:\n" 
	     << write(g21, no_property(), no_property()) 
		 << endl;
	
	// read Graph2, incomplete data in a different order. Write it diffently.
	Graph2 g31;
	ifstream readFile31("data3.txt");
	typedef property< n3_t, MyStruct, property< n1_t, int > > readNodeProp;
	readFile31 >> read( g31, readNodeProp() , EdgeProperty() );
 	cout << "graph g31 from file data3.txt:\n" 
	     << write( g31, property<n3_t, MyStruct>(), EdgeProperty() ) 
		 << endl;
	

	return 0;
}

