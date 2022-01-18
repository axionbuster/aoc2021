#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

#ifndef NDEBUG
#define dbgprintf(...) fprintf(stderr, __VA_ARGS__)
#define dbgflush(...) fflush(...)
#else
#define dbgprintf(...) 0
#define dbgflush(...) 0
#endif

class InsertionException : std::exception {};

class Graph : std::map<std::string, std::set<std::string>> {
public:
  // Assuming an ASCII-C-string representation of an edge relation,
  // add the undirected edge into the graph.
  Graph &operator<<(const char *c_edge) {
    std::vector<char> c_e1(64, '\0'), c_e2(64, '\0');

    // Note there is no way to tell scanf to stop writing to more than
    // 64 bytes of string.
    int scan = sscanf(c_edge, "%[^\n -] - %[^\n -] ", c_e1.data(), c_e2.data());

    if (scan == 2) {
      dbgprintf("%s - %s (read)\n", c_e1.data(), c_e2.data());
      std::string e1(c_e1.data());
      std::string e2(c_e2.data());
      (*this)[e1].insert(e2);
      (*this)[e2].insert(e1);
      return *this;
    } else {
      throw InsertionException();
    }
  }

  void debug(void) {
    printf("Graph (%zu elements)\n", this->size());
    for (auto &[key, elements] : *this) {
      printf("%s -> ", key.c_str());
      size_t i = 0;
      for (auto &e : elements) {
        if (++i != elements.size()) {
          printf("%s, ", e.c_str());
        } else {
          printf("%s\n", e.c_str());
        }
      }
    }
  }
};

int main(void) {
  Graph g;

  g << "start-A"
    << "start-b"
    << "A-c"
    << "A-b"
    << "b-d"
    << "A-end"
    << "b-end";

  dbgprintf("\n");

  g.debug();

  return 0;
}