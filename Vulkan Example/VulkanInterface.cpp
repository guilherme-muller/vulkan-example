#include "VulkanInterface.hpp"
#include "QueueFamilyIndices.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <cstring>
#include <optional>
#include <set>

namespace vulkanExample
{




#pragma region DEBUG_SETTING_CALLS
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	

	//Debug callback. Can treat errors, but now only prints them
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInterface::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		//reduce verbosity
		if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
			return VK_FALSE;

		// print the error/warning/information message to std::cerr
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		// should always return false for now. Returning true means the error was unhandled
		return VK_FALSE;
	}

	// Function to create the debug layer. As it is attached as a plugin, we need to look for the method as there's no header (the layer might not be available)
	VkResult VulkanInterface::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		// search for the method by name
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		// if not null, call the method
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			//Else, extension is not available
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	// Function to destroy the debug layer. As it is attached as a plugin, we need to look for the method as there's no header (the extension might not be available)
	void VulkanInterface::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		//Search for method by name
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		//If not null, call the method
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	

	void VulkanInterface::setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}

	}

	void VulkanInterface::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = this; // passes class reference
	}

#pragma endregion

	// Constructor
	VulkanInterface::VulkanInterface(const uint32_t width, const uint32_t height)
	{
		w_width = width;
		w_height = height;
		// Request Validation Layer (debug)
		validationLayers = {
		"VK_LAYER_KHRONOS_validation"
		};
		//initialize glfw
		initWindow(w_width, w_height);
		// Initialize VulkanAPI
		initVulkan();
		
	}

	VulkanInterface::~VulkanInterface()
	{
		cleanup();
	}

	void VulkanInterface::cleanup()
	{
		vkDestroyDevice(logicalDevice, nullptr);
		//destroy debug validation layer
		vkDestroySurfaceKHR(instance, surface, nullptr);
		
		if (enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		
		//destroys Vulkan instance
		vkDestroyInstance(instance, nullptr);
		// Destroy window
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VulkanInterface::run()
	{
		mainLoop();
	}

	void VulkanInterface::mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}


#pragma region INSTANCE_INIT

	// Search for extensions in both glfw and Vulkan
	std::vector<const char*> VulkanInterface::getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		// Get glfw extensions
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		// create vector with extension info
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		//if debugging, add DEBUG extension
		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	// initialize Vulkan API and Debug Handler
	void VulkanInterface::initVulkan()
	{
		//Create Instance
		createInstance();
		//setup debugging
		setupDebugMessenger();
		//create surface
		createSurface();
		//Pick Physical devices
		pickPhysicalDevices();
		//Logical Device
		createLogicalDevice();
	}
	
	void VulkanInterface::initWindow(const uint32_t width, const uint32_t height)
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

	

	void VulkanInterface::createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

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
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		
		//For instance creation debugging
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers)
		{
			//Gets layer count and populate data
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			//populate debug struct
			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		//creates instance
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			std::cerr << "Failed to create Vulkan API Instance";
			throw std::runtime_error("failed to create instance!");
		}
	}

	
	void VulkanInterface::createSurface()
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create rendering surface");

	}



	bool VulkanInterface::checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}
	



	void VulkanInterface::pickPhysicalDevices()
	{
		
		uint32_t deviceCount = 0;
		// Checks device count first
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
			throw std::runtime_error("Failed to find device with Vulkan support");

		std::vector<VkPhysicalDevice> deviceList(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

		for (VkPhysicalDevice &device : deviceList)
		{
			std::optional<QueueFamilyIndices> result = checkDevice(device);
			if (result.has_value())
			{
				physicalDevice = device;
				queueFamilies = result.value();
				vkGetPhysicalDeviceProperties(device, &deviceProperties);
				vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find a suitable GPU");
	}

	std::optional<QueueFamilyIndices> VulkanInterface::checkDevice(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		// Get device properties from device handle
		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);

		if ((properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) && features.geometryShader)
		{

			QueueFamilyIndices indices = findQueueFamilies(device);
			
#ifndef NDEBUG
			std::cout << "Found suitable device" << std::endl;
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				std::cout << "Device Type:" << "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU" << std::endl;
			else
				std::cout << "Device Type:" << "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU" << std::endl;
			std::cout << "Device Name:" << properties.deviceName << std::endl;
			std::cout << "Vendor ID:" << properties.vendorID << std::endl;
			std::cout << "API Version:" << properties.apiVersion << std::endl;
			std::cout << "Driver Version:" << properties.driverVersion << std::endl;
			std::cout << "Has queue supporting VK_QUEUE_GRAPHICS_BIT: " << (indices.graphicsFamily.has_value() ? "true" : "false") << std::endl;
#endif

			if (indices.isValid())
				return indices;
		}

		return std::nullopt;
	}


	QueueFamilyIndices VulkanInterface::findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiliesProperties.data());

		//Need to find at least one queue that supports VK_QUEUE_GRAPHICS_BIT
		int i = 0;
		for (const auto& queueFamily : queueFamiliesProperties) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
				if (presentSupport)
					indices.presentFamily = i;
			}

			i++;
		}


		return indices;
	}


	void VulkanInterface::createLogicalDevice()
	{
		//Create queue structure first
		
		//This check should never be true at this point, but who knows right? It catches a programmer mistake
		//PS: It happened once
		if (!queueFamilies.graphicsFamily.has_value() || !queueFamilies.presentFamily.has_value())
			throw std::runtime_error("Queue has not been initialized or found");
		
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value() };


		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			//add structure type
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			// add indec
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			//set fixed prio
			queueCreateInfo.pQueuePriorities = new const float{ 1.0f };
			queueCreateInfos.push_back(queueCreateInfo);
		}


		//Now create logical device
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//Add queues and count
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		//Add physical device features
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		//Add validation layers. This is for retro-compatibility with older versions of Vulkan
		deviceCreateInfo.enabledExtensionCount = 0;

		if (enableValidationLayers) {
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
		}

		//create device
		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device");

		//create graphics queue
		vkGetDeviceQueue(logicalDevice, queueFamilies.graphicsFamily.value(), 0, &graphicsQueue);
		if (graphicsQueue == nullptr)
			throw std::runtime_error("Failed to allocate graphics queue");

		//creates present queue
		vkGetDeviceQueue(logicalDevice, queueFamilies.presentFamily.value(), 0, &presentQueue);
		if (presentQueue == nullptr)
			throw std::runtime_error("Failed to allocate present queue");

	}


#pragma endregion


	
	

} //namespace