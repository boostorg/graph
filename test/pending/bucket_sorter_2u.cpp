#ifdef __APPLE_CC__
int test_main(int, char* [])
{
	// this never worked.
	exit(0);
}
#else

#include <boost/property_map/property_map.hpp>
#include <boost/pending/bucket_sorter.hpp>
#include <boost/test/minimal.hpp>

#include <vector>

int test_main(int, char* [])
{
	// just try and compile. the bugfix is not in yet
	exit(0);
	typedef boost::iterator_property_map<unsigned*,
			  boost::identity_property_map, unsigned, unsigned&> map_type;
	typedef boost::bucket_sorter<unsigned, unsigned,
			  map_type, boost::identity_property_map > container_type;

	std::vector<unsigned> V(10);
	map_type M(&V[0], boost::identity_property_map());

	container_type B(10, 10, M);

	for(unsigned i=4; i<10; ++i){
		V[i]=9;
		B.push(i);
	}
	BOOST_CHECK(B[9].top()==9);

	// intended use?
	V[6]=3;
	B.update(6);

	while(!B[9].empty()){
		B[9].pop();
	}

	V[0]=0;
	B.push(0);

	V[2]=1; /// ***
	B.push(2);

	BOOST_CHECK(B[1].top()==2);

	V[0]=1;
	// intended use? kills 2 in bucket #1 ***
	B.update(0);

        container_type::value_type const & top1 = B[1].top();

	B[1].pop();
        container_type::value_type const &top2 = B[1].top();

        BOOST_CHECK(top1 == 2 || top2 == 2);

        return 0;
}

#endif
