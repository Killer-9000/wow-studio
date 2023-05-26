#include "graphics/Renderer.h"
#include "graphics/WindowMgr.h"
#include "graphics/windows/ProjectSelectWindow.h"
#include "graphics/imgui_extensions/ImFileDialog.h"

#include <SDL.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#undef main
int main (int argc, char* argv[])
{
  SRenderer->Initialize();

  SFILE_DIALOG->CreateTexture = [](uint8_t* data, int width, int height, char fmt) -> void*
  {
    if (!data || width < 1 || height < 1 || fmt > 1)
      return nullptr;

    Diligent::TextureDesc textureDesc;
    textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Format = (fmt == 0) ? Diligent::TEX_FORMAT_BGRA8_UNORM : Diligent::TEX_FORMAT_RGBA8_UNORM;
    textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    textureDesc.Usage = Diligent::USAGE_IMMUTABLE;

    Diligent::TextureSubResData textureSubData(data, width * 4);
    Diligent::TextureData textureData(&textureSubData, 1, SRenderer->GetDeviceContext());

    Diligent::ITexture* texture = nullptr;
    SRenderer->GetRenderDevice()->CreateTexture(textureDesc, &textureData, &texture);
    IM_ASSERT(texture);
    return texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
  };
  SFILE_DIALOG->DeleteTexture = [](void* texture)
  {
    if (!texture)
      return;
    Diligent::ITextureView* textureView = (Diligent::ITextureView*)texture;
    Diligent::ITexture* _texture = textureView->GetTexture();
    _texture->Release();
  };

  SDL_Init(SDL_INIT_EVENTS);

  // Initialize ImGui
  IMGUI_CHECKVERSION();

  SWindowMgr->AddWindow(new ProjectSelectWindow());
  while (SWindowMgr->WindowCount())
  {
    SWindowMgr->ProcessSDLEvents();
    SWindowMgr->UpdateWindows();

    SWindowMgr->CleanupWindows();
  }

  SRenderer->DeInitialize();

	return EXIT_SUCCESS;
}
