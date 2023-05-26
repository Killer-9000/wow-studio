// dear imgui: Renderer Backend for Diligent
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'D3D12_GPU_DESCRIPTOR_HANDLE' as ImTextureID. Read the FAQ about ImTextureID!
//  [X] Renderer: Support for large meshes (64k+ vertices) with 16-bit indices.

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)

#include "imgui_impl_diligent.h"
#include "imgui.h"

#include "RefCntAutoPtr.hpp"

#include "fmt/printf.h"

#include <memory>
#include <filesystem>

// Diligent data
struct ImGui_ImplDiligent_RenderBuffers
{
  Diligent::RefCntAutoPtr<Diligent::IBuffer> IndexBuffer;
  Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
  uint64_t IndexBufferSize = 1000;
  uint64_t VertexBufferSize = 500;

  void Release()
  {
    IndexBuffer.Release();
    VertexBuffer.Release();
    IndexBufferSize = 0;
    VertexBufferSize = 0;
  }
};

struct ImGui_ImplDiligent_Data
{
  Diligent::RefCntAutoPtr<Diligent::IRenderDevice> device;
  Diligent::RefCntAutoPtr<Diligent::IDeviceContext> context;
  Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipelineState;
  Diligent::RefCntAutoPtr<Diligent::IBuffer> mvpConstant;
  Diligent::RefCntAutoPtr<Diligent::ITexture> fontResource;
  Diligent::RefCntAutoPtr<Diligent::ITextureView> fontResourceView;
  Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shaderResources;
  ImGui_ImplDiligent_RenderBuffers frameResources;

  ImGui_ImplDiligent_Data() { memset((void*)this, 0, sizeof(*this)); }
  ~ImGui_ImplDiligent_Data() { Release(); }

  void Release()
  {
    shaderResources.Release();
    pipelineState.Release();
    mvpConstant.Release();
    fontResource.Release();
    fontResourceView.Release();
    frameResources.Release();
  }
};

struct VERTEX_CONSTANT_BUFFER_Diligent
{
  float   mvp[4][4];
};

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static ImGui_ImplDiligent_Data* ImGui_ImplDiligent_GetBackendData()
{
  return ImGui::GetCurrentContext() ? (ImGui_ImplDiligent_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

// Functions
static void ImGui_ImplDiligent_SetupRenderState(ImDrawData* draw_data, ImGui_ImplDiligent_RenderBuffers* fr)
{
  ImGui_ImplDiligent_Data* bd = ImGui_ImplDiligent_GetBackendData();

  // Setup orthographic projection matrix into our constant buffer
  // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
  VERTEX_CONSTANT_BUFFER_Diligent vertex_constant_buffer;
  {
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    float mvp[4][4] =
    {
        { 2.0f / (R - L),     0.0f,              0.0f, 0.0f },
        { 0.0f,               2.0f / (T - B),    0.0f, 0.0f },
        { 0.0f,               0.0f,              0.5f, 0.0f },
        { (R + L) / (L - R),  (T + B) / (B - T), 0.5f, 1.0f },
    };
    memcpy(&vertex_constant_buffer.mvp, mvp, sizeof(mvp));
  }

  // Setup viewport
  Diligent::Viewport vp;
  vp.Width = draw_data->DisplaySize.x;
  vp.Height = draw_data->DisplaySize.y;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = vp.TopLeftY = 0.0f;
  bd->context->SetViewports(1, &vp, 1, 1);

  // Bind shader and vertex buffers

  Diligent::IBuffer* vertexBuffers[] = { fr->VertexBuffer };
  uint64_t vertexOffsets[] = { 0 };
  bd->context->SetVertexBuffers(0, 1, vertexBuffers, vertexOffsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);

  bd->context->SetIndexBuffer(fr->IndexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

  void* mvp;
  bd->context->MapBuffer(bd->mvpConstant, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, mvp);
  memcpy(mvp, vertex_constant_buffer.mvp, sizeof(vertex_constant_buffer));
  bd->context->UnmapBuffer(bd->mvpConstant, Diligent::MAP_WRITE);

  bd->context->SetPipelineState(bd->pipelineState);

  bd->context->CommitShaderResources(bd->shaderResources, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

  // Setup blend factor
  const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
  bd->context->SetBlendFactors(blend_factor);
}

// Render function
void ImGui_ImplDiligent_RenderDrawData(ImDrawData* draw_data)
{
  // Avoid rendering when minimized
  if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
    return;

  ImGui_ImplDiligent_Data* bd = ImGui_ImplDiligent_GetBackendData();
  ImGui_ImplDiligent_RenderBuffers* fr = &bd->frameResources;

  // Create and grow vertex/index buffers if needed
  if (!fr->VertexBuffer || fr->VertexBufferSize < draw_data->TotalVtxCount)
  {
    if (fr->VertexBuffer)
      fr->VertexBuffer.Release();

    Diligent::BufferDesc bufferDesc;
    bufferDesc.Size = (draw_data->TotalVtxCount + 5000) * sizeof(ImDrawVert);
    bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
    bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    bufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    bufferDesc.MiscFlags = Diligent::MISC_BUFFER_FLAG_NONE;
    bufferDesc.ElementByteStride = sizeof(ImDrawVert);

    fr->VertexBufferSize = draw_data->TotalVtxCount + 5000;
    bd->device->CreateBuffer(bufferDesc, nullptr,&fr->VertexBuffer);

    if (!fr->VertexBuffer)
      return;
  }
  if (!fr->IndexBuffer || fr->IndexBufferSize < draw_data->TotalIdxCount)
  {
    if (fr->IndexBuffer)
      fr->IndexBuffer.Release();

    Diligent::BufferDesc bufferDesc;
    bufferDesc.Size = (draw_data->TotalIdxCount + 10000) * sizeof(ImDrawIdx);
    bufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
    bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
    bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    bufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    bufferDesc.MiscFlags = Diligent::MISC_BUFFER_FLAG_NONE;
    bufferDesc.ElementByteStride = sizeof(ImDrawIdx);

    fr->IndexBufferSize = draw_data->TotalIdxCount + 10000;
    bd->device->CreateBuffer(bufferDesc, nullptr, &fr->IndexBuffer);

    if (!fr->IndexBuffer)
      return;
  }

  // Upload vertex/index data into a single contiguous GPU buffer
  {
    void* vtx_resource = nullptr, *idx_resource = nullptr;
    bd->context->MapBuffer(fr->VertexBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, vtx_resource);
    bd->context->MapBuffer(fr->IndexBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, idx_resource);

    if (!vtx_resource || !idx_resource)
      return;

    ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource;
    ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
      const ImDrawList* cmd_list = draw_data->CmdLists[n];
      memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
      memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
      vtx_dst += cmd_list->VtxBuffer.Size;
      idx_dst += cmd_list->IdxBuffer.Size;
    }

    bd->context->UnmapBuffer(fr->VertexBuffer, Diligent::MAP_WRITE);
    bd->context->UnmapBuffer(fr->IndexBuffer, Diligent::MAP_WRITE);
  }

  // Setup desired Diligent state
  ImGui_ImplDiligent_SetupRenderState(draw_data, fr);

  // Render command lists
  // (Because we merged all buffers into a single one, we maintain our own offset into them)
  int global_vtx_offset = 0;
  int global_idx_offset = 0;
  ImVec2 clip_off = draw_data->DisplayPos;
  for (int n = 0; n < draw_data->CmdListsCount; n++)
  {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
    {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->ElemCount == 0)
        continue;
      else if (pcmd->UserCallback != nullptr)
      {
        // User callback, registered via ImDrawList::AddCallback()
        // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
          ImGui_ImplDiligent_SetupRenderState(draw_data, fr);
        else
          pcmd->UserCallback(cmd_list, pcmd);
      }
      else
      {
        // Project scissor/clipping rectangles into framebuffer space
        //ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
        //ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
        //if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
        //  continue;

        if (pcmd->TextureId)
        {
          bd->shaderResources->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "texture0")->Set((Diligent::ITextureView*)pcmd->TextureId);
          bd->context->CommitShaderResources(bd->shaderResources, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        bd->context->DrawIndexed(Diligent::DrawIndexedAttribs{ pcmd->ElemCount, Diligent::VT_UINT16, Diligent::DRAW_FLAG_VERIFY_ALL, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0 });
      }
    }
    global_idx_offset += cmd_list->IdxBuffer.Size;
    global_vtx_offset += cmd_list->VtxBuffer.Size;
  }
}

static void ImGui_ImplDiligent_CreateFontsTexture()
{
  // Build texture atlas
  ImGuiIO& io = ImGui::GetIO();
  ImGui_ImplDiligent_Data* bd = ImGui_ImplDiligent_GetBackendData();
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  // Upload texture to graphics system
  {
    // Create texture
    Diligent::TextureDesc textureDesc;
    textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
    textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    textureDesc.Usage = Diligent::USAGE_IMMUTABLE;

    Diligent::TextureSubResData textureSubData (pixels, width * 4);
    Diligent::TextureData textureData (&textureSubData, 1, bd->context);

    bd->device->CreateTexture(textureDesc, &textureData, &bd->fontResource);
    IM_ASSERT(bd->fontResource);

    // Get texture view
    bd->fontResourceView = bd->fontResource->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

    // Upload to shader resources
    bd->shaderResources->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "texture0")->Set(bd->fontResourceView);
  }

  // Store our identifier
  io.Fonts->SetTexID((ImTextureID)bd->fontResourceView);
}

bool ImGui_ImplDiligent_CreateDeviceObjects()
{
  ImGui_ImplDiligent_Data* bd = ImGui_ImplDiligent_GetBackendData();
  if (!bd || !bd->device)
    return false;
  if (bd->pipelineState)
    ImGui_ImplDiligent_InvalidateDeviceObjects();

  Diligent::GraphicsPipelineStateCreateInfo pipelineStateCI;
  pipelineStateCI.PSODesc.Name = "ImGui Pipeline";
  pipelineStateCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

  // Vertex shader
  {
    Diligent::ShaderCreateInfo shaderCI;
    shaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    shaderCI.Desc.UseCombinedTextureSamplers = true; // This is for OpenGL backend to work.
    shaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
    shaderCI.Desc.Name = "ImGui Vertex";

    static const char* vertexShader =
     "cbuffer vertexBuffer\
      {\
        float4x4 ProjectionMatrix; \
      };\
      struct VS_INPUT\
      {\
        float2 pos : ATTRIB0;\
        float2 uv  : ATTRIB1;\
        float4 col : ATTRIB2;\
      };\
      struct PS_INPUT\
      {\
        float4 pos : SV_POSITION;\
        float4 col : COLOR0;\
        float2 uv  : TEXCOORD0;\
      };\
      PS_INPUT main(VS_INPUT input)\
      {\
        PS_INPUT output;\
        output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
        output.col = input.col;\
        output.uv  = input.uv;\
        return output;\
      }";

    shaderCI.Source = vertexShader;
    bd->device->CreateShader(shaderCI, &pipelineStateCI.pVS);
    if (!pipelineStateCI.pVS)
    {
      return false;
    }
  }

  // Pixel shader
  {
    Diligent::ShaderCreateInfo shaderCI;
    shaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    shaderCI.Desc.UseCombinedTextureSamplers = true; // This is for OpenGL backend to work.
    shaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
    shaderCI.Desc.Name = "ImGui Pixel";

    static const char* pixelShader =
     "struct PS_INPUT\
      {\
        float4 pos : SV_POSITION;\
        float4 col : COLOR0;\
        float2 uv  : TEXCOORD0;\
      };\
      SamplerState texture0_sampler;\
      Texture2D texture0;\
      float4 main(PS_INPUT input) : SV_Target\
      {\
        float4 out_col = input.col * texture0.Sample(texture0_sampler, input.uv);\
        return out_col;\
      }";

    shaderCI.Source = pixelShader;
    bd->device->CreateShader(shaderCI, &pipelineStateCI.pPS);
    if (!pipelineStateCI.pPS)
    {
      pipelineStateCI.pVS->Release();
      return false;
    }
  }

  Diligent::LayoutElement layoutElements[3];
  Diligent::ShaderResourceVariableDesc vars[1];
  Diligent::ImmutableSamplerDesc immutableSamplers[1];

  // Pipeline options
  {
    pipelineStateCI.GraphicsPipeline.NumRenderTargets = 1;
    pipelineStateCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB;
    pipelineStateCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D32_FLOAT;
    pipelineStateCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Blend description
    pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
    pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
    pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
    pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = Diligent::BLEND_OPERATION_ADD;
    pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
    pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
    pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
    pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;

    // Rasterizer description
    pipelineStateCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;

    // Depth-Stencil description
    pipelineStateCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;
    pipelineStateCI.GraphicsPipeline.DepthStencilDesc.DepthFunc = Diligent::COMPARISON_FUNC_ALWAYS;
    pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilFailOp = Diligent::STENCIL_OP_KEEP;
    pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilPassOp = Diligent::STENCIL_OP_KEEP;
    pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilFunc = Diligent::COMPARISON_FUNC_ALWAYS;
    pipelineStateCI.GraphicsPipeline.DepthStencilDesc.BackFace = pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace;

    // Pipeline inputs layout
    pipelineStateCI.GraphicsPipeline.InputLayout.NumElements = 3;
    pipelineStateCI.GraphicsPipeline.InputLayout.LayoutElements = layoutElements;
    layoutElements[0].InputIndex = 0;
    layoutElements[0].NumComponents = 2;
    layoutElements[0].ValueType = Diligent::VT_FLOAT32;
    layoutElements[1].InputIndex = 1;
    layoutElements[1].NumComponents = 2;
    layoutElements[1].ValueType = Diligent::VT_FLOAT32;
    layoutElements[2].InputIndex = 2;
    layoutElements[2].NumComponents = 4;
    layoutElements[2].ValueType = Diligent::VT_UINT8;
    layoutElements[2].IsNormalized = true;

    vars[0].Name = "texture0";
    vars[0].ShaderStages = Diligent::SHADER_TYPE_PIXEL;
    vars[0].Type = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;

    pipelineStateCI.PSODesc.ResourceLayout.Variables = vars;
    pipelineStateCI.PSODesc.ResourceLayout.NumVariables = 1;

    Diligent::SamplerDesc samplerLinearClamp;
    samplerLinearClamp.MinFilter = Diligent::FILTER_TYPE_LINEAR;
    samplerLinearClamp.MagFilter = Diligent::FILTER_TYPE_LINEAR;
    samplerLinearClamp.MipFilter = Diligent::FILTER_TYPE_LINEAR;
    samplerLinearClamp.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
    samplerLinearClamp.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
    samplerLinearClamp.AddressW = Diligent::TEXTURE_ADDRESS_WRAP;
    samplerLinearClamp.ComparisonFunc = Diligent::COMPARISON_FUNC_ALWAYS;
    samplerLinearClamp.MaxLOD = 0.0f;

    immutableSamplers[0].SamplerOrTextureName = "texture0";
    immutableSamplers[0].ShaderStages = Diligent::SHADER_TYPE_PIXEL;
    immutableSamplers[0].Desc = samplerLinearClamp;

    pipelineStateCI.PSODesc.ResourceLayout.ImmutableSamplers = immutableSamplers;
    pipelineStateCI.PSODesc.ResourceLayout.NumImmutableSamplers = 1;
  }

  bd->device->CreateGraphicsPipelineState(pipelineStateCI, &bd->pipelineState);
  pipelineStateCI.pVS->Release();
  pipelineStateCI.pPS->Release();

  if (!bd->pipelineState)
    return false;
  
  Diligent::BufferDesc mvpConstantDesc;
  mvpConstantDesc.Name = "ImGui MVP Constant";
  mvpConstantDesc.Size = sizeof(VERTEX_CONSTANT_BUFFER_Diligent);
  mvpConstantDesc.Usage = Diligent::USAGE_DYNAMIC;
  mvpConstantDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
  mvpConstantDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

  bd->device->CreateBuffer(mvpConstantDesc, nullptr, &bd->mvpConstant);
  bd->pipelineState->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "vertexBuffer")->Set(bd->mvpConstant);

  bd->pipelineState->CreateShaderResourceBinding(&bd->shaderResources, true);
  if (!bd->shaderResources)
  {
    bd->pipelineState.Release();
    return false;
  }

  ImGui_ImplDiligent_CreateFontsTexture();

  return true;
}

void ImGui_ImplDiligent_InvalidateDeviceObjects()
{
  ImGui_ImplDiligent_Data* bd = ImGui_ImplDiligent_GetBackendData();
  if (!bd || !bd->device)
    return;
  ImGuiIO& io = ImGui::GetIO();

  bd->Release();
  io.Fonts->SetTexID(0); // We copied bd->pFontTextureView to io.Fonts->TexID so let's clear that as well.

}

bool ImGui_ImplDiligent_Init(Diligent::IRenderDevice* device, Diligent::IDeviceContext* context)
{
  ImGuiIO& io = ImGui::GetIO();
  IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");
  IM_ASSERT(device && context);

  // Setup backend capabilities flags
  ImGui_ImplDiligent_Data* bd = IM_NEW(ImGui_ImplDiligent_Data)();
  io.BackendRendererUserData = (void*)bd;
  io.BackendRendererName = "imgui_impl_diligent";
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

  bd->device = device;
  bd->context = context;

  return true;
}

void ImGui_ImplDiligent_Shutdown()
{
  ImGui_ImplDiligent_Data* bd = ImGui_ImplDiligent_GetBackendData();
  IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
  ImGuiIO& io = ImGui::GetIO();

  // Clean up windows and device objects
  ImGui_ImplDiligent_InvalidateDeviceObjects();
  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  IM_DELETE(bd);
}

void ImGui_ImplDiligent_NewFrame()
{
  ImGui_ImplDiligent_Data* bd = ImGui_ImplDiligent_GetBackendData();
  IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplDiligent_Init()?");

  if (!bd->pipelineState)
    ImGui_ImplDiligent_CreateDeviceObjects();
}
