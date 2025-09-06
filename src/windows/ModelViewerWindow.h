#pragma once

#include "graphics/IWindow.h"
#include "views/WorldView.h"
#include "windows/components/Filebrowser.h"

class ModelViewerWindow : public IWindow
{
public:
    ModelViewerWindow(IWindow* leadingWindow)
        : IWindow("WoW-Studio : Model Viewer", 1600, 900, SDL_WINDOW_RESIZABLE)
    {
        if (leadingWindow)
            leadingWindow->Close();
    }

	virtual bool Init() override;
	virtual void Deinit() override;
	virtual bool Render() override;

	void RenderWorld();
    void EndRender();

    bool CreateViewportTextures();

private:
    bool m_reopenHub = false;
    
    struct
    {
        struct
        {
            vk::Image renderImage;
            vma::Allocation renderMemory;
            vk::ImageView renderImageView;
            vk::Sampler renderSampler;
            vk::DescriptorSet renderDS;
            vk::CommandBuffer cmdBuffer;
            vk::Semaphore renderCompleteSemaphore;
            vk::Fence inFlightFence;
        } frameData[FRAMES_IN_FLIGHT];

        glm::vec2 pos = glm::vec2(0, 0);
        glm::vec2 size = glm::vec2(0, 0);
        bool focused = false;
    } m_viewport;

    WorldView m_worldView;
    Windows::Components::FileBrowser m_fileBrowser;
};
