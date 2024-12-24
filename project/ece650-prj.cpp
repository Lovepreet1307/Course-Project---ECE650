#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <queue>
#include <list>
#include <memory>
#include <algorithm>
#include <time.h>
#include <pthread.h>
#include <map>
#include <atomic>
#include <numeric>
#include <chrono>
#include "minisat/core/Solver.h"
#include "minisat/core/SolverTypes.h"
using namespace Minisat;

struct ThreadArgs
{
    int v;
    std::vector<std::pair<int, int>> pairVector;
};

void *findVectorCoverWrapper(void *arg);
void *findVectorCover_1Wrapper(void *arg);
void *findVectorCover_2Wrapper(void *arg);
void *ioHandlerWrapper(void *arg);
// double calculateMean(const std::vector<double> &ratios);
// double calculateStandardDeviation(const std::vector<double> &ratios, double mean);

void findVectorCover(int v, std::vector<std::pair<int, int>> pairVector);
void findVectorCover_1(int v, std::vector<std::pair<int, int>> pairVector);
void findVectorCover_2(int v, std::vector<std::pair<int, int>> pairVector);

std::vector<int> finalResult_Vector_1;
std::vector<int> finalResult_Vector_2;
std::vector<int> finalResult_Vector_3;
int globalOptimalCover;
void print_Vector1_result();
void print_Vector2_result();
void print_Vector3_result();

double duration_1;
double duration_2;
double duration_3;
bool flag = true;
std::atomic<bool> timed_out(false);
std::vector<double> t1_ratios; // Store t1 approximation ratios
std::vector<double> t2_ratios; // Store t2 approximation ratios
std::vector<double> t3_ratios;

std::vector<double> t1_run; // Store t1 approximation ratios
std::vector<double> t2_run; // Store t2 approximation ratios
std::vector<double> t3_run;

std::atomic<double>
    t1_app(0.0);
std::atomic<double> t2_app(0.0);
std::atomic<double> t3_app(0.0);
pthread_t ioThread;
int main(int argc, char **argv)
{
    pthread_create(&ioThread, nullptr, ioHandlerWrapper, nullptr);
    pthread_join(ioThread, nullptr);
    return 0;
}
void *ioHandlerWrapper(void *arg)
{
    std::vector<std::pair<int, int>> pairVector;
    std::atomic<double> t1_sum(0.0);
    std::atomic<double> t2_sum(0.0);
    std::atomic<double> t3_sum(0.0);
    int vertices = 0;
    std::set<int> setVertices;
    std::unordered_map<int, std::list<int>> adjacenyList;
    bool noEdge = false;

    while (!std::cin.eof())
    {
        std::string line;
        std::getline(std::cin, line);
        if (!(line[0] == 's'))
        {
            // std::cout <<line<<"\n";
        }
        if (line.empty() || line.find_first_not_of(" "))
        {
            continue;
        }
        std::istringstream input(line);
        char command;
        input >> command;

        if (command == 'V')
        {
            bool valid_input = true;
            while (!input.eof())
            {
                input >> vertices;
                if (input.fail())
                {
                    valid_input = false;
                    break;
                }
            }

            if (valid_input == false)
            {
                std::cerr << "Error: Invalid input for vertices \n";
                continue;
            }

            if (vertices <= 1)
            {
                vertices = 0;
                continue;
            }
            if (vertices != 5 && vertices != 10 && vertices != 15)
            {
                flag = false;
            }

            pairVector.clear();
            noEdge = false;
        }

        else if (command == 'E')
        {
            if (noEdge == true)
            {
                vertices = 0;
            }
            if (vertices == 0)
            {
                std::cout << "Error: Missing vertices Input \n";
                continue;
            }

            if (pairVector.size() != 0)
            {
                std::cerr << "Error: Missing vertices Input \n";
                vertices = 0;
                continue;
            }

            char edge_start;
            input >> edge_start;
            bool error = false;
            bool sameEdge = false;

            while (!input.eof())
            {
                char pair_bracket_start;
                input >> pair_bracket_start;

                if (pair_bracket_start == '}')
                {
                    noEdge = true;
                    break;
                }

                int vertice_1;
                input >> vertice_1;

                char comma;
                input >> comma;

                int vertice_2;
                input >> vertice_2;

                char pair_bracket_end;
                input >> pair_bracket_end;

                if (!(vertice_1 >= 1 && vertice_1 <= vertices))
                {
                    error = true;
                    break;
                }

                if (!(vertice_2 >= 1 && vertice_2 <= vertices))
                {
                    error = true;
                    break;
                }

                if (vertice_1 == vertice_2)
                {
                    error = true;
                    break;
                }

                if (pairVector.size() > 0)
                {
                    for (const auto &pair : pairVector)
                    {
                        if (pair.first == vertice_1 && pair.second == vertice_2)
                        {
                            sameEdge = true;
                        }
                    }
                }

                pairVector.emplace_back(vertice_1, vertice_2);

                char comma_or_bracket;
                input >> comma_or_bracket;

                if (comma_or_bracket == '}')
                {
                    break;
                }
            }

            if (error == true)
            {
                pairVector.clear();
                vertices = 0;
                std::cerr << "Error: Invalid Edges \n";
                continue;
            }

            if (pairVector.size() > 1)
            {
                for (const auto &pair : pairVector)
                {
                    for (const auto &pair2 : pairVector)
                    {
                        if (pair.first == pair2.second && pair.second == pair2.first)
                        {
                            sameEdge = true;
                        }
                    }
                }
            }

            if (sameEdge == true)
            {
                pairVector.clear();
                vertices = 0;
                std::cerr << "Error: Invalid Edges\n";
                continue;
            }

            adjacenyList.clear();

            for (const auto &pair : pairVector)
            {
                int vertex_1 = pair.first;
                int vertex_2 = pair.second;
                adjacenyList[vertex_1].push_back(vertex_2);
                adjacenyList[vertex_2].push_back(vertex_1);
            }
            ThreadArgs args1 = {vertices, pairVector};
            ThreadArgs args2 = {vertices, pairVector};
            ThreadArgs args3 = {vertices, pairVector};

            pthread_t thread1, thread2, thread3;

            pthread_create(&thread1, nullptr, findVectorCoverWrapper, &args1);
            pthread_create(&thread2, nullptr, findVectorCover_1Wrapper, &args2);
            pthread_create(&thread3, nullptr, findVectorCover_2Wrapper, &args3);

            clockid_t clockid1;
            int pthread_getcpuclockid(pthread_t thread1, clockid_t clockid1);
            struct timespec start1, end1;
            clock_gettime(clockid1, &start1);

            clockid_t clockid2;
            int pthread_getcpuclockid(pthread_t thread2, clockid_t clockid2);
            struct timespec start2, end2;
            clock_gettime(clockid2, &start2);

            clockid_t clockid3;
            int pthread_getcpuclockid(pthread_t thread3, clockid_t clockid3);
            struct timespec start3, end3;
            clock_gettime(clockid3, &start3);

            pthread_join(thread1, nullptr);
            clock_gettime(clockid1, &end1);
            pthread_join(thread2, nullptr);
            clock_gettime(clockid2, &end2);
            pthread_join(thread3, nullptr);
            clock_gettime(clockid3, &end3);

            double t1 = ((end1.tv_sec - start1.tv_sec) * 1e6) + ((end1.tv_nsec - start1.tv_nsec) / 1e3);
            double t2 = ((end2.tv_sec - start2.tv_sec) * 1e6) + ((end2.tv_nsec - start2.tv_nsec) / 1e3);
            double t3 = ((end3.tv_sec - start3.tv_sec) * 1e6) + ((end3.tv_nsec - start3.tv_nsec) / 1e3);
            // t1_run.push_back(t1);
            t2_run.push_back(t2);
            t3_run.push_back(t3);
            // t1_sum = t1_sum + t1;
            t2_sum = t2_sum + t2;
            t3_sum = t3_sum + t3;
            print_Vector1_result();
            print_Vector2_result();
            print_Vector3_result();
            globalOptimalCover = 1;
        }
        else
        {
            std::cerr << "Error: Invalid Command \n";
        }
    }
}

// double calculateMean(const std::vector<double> &ratios)
// {
//     double sum = std::accumulate(ratios.begin(), ratios.end(), 0.0);
//     return sum / ratios.size();
// }
// double calculateStandardDeviation(const std::vector<double> &ratios, double mean)
// {
//     double sum_sq_diff = 0.0;
//     for (double ratio : ratios)
//     {
//         sum_sq_diff += (ratio - mean) * (ratio - mean);
//     }
//     return std::sqrt(sum_sq_diff / ratios.size());
// }
void findVectorCover(int v, std::vector<std::pair<int, int>> pairVector)
{
    auto start_time = std::chrono::steady_clock::now();
    std::set<int> setVertices;
    for (const auto &pair : pairVector)
    {

        setVertices.insert(pair.first);
        setVertices.insert(pair.second);
    }

    std::unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
    for (int k = 1; k <= v; ++k)
    {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
        if (timed_out.load())
        {
            flag = false;
            return;
        }

        if (elapsed_time > 15)
        {
            timed_out.store(true);
            flag = false;
            return;
        }
        if (elapsed_time > 15)
        {
            timed_out.store(true);
            flag = false;
            return;
        }

        std::vector<std::vector<Lit>> literals_2d(v + 1, std::vector<Lit>(k + 1));
        for (int i = 1; i <= v; ++i)
        {
            for (int j = 1; j <= k; ++j)
            {
                Lit lit = mkLit(solver->newVar());
                literals_2d[i][j] = lit;
            }
        }
        for (int vertex = 1; vertex <= k; ++vertex)
        {

            vec<Lit> clause;
            for (int n = 1; n <= v; ++n)
            {
                clause.push(literals_2d[n][vertex]);
            }
            solver->addClause(clause);
            clause.clear();
        }
        for (int n = 1; n <= v; ++n)
        {

            if (k > 1)
            {

                for (int vertex1 = 1; vertex1 <= k; ++vertex1)
                {

                    vec<Lit> clause;
                    for (int vertex2 = vertex1 + 1; vertex2 <= k; ++vertex2)
                    {
                        clause.push(~literals_2d[n][vertex1]);
                        clause.push(~literals_2d[n][vertex2]);
                        solver->addClause(clause);
                        clause.clear();
                    }
                }
            }
        }
        for (int vertex = 1; vertex <= k; ++vertex)
        {

            for (int n1 = 1; n1 <= v; ++n1)
            {

                vec<Lit> clause;
                for (int n2 = n1 + 1; n2 <= v; ++n2)
                {
                    clause.push(~literals_2d[n1][vertex]);
                    clause.push(~literals_2d[n2][vertex]);
                    solver->addClause(clause);
                    clause.clear();
                }
            }
        }
        for (const auto &pair : pairVector)
        {

            int edge1 = pair.first;
            int edge2 = pair.second;
            vec<Lit> clause;

            for (int vertex = 1; vertex <= k; ++vertex)
            {
                clause.push(literals_2d[edge1][vertex]);
            }

            for (int vertex = 1; vertex <= k; ++vertex)
            {
                clause.push(literals_2d[edge2][vertex]);
            }

            solver->addClause(clause);
            clause.clear();
        }

        bool res = solver->solve();
        if (res == 1)
        {
            for (int i = 1; i <= v; ++i)
            {
                for (int j = 1; j <= k; ++j)
                {
                    if (solver->modelValue(literals_2d[i][j]) == l_True)
                    {
                        finalResult_Vector_1.emplace_back(i);
                    }
                }
            }
            break;
        }
        literals_2d.clear();
        solver.reset(new Minisat::Solver());
    }
}
void findVectorCover_1(int v, std::vector<std::pair<int, int>> pairVector)
{
    std::unordered_map<int, std::list<int>> adjacenyList;
    for (const auto &pair : pairVector)
    {

        adjacenyList[pair.first].push_back(pair.second);
        adjacenyList[pair.second].push_back(pair.first);
    }

    while (adjacenyList.size() != 0)
    {
        int vertex_max = 0;
        int count_edges = 0;

        for (const auto &pair : adjacenyList)
        {
            int vertex = pair.first;
            const std::list<int> &neighbors = pair.second;

            if (neighbors.size() > static_cast<size_t>(count_edges))
            {
                vertex_max = vertex;
                count_edges = neighbors.size();
            }
        }

        finalResult_Vector_2.emplace_back(vertex_max);

        std::list<int> &neighbors = adjacenyList[vertex_max];

        for (int value : neighbors)
        {
            if (adjacenyList[value].size() == 1)
            {
                auto it = adjacenyList.find(value);
                if (it != adjacenyList.end())
                {
                    adjacenyList.erase(it);
                }
            }
            else
            {
                adjacenyList[value].remove(vertex_max);
            }
        }

        auto it = adjacenyList.find(vertex_max);
        if (it != adjacenyList.end())
        {
            adjacenyList.erase(it);
        }
    }
}

void findVectorCover_2(int v, std::vector<std::pair<int, int>> pairVector)
{
    while (pairVector.size() != 0)
    {
        int first_vector = pairVector[0].first;
        int second_vector = pairVector[0].second;

        finalResult_Vector_3.emplace_back(first_vector);
        finalResult_Vector_3.emplace_back(second_vector);

        for (auto it = pairVector.begin(); it != pairVector.end();)
        {
            if (it->first == first_vector || it->second == first_vector)
            {
                it = pairVector.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (auto it = pairVector.begin(); it != pairVector.end();)
        {
            if (it->first == second_vector || it->second == second_vector)
            {
                it = pairVector.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}
void *findVectorCoverWrapper(void *args)
{
    ThreadArgs *threadArgs = reinterpret_cast<ThreadArgs *>(args);
    if (flag == true)
    {
        findVectorCover(threadArgs->v, threadArgs->pairVector);
    }
    return nullptr;
}

void *findVectorCover_1Wrapper(void *args)
{
    ThreadArgs *threadArgs = reinterpret_cast<ThreadArgs *>(args);
    findVectorCover_1(threadArgs->v, threadArgs->pairVector);
    return nullptr;
}

void *findVectorCover_2Wrapper(void *args)
{
    ThreadArgs *threadArgs = reinterpret_cast<ThreadArgs *>(args);
    findVectorCover_2(threadArgs->v, threadArgs->pairVector);
    return nullptr;
}

void print_Vector1_result()
{
    if (flag == 0)
    {
        std::cout << "CNF-SAT-VC: timeout";
        std::cout << "\n";
        flag = true;
    }
    else
    {
        std::cout << "CNF-SAT-VC: ";

        std::sort(finalResult_Vector_1.begin(), finalResult_Vector_1.end());
        for (int num : finalResult_Vector_1)
        {
            std::cout << num << " ";
        }
        std::cout << "\n";
        globalOptimalCover = finalResult_Vector_1.size();
        double approx_t1 = static_cast<double>(finalResult_Vector_1.size()) / globalOptimalCover;
        t1_ratios.push_back(approx_t1);
        t1_app = t1_app + approx_t1;
        // std::cout << "the duration time for algo 1 is: " << duration_1 << "\n";
        finalResult_Vector_1.clear();
    }
}

void print_Vector2_result()
{
    std::cout << "APPROX-VC-1: ";
    std::sort(finalResult_Vector_2.begin(), finalResult_Vector_2.end());

    for (int num : finalResult_Vector_2)
    {
        std::cout << num << " ";
    }
    double approx_t2 = static_cast<double>(finalResult_Vector_2.size()) / globalOptimalCover;
    t2_app = t2_app + approx_t2;
    t2_ratios.push_back(approx_t2);
    std::cout << "\n";
    finalResult_Vector_2.clear();
}

void print_Vector3_result()
{
    std::cout << "APPROX-VC-2: ";
    std::sort(finalResult_Vector_3.begin(), finalResult_Vector_3.end());
    // Printing the sorted vector
    for (int num : finalResult_Vector_3)
    {
        std::cout << num << " ";
    }
    std::cout << "\n";
    double approx_t3 = static_cast<double>(finalResult_Vector_3.size()) / globalOptimalCover;
    t3_app = t3_app + approx_t3;
    t3_ratios.push_back(approx_t3);
    finalResult_Vector_3.clear();
}