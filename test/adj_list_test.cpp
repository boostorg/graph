#include <iostream>
#include <fstream>
#include <string>

std::string container_types [] = { "vecS", "listS", "setS", "slistS" };
const int N = sizeof(container_types)/sizeof(std::string);

std::string directed_types[] = { "bidirectionalS", "directedS", "undirectedS"};
const int D = sizeof(directed_types)/sizeof(std::string);

int
main()
{
  int i, j, k, ret = 0, rc;
  for (i = 0; i < N; ++i)
    for (j = 0; j < N; ++j)
      for (k = 0; k < D; ++k) {

        std::string file_name = "graph_type.hpp";
        std::ofstream header(file_name.c_str());
        if (!header) {
          std::cerr << "could not open file " << file_name << std::endl;
          return -1;
        }

        header << "#include <boost/graph/adjacency_list.hpp>" << std::endl
               << "typedef boost::adjacency_list<boost::" << container_types[i]
               << ", boost::" << container_types[j]
               << ", boost::" << directed_types[k]
               << ", boost::property<vertex_id_t, std::size_t>"
	       << ", boost::property<edge_id_t, std::size_t> > Graph;"
               << std::endl
               << "typedef boost::property<vertex_id_t, std::size_t> VertexId;"
               << std::endl
               << "typedef boost::property<edge_id_t, std::size_t> EdgeID;"
               << std::endl;
        system("rm -f graph.exe graph.o graph.obj");
	// the following system call should be replaced by a
	// portable "compile" command.
        rc = system("g++ -I../../.. graph.cpp -o graph.exe");
        if (rc != 0) {
          std::cerr << "compile failed for " << container_types[i] << " "
                    << container_types[j] << " " << directed_types[k]
                    << std::endl;
          ret = -1;
        } else {
          rc = system("./graph.exe");
          if (rc != 0) {
            std::cerr << "run failed for " << container_types[i] << " "
                      << container_types[j] << " " << directed_types[k]
                      << std::endl;
            ret = -1;
          } 
        }
      }
  
  return ret;
}
