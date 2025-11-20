# Dijkstra Pathfinding Visualization

## **Overview**

A simple visual representation of **Dijkstra's shortest path algorithm** using **SFML**. The project allows users to interact with a grid, place obstacles, and visualize the shortest path between two points.

## Features

- Left-click to place obstacles.
- Right-click to set start and end points.
- Visualizes the shortest path in real-time.
- Reset the grid to start over.

## Requirements

- **SFML** library.
- C++11 or higher.

# Sudoku Solver

## **Overview**

A simple implementation of a **Sudoku Solver** in **C++** using the backtracking algorithm. The project provides a way for users to input puzzles and visualize the solving process.

## Features

- Input Sudoku puzzles manually.
- Solves puzzles using the backtracking algorithm.
- Real-time visualization of the solving process.
- Reset functionality to input new puzzles.

## Requirements

- **C++11** or higher.
- **SFML** library (for visualization).
# File Zipper Using Huffman Coding

## **Overview**

A **file compression and decompression tool** built using the **Huffman Coding algorithm** in **C++**. The project compresses text files and decompresses them to their original form.

## Features

- Compresses files using Huffman Coding.
- Decompresses files back to their original content.
- Provides output as `compressed.txt` and `decompressed.txt`.
- Simple and efficient text file compression.
  # C++ Plagiarism Checker

## **OVERVIEW**

A simple **C++ plagiarism checker** that compares two text files to detect plagiarism by analyzing and comparing the frequency of matching words. The program normalizes the text by removing punctuation and converting all words to lowercase before calculating a similarity score based on common word occurrences.

## Features

- Reads two text files and normalizes the content (lowercase, no punctuation).
- Splits the text into individual words.
- Calculates the similarity score based on common word frequency.
- Provides the similarity percentage between the two files.

## Requirements

- **C++11** or higher.

# Social Feed Ranking Engine (C++ | Graphs, Priority Queues)

A simplified implementation of a social-media-style feed ranking algorithm inspired by Meta’s recommendation logic.
## **OVERVIEW**
The engine models users and their relationships using adjacency lists, and computes personalized feeds using:

Engagement scoring

Affinity scoring

Recency scoring

Min-heap based Top-K ranking
Achieves O(N log K) efficiency across thousands of posts.

# Autocomplete Engine (C++ | Trie, Min-Heaps, File I/O)
## **OVERVIEW**

A high-performance autocomplete/search suggestion engine using:

Trie for prefix matching

Per-node Top-K caching (O(|prefix| + K))

Indexed dictionary to reduce memory usage

File-based keyword loading/saving

## CV → Repository Mapping

- Dijkstra Pathfinding Visualizer → /dijkstra_pathFinder
- File Zipper (Huffman Coding) → /filzipper
- Social Feed Ranking Engine → /soc ranker
- Autocomplete Engine → /word serch
