#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

VkBool32 VulkanDebugUtilsCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
	vk::DebugUtilsMessageTypeFlagsEXT              messageTypes,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

class VulkanDispatchLoader : public vk::detail::DispatchLoaderStatic
{
public:
	VulkanDispatchLoader()
		: vk::detail::DispatchLoaderStatic()
	{ }

	void init();

	VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pMessenger) const noexcept
	{
		return pfn_vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
	}
	void vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT messenger,
		const VkAllocationCallbacks* pAllocator) const noexcept
	{
		return pfn_vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
	}

	void vkCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) const noexcept
	{
		return pfn_vkCmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
	}
	void vkCmdEndRenderingKHR(VkCommandBuffer commandBuffer) const noexcept
	{
		return pfn_vkCmdEndRenderingKHR(commandBuffer);
	}

private:
	/* --- VK_EXT_debug_utils --- */
	PFN_vkCreateDebugUtilsMessengerEXT pfn_vkCreateDebugUtilsMessengerEXT = nullptr;
	PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;

	/* --- VK_KHR_dynamic_rendering --- */
	PFN_vkCmdBeginRenderingKHR pfn_vkCmdBeginRenderingKHR = nullptr;
	PFN_vkCmdEndRenderingKHR pfn_vkCmdEndRenderingKHR = nullptr;
};

class Rendering
{
	Rendering() { }
	~Rendering() { }

public:
	static Rendering& Instance()
	{
		static Rendering instance;
		return instance;
	}

	void Initialize()
	{
		SDL_Vulkan_LoadLibrary(nullptr);

		// Create instance
		{
			std::vector<const char*> layers =
			{
				//"VK_LAYER_KHRONOS_validation"
			};
			std::vector<const char*> extensions =
			{
				//"VK_EXT_debug_utils"
			};

			uint32_t count;
			const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&count);
			for (uint32_t i = 0; i < count; i++)
				extensions.emplace_back(sdlExtensions[i]);

			vk::ApplicationInfo appInfo("WoW-Studio", 0, "WoW-Studio", 0, VK_API_VERSION_1_3);
			vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo, layers, extensions);

			_instance = vk::createInstance(createInfo);
		}

		_dispatchLoader.init();

		// Debug reporting
		//{
		//	vk::DebugUtilsMessengerCreateInfoEXT debugCI(
		//		vk::DebugUtilsMessengerCreateFlagsEXT(),
		//		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
		//		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
		//		  | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
		//		(vk::PFN_DebugUtilsMessengerCallbackEXT)VulkanDebugUtilsCallback
		//	);
		//
		//	_vulkanDebugReportCallback = _instance.createDebugUtilsMessengerEXT(debugCI, nullptr, GetDispatch());
		//}

		// Create device to use.
		{
			_physicalDevice = _instance.enumeratePhysicalDevices().front();

			std::vector<vk::QueueFamilyProperties> qfp = _physicalDevice.getQueueFamilyProperties();

			_graphicsQueueIndex = UINT32_MAX;
			for (int i = 0; i < qfp.size(); i++)
			{
				if (qfp[i].queueFlags & vk::QueueFlagBits::eGraphics)
				{
					_graphicsQueueIndex = i;
					break;
				}
			}
			assert(_graphicsQueueIndex != UINT32_MAX && "Failed to find graphics queue on device.");

			std::vector<const char*> layers =
			{
			};
			std::vector<const char*> extensions =
			{
				"VK_KHR_swapchain",
				//"VK_KHR_create_renderpass2",
				//"VK_KHR_depth_stencil_resolve",
				//"VK_KHR_dynamic_rendering"
			};

			// Features seem to automatically set.
			vk::PhysicalDevice8BitStorageFeatures byteStorageFeature(false, true, false, nullptr);
			vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeature(true, false, false, &byteStorageFeature);
			vk::PhysicalDeviceVulkanMemoryModelFeatures vulkanMemoryModelFeatures(true, true, false, &bufferDeviceAddressFeature);
			vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT pageableMemoryFeature(true, &vulkanMemoryModelFeatures);
			vk::PhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeature(true, &pageableMemoryFeature);

			// Wanted features
			vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature(true, &timelineSemaphoreFeature);
			vk::PhysicalDeviceFeatures2 features(vk::PhysicalDeviceFeatures(), &dynamicRenderingFeature);

			// Features seem to automatically set.
			features.features.fragmentStoresAndAtomics = true;
			features.features.vertexPipelineStoresAndAtomics = true;
			features.features.shaderInt64 = true;

			float queuePriority = 0.0f;
			vk::DeviceQueueCreateInfo createInfo(vk::DeviceQueueCreateFlags(), _graphicsQueueIndex, 1, &queuePriority);
			_device = _physicalDevice.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), createInfo, layers, extensions, nullptr, &features));

			_commandPool = _device.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _graphicsQueueIndex));

			_graphicsQueue = _device.getQueue(_graphicsQueueIndex, 0);
		}
	}

	void Destroy()
	{
		_device.destroyCommandPool(_commandPool);
		_device.destroy();
		//_instance.destroyDebugUtilsMessengerEXT(_vulkanDebugReportCallback, nullptr, GetDispatch());
		_instance.destroy();
	}

	const VulkanDispatchLoader& GetDispatch() const { return _dispatchLoader; }
	VulkanDispatchLoader& GetDispatch() { return _dispatchLoader; }

	VulkanDispatchLoader _dispatchLoader;

	vk::Instance _instance;
	//vk::DebugUtilsMessengerEXT _vulkanDebugReportCallback;
	vk::PhysicalDevice _physicalDevice;
	vk::Device _device;
	vk::CommandPool _commandPool;
	uint32_t _graphicsQueueIndex;
	vk::Queue _graphicsQueue;
};

#define SRendering Rendering::Instance()