#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <unistd.h>

#ifndef NDEBUG
#define dbgprintf(...) fprintf(stderr, __VA_ARGS__)
#define dbgflush(...) fflush(__VA_ARGS__)
#define dbgsleep(...) sleep(__VA_ARGS__)
#else
#define dbgprintf(...) 0
#define dbgflush(...) 0
#define dbgsleep(...) 0u
#endif

#if WEAKDEBUG || !NDEBUG
#define weakprintf(...) fprintf(stderr, __VA_ARGS__)
#define weakflush(...) fflush(__VA_ARGS__)
#define weaksleep(...) sleep(__VA_ARGS__)
#else
#define weakprintf(...) 0
#define weakflush(...) 0
#define weaksleep(...) 0u
#endif

class InsertionException : std::exception {};

class RouteMemory {
public:
  // If not visited twice, must determine each time if any
  // small cave was visited twice. If this is true, no need
  // to check.
  bool visit_twice;
  std::vector<std::string> vis;
  std::string from;
  RouteMemory() : visit_twice(false), vis(), from("start") {}
};

class Graph : std::map<std::string, std::set<std::string>> {
private:
  void dbg_print_trace(std::vector<std::string> const &trace) {
    size_t i = 0;
    if (!trace.empty() && trace.at(trace.size() - 1) == "end")
      for (auto s : trace) {
        if (++i != trace.size())
          weakprintf("%s,", s.c_str());
        else
          weakprintf("%s\n", s.c_str());
      }
    else
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

  size_t end_paths(RouteMemory rm, std::vector<std::string> &trace) {
    assert(this->find(rm.from) != this->end());
    trace.push_back(rm.from);

    dbgprintf("ENTER: ");
    dbg_print_trace(trace);

    dbgprintf("VISITED: ");
    dbg_print_trace(rm.vis);

    // dbgsleep(1);

    if (rm.from == "end") {
      dbgprintf("\"end\" reached, local return 1.\n");
      trace.pop_back();
      return 1;
    }

    auto first_find = std::find(std::begin(rm.vis), std::end(rm.vis), rm.from);
    if (first_find != std::end(rm.vis)) {
      if (!rm.visit_twice && rm.from != "start") {
        dbgprintf("\"%s\" was already visited, but it can be visited again.\n",
                  rm.from.c_str());
      } else {
        dbgprintf(
            "\"%s\" already visited twice or was \"start,\" local return 0.\n",
            rm.from.c_str());
        trace.pop_back();
        return 0;
      }
    }

    if (islower(*rm.from.c_str())) {
      if (rm.from == "start") {
        dbgprintf("\"start\" --> never visit again.\n");
      } else {
        dbgprintf("\"%s\" lowercase, mark as never visit more than twice.\n",
                  rm.from.c_str());
      }
      rm.vis.push_back(rm.from);

      // Does this cause two or more elements to exist?
      if (first_find != std::end(rm.vis)) {
        first_find = std::find(std::begin(rm.vis), std::end(rm.vis), rm.from);
        first_find++;
        auto second_find = std::find(first_find, std::end(rm.vis), rm.from);
        if (second_find != std::end(rm.vis)) {
          dbgprintf("\"%s\" will cause this cave to have been visited twice.\n",
                    rm.from.c_str());
          rm.visit_twice = true;
        }
      }
    }

    size_t local = 0;
    for (auto neigh : this->at(rm.from)) {
      RouteMemory rm2(rm);
      rm2.from = neigh;
      local += this->end_paths(rm2, trace);
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

  std::vector<std::string> trace;
  RouteMemory rm;
  size_t n = g.end_paths(rm, trace);
  printf("There are %zu ways to go from \"%s\" to end.\n", n, rm.from.c_str());

  return 0;
}