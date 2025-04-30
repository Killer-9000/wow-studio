#include "graphics/Rendering.h"
#include "graphics/WindowMgr.h"
#include "graphics/windows/ProjectSelectWindow.h"

#include <tracy/Tracy.hpp>

#undef main
int main(int argc, char* argv[])
{
  ZoneScoped;

  if (!SDL_Init(SDL_INIT_VIDEO))
    return EXIT_FAILURE;

  SRendering.Initialize();

  // Initialize ImGui
  IMGUI_CHECKVERSION();

  new ProjectSelectWindow(nullptr, true);
  while (SWindowMgr->WindowCount())
  {
    SWindowMgr->ProcessSDLEvents();
    SWindowMgr->UpdateWindows();

    SWindowMgr->CleanupWindows();
  }

  SRendering.Destroy();
  SDL_Quit();

	return EXIT_SUCCESS;
}
