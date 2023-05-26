#include "IWindow.h"

#include "Renderer.h"
#include "WindowMgr.h"

#include "graphics/imgui_extensions/ImFileDialog.h"
#include <backends/imgui_impl_diligent.h>
#include <imgui_internal.h>

bool IWindow::Init()
{
  // Create a window
  m_window = SDL_CreateWindow(m_windowName.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_windowWidth, m_windowHeight, SDL_WINDOW_VULKAN | m_extraFlags);
  if (!m_window)
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error during startup", "Failed to create window during startup", nullptr);
    return false;
  }
  SDL_SetWindowData(m_window, "container", this);

  // Initialize Diligent
  Diligent::SwapChainDesc SCDesc(m_windowWidth, m_windowHeight, Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB, Diligent::TEX_FORMAT_D32_FLOAT);

  SRenderer->GetFactory()->CreateSwapChainVk(SRenderer->GetRenderDevice(), SRendererContext, SCDesc, Diligent::NativeWindow(m_window, true), &m_swapchain);
  if (!m_swapchain)
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error during startup", "Failed to create graphical swapchain during startup", nullptr);
    return false;
  }

  // Initialize ImGui
  m_imguiContext = ImGui::CreateContext();
  ImGui::SetCurrentContext(m_imguiContext);
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  ImGui::StyleColorsDark();

  // File dialog settings handler
  {
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "FileDialog";
    ini_handler.TypeHash = ImHashStr("FileDialog");
    ini_handler.ReadOpenFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* line) -> void*
    {
      // This is to fake data.
      return (void*)0x1;
    };
    ini_handler.ReadLineFn = [](ImGuiContext* ctx, ImGuiSettingsHandler*, void* entry, const char* line)
    {
      char directory[260] = { '\0' };
      if (sscanf(line, "Last Directory=%s", directory) == 1) { SFILE_DIALOG->SetDirectory(directory); }
    };
    ini_handler.WriteAllFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
    {
      std::string currentDir = SFILE_DIALOG->GetDirectory().string();
      buf->reserve(buf->size() + (int)currentDir.size() + 36);
      buf->appendf("[%s][Settings]\n", handler->TypeName);
      buf->appendf("Last Directory=%s\n\n", currentDir.c_str());
    };
    ini_handler.UserData = SFILE_DIALOG;
    ImGui::AddSettingsHandler(&ini_handler);
    ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);
  }

  if (!ImGui_ImplSDL2_InitForVulkan(m_window))
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error during startup", "Failed to initialize ImGui platform backend", nullptr);
    return false;
  }

  if (!ImGui_ImplDiligent_Init(SRendererDevice, SRendererContext))
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error during startup", "Failed to initialize ImGui renderer backend", nullptr);
    return false;
  }

  m_shouldClose = false;

  return true;
}

void IWindow::Deinit()
{
  m_shouldClose = true;

  ImGui_ImplDiligent_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext(m_imguiContext);

  m_swapchain.Release();

  SDL_DestroyWindow(m_window);
}

IWindow::IWindow(const std::string& windowName, int width, int height, SDL_WindowFlags extraFlags)
  : m_windowName{ windowName }, m_windowWidth{ width }, m_windowHeight{ height }, m_extraFlags{ extraFlags }
{
}

bool IWindow::StartRender()
{
  if (m_minimized)
    return false;

  ImGui::SetCurrentContext(m_imguiContext);

  auto* pRTV = m_swapchain->GetCurrentBackBufferRTV();
  auto* pDSV = m_swapchain->GetDepthBufferDSV();
  SRendererContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
  SRendererContext->ClearRenderTarget(pRTV, m_clearColour, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
  SRendererContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

  // Start frame
  ImGui_ImplSDL2_NewFrame();
  ImGui_ImplDiligent_NewFrame();
  ImGui::NewFrame();

	return true;
}

void IWindow::EndRender()
{
  ImGui::SetCurrentContext(m_imguiContext);

  // Render frame
  ImGui::Render();
  ImGui_ImplDiligent_RenderDrawData(ImGui::GetDrawData());

  m_swapchain->Present();
}
