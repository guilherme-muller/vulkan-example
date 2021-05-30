#include "App.hpp"

// std
#include <iostream>
#include <stdexcept>
#include <cstdlib>


using namespace vulkanExample;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

int main() {
    App app;
    try
    {
        //runs app with defined size
        app.run(WIDTH, HEIGHT);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}