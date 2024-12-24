#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

// Structure to hold pipe file descriptors
struct PipePair {
    int read_fd;
    int write_fd;
    PipePair() : read_fd(-1), write_fd(-1) {}  // Initialize to invalid fd
    PipePair(int r, int w) : read_fd(r), write_fd(w) {}
};

// Global vector to store child PIDs for signal handler
std::vector<pid_t> g_child_pids;

// Signal handler function to clean up child processes
void signal_handler(int signum) {
    // Clean up child processes
    for (pid_t pid : g_child_pids) {
        if (pid > 0) {
            kill(pid, SIGTERM);
        }
    }
    // Exit the program
    exit(signum);
}

// Helper function to safely close a file descriptor
void safeClose(int& fd) {
    if (fd >= 0) {
        close(fd);
        fd = -1;  // Mark as closed
    }
}

// [Rest of the code remains exactly the same...]
// Helper function implementations
PipePair createPipe() {
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        throw std::runtime_error("Failed to create pipe: " + std::string(strerror(errno)));
    }
    return PipePair(pipe_fds[0], pipe_fds[1]);
}

void closePipe(const PipePair& pipe) {
    if (pipe.read_fd >= 0) {
        if (close(pipe.read_fd) == -1) {
            if (errno != EBADF) {  // Only report if it's not already closed
                std::cerr << "Warning: Failed to close read end of pipe: " << strerror(errno) << std::endl;
            }
        }
    }
    if (pipe.write_fd >= 0) {
        if (close(pipe.write_fd) == -1) {
            if (errno != EBADF) {  // Only report if it's not already closed
                std::cerr << "Warning: Failed to close write end of pipe: " << strerror(errno) << std::endl;
            }
        }
    }
}

pid_t createChildProcess() {
    pid_t pid = fork();
    if (pid < 0) {
        throw std::runtime_error("Fork failed: " + std::string(strerror(errno)));
    }
    return pid;
}

void redirectIO(int input_fd, int output_fd) {
    if (input_fd != -1) {
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            throw std::runtime_error("Failed to redirect stdin: " + std::string(strerror(errno)));
        }
        safeClose(input_fd);
    }
    if (output_fd != -1) {
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            throw std::runtime_error("Failed to redirect stdout: " + std::string(strerror(errno)));
        }
        safeClose(output_fd);
    }
}

// Process execution functions
int randomGenerator(int argc, char *argv[]) {
    std::vector<const char*> args;
    args.push_back("./rgen");
    
    // Add other command line arguments
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    args.push_back(nullptr);  // Null terminator

    execv(args[0], const_cast<char* const*>(args.data()));
    std::cerr << "pid: " << getpid() << " Error: failed to execute rgen" << std::endl;
    return 1;
}

int pythonRunner() {
    execlp("python3", "python3", "ece650-a1.py", nullptr);
    std::cerr << "pid: " << getpid() << " Error: failed to execute the Python program" << std::endl;
    return 1;
}

int shortestPath() {
    const char* args[] = {"./ece650-a2", nullptr};
    execv(args[0], const_cast<char* const*>(args));
    std::cerr << "pid: " << getpid() << " Error: failed to execute ece650-a2" << std::endl;
    return 1;
}

bool monitorChildren(const std::vector<pid_t>& pids) {
    for (pid_t pid : pids) {
        if (pid > 0) {
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            
            if (result > 0) {  // Process terminated
                if (WIFEXITED(status)) {
                    int exit_status = WEXITSTATUS(status);
                    if (exit_status != 0) {
                        //std::cerr << "Process " << pid << " exited with status " << exit_status << std::endl;
                        return true;  // Signal that we should terminate
                    }
                } else if (WIFSIGNALED(status)) {
                    //std::cerr << "Process " << pid << " terminated by signal " << WTERMSIG(status) << std::endl;
                    return true;  // Signal that we should terminate
                }
            }
        }
    }
    return false;  // No termination needed
}

void cleanupProcesses(const std::vector<pid_t>& pids) {
    for (pid_t pid : pids) {
        if (pid > 0) {
            kill(pid, SIGTERM);
            int status;
            int retries = 3;
            while (retries > 0) {
                if (waitpid(pid, &status, WNOHANG) == pid) {
                    break;
                }
                usleep(100000);  // 100ms
                retries--;
                if (retries == 0) {
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        // Set up signal handlers
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = signal_handler;
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGINT, &sa, nullptr);
        
        std::vector<pid_t> child_pids;
        PipePair pipe1 = createPipe();
        PipePair pipe2 = createPipe();

        // First child process (Random Generator)
        pid_t pid1 = createChildProcess();
        if (pid1 == 0) {
            signal(SIGTERM, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            
            redirectIO(-1, pipe1.write_fd);
            safeClose(pipe1.read_fd);
            safeClose(pipe2.read_fd);
            safeClose(pipe2.write_fd);
            return randomGenerator(argc, argv);
        }
        child_pids.push_back(pid1);
        g_child_pids.push_back(pid1);

        // Second child process (Python Runner)
        pid_t pid2 = createChildProcess();
        if (pid2 == 0) {
            signal(SIGTERM, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            
            redirectIO(pipe1.read_fd, pipe2.write_fd);
            safeClose(pipe1.write_fd);
            safeClose(pipe2.read_fd);
            return pythonRunner();
        }
        child_pids.push_back(pid2);
        g_child_pids.push_back(pid2);

        // Third child process (Shortest Path)
        pid_t pid3 = createChildProcess();
        if (pid3 == 0) {
            signal(SIGTERM, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            
            redirectIO(pipe2.read_fd, -1);
            safeClose(pipe1.read_fd);
            safeClose(pipe1.write_fd);
            safeClose(pipe2.write_fd);
            return shortestPath();
        }
        child_pids.push_back(pid3);
        g_child_pids.push_back(pid3);

        // Parent process
        safeClose(pipe1.read_fd);
        safeClose(pipe1.write_fd);
        safeClose(pipe2.read_fd);
        
        if (dup2(pipe2.write_fd, STDOUT_FILENO) == -1) {
            throw std::runtime_error("Failed to redirect stdout in parent");
        }
        safeClose(pipe2.write_fd);

        // Process input while monitoring child processes
        std::string input;
        while (!std::cin.eof()) {
            // Check if any child process has terminated
            if (monitorChildren(child_pids)) {
                cleanupProcesses(child_pids);
                return 1;
            }

            // Non-blocking read with shorter timeout
            fd_set readfds;
            struct timeval tv;
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            tv.tv_sec = 0;
            tv.tv_usec = 50000;  // 50ms timeout

            if (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv) > 0) {
                if (std::getline(std::cin, input)) {
                    if (!std::cin.eof()) {
                        std::cout << input << std::endl;
                        std::cout.flush();
                    }
                }
            }
        }

        cleanupProcesses(child_pids);
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cleanupProcesses(g_child_pids);
        return 1;
    }
}