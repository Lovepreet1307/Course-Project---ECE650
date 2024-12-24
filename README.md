# ECE 650 - Methods and Tools for Software Engineering

This repository contains assignments and the final project for the ECE 650 course.

---

## Assignments

### Assignment 1: Graph Construction
**Objective**: Develop a Python script to create undirected graphs based on street inputs and generate a vertex-edge representation.  
**Features**:
- Accepts commands to add, modify, or remove streets.
- Detects and handles intersections to form graph vertices.
- Outputs graph in a specific format upon request (`gg` command).

**Files**: `a1ece650.py`  
**Submission Directory**: `a1`

---

### Assignment 2: Shortest Path in Graphs
**Objective**: Implement a C++ program to process graph specifications and compute the shortest path between two vertices using BFS or Bellman-Ford.  
**Features**:
- Reads vertex (`V`) and edge (`E`) inputs.
- Outputs shortest path or error if unreachable.

**Files**: `ece650-a2.cpp`, `CMakeLists.txt`  
**Submission Directory**: `a2`

---

### Assignment 3: Inter-Process Communication
**Objective**: Link a Python script (Assignment 1) with C++ code (Assignment 2) using Inter-Process Communication (IPC).  
**Features**:
- Random input generation (`rgen`).
- Driver program to automate input-output linking.
- Enhanced error handling.

**Files**: `rgen.cpp`, `ece650-a3.cpp`, `CMakeLists.txt`  
**Submission Directory**: `a3`

---

### Assignment 4: Vertex Cover via SAT
**Objective**: Solve the Minimum Vertex Cover problem using SAT reduction.  
**Features**:
- Polynomial-time reduction to CNF-SAT.
- Uses MiniSat solver for optimal vertex cover computation.

**Files**: `ece650-a4.cpp`, `CMakeLists.txt`  
**Submission Directory**: `a4`

---

## Final Project: Comparative Analysis of Vertex Cover Algorithms
**Objective**: Extend Assignment 4 by implementing and analyzing two additional approximation algorithms for vertex cover.  
**Features**:
- Multi-threaded implementation.
- Approximation ratio and runtime analysis.
- Report generation with performance graphs.

**Files**: `ece650-prj.cpp`, `CMakeLists.txt`, `report.pdf`  
**Submission Directory**: `project`

