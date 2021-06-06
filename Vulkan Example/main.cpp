#include "VulkanInterface.hpp"

// std
#include <iostream>
#include <stdexcept>
#include <cstdlib>


using namespace vulkanExample;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;





int WinMain() {
    VulkanInterface app(WIDTH, HEIGHT);
    try
    {
        //runs app with defined size
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int main() { return WinMain(); } //To allow switching between Windows and Console (debug)