#include "App.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>


namespace vulkanExample
{

	void App::run(const uint32_t width, const uint32_t height)
	{
		initWindow(width, height);
		initVulkan();
		mainLoop();
		cleanup();
	}

	void App::initWindow(const uint32_t width, const uint32_t height)
	{
		//initialize glfw
		glfwInit();
		//request to not create an OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//can't resize
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		// create window. 
		// First null pointer refers to monitor where window should be created
		// Second null pointer is only relevant to OpenGL
		window = glfwCreateWindow(width, height, "Vulkan Example", nullptr, nullptr);
	}

	void App::initVulkan()
	{
		createInstance();
	}
	
	void App::createInstance()
	{
		uint32_t glfwExtensionCount, extensionCount = 0;
		const char** glfwExtensions;

		// Application infor. Most information is optional
		VkApplicationInfo appInfo{};
		// Vulkan requires you to specify the structure type in sType member
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Example";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "GM Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Get extensions from GLFW 
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		
		VkInstanceCreateInfo createInfo{};
		// Vulkan requires you to specify the structure type in sType member
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		// Pass GLFW extensions to Vulkan
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		
		// Leave this as 0 for now
		createInfo.enabledLayerCount = 0;

		//creates instance
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}

		
		// Get extenstions count
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		//allocate vector for extensions information
		std::vector<VkExtensionProperties> extensions(extensionCount);
		// finally now grab all extensions info
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		//print them out
		std::cout << "available extensions:\n";

		for (const auto& extension : extensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}

	}
	
	void App::mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}
	
	void App::cleanup()
	{
		//destroys Vulkan instance
		vkDestroyInstance(instance, nullptr);
		// Destroy window
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	
} //namespace