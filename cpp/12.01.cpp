#include <algorithm>
#include <cassert>
#include <cctype>
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
private:
  void dbg_print_trace(std::vector<std::string> const &trace) {
    size_t i = 0;
    for (auto s : trace) {
      if (++i != trace.size())
        dbgprintf("%s,", s.c_str());
      else
        dbgprintf("%s\n", s.c_str());
    }
  }

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
    } else if (scan != 0) {
      throw InsertionException();
    } else {
      dbgprintf(
          "WARN: Graph string insertion: no insertion due to no token.\n");
      return *this;
    }
  }

  void debug(void) {
    dbgprintf("Graph (%zu elements)\n", this->size());
    for (auto &[key, elements] : *this) {
      dbgprintf("%s -> ", key.c_str());
      size_t i = 0;
      for (auto &e : elements) {
        if (++i != elements.size()) {
          dbgprintf("%s, ", e.c_str());
        } else {
          dbgprintf("%s\n", e.c_str());
        }
      }
    }
  }

  size_t end_paths(std::vector<std::string> vis, std::string from,
                   std::vector<std::string> &trace) {
    assert(this->find(from) != this->end());
    trace.push_back(from);

    dbgprintf("ENTER: ");
    dbg_print_trace(trace);

    if (from == "end") {
      dbgprintf("\"end\" reached, local return 1.\n");
      dbg_print_trace(trace);
      trace.pop_back();
      return 1;
    }

    if (std::find(std::begin(vis), std::end(vis), from) != std::end(vis)) {
      dbgprintf("\"%s\" already visited, local return 0.\n", from.c_str());
      trace.pop_back();
      return 0;
    }

    dbgprintf("VISITED: ");
    dbg_print_trace(vis);

    if (islower(*from.c_str())) {
      dbgprintf("\"%s\" lowercase, mark as never visit again.\n", from.c_str());
      vis.push_back(from);
    }

    size_t local = 0;

    for (auto neigh : this->at(from)) {
      // NOTE: The inner content of &vis changes within this loop.
      local += this->end_paths(vis, neigh, trace);
    }
    trace.pop_back();
    return local;
  }
};

int main(void) {
  Graph g;

  while (!std::cin.eof()) {
    std::string line;
    std::getline(std::cin, line);
    try {
      g << line.c_str();
    } catch (InsertionException &e) {
      std::cerr << "Looks like line \"" << line << "\" is badly formatted!\n";
    }
  }

  dbgprintf("\n");

  g.debug();

  dbgprintf("\n");

  std::vector<std::string> visited, trace;
  std::string start = "start";
  size_t n = g.end_paths(visited, start, trace);
  printf("There are %zu ways to go from \"%s\" to end.\n", n, start.c_str());

  return 0;
}