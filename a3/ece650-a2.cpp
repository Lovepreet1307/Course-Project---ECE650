#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <list>

std::vector<int> findShortestPath(const std::unordered_map<int, std::list<int>>& graphAdjList, 
                                 const std::set<int>& graphVertices, 
                                 int startVertex, 
                                 int endVertex);
void displayPath(const std::vector<int>& pathResult);

int main(int argc, char** argv) {
    // Graph data structures
    std::vector<std::pair<int, int>> edgeList;
    int vertexCount = 0;
    std::set<int> graphVertices;
    std::unordered_map<int, std::list<int>> graphAdjList;
    bool isEmptyGraph = false;
      
    while (!std::cin.eof()) {
        std::string inputLine;
        std::getline(std::cin, inputLine);

        if(!(inputLine[0]=='s')){
            std::cout << inputLine << "\n";
        }

        if (inputLine.empty() || inputLine.find_first_not_of(" ")){
            continue;
        }

        std::istringstream inputStream(inputLine);
        char cmd;
        inputStream >> cmd;

        if (cmd == 'V'){
            bool isValidInput = true;

            while(!inputStream.eof()){
                inputStream >> vertexCount;
                if (inputStream.fail()) {
                    isValidInput = false;
                    break;
                }
            }

            if (!isValidInput){
                std::cerr << "Error: Invalid input for vertices \n";
                continue;
            }

            if (vertexCount <= 1){
                vertexCount = 0;
                continue;
            }

            edgeList.clear();
            isEmptyGraph = false;
        }

        else if(cmd == 'E'){
            if(isEmptyGraph){
                vertexCount = 0;
            }
            if (vertexCount == 0){
                std::cout << "Error: Missing vertices Input \n";
                continue;
            }

            if (!edgeList.empty()){
                std::cerr << "Error: Missing vertices Input \n";
                vertexCount = 0;
                continue;
            }

            char bracketStart;
            inputStream >> bracketStart;

            bool hasError = false;
            bool hasDuplicateEdge = false;

            while(!inputStream.eof()){
                char pairStart;
                inputStream >> pairStart;

                if(pairStart == '}'){
                    isEmptyGraph = true;
                    break;
                }

                int vertex1, vertex2;
                char delimiter, pairEnd;
                inputStream >> vertex1;
                inputStream >> delimiter;
                inputStream >> vertex2;
                inputStream >> pairEnd;
                       
                if (vertex1 < 1 || vertex1 > vertexCount || 
                    vertex2 < 1 || vertex2 > vertexCount || 
                    vertex1 == vertex2){
                    hasError = true;
                    break;
                }
            
                // Check for duplicate edges
                if(!edgeList.empty()){
                    for (const auto& edge : edgeList) {
                        if(edge.first == vertex1 && edge.second == vertex2){
                            hasDuplicateEdge = true;
                        }
                    }
                }

                edgeList.emplace_back(vertex1, vertex2);

                char nextChar;
                inputStream >> nextChar;

                if (nextChar == '}'){
                    break;
                }
            }

            if (hasError){
                edgeList.clear();
                vertexCount = 0;
                std::cerr << "Error: Invalid Edges \n";
                continue;
            }

            // Check for bidirectional duplicate edges
            if(edgeList.size() > 1){
                for (const auto& edge1 : edgeList) {
                    for(const auto& edge2 : edgeList){
                        if(edge1.first == edge2.second && edge1.second == edge2.first){
                            hasDuplicateEdge = true;
                        }
                    }
                }
            }

            if (hasDuplicateEdge){
                edgeList.clear();
                vertexCount = 0;
                std::cerr << "Error: Invalid Edges\n";
                continue;
            }

            // Build adjacency list
            graphAdjList.clear();
            for (const auto& edge : edgeList) {
                int v1 = edge.first;
                int v2 = edge.second;
                graphAdjList[v1].push_back(v2);
                graphAdjList[v2].push_back(v1);
            }
        }

        else if (cmd == 's'){
            bool currentlyEmpty = isEmptyGraph;

            if (vertexCount == 0){
                std::cerr << "Error: Missing vertices information \n";
                isEmptyGraph = false;
                continue;
            }

            if (edgeList.empty() && !isEmptyGraph){
                vertexCount = 0;
                std::cerr << "Error: Missing Edges information  \n";
                continue;
            }

            int startVertex;
            inputStream >> startVertex;
            if (inputStream.fail()) {
                std::cerr << "Error: Invalid format of command \n";
                isEmptyGraph = false;
                continue;
            }

            int endVertex;
            inputStream >> endVertex;
            if (inputStream.fail()) {
                std::cerr << "Error: Invalid format of command \n";
                isEmptyGraph = false;
                continue;
            }

            graphVertices.clear();
            for (const auto& edge : edgeList) {
                graphVertices.insert(edge.first);
                graphVertices.insert(edge.second);
            }

            if (startVertex < 1 || startVertex > vertexCount || 
                endVertex < 1 || endVertex > vertexCount || 
                startVertex == endVertex){
                std::cerr << "Error: Vertex input invalid for command s \n";
                continue;
            }

            if(currentlyEmpty){
                std::cerr << "Error: Path does not exist \n";
                vertexCount = 0;
                continue;
            }

            bool isStartValid = false;
            bool isEndValid = false;

            for (const auto& vertex : graphVertices) {
                if (startVertex == vertex) isStartValid = true;
                if (endVertex == vertex) isEndValid = true;
            }

            if (!isStartValid || !isEndValid){
                std::cerr << "Error: Path does not exist \n";
                continue;
            }

            std::vector<int> pathResult = findShortestPath(graphAdjList, graphVertices, startVertex, endVertex);
            displayPath(pathResult);
            std::cout << "\n";
            
            pathResult.clear();
            graphVertices.clear();
        }
        else {
            std::cerr << "Error: Invalid Command \n";
        }
    }
    return 0;
}

std::vector<int> findShortestPath(const std::unordered_map<int, std::list<int>>& graphAdjList, 
                                 const std::set<int>& graphVertices, 
                                 int startVertex, 
                                 int endVertex) {
    
    // Track visited vertices and parent relationships
    std::unordered_set<int> visited;
    std::unordered_map<int, int> parentMap;
    
    // Initialize BFS queue with start vertex
    std::queue<int> bfsQueue;
    bfsQueue.push(startVertex);
    visited.insert(startVertex);
    parentMap[startVertex] = -1;
    
    // Perform BFS
    bool pathFound = false;
    while (!bfsQueue.empty() && !pathFound) {
        int currentVertex = bfsQueue.front();
        bfsQueue.pop();
        
        // Check all neighbors of current vertex
        for (const int& neighbor : graphAdjList.at(currentVertex)) {
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                parentMap[neighbor] = currentVertex;
                bfsQueue.push(neighbor);
                
                // If we found the destination, we can stop BFS
                if (neighbor == endVertex) {
                    pathFound = true;
                    break;
                }
            }
        }
    }
    
    // Reconstruct path if end vertex was reached
    std::vector<int> path;
    if (!pathFound) {
        return path;  // Return empty path if no path exists
    }
    
    // Trace back from end vertex to start vertex
    int currentNode = endVertex;
    while (currentNode != -1) {
        path.push_back(currentNode);
        currentNode = parentMap[currentNode];
    }
    
    return path;
}

void displayPath(const std::vector<int>& pathResult){
    if(pathResult.empty()){
        std::cerr << "Error: Path does not exist";
        return;
    }

    for(int i = pathResult.size()-1; i >= 0; --i){
        std::cout << pathResult[i];
        if (i > 0){
            std::cout << "-";
        }
    }
}