// autocomplete.cpp
// Enhanced resume-grade autocomplete:
// - central dictionary (vector<string>) + index-based trie nodes (saves memory)
// - per-node Top-K indices maintained at insert/update for O(|prefix|+K) queries
// - interactive CLI: suggest/add/update/remove/save/benchmark/help/exit
// - optional append-on-add to keywords file for durability of new words
//
// Compile: g++ -std=c++17 -O2 -Wall autocomplete.cpp -o autocomplete
// Run: ./autocomplete keywords.txt

#include <bits/stdc++.h>
using namespace std;

// -------------------- Config --------------------
static const int DEFAULT_PER_NODE_K = 10; // how many top entries we keep per trie node
// ------------------------------------------------

struct TrieNode {
    // children: we use unordered_map<char, TrieNode*> to support any ascii characters in keywords
    unordered_map<char, TrieNode*> children;

    // If wordIndex >= 0, this node represents an end of a word (index into dict)
    int wordIndex = -1;

    // per-node top-K list of word indices, sorted by our ranking (desc freq, asc word)
    vector<int> topK; // size <= perNodeK

    ~TrieNode() {
        for (auto &p : children) delete p.second;
    }
};

// Helper comparator object (captures references)
struct RankHelper {
    const vector<long long> &freqs;
    const vector<string> &dict;
    RankHelper(const vector<long long> &f, const vector<string> &d): freqs(f), dict(d) {}

    // returns true if a should come before b
    bool higher(int a, int b) const {
        long long fa = (a >=0 && a < (int)freqs.size()) ? freqs[a] : 0;
        long long fb = (b >=0 && b < (int)freqs.size()) ? freqs[b] : 0;
        if (fa != fb) return fa > fb;
        // tie-breaker: lexicographic ascending
        const string &sa = dict[a];
        const string &sb = dict[b];
        return sa < sb;
    }
};

class AutocompleteEngine {
public:
    AutocompleteEngine(int perNodeK = DEFAULT_PER_NODE_K)
        : root(new TrieNode()), perNodeK(perNodeK)
    {}

    ~AutocompleteEngine() { delete root; }

    // Load initial keywords from file (word freq per line)
    bool loadFromFile(const string& filename) {
        ifstream fin(filename);
        if (!fin.is_open()) return false;
        string word;
        long long freq;
        int loaded = 0;
        while (fin >> word >> freq) {
            int idx = ensureWordIndex(word);
            freqs[idx] = freq;
            // Insert into trie and update per-node topK
            insertIntoTrie(word, idx);
            ++loaded;
        }
        fin.close();
        return true;
    }

    // Insert or increment by freq
    // If autosaveAppendFile is set (non-empty) and the word was newly added, we append to file
    void insert(const string& keyword, long long freq = 1, const string& autosaveAppendFile = "") {
        if (keyword.empty()) return;
        int idx = ensureWordIndex(keyword);
        bool isNew = (freqs[idx] == 0 && dict[idx] == keyword && !wordSeen[idx]);
        freqs[idx] += freq;
        wordSeen[idx] = true;
        // update per-node topK along path
        updateTopKForWord(keyword, idx);

        // If requested, append new words to file (only when word previously unseen in file)
        if (!autosaveAppendFile.empty() && isNew) {
            ofstream fout(autosaveAppendFile, ios::app);
            if (fout.is_open()) {
                fout << keyword << " " << freqs[idx] << "\n";
            }
        }
    }

    // Update to absolute frequency
    void updateFrequency(const string& keyword, long long newFreq) {
        if (keyword.empty()) return;
        int idx = ensureWordIndex(keyword);
        freqs[idx] = newFreq;
        wordSeen[idx] = true;
        updateTopKForWord(keyword, idx);
    }

    // Remove a word (mark non-word)
    void remove(const string& keyword) {
        if (keyword.empty()) return;
        auto it = wordToIndex.find(keyword);
        if (it == wordToIndex.end()) return;
        int idx = it->second;
        freqs[idx] = 0;
        wordSeen[idx] = false;
        // find node for word and mark wordIndex = -1
        TrieNode* node = root;
        for (char c : keyword) {
            if (!node->children.count(c)) return;
            node = node->children[c];
        }
        if (node) node->wordIndex = -1;
        // Need to remove index from per-node topK along path
        removeIndexFromPath(keyword, idx);
    }

    // Get top-K suggestions for prefix (fast: traverse to node then return its cached list)
    vector<pair<string,long long>> getTopK(const string& prefix, int K) {
        vector<pair<string,long long>> out;
        if (K <= 0) return out;

        TrieNode* node = root;
        for (char c : prefix) {
            if (!node->children.count(c)) return out;
            node = node->children[c];
        }
        // node->topK already sorted by our ranking
        int upto = min(K, (int)node->topK.size());
        for (int i = 0; i < upto; ++i) {
            int idx = node->topK[i];
            out.emplace_back(dict[idx], freqs[idx]);
        }
        return out;
    }

    // Persist current freqs back to file (overwrites)
    bool saveToFile(const string& filename) const {
        ofstream fout(filename);
        if (!fout.is_open()) return false;
        // write all seen words (wordSeen true) sorted by word for readability
        vector<int> idxs;
        for (int i = 0; i < (int)dict.size(); ++i) if (wordSeen[i]) idxs.push_back(i);
        sort(idxs.begin(), idxs.end(), [&](int a, int b){ return dict[a] < dict[b]; });
        for (int id : idxs) {
            fout << dict[id] << " " << freqs[id] << "\n";
        }
        fout.close();
        return true;
    }

    void setPerNodeK(int k) {
        perNodeK = max(1, k);
    }

    // Quick benchmark: performs many random prefix queries and reports average time
    // We simulate queries by repeatedly taking random prefixes of words in dict.
    void benchmark(int numQueries, int prefixLen) {
        vector<string> sample;
        for (const auto &w : dict) if (!w.empty()) sample.push_back(w);
        if (sample.empty()) {
            cout << "No data to benchmark.\n";
            return;
        }
        std::mt19937_64 rng(123456789);
        std::uniform_int_distribution<size_t> di(0, sample.size()-1);

        // warm-up
        for (int i = 0; i < min(100, (int)numQueries); ++i) {
            auto &w = sample[di(rng)];
            string p = w.substr(0, min((int)w.size(), prefixLen));
            (void)getTopK(p, 10);
        }

        auto t0 = chrono::high_resolution_clock::now();
        for (int i = 0; i < numQueries; ++i) {
            auto &w = sample[di(rng)];
            string p = w.substr(0, min((int)w.size(), prefixLen));
            (void)getTopK(p, 10);
        }
        auto t1 = chrono::high_resolution_clock::now();
        double sec = chrono::duration<double>(t1 - t0).count();
        cout<< "Ran " << numQueries << " queries in " << sec << "s (avg " << (sec*1e6/numQueries) << " us/query)\n";
    }

    // Debug helper (optional)
    void debugStats() const {
        size_t nodes = countNodes(root);
        cout << "Dict size: " << dict.size() << ", nodes in trie: " << nodes << "\n";
    }

private:
    TrieNode* root;
    int perNodeK;

    // central dictionary & frequencies
    vector<string> dict;                  // index -> word
    vector<long long> freqs;              // index -> freq
    vector<char> wordSeen;                // index -> whether word was loaded/added from file (for save)
    unordered_map<string,int> wordToIndex;// word -> index

    int ensureWordIndex(const string &word) {
        auto it = wordToIndex.find(word);
        if (it != wordToIndex.end()) return it->second;
        int idx = dict.size();
        dict.push_back(word);
        freqs.push_back(0);
        wordSeen.push_back(0);
        wordToIndex[word] = idx;
        return idx;
    }

    // Insert node path and set node->wordIndex
    void insertIntoTrie(const string &word, int idx) {
        TrieNode* node = root;
        for (char c : word) {
            if (!node->children.count(c)) node->children[c] = new TrieNode();
            node = node->children[c];
            // maintain topK at each node (new index may be pushed later via updateTopKForWord)
        }
        node->wordIndex = idx;
        // update per-node topK along path because we loaded initial freq
        updateTopKForWord(word, idx);
        wordSeen[idx] = true;
    }

    // Remove index from topK lists along path
    void removeIndexFromPath(const string &word, int idx) {
        TrieNode* node = root;
        for (char c : word) {
            if (!node->children.count(c)) return;
            node = node->children[c];
            auto &v = node->topK;
            auto it = find(v.begin(), v.end(), idx);
            if (it != v.end()) v.erase(it);
        }
    }

    // Update (or insert) index into per-node topK lists along the word path.
    // This is the core maintenance function. It uses simple vector insertion/removal
    // but perNodeK is small (default 10) so it's fast.
    void updateTopKForWord(const string &word, int idx) {
        RankHelper rh(freqs, dict);
        TrieNode* node = root;
        for (char c : word) {
            if (!node->children.count(c)) node->children[c] = new TrieNode();
            node = node->children[c];
            auto &v = node->topK;
            auto it = find(v.begin(), v.end(), idx);
            if (it == v.end()) {
                // insert
                v.push_back(idx);
            }
            // reorder v according to ranking and trim
            sort(v.begin(), v.end(), [&](int a, int b){ return rh.higher(a,b); });
            if ((int)v.size() > perNodeK) v.resize(perNodeK);
        }
        // set wordIndex at end node
        if (node) node->wordIndex = idx;
    }

    size_t countNodes(const TrieNode* n) const {
        if (!n) return 0;
        size_t cnt = 1;
        for (auto &p : n->children) cnt += countNodes(p.second);
        return cnt;
    }
};

// ---------------- interactive CLI ----------------
void printHelp() {
    cout << "Commands:\n";
    cout << "  suggest <prefix> <K>          : show top-K suggestions for prefix\n";
    cout << "  add <word> <freq>             : add word (or increment by freq)\n";
    cout << "  update <word> <freq>          : set word frequency to freq\n";
    cout << "  remove <word>                 : remove word\n";
    cout << "  save                          : save current keywords to file (overwrite)\n";
    cout << "  benchmark <num> <prefix_len>  : run quick benchmark\n";
    cout << "  stats                         : print debug stats (dict size, nodes)\n";
    cout << "  help                          : show this help\n";
    cout << "  exit                          : quit\n";
}

static void trim(string &s) {
    while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
    while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string filename = "keywords.txt";
    if (argc >= 2) filename = argv[1];

    AutocompleteEngine engine(12); // keep per-node top 12 by default
    bool loaded = engine.loadFromFile(filename);
    if (!loaded) {
        cerr << "Warning: could not open '" << filename << "' — starting with empty dataset.\n";
    } else {
        cerr << "Loaded keywords from '" << filename << "'.\n";
    }

    cout << "Autocomplete interactive (per-node Top-K). Type 'help' for commands.\n";
    cout << "Note: 'add' will append new words to the original file automatically.\n";

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break;
        trim(line);
        if (line.empty()) continue;

        stringstream ss(line);
        string cmd;
        ss >> cmd;
        if (cmd == "exit") break;
        else if (cmd == "help") printHelp();
        else if (cmd == "suggest") {
            string prefix;
            int K = 5;
            ss >> prefix >> K;
            if (prefix.empty()) { cout << "Usage: suggest <prefix> <K>\n"; continue; }
            auto res = engine.getTopK(prefix, K);
            if (res.empty()) { cout << "(no suggestions)\n"; continue; }
            for (auto &pr : res) cout << pr.first << " (" << pr.second << ")\n";
        } else if (cmd == "add") {
            string w; long long f = 1;
            ss >> w >> f;
            if (w.empty()) { cout << "Usage: add <word> <freq>\n"; continue; }
            engine.insert(w, f, filename); // append new words to file
            cout << "Added/incremented '" << w << "' by " << f << ".\n";
        } else if (cmd == "update") {
            string w; long long f;
            ss >> w >> f;
            if (w.empty()) { cout << "Usage: update <word> <freq>\n"; continue; }
            engine.updateFrequency(w, f);
            cout << "Updated '" << w << "' to freq " << f << ".\n";
        } else if (cmd == "remove") {
            string w; ss >> w;
            if (w.empty()) { cout << "Usage: remove <word>\n"; continue; }
            engine.remove(w);
            cout << "Removed (or marked non-word) '" << w << "'.\n";
        } else if (cmd == "save") {
            if (engine.saveToFile(filename)) cout << "Saved to " << filename << "\n";
            else cout << "Failed to save to " << filename << "\n";
        } else if (cmd == "benchmark") {
            int num = 10000, plen = 3;
            ss >> num >> plen;
            engine.benchmark(num, plen);
        } else if (cmd == "stats") {
            engine.debugStats();
        } else {
            cout << "Unknown command. Type 'help' for usage.\n";
        }
    }

    cout << "Exiting. Goodbye.\n";
    return 0;
}
