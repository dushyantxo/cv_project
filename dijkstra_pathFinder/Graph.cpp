#include "Graph.h"

Graph::Graph(int n) : n(n), adj(n) {}

void Graph::addEdge(int u, int v, int w) {
    adj[u].emplace_back(v, w);
    adj[v].emplace_back(u, w);
}

vector<int> Graph::dijkstra(int src, int dest) {
    vector<int> dist(n, numeric_limits<int>::max());
    vector<int> prev(n, -1);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    vector<int> path;

    dist[src] = 0;
    pq.emplace(0, src);

    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        if (u == dest) break;

        for (const auto &neighbor : adj[u]) {
            int v = neighbor.first;
            int weight = neighbor.second;

            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                prev[v] = u;
                pq.emplace(dist[v], v);
            }
        }
    }

    for (int at = dest; at != -1; at = prev[at])
        path.push_back(at);
    
    reverse(path.begin(), path.end());

    return path;
}
