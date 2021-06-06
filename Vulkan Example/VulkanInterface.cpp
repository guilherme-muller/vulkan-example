#include "VulkanInterface.hpp"
#include "QueueFamilyIndices.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint> 
#include <algorithm> 
#include <fstream>

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
		//destroy framebuffers
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
		}
		vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
		vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(logicalDevice, imageView, nullptr);
		}
		//destroys swap chain and device
		vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
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
		//Creates swap chain
		createSwapChain();
		//creates image views
		createImageViews();
		//create render pass
		createRenderPass();
		//create graphics pipeline
		createGraphicsPipeline();
		//creates frame buffer
		createFrameBuffers();
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

	void VulkanInterface::createSwapChain()
	{
		//Grabs swap chain support data
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		// Get surface format
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		// Get Presenting mode
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		// Get extent
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		//simply sticking to this minimum means that we may sometimes have to wait on the driver to complete internal operations before we can acquire another image to render to
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		// Get maximum image count. If 0, means is unlimited
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// initialize swap chain create structure
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		//This is only 1 unless you are developing stereoscopic 3D app
		createInfo.imageArrayLayers = 1;
		//No post-processing, so rendering directly to the image. Otherwise, use VK_IMAGE_USAGE_TRANSFER_DST_BIT and make memory transfer operations to render
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


		uint32_t queueFamilyIndices[] = { queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value() };
		//if not the same queue, then data has to be transfered explicitly between graphics family queue and presentation family queue; however, this brings better performance
		//requires having 2 different queues
		if (queueFamilies.graphicsFamily != queueFamilies.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		// this will be the case for most HW
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		//no transformation in image (e.g., 90 degrees rotation)
		// might be useful for phones development orientation
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

		// this tells the window system that it should ignore alpha blending with other windows
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		//clipping makes it ignore pixels that are not presented in the screen (e.g., because there's another window in front of the application)
		createInfo.clipped = VK_TRUE;

		//this has to be handled later. When creating new swap chains (e.g., because window was resized), the old one has to be passed as a reference here
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		//creates swap chain
		if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		//saves swap chain format and extent
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;

		//get number of images
		vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());

	}
	
	std::vector<char> VulkanInterface::readFile(const std::string& filename) 
	{
		//opens file, start at the end
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		//grab file size
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		//change pointer and read buffer
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		//close file
		file.close();

		return buffer;
	}


	void VulkanInterface::createImageViews()
	{
		//resize vector
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			//initialize structure
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			// selects 2D texture/image type. This can be 1D, 2D or 3D textures/cube maps
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;

			//Allows you to map channels (e.g., all channels to red). Now we just stick with default
			//TODO: Check later if this has an impact on painting the triangle
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			//use color targets, without mipmaping and using single layer
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}

		}


	}


	void VulkanInterface::createRenderPass()
	{
		//we only have 1 single color buffer, so no multisampling
		VkAttachmentDescription colorAttachment{};
		//image format must match swap chain format
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		//clear color data on load
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//store color after rendering
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		//same here, but with stencil. Just don't care now, as we are not using stencils
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		//we don't care about the inial layout as the image will be cleared
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// after rendering, the image is ready to be presented
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//configures color attachment reference for sub-passes
		VkAttachmentReference colorAttachmentRef{};
		//index of VkAttachmentDescription (therefore 0)
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//configures subpass
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		//this is directly referenced in the fragment shader index "layout(location = 0) out vec4 outColor;" directive
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		//creates render pass info structure and populate data
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		//creates render pass
		if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

	}

	void VulkanInterface::createGraphicsPipeline()
	{
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		//creates pipeline info structure for vertex shader
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		//now for the fragment shader
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";


		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//Vertex input. As the vertex input is hardcoded in the vertex shader (for the triangle example), we leave it empty for now
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		//primitives will be drawn as normal triangles for now
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		//defines viewport
		VkViewport viewport{};
		//starts at 0
		viewport.x = 0.0f; 
		viewport.y = 0.0f;
		//maximum is calculated swap chain extent width and height
		viewport.width = (float)swapChainExtent.width; 
		viewport.height = (float)swapChainExtent.height;
		// define framebuffer depth limits
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//scissors can define an area of the viewport where pixels are discarded by the rasterizer
		//configuring scissor to be the entire frame buffer (no ignored pixels)
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		//Configure viewport structure with configuration data
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;


		//rasterizer initializarion
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		//setting this to true disables any output to framebuffer
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		//make fill polygons. Using anything different than that requires enabling a GPU feature
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		//line width
		rasterizer.lineWidth = 1.0f;
		//this is better explained here: https://learnopengl.com/Advanced-OpenGL/Face-culling
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		//not using this, could be used for shadow mapping
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		
		//for now, we disable multi-sampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		//uses alpha blending for color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		//leaves logid operator disabled for color blending
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional


		//define dynamic states as viewport and line width
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};
		//configure dynamic states
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		//Creates pipeline layout infor
		//They are commonly used to pass the transformation matrix to the vertex shader, or to create texture samplers in the fragment shader
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		
		//creates pipeline layout
		if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		//now that the layout is done, we can create the pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		//pass vertex and fragment shaders
		pipelineInfo.pStages = shaderStages;
		//populate data from above
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		//Vulkan allows you to define a base pipeline from an existing one, so it makes derivation easier. Not doing this here though
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		//creates pipeline
		if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		//destroy shaders (WHY??)
		vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
	}

	void VulkanInterface::createFrameBuffers()
	{
		//resize array of frame buffers
		swapChainFramebuffers.resize(swapChainImageViews.size());

		//for each swap chain image view, create a framebuffer
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};
			//populate data
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			//render pass that it should be compatible with
			framebufferInfo.renderPass = renderPass;
			//VkImageView objects that should be bound to it
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	VkShaderModule VulkanInterface::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		//has to cast from char to uint32 to satisfy pointer type
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		//creates shader
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
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
		// prints device information if debugging
		printDeviceExtensionSupport(physicalDevice);
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
			
			if (indices.isValid() && checkDeviceExtensionSupport(device) && querySwapChainSupport(device).isAdequate())
				return indices;
		}

		return std::nullopt;
	}

	//Checks all available capabiliti4es, formats and present modes available from the swap chain and populate SwapChainSupportDetails struct
	SwapChainSupportDetails VulkanInterface::querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
		
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}


		return details;
	}


	VkPresentModeKHR VulkanInterface::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
	{
	// if Mailboox (similar to tripple buffer) is available, select this one
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		//otherwise just uses nomal FIFO (double buffer)
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	
	VkExtent2D VulkanInterface::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		//Vulkan requires that we match the resolution of the window by setting the width and height in the currentExtent member
		// However, we can differ from that and set minimum and maximum window resolution by using UINT32_MAX and using minImageExtent and maxImageExtent 
		// GLFW can use both coordinates or pixels to measure size, but Vulkan only uses pixels, so the swap chain must also use pixels
		// When using a high DPI display, coordinates might not match pixels, therefore the resolution in pixels will be larger than the resolution in coordinates
		// Therefore we need to query GLFW for the resolution of the window in pixels before matching it with minimum and maximum image extent
		
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}


	VkSurfaceFormatKHR VulkanInterface::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		
		// Look for SRGB 8 bits and non-linear color space
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		//if not available, just returns the first available type (for educational reasons... Normally we would sort by preference)
		return availableFormats[0];

	}

	


	bool VulkanInterface::checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void VulkanInterface::printDeviceExtensionSupport(VkPhysicalDevice device)
	{
#ifndef NDEBUG
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		VkPhysicalDeviceProperties properties;
		// Get device properties from device handle
		vkGetPhysicalDeviceProperties(device, &properties);

		std::cout << "Available extensions for device " << properties.deviceName << std::endl;
		for (const auto& extension : availableExtensions) {
			std::cout << "Name: " << extension.extensionName << " Version: " << extension.specVersion << std::endl;
		}
#endif
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

		//Adds extensions
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

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