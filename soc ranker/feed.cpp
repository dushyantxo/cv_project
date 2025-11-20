// feed_ranking.cpp
// Fully interactive Social Feed Ranking Engine
// (C++ | Graphs | Priority Queues | Weighted Scoring)
//
// Compile: g++ -std=c++17 -O2 -Wall feed_ranking.cpp -o feed_ranking

#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// ------------------ SCORING CONSTANTS ------------------
const double W_ENGAGEMENT = 1.0;
const double W_AFFINITY   = 1.2;
const double W_RECENCY    = 1.5;
const double RECENCY_LAMBDA = 1.0 / (60.0 * 60.0 * 24.0);  // recency decay per day
const int MAX_HOPS = 2;   // friends + friends-of-friends
// --------------------------------------------------------

struct Engagement {
    int likes{}, comments{}, shares{};
    int total() const { return likes + comments*2 + shares*3; }
};

struct Post {
    int postId{}, authorId{};
    ll timestamp{};
    string text;
    Engagement eng;
};

class SocialGraph {
public:
    unordered_map<int, unordered_map<int,double>> adj;

    void addUser(int u) {
        if (!adj.count(u)) adj[u] = {};
    }

    void addInteraction(int u, int v, double delta) {
        addUser(u);
        addUser(v);
        if (u == v) return;
        adj[u][v] += delta;
        adj[v][u] += delta;
    }

    double affinity(int u, int v) const {
        auto it = adj.find(u);
        if (it == adj.end()) return 0;
        auto it2 = it->second.find(v);
        return (it2 == it->second.end()) ? 0 : it2->second;
    }

    vector<int> neighbors(int u) const {
        vector<int> out;
        auto it = adj.find(u);
        if (it == adj.end()) return out;
        for (auto &p : it->second) out.push_back(p.first);
        return out;
    }

    bool hasUser(int u) const { return adj.count(u); }
};

class FeedEngine {
public:
    FeedEngine() : nextPostId(1) {}

    void addUser(int uid) {
        graph.addUser(uid);
    }

    void addInteraction(int u, int v, double delta) {
        graph.addInteraction(u, v, delta);
    }

    int createPost(int authorId, ll ts, const string &text) {
        posts[nextPostId] = Post{nextPostId, authorId, ts, text};
        userPosts[authorId].push_back(nextPostId);
        return nextPostId++;
    }

    bool updateEngagement(int pid, int l, int c, int s) {
        if (!posts.count(pid)) return false;
        posts[pid].eng.likes += l;
        posts[pid].eng.comments += c;
        posts[pid].eng.shares += s;
        return true;
    }

    vector<pair<int,double>> getFeed(int userId, int K, ll now) {
        vector<pair<int,double>> res;
        if (K <= 0 || !graph.hasUser(userId)) return res;

        unordered_map<int,int> dist;
        bfsUsers(userId, dist);

        vector<int> candidates;
        for (auto &p : dist) {
            int u = p.first;
            for (int pid : userPosts[u]) candidates.push_back(pid);
        }

        auto cmp = [](auto &a, auto &b) {
            if (a.first != b.first) return a.first > b.first;
            return a.second < b.second;
        };
        priority_queue<pair<double,int>, vector<pair<double,int>>, decltype(cmp)> minheap(cmp);

        for (int pid : candidates) {
            auto &post = posts[pid];
            double score = computeScore(userId, post, dist, now);
            if ((int)minheap.size() < K) {
                minheap.push({score, pid});
            } else {
                if (score > minheap.top().first) {
                    minheap.pop();
                    minheap.push({score, pid});
                }
            }
        }

        while (!minheap.empty()) {
            res.push_back({minheap.top().second, minheap.top().first});
            minheap.pop();
        }
        reverse(res.begin(), res.end());
        return res;
    }

    void printPost(int pid) {
        if (!posts.count(pid)) {
            cout << "Post not found\n";
            return;
        }
        auto &p = posts[pid];
        cout << "Post " << pid << " by user " << p.authorId << "\n";
        cout << "  \"" << p.text << "\"\n";
        cout << "  likes=" << p.eng.likes
             << " comments=" << p.eng.comments
             << " shares=" << p.eng.shares << "\n";
    }

private:
    SocialGraph graph;
    unordered_map<int, Post> posts;
    unordered_map<int, vector<int>> userPosts;
    int nextPostId;

    void bfsUsers(int start, unordered_map<int,int> &dist) {
        queue<int> q;
        dist[start] = 0;
        q.push(start);

        while (!q.empty()) {
            int u = q.front(); q.pop();
            if (dist[u] >= MAX_HOPS) continue;
            for (int nb : graph.neighbors(u)) {
                if (!dist.count(nb)) {
                    dist[nb] = dist[u] + 1;
                    q.push(nb);
                }
            }
        }
    }

    double computeScore(int viewer, const Post &p,
                        const unordered_map<int,int> &dist,
                        ll now) {
        double engScore = p.eng.likes + p.eng.comments*1.5 + p.eng.shares*3.0;

        double aff = 0.0;
        if (dist.count(p.authorId)) {
            int hops = dist.at(p.authorId);
            double base = graph.affinity(viewer, p.authorId);
            double hopFactor = (hops == 0 ? 1.0 : (hops == 1 ? 1.0 : 0.6));
            aff = base * hopFactor;
        }

        ll age = max(0LL, now - p.timestamp);
        double rec = exp(-RECENCY_LAMBDA * age);

        return W_ENGAGEMENT * engScore +
               W_AFFINITY * aff +
               W_RECENCY * rec;
    }
};

// --------------------- CLI ----------------------

void help() {
    cout << "Commands:\n";
    cout << "  adduser <uid>\n";
    cout << "  interact <u> <v> <delta>\n";
    cout << "  post <author> <timestamp> <text>\n";
    cout << "  engage <postId> <likes> <comments> <shares>\n";
    cout << "  feed <userId> <K>\n";
    cout << "  show <postId>\n";
    cout << "  help\n";
    cout << "  exit\n";
}

string tailString(stringstream &ss) {
    string rest;
    getline(ss, rest);
    if (!rest.empty() && rest[0] == ' ') rest.erase(rest.begin());
    return rest;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    FeedEngine engine;
    cout << "Social Feed Ranking Engine (Interactive)\n";
    cout << "Type 'help' for commands.\n";

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break;
        if (line.empty()) continue;

        stringstream ss(line);
        string cmd; ss >> cmd;

        if (cmd == "help") help();
        else if (cmd == "exit") break;

        else if (cmd == "adduser") {
            int u; ss >> u;
            engine.addUser(u);
            cout << "User " << u << " added.\n";
        }

        else if (cmd == "interact") {
            int u,v; double d;
            ss >> u >> v >> d;
            engine.addInteraction(u,v,d);
            cout << "Affinity increased between " << u << " and " << v << " by " << d << "\n";
        }

        else if (cmd == "post") {
            int a; ll ts;
            ss >> a >> ts;
            string text = tailString(ss);
            int pid = engine.createPost(a, ts, text);
            cout << "Post " << pid << " created by user " << a << "\n";
        }

        else if (cmd == "engage") {
            int pid,l,c,s;
            ss >> pid >> l >> c >> s;
            if (!engine.updateEngagement(pid,l,c,s)) cout << "Post not found.\n";
            else cout << "Engagement updated.\n";
        }

        else if (cmd == "feed") {
            int uid,K; ss >> uid >> K;
            ll now = time(nullptr);
            auto feed = engine.getFeed(uid,K,now);
            cout << "Top " << K << " posts for user " << uid << ":\n";
            for (auto &p : feed) {
                cout << "--------------------------------\n";
                engine.printPost(p.first);
                cout << "  Score = " << p.second << "\n";
            }
        }

        else if (cmd == "show") {
            int pid; ss >> pid;
            engine.printPost(pid);
        }

        else cout << "Unknown command\n";
    }

    cout << "Exiting...\n";
    return 0;
}
