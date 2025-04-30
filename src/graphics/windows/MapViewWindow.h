#pragma once

#include "graphics/IWindow.h"
#include "views/WorldView.h"

class MapViewWindow : public IWindow
{
public:
	MapViewWindow(IWindow* leadingWindow, std::string mapName, uint32_t mapID, WowLocaleEnum locale)
		: IWindow(fmt::sprintf("WoW-Studio : %i - %s", mapID, mapName), 1600, 900, SDL_WINDOW_RESIZABLE)
	{
		if (leadingWindow)
			leadingWindow->Close();

		m_worldView = std::make_unique<WorldView>(mapID, locale);
	}
	
	virtual bool Init() override
	{
		if (!IWindow::Init())
			return false;

		OnWindowResize();

		SAsyncLoader->AddWork([&]() {
			CreateWorldGridShader();
			shadersCreated = true;
		});

		return true;
	}

	virtual bool ProcessSDLEvent(const SDL_Event& event) override;
	virtual bool Render() override;
	virtual bool Update() override;
	void RenderWorld();

	void OnWindowResize();

	void UpdateCameraVPMatrix();

private:
	void CreateWorldGridShader();

	//Diligent::RefCntAutoPtr<Diligent::ITexture> m_renderTexture, m_depthTexture;
	glm::vec2 m_renderViewportPos, m_renderViewportSize;
	bool m_renderViewportFocused = false;
	std::unique_ptr<WorldView> m_worldView;
	float m_worldClearColour[4] = { 0.0f, 0.0f, 0.0f, 1.0f};

	//struct
	//{
	//	Diligent::RefCntAutoPtr<Diligent::IBuffer> indexBuffer;
	//	Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexBuffer;
	//	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shaderResources;
	//	Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline;
	//} m_worldGrid;

	bool shadersCreated = false;

	glm::vec2 mousePoint, mouseDiff = { 0.0f, 0.0f };
	bool mouseDown = false;
};
