#include <iostream>
#include <dlfcn.h>
#include <string>

int main() {
    std::cout << "Plugin test loader" << std::endl;
    
    // Try to load the plugin
    std::string plugin_path = "/home/aditya/cppinterop/xeus-cpp/build/libfile_magic.so";
    std::cout << "Loading: " << plugin_path << std::endl;
    
    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load plugin: " << dlerror() << std::endl;
        return 1;
    }
    
    std::cout << "Successfully loaded library, looking for create_plugin..." << std::endl;
    
    // Look up the create_plugin function
    using create_plugin_t = void*(*)();
    auto create_plugin = reinterpret_cast<create_plugin_t>(dlsym(handle, "create_plugin"));
    
    if (!create_plugin) {
        std::cerr << "Failed to find create_plugin function: " << dlerror() << std::endl;
        dlclose(handle);
        return 1;
    }
    
    std::cout << "Found create_plugin function!" << std::endl;
    
    // Call the function
    void* plugin = create_plugin();
    if (!plugin) {
        std::cerr << "Plugin creation failed." << std::endl;
        dlclose(handle);
        return 1;
    }
    
    std::cout << "Successfully created plugin object!" << std::endl;
    
    // Clean up
    dlclose(handle);
    std::cout << "Test completed successfully" << std::endl;
    
    return 0;
}
