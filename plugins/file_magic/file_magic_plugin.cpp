#include "file_magic_plugin.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace xcpp_plugins
{
    void file_magic_line::operator()(const std::string& line)
    { 
        std::ofstream debug_file("/tmp/xeus_cpp_debug.log", std::ios_base::app);
        debug_file << "=== Plugin implementation called ===" << std::endl;
        debug_file.close();
        std::cout << "*************** USING PLUGIN IMPLEMENTATION ***************" << std::endl;
        std::istringstream iss(line);
        std::string cmd, operation, filename;
        iss >> cmd >> operation >> filename;
        
        if (filename.empty()) {
            std::cerr << "Usage: %file [load|run] filename" << std::endl;
            return;
        }
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        
        if (operation == "load") {
            std::cout << "Content of " << filename << ":" << std::endl;
            std::cout << content << std::endl;
        }
        else if (operation == "run") {
            std::cout << "Running file " << filename << "..." << std::endl;
            std::cout << "Content to run:\n" << content << std::endl;
        }
        else {
            std::cerr << "Unknown operation: " << operation << std::endl;
            std::cerr << "Available operations: load, run" << std::endl;
        }
    }
}

// Export the C function for plugin creation
extern "C" {
    xcpp::xmagic_plugin* create_plugin() {
        return new xcpp_plugins::file_magic_plugin();
    }
}