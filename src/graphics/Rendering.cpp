#include "Rendering.h"

void VulkanDispatchLoader::init()
{
	vk::Instance instance = SRendering._instance;

	/* --- VK_EXT_debug_utils --- */
	pfn_vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
	pfn_vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT");

	/* --- VK_KHR_dynamic_rendering --- */
	pfn_vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)instance.getProcAddr("vkCmdBeginRenderingKHR");
	pfn_vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)instance.getProcAddr("vkCmdEndRenderingKHR");
}

VkBool32 VulkanDebugUtilsCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
	vk::DebugUtilsMessageTypeFlagsEXT              messageTypes,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (pCallbackData)
	{
		std::string severityString = vk::to_string(messageSeverity);
		std::string typeString = vk::to_string(messageTypes);

		printf("[%s] %s -> '%s'\n", severityString.c_str(), typeString.c_str(), pCallbackData->pMessage);
	}

	return VK_FALSE;
}