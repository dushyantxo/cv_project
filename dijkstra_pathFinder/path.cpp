#include <SFML/Graphics.hpp>
#include "Graph.h"
#include <vector>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int GRID_SIZE = 20;
const int CELL_SIZE = WINDOW_WIDTH / GRID_SIZE;

enum CellState {
    EMPTY,
    OBSTACLE,
    START,
    END,
    PATH
};

void drawGrid(sf::RenderWindow &window, vector<vector<CellState>> &grid, const vector<pair<int, int>> &path) {
    window.clear(sf::Color::White);

    // Draw grid lines
    for (int i = 0; i <= GRID_SIZE; ++i) {
        sf::Vertex verticalLine[] = {
            sf::Vertex(sf::Vector2f(i * CELL_SIZE, 0), sf::Color::Black),
            sf::Vertex(sf::Vector2f(i * CELL_SIZE, WINDOW_HEIGHT), sf::Color::Black)
        };
        window.draw(verticalLine, 2, sf::Lines);

        sf::Vertex horizontalLine[] = {
            sf::Vertex(sf::Vector2f(0, i * CELL_SIZE), sf::Color::Black),
            sf::Vertex(sf::Vector2f(WINDOW_WIDTH, i * CELL_SIZE), sf::Color::Black)
        };
        window.draw(horizontalLine, 2, sf::Lines);
    }

    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
            cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);

            switch (grid[y][x]) {
                case EMPTY: cell.setFillColor(sf::Color::White); break;
                case OBSTACLE: cell.setFillColor(sf::Color::Black); break;
                case START: cell.setFillColor(sf::Color::Green); break;
                case END: cell.setFillColor(sf::Color::Red); break;
                case PATH: cell.setFillColor(sf::Color::Blue); break;
            }

            window.draw(cell);
        }
    }

    for (const auto &p : path) {
        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
        cell.setPosition(p.second * CELL_SIZE, p.first * CELL_SIZE);
        cell.setFillColor(sf::Color::Yellow);
        window.draw(cell);
    }

    window.display();
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Shortest Path Visualization");
    vector<vector<CellState>> grid(GRID_SIZE, vector<CellState>(GRID_SIZE, EMPTY));

    Graph graph(GRID_SIZE * GRID_SIZE);

    bool startSet = false, endSet = false;
    int startX = -1, startY = -1, endX = -1, endY = -1;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                int x = event.mouseButton.x / CELL_SIZE;
                int y = event.mouseButton.y / CELL_SIZE;

                if (event.mouseButton.button == sf::Mouse::Left) {
                    grid[y][x] = OBSTACLE;
                } else if (event.mouseButton.button == sf::Mouse::Right) {
                    if (!startSet) {
                        startX = x;
                        startY = y;
                        grid[y][x] = START;
                        startSet = true;
                    } else if (!endSet) {
                        endX = x;
                        endY = y;
                        grid[y][x] = END;
                        endSet = true;
                    }
                }
            }
        }

        if (startSet && endSet) {
            for (int y = 0; y < GRID_SIZE; ++y) {
                for (int x = 0; x < GRID_SIZE; ++x) {
                    if (grid[y][x] == EMPTY) {
                        int nodeIndex = y * GRID_SIZE + x;
                        if (y > 0 && grid[y - 1][x] != OBSTACLE) {
                            graph.addEdge(nodeIndex, (y - 1) * GRID_SIZE + x, 1);
                        }
                        if (y < GRID_SIZE - 1 && grid[y + 1][x] != OBSTACLE) {
                            graph.addEdge(nodeIndex, (y + 1) * GRID_SIZE + x, 1);
                        }
                        if (x > 0 && grid[y][x - 1] != OBSTACLE) {
                            graph.addEdge(nodeIndex, y * GRID_SIZE + (x - 1), 1);
                        }
                        if (x < GRID_SIZE - 1 && grid[y][x + 1] != OBSTACLE) {
                            graph.addEdge(nodeIndex, y * GRID_SIZE + (x + 1), 1);
                        }
                    }
                }
            }

            vector<int> shortestPath = graph.dijkstra(startY * GRID_SIZE + startX, endY * GRID_SIZE + endX);
            vector<pair<int, int>> pathCoordinates;

            for (int node : shortestPath) {
                int y = node / GRID_SIZE;
                int x = node % GRID_SIZE;
                pathCoordinates.push_back({y, x});
                if (grid[y][x] != START && grid[y][x] != END) {
                    grid[y][x] = PATH;
                }
            }
            drawGrid(window, grid, pathCoordinates);
            window.display();
        } else {
            drawGrid(window, grid, {});
        }
    }

    return 0;
}
