#pragma once
#include <optional>

namespace vulkanExample
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isValid() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};


} //namespace VulkanExample