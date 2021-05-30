#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace vulkanExample
{
	class App
	{
	public:
		void run(const uint32_t width, const uint32_t height);


	private:
		GLFWwindow* window;
		VkInstance instance;

		void initWindow(const uint32_t width, const uint32_t height);
		void initVulkan();
		void mainLoop();
		void cleanup();
		void createInstance();
	};

} //namespace


