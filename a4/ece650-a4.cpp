#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include <algorithm>
#include <set>
#include <memory>
#include "minisat/core/Solver.h"
#include "minisat/core/SolverTypes.h"

using namespace Minisat;
using namespace std;

void handleVertexCommand(string &line, int &vertexCount, vector<pair<int, int>> &edges, unordered_map<int, vector<int>> &adjacencyList);
void handleEdgeCommand(string &line, int vertexCount, vector<pair<int, int>> &edges, unordered_map<int, vector<int>> &adjacencyList);
void findVertexCover(int v, vector<pair<int, int>> &edges);

int main() {
    vector<pair<int, int>> edges;                  
    unordered_map<int, vector<int>> adjacencyList; 
    int vertexCount = 0;

    while (!cin.eof()) {
        string line;
        getline(cin, line);

        if (line.empty()) {
            continue;
        }

        istringstream input(line);
        char c;
        input >> c;

        switch (c) {
            case 'V':
                handleVertexCommand(line, vertexCount, edges, adjacencyList);
                break;
            case 'E':
                handleEdgeCommand(line, vertexCount, edges, adjacencyList);
                if (!edges.empty() ) {
                    findVertexCover(vertexCount, edges);
                }
                break;
            default:
                cout << "Error: Unrecognized command. Valid commands are 'V', 'E'.\n";
                break;
        }
    }

    return 0;
}

void handleVertexCommand(string &line, int &vertexCount, vector<pair<int, int>> &edges, unordered_map<int, vector<int>> &adjacencyList) {
    istringstream input(line);
    char c;
    input >> c;
    input >> vertexCount;
    if (input.fail() || vertexCount <= 1) {
        cout << "Error: Invalid number of vertices. The vertex count must be greater than 1.\n";
        return;
    }

    edges.clear();
    adjacencyList.clear();
}

void handleEdgeCommand(string &line, int vertexCount, vector<pair<int, int>> &edges, unordered_map<int, vector<int>> &adjacencyList) {
    if (vertexCount == 0) {
        cout << "Error: Define vertices first using the 'V' command before adding edges.\n";
        return;
    }

    istringstream input(line);
    char c, edgeStart;
    input >> c >> edgeStart;

    if (edgeStart != '{') {
        cout << "Error: Edges should start with '{'.\n";
        return;
    }

    edges.clear();
    set<pair<int, int>> uniqueEdges;  

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
            cout << "Error: Each edge should start with '<'.\n";
            return;
        }

        input >> v1 >> comma >> v2 >> bracketEnd;

        if (comma != ',' || bracketEnd != '>') {
            cout << "Error: Invalid edge format. Expected format is <v1,v2>.\n";
            return;
        }

        if (!(v1 >= 1 && v1 <= vertexCount && v2 >= 1 && v2 <= vertexCount) || v1 == v2) {
            cout << "Error: Invalid edge (" << v1 << "," << v2 << "). Ensure vertices are within range and no self-loops exist.\n";
            return;
        }

        pair<int, int> edge = {min(v1, v2), max(v1, v2)};
        if (uniqueEdges.find(edge) != uniqueEdges.end()) {  
            cout << "Error: Duplicate undirected edge detected (" << v1 << "," << v2 << ").\n";
            return;  
        }

        uniqueEdges.insert(edge);
        edges.emplace_back(edge);

        input >> nextChar;
        if (nextChar == '}') {
            break;
        }
        if (nextChar != ',') {  
            cout << "Error: Edges should be separated by commas.\n";
            return;
        }
    }

    if (nextChar != '}') {
        cout << "Error: Edge list should end with '}'.\n";
    }

    adjacencyList.clear();
    for (const auto &edge : edges) {
        adjacencyList[edge.first].push_back(edge.second);
        adjacencyList[edge.second].push_back(edge.first);
    }
}

void findVertexCover(int v, vector<pair<int, int>> &edges) {
    // Create a unique set of vertices from edges
    set<int> setVertices;
    for (const auto& pair : edges) {
        setVertices.insert(pair.first);
        setVertices.insert(pair.second);
    }

    // Create solver instance
    unique_ptr<Solver> solver(new Solver());

    // Try each possible vertex cover size from 1 to v
    for (int k = 1; k <= v; ++k) {
        // Create 2D vector of literals (vertices x positions)
        vector<vector<Lit>> literals_2d(v + 1, vector<Lit>(k + 1));
        
        // Create variables for each vertex-position pair
        for (int i = 1; i <= v; ++i) {
            for (int j = 1; j <= k; ++j) {
                literals_2d[i][j] = mkLit(solver->newVar());
            }
        }

        // 1. At least one vertex is the ith vertex in the cover
        for (int i = 1; i <= k; ++i) {
            vec<Lit> clause;
            for (int n = 1; n <= v; ++n) {
                clause.push(literals_2d[n][i]);
            }
            solver->addClause(clause);
        }

        // 2. No vertex appears twice in the cover
        if (k > 1) {
            for (int n = 1; n <= v; ++n) {
                for (int pos1 = 1; pos1 <= k; ++pos1) {
                    for (int pos2 = pos1 + 1; pos2 <= k; ++pos2) {
                        vec<Lit> clause;
                        clause.push(~literals_2d[n][pos1]);
                        clause.push(~literals_2d[n][pos2]);
                        solver->addClause(clause);
                    }
                }
            }
        }

        // 3. No position in the cover contains two vertices
        for (int pos = 1; pos <= k; ++pos) {
            for (int v1 = 1; v1 <= v; ++v1) {
                for (int v2 = v1 + 1; v2 <= v; ++v2) {
                    vec<Lit> clause;
                    clause.push(~literals_2d[v1][pos]);
                    clause.push(~literals_2d[v2][pos]);
                    solver->addClause(clause);
                }
            }
        }

        // 4. Every edge is incident to at least one vertex in the cover
        for (const auto& edge : edges) {
            vec<Lit> clause;
            for (int pos = 1; pos <= k; ++pos) {
                clause.push(literals_2d[edge.first][pos]);
                clause.push(literals_2d[edge.second][pos]);
            }
            solver->addClause(clause);
        }

        // Solve and output if satisfiable
        if (solver->solve()) {
            vector<int> cover;
            for (int i = 1; i <= v; ++i) {
                for (int j = 1; j <= k; ++j) {
                    if (solver->modelValue(literals_2d[i][j]) == l_True) {
                        cover.push_back(i);
                        break;
                    }
                }
            }
            
            // Sort vertices in ascending order
            sort(cover.begin(), cover.end());
            
            // Output the vertex cover
            for (size_t i = 0; i < cover.size(); ++i) {
                cout << cover[i];
                if (i < cover.size() - 1) {
                    cout << " ";
                }
            }
            cout << endl;
            return;
        }

        // Reset solver for next iteration
        solver.reset(new Solver());
    }
}