#include "MapViewWindow.h"

#include "graphics/renderers/WorldRenderer.h"
#include "windows/MapSelectWindow.h"
#include <fmt/printf.h>
#include "ModelViewerWindow.h"

bool MapViewWindow::ProcessSDLEvent(const SDL_Event& event)
{
	if (!IWindow::ProcessSDLEvent(event))
		return false;

 	switch (event.type)
	{
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		mouseDown = true;
		mousePoint = { event.button.x, event.button.y };
		mouseDiff = { 0.0f, 0.0f };
		break;
	case SDL_EVENT_MOUSE_BUTTON_UP:
		mouseDiff = { 0.0f, 0.0f };
		mouseDown = false;
		break;
	case SDL_EVENT_MOUSE_MOTION:
	{
		// If mouseDown and Window selected, and mouse in window.
		bool inBounds = true;
		inBounds &= ImGui::GetMousePos().x > m_renderViewportPos.x && ImGui::GetMousePos().y > m_renderViewportPos.y;
		inBounds &= ImGui::GetMousePos().x < m_renderViewportPos.x + m_renderViewportSize.x && ImGui::GetMousePos().y < m_renderViewportPos.y + m_renderViewportSize.y;
		if (mouseDown && inBounds)
		{
			mouseDiff.x = event.button.x - mousePoint.x;
			mouseDiff.y = mousePoint.y - event.button.y;
			mousePoint = { event.button.x, event.button.y };
			m_worldView->CameraRot.x += mouseDiff.y / _frameDelta;
			m_worldView->CameraRot.y += mouseDiff.x / _frameDelta;
			m_worldView->CameraRot.x = std::clamp(m_worldView->CameraRot.x, -89.9f, 89.9f);
			if (m_worldView->CameraRot.y > 180.5f) m_worldView->CameraRot.y = -179.5f;
			else if (m_worldView->CameraRot.y < -180.5f) m_worldView->CameraRot.y = 179.5f;

			if (isinf(m_worldView->CameraRot.x) || isnan(m_worldView->CameraRot.x))
				m_worldView->CameraRot.x = 0;
			if (isinf(m_worldView->CameraRot.y) || isnan(m_worldView->CameraRot.y))
				m_worldView->CameraRot.y = 0;
		}
	} break;
	case SDL_EVENT_MOUSE_WHEEL:
	{
		m_worldView->CameraSpeed += (float)event.wheel.y / _frameDelta * 10.0f;
		m_worldView->CameraSpeed = std::clamp(m_worldView->CameraSpeed, 0.0f, 1000.0f);
	} break;
	case SDL_EVENT_WINDOW_RESIZED:
	{
		OnWindowResize();
	} break;
	}
	return true;
}

bool MapViewWindow::Render()
{
	static bool quitMenu = false;
	if (!StartRender())
		return true;

	ImGui::DockSpaceOverViewport();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			ImGui::MenuItem("Quit", nullptr, &quitMenu);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		m_renderViewportFocused = m_imguiContext->NavWindow == ImGui::GetCurrentWindowRead();
		ImVec2 viewport = ImGui::GetContentRegionAvail();
		ImVec2 padding = ImGui::GetCurrentWindow()->WindowPadding;
		float x = ImGui::GetCursorPosX() - padding.x;
		float y = ImGui::GetCursorPosY() - padding.y;
		ImGui::SetCursorPos({ x, y });
		m_renderViewportPos = glm::vec2{ ImGui::GetCurrentWindow()->Pos.x + x, ImGui::GetCurrentWindow()->Pos.y + y };
		m_renderViewportSize = { viewport.x + (padding.x * 2), viewport.y + (padding.y * 2) };
		//ImGui::Image(m_renderTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE), { m_renderViewportSize.x, m_renderViewportSize.y });

		if (y == 0.0f)
		{
			x += ImGui::GetWindowPos().x;
			y += ImGui::GetWindowPos().y;
			ImU32 col;
			if (ImGui::GetFocusID() == ImGui::GetID("#UNHIDE"))
				col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
			else if (ImGui::GetHoveredID() == ImGui::GetID("#UNHIDE"))
				col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
			else
				col = ImGui::GetColorU32(ImGuiCol_Button);
			ImGui::GetCurrentWindow()->DrawList->AddTriangleFilled({ x, y }, { x + 12, y }, { x, y + 12 }, col);
		}
	}
	ImGui::End();

	if (ImGui::Begin("Debug"))
	{
		ImGui::Text("Mouse Diff:   %.1f %.1f", mouseDiff.x, mouseDiff.y);
		ImGui::Text("Camera pos:   %.3f %.3f %.3f", m_worldView->CameraPos.x, m_worldView->CameraPos.y, m_worldView->CameraPos.z);
		ImGui::Text("Camera rot:   %.3f %.3f %.3f", m_worldView->CameraRot.x, m_worldView->CameraRot.y, m_worldView->CameraRot.z);
		ImGui::Text("Camera speed: %.3f", m_worldView->CameraSpeed);
	}
	ImGui::End();

	EndRender();

	RenderWorld();

	if (quitMenu)
	{
		quitMenu = false;
		new MapSelectWindow(this);
	}

	return true;
}

bool MapViewWindow::Update()
{
	int keyCount = 0;
	const bool* keyState = (bool*)SDL_GetKeyboardState(&keyCount);

	// Viewport movement
	if (m_renderViewportFocused)
	{
		if (keyState[SDL_SCANCODE_W] && keyState[SDL_SCANCODE_S]) {}
		else if (keyState[SDL_SCANCODE_W])
		{
			glm::vec3 direction = m_worldView->GetCameraForward();
			m_worldView->CameraPos += direction / glm::vec3((float)_frameDelta) * m_worldView->CameraSpeed;
		}
		else if (keyState[SDL_SCANCODE_S])
		{
			glm::vec3 direction = m_worldView->GetCameraForward();
			m_worldView->CameraPos -= direction / glm::vec3((float)_frameDelta) * m_worldView->CameraSpeed;
		}

		if (keyState[SDL_SCANCODE_A] && keyState[SDL_SCANCODE_D]) {}
		else if (keyState[SDL_SCANCODE_A])
		{
			glm::vec3 direction = m_worldView->GetCameraRight();
			m_worldView->CameraPos -= direction / glm::vec3((float)_frameDelta) * m_worldView->CameraSpeed;
		}
		else if (keyState[SDL_SCANCODE_D])
		{
			glm::vec3 direction = m_worldView->GetCameraRight();
			m_worldView->CameraPos += direction / glm::vec3((float)_frameDelta) * m_worldView->CameraSpeed;
		}

		if (keyState[SDL_SCANCODE_Q] && keyState[SDL_SCANCODE_E]) {}
		else if (keyState[SDL_SCANCODE_Q])
		{
			m_worldView->CameraRot.y -= 0.1f / (float)_frameDelta * m_worldView->CameraSpeed;
		}
		else if (keyState[SDL_SCANCODE_E])
		{
			m_worldView->CameraRot.y += 0.1f / (float)_frameDelta * m_worldView->CameraSpeed;
		}

		if (keyState[SDL_SCANCODE_LCTRL] && keyState[SDL_SCANCODE_SPACE]) {}
		else if (keyState[SDL_SCANCODE_LCTRL])
		{
			m_worldView->CameraPos.y -= 1.0f / (float)_frameDelta * m_worldView->CameraSpeed;
		}
		else if (keyState[SDL_SCANCODE_SPACE])
		{
			m_worldView->CameraPos.y += 1.0f / (float)_frameDelta * m_worldView->CameraSpeed;
		}

		bool moving = false;
		moving |= keyState[SDL_SCANCODE_W] | keyState[SDL_SCANCODE_S];
		moving |= keyState[SDL_SCANCODE_A] | keyState[SDL_SCANCODE_D];
		moving |= keyState[SDL_SCANCODE_Q] | keyState[SDL_SCANCODE_E];
		moving |= keyState[SDL_SCANCODE_LCTRL] | keyState[SDL_SCANCODE_SPACE];
		if (moving)
		{
			if (isinf(m_worldView->CameraPos.x) || isnan(m_worldView->CameraPos.x))
				m_worldView->CameraPos.x = 0;
			if (isinf(m_worldView->CameraPos.y) || isnan(m_worldView->CameraPos.y))
				m_worldView->CameraPos.y = 0;
			if (isinf(m_worldView->CameraPos.z) || isnan(m_worldView->CameraPos.z))
				m_worldView->CameraPos.z = 0;
		}
	}
	
	return true;
}

void MapViewWindow::RenderWorld()
{
	//Diligent::ITextureView* pRTV = m_renderTexture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
	//Diligent::ITextureView* pDSV = m_depthTexture->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
	//SRendererContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	//SRendererContext->ClearRenderTarget(pRTV, m_worldClearColour, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	//SRendererContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	//
	//if (shadersCreated)
	//{
	//	Diligent::Viewport vp;
	//	vp.Width = m_windowWidth;
	//	vp.Height = m_windowHeight;
	//	vp.MinDepth = 0.25f;
	//	vp.MaxDepth = 10000.0f;
	//	vp.TopLeftX = vp.TopLeftY = 0.0f;
	//	SRendererContext->SetViewports(1, &vp, m_windowWidth, m_windowHeight);

	//	Diligent::Rect sr;
	//	sr.left = 0;
	//	sr.top = 0;
	//	sr.right = m_windowWidth;
	//	sr.bottom = m_windowHeight;
	//	SRendererContext->SetScissorRects(1, &sr, m_windowWidth, m_windowHeight);

	//	UpdateCameraVPMatrix();

	//	//// Draw world grid.
	//	if (m_worldGrid.pipeline)
	//	{
	//		SRendererContext->SetPipelineState(m_worldGrid.pipeline);

	//		Diligent::IBuffer* vertexBuffers[] = { m_worldGrid.vertexBuffer };
	//		uint64_t vertexOffsets[] = { 0 };
	//		SRendererContext->SetVertexBuffers(0, 1, vertexBuffers, vertexOffsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);
	//		SRendererContext->SetIndexBuffer(m_worldGrid.indexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	//		SRendererContext->CommitShaderResources(m_worldGrid.shaderResources, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	//		SRendererContext->DrawIndexed(Diligent::DrawIndexedAttribs{ 6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL });
	//	}
	//}
	//

	m_worldView->worldRenderer.Render(m_worldView.get());
}

void MapViewWindow::OnWindowResize()
{
	// Create render image
	//m_renderTexture.Release();
	//m_depthTexture.Release();

	//Diligent::TextureDesc textureDesc;
	//textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
	//textureDesc.Width = m_windowWidth;
	//textureDesc.Height = m_windowHeight;
	//textureDesc.Format = m_swapchain->GetDesc().ColorBufferFormat;
	//textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
	//textureDesc.Usage = Diligent::USAGE_DEFAULT;
	//	
	//SRenderer->GetRenderDevice()->CreateTexture(textureDesc, nullptr, &m_renderTexture);

	//textureDesc.Format = m_swapchain->GetDesc().DepthBufferFormat;
	//textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_DEPTH_STENCIL;
	//SRenderer->GetRenderDevice()->CreateTexture(textureDesc, nullptr, &m_depthTexture);
	//IM_ASSERT((m_renderTexture && m_depthTexture) && "Failed to create render or depth textures.");
}

void MapViewWindow::UpdateCameraVPMatrix()
{
	m_worldView->CameraMatrix = glm::lookAt(
		m_worldView->CameraPos,
		m_worldView->CameraPos + m_worldView->GetCameraForward(),
		glm::vec3(0, 1, 0)
	);

	m_worldView->ProjectionMatrix = glm::perspective(glm::radians(65.0f), m_renderViewportSize.x / m_renderViewportSize.y, 0.25f, 10000.0f);

	struct
	{
		glm::mat4 matrix;
		glm::vec3 cameraPos;
	} data;

	data.matrix = m_worldView->ProjectionMatrix * m_worldView->CameraMatrix * glm::mat4(1.0f);
	data.cameraPos = m_worldView->CameraPos;
	//SRenderer->UpdateBuffer(m_worldView->CameraVPMatrix, &data, sizeof(data));
}

void MapViewWindow::CreateWorldGridShader()
{
	//Diligent::GraphicsPipelineStateCreateInfo pipelineStateCI;
	//pipelineStateCI.PSODesc.Name = "World Grid Pipeline";
	//pipelineStateCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

	//{
	//	Diligent::ShaderCreateInfo shaderCI;
	//	shaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
	//	shaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
	//	shaderCI.Desc.Name = "World Grid Vertex";
	//	shaderCI.FilePath = "resources/shaders/WorldGrid.hlsl";
	//	shaderCI.EntryPoint = "VSMain";
	//	shaderCI.pShaderSourceStreamFactory = SRenderer->GetShaderInputFactory();

	//	SRendererDevice->CreateShader(shaderCI, &pipelineStateCI.pVS);
	//	if (!pipelineStateCI.pVS)
	//		assert(false && "Failed to create WorldGrid Vertex shader.");

	//	shaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
	//	shaderCI.Desc.Name = "World Grid Pixel";
	//	shaderCI.EntryPoint = "PSMain";

	//	SRendererDevice->CreateShader(shaderCI, &pipelineStateCI.pPS);
	//	if (!pipelineStateCI.pPS)
	//		assert(false && "Failed to create WorldGrid Pixel shader.");
	//}

	//pipelineStateCI.GraphicsPipeline.NumRenderTargets = 1;
	//pipelineStateCI.GraphicsPipeline.RTVFormats[0] = m_swapchain->GetDesc().ColorBufferFormat;
	//pipelineStateCI.GraphicsPipeline.DSVFormat = m_swapchain->GetDesc().DepthBufferFormat;
	//pipelineStateCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	//// Blend description
	//pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
	//pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
	//pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
	//pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = Diligent::BLEND_OPERATION_ADD;
	//pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
	//pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
	//pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
	//pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;

	//// Rasterizer description
	//pipelineStateCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;

	//// Depth-Stencil description
	//pipelineStateCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;
	//pipelineStateCI.GraphicsPipeline.DepthStencilDesc.DepthFunc = Diligent::COMPARISON_FUNC_ALWAYS;
	//pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilFailOp = Diligent::STENCIL_OP_KEEP;
	//pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilPassOp = Diligent::STENCIL_OP_KEEP;
	//pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilFunc = Diligent::COMPARISON_FUNC_ALWAYS;
	//pipelineStateCI.GraphicsPipeline.DepthStencilDesc.BackFace = pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace;

	//// Pipeline inputs layout
	//Diligent::LayoutElement layoutElements[2];

	//pipelineStateCI.GraphicsPipeline.InputLayout.NumElements = 2;
	//pipelineStateCI.GraphicsPipeline.InputLayout.LayoutElements = layoutElements;
	//layoutElements[0].InputIndex = 0;
	//layoutElements[0].NumComponents = 3;
	//layoutElements[0].ValueType = Diligent::VT_FLOAT32;
	//layoutElements[1].InputIndex = 1;
	//layoutElements[1].NumComponents = 2;
	//layoutElements[1].ValueType = Diligent::VT_FLOAT32;

	//pipelineStateCI.PSODesc.ResourceLayout.NumVariables = 0;

	//SRendererDevice->CreateGraphicsPipelineState(pipelineStateCI, &m_worldGrid.pipeline);
	//pipelineStateCI.pVS->Release();
	//pipelineStateCI.pPS->Release();

	//if (!m_worldGrid.pipeline)
	//	assert(false && "Failed to create WorldGrid pipeline.");

	//{
	//	Diligent::BufferDesc bufferDesc;
	//	bufferDesc.Usage = Diligent::USAGE_DEFAULT;
	//	bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
	//	bufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
	//	bufferDesc.MiscFlags = Diligent::MISC_BUFFER_FLAG_NONE;

	//	{
	//		struct VertexData
	//		{
	//			glm::vec3 pos;
	//			glm::vec2 uv;
	//		};
	//		VertexData vertexData[] =
	//		{
	//			{ {-1, 0,-1 }, { 0, 0 } },
	//			{ { 1, 0,-1 }, { 1, 0 } },
	//			{ {-1, 0, 1 }, { 0, 1 } },
	//			{ { 1, 0, 1 }, { 1, 1 } }
	//		};
	//		bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
	//		bufferDesc.Size = 4 * sizeof(VertexData);
	//		bufferDesc.ElementByteStride = sizeof(VertexData);

	//		Diligent::BufferData bufferData;
	//		bufferData.pContext = SRendererContext;
	//		bufferData.DataSize = 4 * sizeof(VertexData);
	//		bufferData.pData = vertexData;

	//		SRendererDevice->CreateBuffer(bufferDesc, &bufferData, &m_worldGrid.vertexBuffer);
	//		
	//		if (!m_worldGrid.vertexBuffer)
	//			assert(false && "Failed to create WorldGrid vertex buffer.");
	//	}
	//	
	//	{
	//		uint32_t indexData[] =
	//		{
	//			0, 1, 2,
	//			2, 3, 1
	//		};
	//		bufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
	//		bufferDesc.Size = 6 * sizeof(uint32_t);
	//		bufferDesc.ElementByteStride = sizeof(uint32_t);

	//		Diligent::BufferData bufferData;
	//		bufferData.pContext = SRendererContext;
	//		bufferData.DataSize = 6 * sizeof(uint32_t);
	//		bufferData.pData = indexData;

	//		SRendererDevice->CreateBuffer(bufferDesc, &bufferData, &m_worldGrid.indexBuffer);
	//		if (!m_worldGrid.indexBuffer)
	//			assert(false && "Failed to create WorldGrid index buffer.");
	//	}

	//}

	//// View Projection buffer
	//{
	//	Diligent::BufferDesc mvpConstantDesc;
	//	mvpConstantDesc.Name = "Camera ViewProjection Constant";
	//	mvpConstantDesc.Size = sizeof(glm::mat4) + sizeof(glm::vec3);
	//	mvpConstantDesc.Usage = Diligent::USAGE_DYNAMIC;
	//	mvpConstantDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
	//	mvpConstantDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

	//	SRendererDevice->CreateBuffer(mvpConstantDesc, nullptr, &m_worldView->CameraVPMatrix);
	//	UpdateCameraVPMatrix();

	//	m_worldGrid.pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ViewProjection")->Set(m_worldView->CameraVPMatrix);
	//}
	//
	//// Create shader resource.
	//m_worldGrid.pipeline->CreateShaderResourceBinding(&m_worldGrid.shaderResources, true);
	//if (!m_worldGrid.shaderResources)
	//	assert(false && "Failed to create WorldGrid shader resource.");
}
