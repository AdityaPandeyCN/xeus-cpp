/************************************************************************************
 * Copyright (c) 2023, xeus-cpp contributors                                        *
 * Copyright (c) 2023, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
 *                                                                                  *
 * Distributed under the terms of the BSD 3-Clause License.                         *
 *                                                                                  *
 * The full license is in the file LICENSE, distributed with this software.         *
 ************************************************************************************/

#ifndef XEUS_CPP_MANAGER_HPP
#define XEUS_CPP_MANAGER_HPP

#include <map>
#include <memory>
#include <regex>
#include <string>
#include <type_traits>
#include <vector>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "xholder.hpp"
#include "xmagics.hpp"
#include "xpreamble.hpp"
#include "xmagic_plugin.hpp"

namespace nl = nlohmann;

namespace xcpp
{
    struct xpreamble_manager
    {
        std::map<std::string, xholder_preamble> preamble;

        template <typename preamble_type>
        void register_preamble(const std::string& name, std::unique_ptr<preamble_type> pre)
        {
            preamble[name] = xholder_preamble(std::move(pre));
        }

        void unregister_preamble(const std::string& name)
        {
            preamble.erase(name);
        }

        xholder_preamble& operator[](const std::string& name)
        {
            return preamble[name];
        }
    };

    class xmagics_manager : public xpreamble
    {
    public:

        using xpreamble::pattern;

        xmagics_manager(const std::string& plugin_dir = "")
        {
            pattern = R"(^(?:\%{2}|\%)(\w+))";
        }

        template <typename xmagic_type>
        void register_magic(const std::string& magic_name, xmagic_type magic)
        {
            auto shared = std::make_shared<xmagic_type>(magic);
            if (std::is_base_of<xmagic_line, xmagic_type>::value)
            {
                m_magic_line[magic_name] = std::dynamic_pointer_cast<xmagic_line>(shared);
            }
            if (std::is_base_of<xmagic_cell, xmagic_type>::value)
            {
                m_magic_cell[magic_name] = std::dynamic_pointer_cast<xmagic_cell>(shared);
            }
        }

        void unregister_magic(const std::string& magic_name)
        {
            m_magic_cell.erase(magic_name);
            m_magic_line.erase(magic_name);
        }

        bool contains(const std::string& magic_name, const xmagic_type type = xmagic_type::cell)
        {
            if (type == xmagic_type::cell)
            {
                return m_magic_cell.find(magic_name) != m_magic_cell.end();
            }
            if (type == xmagic_type::line)
            {
                return m_magic_line.find(magic_name) != m_magic_line.end();
            }
            return false;
        }

        void apply(const std::string& magic_name, const std::string& line, const std::string& cell)
        {
            if (cell.empty())
            {
                std::cerr << "UsageError: %%" << magic_name << " is a cell magic, but the cell body is empty."
                          << std::endl;
                std::cerr << "If you only intend to display %%" << magic_name
                          << " help, please use a double line break to fill in the cell body.";
                if (contains(magic_name, xmagic_type::line))
                {
                    std::cerr << " Did you mean the line magic %" << magic_name << " (single %)?";
                }
                std::cerr << "\n";
                return;
            }
            try
            {
                (*m_magic_cell[magic_name])(line, cell);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Exception occurred. Recovering...\n";
            }
        }

        void apply(const std::string& magic_name, const std::string& line)
        {
            try
            {
                (*m_magic_line[magic_name])(line);
            }
            catch (const std::runtime_error& e)
            {
                std::cerr << e.what() << std::endl;
            }
            catch (const std::logic_error& e)
            {
                std::cerr << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Exception occurred. Recovering...\n";
            }
        }

        void apply(const std::string& code, nl::json& kernel_res) override
        {
            std::regex re_magic_cell(R"(^\%{2}(\w+))");
            std::smatch magic_name;
            if (std::regex_search(code, magic_name, re_magic_cell))
            {
                if (!contains(magic_name.str(1)))
                {
                    std::cerr << "Unknown magic cell function %%" << magic_name[1] << "\n";
                    std::cout << std::flush;
                    kernel_res["status"] = "error";
                    kernel_res["ename"] = "ename";
                    kernel_res["evalue"] = "evalue";
                    kernel_res["traceback"] = nl::json::array();
                    return;
                }
                std::regex re_magic_cell(R"(^\%{2}(\w+(?:\s.*)?)\n((?:.*\n?)*))");
                std::smatch split_code;
                std::regex_search(code, split_code, re_magic_cell);
                apply(magic_name[1], split_code[1], split_code[2]);
                std::cout << std::flush;
                kernel_res["status"] = "ok";
            }

            std::regex re_magic_line(R"(^\%(\w+))");
            if (std::regex_search(code, magic_name, re_magic_line))
            {
                if (!contains(magic_name.str(1), xmagic_type::line))
                {
                    std::cerr << "Unknown magic line function %" << magic_name[1] << "\n";
                    std::cout << std::flush;
                    kernel_res["status"] = "error";
                    kernel_res["ename"] = "ename";
                    kernel_res["evalue"] = "evalue";
                    kernel_res["traceback"] = {};
                    return;
                }
                std::regex re_magic_line(R"(^\%(\w+(?:\s.*)?))");
                std::smatch split_code;
                std::regex_search(code, split_code, re_magic_line);
                apply(magic_name[1], split_code[1]);
                std::cout << std::flush;
                kernel_res["status"] = "ok";
            }
        }

        [[nodiscard]] std::unique_ptr<xpreamble> clone() const override
        {
            return std::make_unique<xmagics_manager>(*this);
        }
        
       void load_plugins(const std::string& plugin_dir)
{
    std::ofstream debug_file("/tmp/xeus_cpp_debug.log", std::ios_base::app);
    debug_file << "=== Plugin loading started ===" << std::endl;
    debug_file << "Plugin directory: " << plugin_dir << std::endl;
    
    if (plugin_dir.empty())
    {
        debug_file << "Plugin directory is empty" << std::endl;
        debug_file.close();
        return;
    }
    
    namespace fs = std::filesystem;
    
    // Helper function to check string endings
    auto ends_with = [](const std::string& str, const std::string& suffix) -> bool {
        if (str.length() < suffix.length())
            return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    };
    
    try {
        // Special case: if plugin_dir is a direct file path, load it directly
        if (fs::is_regular_file(plugin_dir) && 
            (ends_with(plugin_dir, ".so") || ends_with(plugin_dir, ".dll"))) 
        {
            debug_file << "Loading specific plugin file: " << plugin_dir << std::endl;
            // Try to load the shared library directly
            void* handle = dlopen(plugin_dir.c_str(), RTLD_LAZY);
            if (!handle) {
                debug_file << "Failed to load plugin: " << dlerror() << std::endl;
                debug_file.close();
                return;
            }
            
            // Rest of plugin loading code for direct file...
            debug_file << "Successfully loaded library, looking for create_plugin..." << std::endl;
            
            // Look up the factory function
            using create_plugin_t = xmagic_plugin*(*)();
            auto create_plugin = reinterpret_cast<create_plugin_t>(dlsym(handle, "create_plugin"));
            
            if (!create_plugin) {
                debug_file << "Failed to find create_plugin function: " << dlerror() << std::endl;
                dlclose(handle);
                debug_file.close();
                return;
            }
            
            // Create the plugin
            xmagic_plugin* magic = create_plugin();
            if (!magic) {
                debug_file << "Plugin creation failed." << std::endl;
                dlclose(handle);
                debug_file.close();
                return;
            }
            
            std::string name = magic->name();
            debug_file << "Loaded magic plugin: " << name << std::endl;
            
            if (magic->supports_line_magic()) {
                debug_file << "Plugin supports line magic" << std::endl;
                m_magic_line[name] = std::shared_ptr<xmagic_line>(magic->as_line_magic(), 
                    [handle](xmagic_line*) { dlclose(handle); });
            }
            
            if (magic->supports_cell_magic()) {
                debug_file << "Plugin supports cell magic" << std::endl;
                m_magic_cell[name] = std::shared_ptr<xmagic_cell>(magic->as_cell_magic(),
                    [handle](xmagic_cell*) { dlclose(handle); });
            }
            
            // If plugin doesn't support any magic type, close it immediately
            if (!magic->supports_line_magic() && !magic->supports_cell_magic()) {
                debug_file << "Plugin doesn't support any magic type" << std::endl;
                delete magic;
                dlclose(handle);
            } else {
                // Store the handle to keep the library loaded
                m_handles.push_back(handle);
                debug_file << "Plugin successfully registered" << std::endl;
            }
            
            debug_file.close();
            return;
        }
        
        // Normal directory scanning mode
        debug_file << "Scanning for plugins in: " << plugin_dir << std::endl;
        
        // Check if directory exists
        if (!fs::exists(plugin_dir)) {
            debug_file << "Directory does not exist: " << plugin_dir << std::endl;
            debug_file.close();
            return;
        }
        
        if (!fs::is_directory(plugin_dir)) {
            debug_file << "Path is not a directory: " << plugin_dir << std::endl;
            debug_file.close();
            return;
        }
        
        // List all files in the directory
        debug_file << "Files in plugin directory:" << std::endl;
        bool has_files = false;
        for (const auto& entry : fs::directory_iterator(plugin_dir)) {
            has_files = true;
            debug_file << "  " << entry.path().filename().string() << std::endl;
        }
        
        if (!has_files) {
            debug_file << "No files found in directory" << std::endl;
        }
        
        // Then try to load .so files
        bool found_plugins = false;
        for (const auto& entry : fs::directory_iterator(plugin_dir)) {
            // Check for known plugin files by name to avoid extension issues
            std::string filename = entry.path().filename().string();
            bool is_plugin = false;
            
            if (entry.is_regular_file()) {
                if (filename == "libfile_magic.so" || 
                    filename == "file_magic.so" || 
                    entry.path().extension() == ".so") {
                    is_plugin = true;
                }
            }
            
            if (!is_plugin)
                continue;
                
            found_plugins = true;
            debug_file << "Attempting to load: " << entry.path().string() << std::endl;
                
            // Try to load the shared library
            void* handle = dlopen(entry.path().c_str(), RTLD_LAZY);
            if (!handle) {
                debug_file << "Failed to load plugin: " << dlerror() << std::endl;
                continue;
            }
            
            debug_file << "Successfully loaded library, looking for create_plugin..." << std::endl;
            
            // Look up the factory function
            using create_plugin_t = xmagic_plugin*(*)();
            auto create_plugin = reinterpret_cast<create_plugin_t>(dlsym(handle, "create_plugin"));
            
            if (!create_plugin) {
                debug_file << "Failed to find create_plugin function: " << dlerror() << std::endl;
                dlclose(handle);
                continue;
            }
            
            // Create the plugin
            xmagic_plugin* magic = create_plugin();
            if (!magic) {
                debug_file << "Plugin creation failed." << std::endl;
                dlclose(handle);
                continue;
            }
            
            std::string name = magic->name();
            debug_file << "Loaded magic plugin: " << name << std::endl;
            
            if (magic->supports_line_magic()) {
                debug_file << "Plugin supports line magic" << std::endl;
                m_magic_line[name] = std::shared_ptr<xmagic_line>(magic->as_line_magic(), 
                    [handle](xmagic_line*) { dlclose(handle); });
            }
            
            if (magic->supports_cell_magic()) {
                debug_file << "Plugin supports cell magic" << std::endl;
                m_magic_cell[name] = std::shared_ptr<xmagic_cell>(magic->as_cell_magic(),
                    [handle](xmagic_cell*) { dlclose(handle); });
            }
            
            // If plugin doesn't support any magic type, close it immediately
            if (!magic->supports_line_magic() && !magic->supports_cell_magic()) {
                debug_file << "Plugin doesn't support any magic type" << std::endl;
                delete magic;
                dlclose(handle);
            } else {
                // Store the handle to keep the library loaded
                m_handles.push_back(handle);
                debug_file << "Plugin successfully registered" << std::endl;
            }
        }
        
        if (!found_plugins) {
            debug_file << "No .so files found in directory" << std::endl;
        }
    } catch (const std::exception& e) {
        debug_file << "Error loading plugins: " << e.what() << std::endl;
    }
    
    debug_file.close();
}
    private:
        std::map<std::string, std::shared_ptr<xmagic_cell>> m_magic_cell;
        std::map<std::string, std::shared_ptr<xmagic_line>> m_magic_line;
        std::vector<void*> m_handles; // Store library handles
    };
}

#endif