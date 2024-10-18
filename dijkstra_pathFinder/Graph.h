#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <utility>
#include <limits>
#include <queue>
#include <SFML/Graphics.hpp>

using namespace std;

class Graph {
public:
    Graph(int n);
    void addEdge(int u, int v, int w);
    vector<int> dijkstra(int src, int dest);
private:
    int n;
    vector<vector<pair<int, int>>> adj;
};

#endif
