#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
using namespace std;

// Node structure for Huffman tree
struct Node {
    char character;
    int freq;
    Node *left, *right;

    Node(char c, int f) : character(c), freq(f), left(nullptr), right(nullptr) {}
};

// Comparison function for priority queue
struct Compare {
    bool operator()(Node* left, Node* right) {
        return left->freq > right->freq;
    }
};

// Function to build Huffman tree
Node* buildHuffmanTree(char data[], int freq[], int size) {
    priority_queue<Node*, vector<Node*>, Compare> minHeap;

    for (int i = 0; i < size; ++i) {
        minHeap.push(new Node(data[i], freq[i]));
    }

    while (minHeap.size() > 1) {
        Node* left = minHeap.top(); minHeap.pop();
        Node* right = minHeap.top(); minHeap.pop();

        Node* top = new Node('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        minHeap.push(top);
    }

    return minHeap.top();
}

// Function to generate Huffman codes and store them in a map
void generateCodes(Node* root, string code, map<char, string>& codes) {
    if (root == nullptr) return;

    if (!root->left && !root->right) {
        codes[root->character] = code;
    }

    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}

// Function to encode input text using Huffman codes
void encodeText(ifstream& inFile, ofstream& outFile, map<char, string>& codes) {
    char ch;
    string encodedText = "";

    while (inFile.get(ch)) {
        encodedText += codes[ch];
    }

    int padding = 8 - (encodedText.length() % 8);
    encodedText.append(padding, '0'); // Adding padding bits

    for (int i = 0; i < encodedText.length(); i += 8) {
        string byte = encodedText.substr(i, 8);
        char c = static_cast<char>(stoi(byte, nullptr, 2));
        outFile.put(c);
    }
}

// Function to decode Huffman encoded text
void decodeText(ifstream& inFile, ofstream& outFile, Node* root, int originalSize) {
    Node* current = root;
    char bit;
    int decodedSize = 0;

    while (decodedSize < originalSize) {
        inFile.get(bit);

        for (int i = 7; i >= 0; --i) {
            int b = (bit >> i) & 1;

            if (b == 0) current = current->left;
            else current = current->right;

            if (current->left == nullptr && current->right == nullptr) {
                outFile.put(current->character);
                current = root;
                decodedSize++;
                if (decodedSize == originalSize) break;
            }
        }
    }
}

int main() {
    // Example input frequencies and data
    char data[] = { 'a', 'b', 'c', 'd', 'e', 'f' };
    int freq[] = { 5, 9, 12, 13, 16, 45 };
    int size = sizeof(data) / sizeof(data[0]);

    // Build Huffman tree
    Node* root = buildHuffmanTree(data, freq, size);

    // Generate Huffman codes
    map<char, string> codes;
    generateCodes(root, "", codes);

    // Open input and output files
    ifstream inFile("input.txt");
    ofstream compressedFile("compressed.txt", ios::binary);

    // Encode input file
    encodeText(inFile, compressedFile, codes);

    // Close input and output files
    inFile.close();
    compressedFile.close();

    // Open compressed file for decompression
    ifstream compressedInput("compressed.txt", ios::binary);
    ofstream decompressedOutput("decompressed.txt");

    // Decode compressed file
    decodeText(compressedInput, decompressedOutput, root, 100); // Assuming original file size is 100 characters

    // Close decompressed file
    compressedInput.close();
    decompressedOutput.close();

    return 0;
}

