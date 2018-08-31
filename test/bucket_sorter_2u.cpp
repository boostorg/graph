#include <iostream>
#include <boost/property_map/property_map.hpp>
#include <boost/pending/bucket_sorter.hpp>

// test bucket sorter. return 0 if OK, 1 otherwise.

int main(){

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
	assert(B[9].top()==9);

	// intended use?
	V[6]=3;
	B.update(6);

	while(!B[9].empty()){
		std::cout<< B[9].top() << "\n";
		B[9].pop();
	}

	std::cout << "=====\n";

	V[0]=0;
	B.push(0);

	V[2]=1; /// ***
	B.push(2);

	assert(B[1].top()==2);
	std::cout<< B[1].top() << "\n";

	V[0]=1;
	// intended use? kills 2 in bucket #1 ***
	B.update(0);

	std::cout<< B[1].top() << "\n";
	auto top1=B[1].top();

	B[1].pop();
	std::cout<< B[1].top() << "\n";
	auto top2=B[1].top();

	if(top1 == 2 || top2 == 2){
		// that would be nice
		std::cerr << "good\n";
		return 0;
	}else{
		std::cerr << "broken\n";
		return 1;
	}
}
