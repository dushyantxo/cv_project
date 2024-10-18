#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;


string normalizeText(const string& str) {
    string result;
    for (char ch : str) {
        if (isalnum(ch) || isspace(ch)) {
            result += tolower(ch);
        }
    }
    return result;
}


string readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return "";
    }

    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return normalizeText(buffer.str());
}


vector<string> splitText(const string& text) {
    vector<string> words;
    stringstream ss(text);
    string word;

    while (ss >> word) {
        words.push_back(word);
    }

    return words;
}


double calculateSimilarity(const vector<string>& words1, const vector<string>& words2) {
    unordered_map<string, int> wordCount1, wordCount2;
    
    for (const auto& word : words1) {
        wordCount1[word]++;
    }
    for (const auto& word : words2) {
        wordCount2[word]++;
    }

    
    double commonWordCount = 0;
    for (const auto& entry : wordCount1) {
        if (wordCount2.find(entry.first) != wordCount2.end()) {
            commonWordCount += min(entry.second, wordCount2[entry.first]);
        }
    }

    
    double totalWordCount = words1.size() + words2.size();

    
    return (2 * commonWordCount / totalWordCount) * 100;
}

int main() {
    string file1, file2;

    
    cout << "Enter the first file name: ";
    cin >> file1;
    cout << "Enter the second file name: ";
    cin >> file2;

   
    string content1 = readFile(file1);
    string content2 = readFile(file2);

    if (content1.empty() || content2.empty()) {
        cerr << "One or both files could not be processed." << endl;
        return 1;
    }

    
    vector<string> words1 = splitText(content1);
    vector<string> words2 = splitText(content2);

    
    double similarity = calculateSimilarity(words1, words2);

   
    cout << "Similarity score between the two files: " << similarity << "%" << endl;

    return 0;
}
