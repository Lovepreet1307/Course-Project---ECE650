#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <random>

// Renamed structures with new variable names
struct Coordinate {
    int row, col;
};

struct Edge {
    Coordinate from;
    Coordinate to;
};

struct Path {
    std::string identifier;
    std::vector<Edge> segments;
};

// Global variables (maintained as per original)
int counter_Print = 0;
std::vector<std::string> street_Names;

class StreetGenerator {
private:
    int argument_street_number;
    int argument_line_segments;
    int argument_coordinate;
    std::ifstream urandom;

    unsigned int getRandomNumber() {
        unsigned int result = 42;
        urandom.read((char*)&result, sizeof(int));
        return result;
    }

    std::string generateStreetName() {
        char alphabets[26] = {'A','B','C','D','E','F','G','H','I','J','K','L',
                             'M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
        char street_name[10];
        int j = 0;
        while(j < 10) {
            unsigned int alphabet = getRandomNumber() % 26;
            street_name[j] = alphabets[alphabet];
            j++;
        }
        std::string final_name = "";
        for(int i = 0; i < 10; i++) {
            final_name = final_name + street_name[i];
        }
        return final_name;
    }

    Coordinate generateCoordinate() {
        Coordinate p;
        unsigned int x_coordinate = getRandomNumber();
        unsigned int y_coordinate = getRandomNumber();
        p.row = (x_coordinate % (2 * argument_coordinate + 1)) - argument_coordinate;
        p.col = (y_coordinate % (2 * argument_coordinate + 1)) - argument_coordinate;
        return p;
    }

    bool isPointOnSegment(const Coordinate& coord, const Edge& edge) {
        if ((coord.row >= std::min(edge.from.row, edge.to.row) && 
             coord.row <= std::max(edge.from.row, edge.to.row)) &&
            (coord.col >= std::min(edge.from.col, edge.to.col) && 
             coord.col <= std::max(edge.from.col, edge.to.col)) &&
            (coord.row - edge.from.row) * (edge.to.col - edge.from.col) == 
            (coord.col - edge.from.col) * (edge.to.row - edge.from.row)) {
            return true;
        }
        return false;
    }

public:
    StreetGenerator(int s_num, int l_seg, int coord_range) 
        : argument_street_number(s_num), argument_line_segments(l_seg), 
          argument_coordinate(coord_range) {
        urandom.open("/dev/urandom");
        if (urandom.fail()) {
            throw std::runtime_error("Cannot open /dev/urandom");
        }
    }

    ~StreetGenerator() {
        if (urandom.is_open()) {
            urandom.close();
        }
    }

    std::vector<Path> generateStreets() {
        std::vector<Path> paths;
        std::vector<Edge> edges;
        
        unsigned int street_Number = getRandomNumber();
        street_Number = (street_Number % (argument_street_number-1)) + 2;

        for(size_t i = 1; i <= street_Number; i++) {
            unsigned int line_segments = getRandomNumber();
            line_segments = (line_segments % argument_line_segments) + 1;

            for(size_t j = 1; j <= line_segments; j++) {
                if(edges.empty()) {
                    Edge e;
                    e.from = generateCoordinate();
                    e.to = generateCoordinate();
                    edges.push_back(e);
                } else {
                    Edge e;
                    e.from = edges.back().to;
                    e.to = generateCoordinate();
                    edges.push_back(e);
                }
            }

            Path path;
            path.identifier = generateStreetName();
            path.segments = edges;
            paths.push_back(path);
            edges.clear();
        }
        return paths;
    }

    bool checkStreets(const std::vector<Path>& paths) {
        for (size_t i = 0; i < paths.size(); ++i) {
            const Path& p1 = paths[i];
            
            // Check individual path segments
            for(size_t k = 0; k < p1.segments.size(); ++k) {
                if((p1.segments[k].from.row == p1.segments[k].to.row) && 
                   (p1.segments[k].from.col == p1.segments[k].to.col)) {
                    return false;
                }
            }

            // Check segments within same path
            for(size_t k = 0; k < p1.segments.size(); ++k) {
                for(size_t l = k+1; l < p1.segments.size(); ++l) {
                    // Complete overlap checks
                    if (isPointOnSegment(p1.segments[l].from, p1.segments[k]) && 
                        isPointOnSegment(p1.segments[l].to, p1.segments[k])) {
                        return false;
                    }
                    if (isPointOnSegment(p1.segments[k].from, p1.segments[l]) && 
                        isPointOnSegment(p1.segments[k].to, p1.segments[l])) {
                        return false;
                    }
                    
                    // Partial overlap checks
                    if((p1.segments[k].from.row != p1.segments[l].from.row) && 
                       (p1.segments[k].from.row != p1.segments[l].to.row) && 
                       (p1.segments[k].to.row != p1.segments[l].from.row) && 
                       (p1.segments[k].to.row != p1.segments[l].to.row)) {
                        if ((isPointOnSegment(p1.segments[l].from, p1.segments[k]) || 
                             isPointOnSegment(p1.segments[l].to, p1.segments[k])) && 
                            (isPointOnSegment(p1.segments[k].from, p1.segments[l]) || 
                             isPointOnSegment(p1.segments[k].to, p1.segments[l]))) {
                            return false;
                        }
                    }

                    // Check for identical segments
                    if((p1.segments[k].from.row == p1.segments[l].from.row && 
                        p1.segments[k].from.col == p1.segments[l].from.col) && 
                       (p1.segments[k].to.row == p1.segments[l].to.row && 
                        p1.segments[k].to.col == p1.segments[l].to.col)) {
                        return false;
                    }
                    if((p1.segments[k].from.row == p1.segments[l].to.row && 
                        p1.segments[k].from.col == p1.segments[l].to.col) && 
                       (p1.segments[k].to.row == p1.segments[l].from.row && 
                        p1.segments[k].to.col == p1.segments[l].from.col)) {
                        return false;
                    }
                }
            }

            // Check segments between different paths
            for(size_t j = i+1; j < paths.size(); ++j) {
                const Path& p2 = paths[j];
                for(size_t k = 0; k < p1.segments.size(); ++k) {
                    for(size_t l = 0; l < p2.segments.size(); ++l) {
                        // Complete overlap checks
                        if (isPointOnSegment(p2.segments[l].from, p1.segments[k]) && 
                            isPointOnSegment(p2.segments[l].to, p1.segments[k])) {
                            return false;
                        }
                        if (isPointOnSegment(p1.segments[k].from, p2.segments[l]) && 
                            isPointOnSegment(p1.segments[k].to, p2.segments[l])) {
                            return false;
                        }

                        // Partial overlap checks
                        if((p1.segments[k].from.row != p2.segments[l].from.row) && 
                           (p1.segments[k].from.row != p2.segments[l].to.row) && 
                           (p1.segments[k].to.row != p2.segments[l].from.row) && 
                           (p1.segments[k].to.row != p2.segments[l].to.row)) {
                            if ((isPointOnSegment(p2.segments[l].from, p1.segments[k]) || 
                                 isPointOnSegment(p2.segments[l].to, p1.segments[k])) && 
                                (isPointOnSegment(p1.segments[k].from, p2.segments[l]) || 
                                 isPointOnSegment(p1.segments[k].to, p2.segments[l]))) {
                                return false;
                            }
                        }

                        // Check for identical segments between different paths
                        if((p1.segments[k].from.row == p2.segments[l].from.row && 
                            p1.segments[k].from.col == p2.segments[l].from.col) && 
                           (p1.segments[k].to.row == p2.segments[l].to.row && 
                            p1.segments[k].to.col == p2.segments[l].to.col)) {
                            return false;
                        }
                        if((p1.segments[k].from.row == p2.segments[l].to.row && 
                            p1.segments[k].from.col == p2.segments[l].to.col) && 
                           (p1.segments[k].to.row == p2.segments[l].from.row && 
                            p1.segments[k].to.col == p2.segments[l].from.col)) {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }
};

void printStreets(const std::vector<Path>& paths) {
    std::ostringstream output;
    
    if (counter_Print > 0) {
        for (const auto& street_name : street_Names) {
            output << "rm \"" << street_name << "\"\n";
        }
    }
    
    street_Names.clear();
    
    for (const auto& path : paths) {
        output << "add \"" << path.identifier << "\" ";
        
        for (size_t i = 0; i < path.segments.size(); ++i) {
            const auto& edge = path.segments[i];
            output << "(" << edge.from.row << "," << edge.from.col << ") ";
            if (i == path.segments.size() - 1) {
                output << "(" << edge.to.row << "," << edge.to.col << ")\n";
            }
        }
        street_Names.push_back(path.identifier);
    }
    
    std::cout << output.str() << "gg\n";
    std::cout.flush();
    ++counter_Print;
}

class CommandLineParser {
public:
    struct Arguments {
        int street_number = 10;
        int line_segments = 5;
        int wait_time = 5;
        int coordinate = 20;
    };

    static Arguments parse(int argc, char* argv[]) {
        Arguments args;
        
        for (int i = 1; i < argc; ++i) {
            if (i + 1 >= argc) break;
            
            if (strcmp(argv[i], "-s") == 0) {
                // First, check if the value is an integer
                std::string value = argv[i + 1];
                bool isInteger = !value.empty() && 
                    (std::isdigit(value[0]) || value[0] == '-' || value[0] == '+') && 
                    std::all_of(value.begin() + (value[0] == '-' || value[0] == '+'), 
                               value.end(), ::isdigit);
                if (!isInteger) {
                    std::cerr << "Error: Non-integer command line argument for -s\n";
                    exit(1);
                }
                // Convert to integer after validation
                args.street_number = std::stoi(value);
                // Check if it meets the minimum value requirement
                if (args.street_number < 2) {
                    std::cerr << "Error: Argument for -s must be at least 2\n";
                    exit(1);
                }
            } else if (strcmp(argv[i], "-n") == 0) {
                // First, check if the value is an integer
                std::string value = argv[i + 1];
                bool isInteger = !value.empty() && 
                    (std::isdigit(value[0]) || value[0] == '-' || value[0] == '+') && 
                    std::all_of(value.begin() + (value[0] == '-' || value[0] == '+'), 
                               value.end(), ::isdigit);
                if (!isInteger) {
                    std::cerr << "Error: Non-integer command line argument for -n\n";
                    exit(1);
                }
                // Convert to integer after validation
                args.line_segments = std::stoi(value);
                // Check if it meets the minimum value requirement
                if (args.line_segments < 1) {
                    std::cerr << "Error: Argument for -n must be at least 1\n";
                    exit(1);
                }
            } else if (strcmp(argv[i], "-l") == 0) {
                // First, check if the value is an integer
                std::string value = argv[i + 1];
                bool isInteger = !value.empty() && 
                    (std::isdigit(value[0]) || value[0] == '-' || value[0] == '+') && 
                    std::all_of(value.begin() + (value[0] == '-' || value[0] == '+'), 
                               value.end(), ::isdigit);
                if (!isInteger) {
                    std::cerr << "Error: Non-integer command line argument for -l\n";
                    exit(1);
                }
                // Convert to integer after validation
                args.wait_time = std::stoi(value);
                // Check if it meets the minimum value requirement
                if (args.wait_time < 5) {
                    std::cerr << "Error: Argument for -l must be at least 5\n";
                    exit(1);
                }
            } else if (strcmp(argv[i], "-c") == 0) {
                // First, check if the value is an integer
                std::string value = argv[i + 1];
                bool isInteger = !value.empty() && 
                    (std::isdigit(value[0]) || value[0] == '-' || value[0] == '+') && 
                    std::all_of(value.begin() + (value[0] == '-' || value[0] == '+'), 
                               value.end(), ::isdigit);
                if (!isInteger) {
                    std::cerr << "Error: Non-integer command line argument for -c\n";
                    exit(1);
                }
                // Convert to integer after validation
                args.coordinate = std::stoi(value);
                // Check if it meets the minimum value requirement
                if (args.coordinate < 1) {
                    std::cerr << "Error: Argument for -c must be at least 1\n";
                    exit(1);
                }
            }
        }
        return args;
    }
};

int main(int argc, char* argv[]) {
    auto args = CommandLineParser::parse(argc, argv);
    
    std::ifstream urandom("/dev/urandom");
    if (urandom.fail()) {
        std::cerr << "Error: cannot open /dev/urandom\n";
        return 1;
    }

    unsigned int wait_time = 42;
    urandom.read(reinterpret_cast<char*>(&wait_time), sizeof(int));
    wait_time = (wait_time % (args.wait_time - 4)) + 5;

    StreetGenerator generator(args.street_number, args.line_segments, args.coordinate);

    while (true) {
        bool success = false;
        int attempts = 0;
        
        while (!success && attempts < 25) {
            auto streets = generator.generateStreets();
            if (generator.checkStreets(streets)) {
                printStreets(streets);
                success = true;
            }
            ++attempts;
        }
        
        if (!success) {
            std::cerr << "Error: failed to generate valid input for 25 simultaneous attempts\n";
            return 0;
        }
        
        sleep(wait_time);
    }

    return 0;
}