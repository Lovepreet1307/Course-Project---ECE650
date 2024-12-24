#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include <algorithm>
#include <set>

using namespace std;

void handleVertexCommand(string &line, int &vertexCount, vector<pair<int, int>> &edges, unordered_map<int, vector<int>> &adjacencyList);
void handleEdgeCommand(string &line, int vertexCount, vector<pair<int, int>> &edges, unordered_map<int, vector<int>> &adjacencyList);
void handleShortestPathCommand(string &line, int vertexCount, const unordered_map<int, vector<int>> &adjacencyList);
std::vector<int> findShortestPathBFS(const unordered_map<int, vector<int>> &adjacencyList, int start, int end);
void printPathResult(const vector<int> &path);

int main(int argc, char** argv) {
    std::vector<std::pair<int, int>> edges;                  
    std::unordered_map<int, vector<int>> adjacencyList; 
    int vertexCount = 0;

    while (!std::cin.eof()) {
        std::string line;
        std::getline(std::cin, line);

        if (line.empty()) {
            continue;
        }

        std::istringstream input(line);
        char c;
        input >> c;

        switch (c) {
            case 'V':
                handleVertexCommand(line, vertexCount, edges, adjacencyList);
                break;
            case 'E':
                handleEdgeCommand(line, vertexCount, edges, adjacencyList);
                break;
            case 's':
                handleShortestPathCommand(line, vertexCount, adjacencyList);
                break;
            default:
                std::cout << "Error: Unrecognized command. Valid commands are 'V', 'E', or 's'.\n";
                exit(0);
        }
    }

    return 0;
}

void handleVertexCommand(string &line, int &vertexCount, vector<pair<int, int>> &edges, unordered_map<int, vector<int>> &adjacencyList) {
    std::istringstream input(line);
    char c;
    input >> c;

    input >> vertexCount;
    if (input.fail() || vertexCount <= 1) {
        std::cout << "Error: Invalid number of vertices. The vertex count must be greater than 1.\n";
        exit(0);
    }

    edges.clear();
    adjacencyList.clear();
}

void handleEdgeCommand(string &line, int vertexCount, vector<pair<int, int>> &edges, unordered_map<int, vector<int>> &adjacencyList) {
    if (vertexCount == 0) {
        std::cout << "Error: Define vertices first using the 'V' command before adding edges.\n";
        exit(0);
    }

    std::istringstream input(line);
    char c, edgeStart;
    input >> c >> edgeStart;

    if (edgeStart != '{') {
        std::cout << "Error: Edges should start with '{'.\n";
        exit(0);
    }

    edges.clear();
    std::set<pair<int, int>> uniqueEdges;  

    char nextChar;
    input >> nextChar;

    if (nextChar == '}') {
        adjacencyList.clear();
        return; 
    } else {
        input.putback(nextChar); 
    }

    while (!input.eof()) {
        char bracketStart, comma, bracketEnd;
        int v1, v2;

        input >> bracketStart;
        if (bracketStart == '}') {
            break; 
        }
        if (bracketStart != '<') {  
            std::cout << "Error: Each edge should start with '<'.\n";
            exit(0);
        }

        input >> v1 >> comma >> v2 >> bracketEnd;

        if (comma != ',' || bracketEnd != '>') {
            std::cout << "Error: Invalid edge format. Expected format is <v1,v2>.\n";
            exit(0);
        }

        if (!(v1 >= 1 && v1 <= vertexCount && v2 >= 1 && v2 <= vertexCount) || v1 == v2) {
            std::cout << "Error: Invalid edge (" << v1 << "," << v2 << "). Ensure vertices are within range and no self-loops exist.\n";
            exit(0);
        }

        pair<int, int> edge = {min(v1, v2), max(v1, v2)};
        if (uniqueEdges.find(edge) != uniqueEdges.end()) {  
            std::cout << "Error: Duplicate undirected edge detected (" << v1 << "," << v2 << ").\n";
            exit(0);  
        }

        uniqueEdges.insert(edge);
        edges.emplace_back(edge);

        input >> nextChar;
        if (nextChar == '}') {
            break;
        }
        if (nextChar != ',') {  
            std::cout << "Error: Edges should be separated by commas.\n";
            exit(0);
        }
    }

    if (nextChar != '}') {
        std::cout << "Error: Edge list should end with '}'.\n";
        exit(0);
    }

    adjacencyList.clear();
    for (const auto &edge : edges) {
        adjacencyList[edge.first].push_back(edge.second);
        adjacencyList[edge.second].push_back(edge.first);
    }
}


void handleShortestPathCommand(string &line, int vertexCount, const unordered_map<int, vector<int>> &adjacencyList) {
    if (vertexCount == 0 || adjacencyList.empty()) {
        std::cout << "Error: No graph defined. Use 'V' and 'E' commands to create a graph first.\n";
        exit(0);
    }

    std::istringstream input(line);
    char c;
    int source, destination;
    input >> c >> source >> destination;

    if (input.fail()) {
        std::cout << "Error: Invalid format for shortest path command. Use: s <source> <destination>\n";
        exit(0);
    }

    if (source == destination || source < 1 || destination < 1 || source > vertexCount || destination > vertexCount) {
        std::cout <<"Error: Invalid source or destination. They must be different and within the range of defined vertices.\n";
        exit(0);
    }

    vector<int> path = findShortestPathBFS(adjacencyList, source, destination);
    printPathResult(path);
}

std::vector<int> findShortestPathBFS(const unordered_map<int, vector<int>> &adjacencyList, int start, int end) {
    std::unordered_map<int, int> parents; 
    std::unordered_set<int> visited;      
    std::queue<int> nodeQueue;            

    nodeQueue.push(start);
    visited.insert(start);
    parents[start] = -1;

    while (!nodeQueue.empty()) {
        int currentNode = nodeQueue.front();
        nodeQueue.pop();
        if (currentNode == end) {
            std::vector<int> path;
            for (int v = end; v != -1; v = parents[v]) {
                path.push_back(v);
            }
            reverse(path.begin(), path.end());
            return path;
        }
        for (int neighbor : adjacencyList.at(currentNode)) {
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                parents[neighbor] = currentNode;
                nodeQueue.push(neighbor);
            }
        }
    }
    return {};
}

void printPathResult(const vector<int> &path) {
    if (path.empty()) {
        std::cout << "Error: No path exists between the specified vertices.\n";
        exit(0);
    } else {
        for (size_t i = 0; i < path.size(); ++i) {
            std::cout << path[i];
            if (i < path.size() - 1) {
                std::cout << "-";  
            }
        }
        std::cout << "\n";
    }
}