#include "MapTileRenderer.h"
#include "components/MapTile.h"
#include <views/WorldView.h>

void MapTileRenderer::Upload()
{
	//Diligent::BufferDesc mvpConstantDesc;
	//mvpConstantDesc.Name = "MapTile GPU Data Buffer";
	//mvpConstantDesc.Size = sizeof(MapTile::GPUData) * (16 * 16) * (64 * 64);
	//mvpConstantDesc.Usage = Diligent::USAGE_STAGING;
	//mvpConstantDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
	//mvpConstantDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

	//SRendererDevice->CreateBuffer(mvpConstantDesc, nullptr, &gpuDataBuffer);

	//// Vertex data
	//{
	//	Diligent::BufferDesc bufferDesc;
	//	bufferDesc.Usage = Diligent::USAGE_DEFAULT;
	//	bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
	//	bufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
	//	bufferDesc.MiscFlags = Diligent::MISC_BUFFER_FLAG_NONE;
	//	bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
	//	bufferDesc.Size = (8*8 + 9*9) * sizeof(glm::vec3);
	//	bufferDesc.ElementByteStride = sizeof(glm::vec3);

	//	glm::vec3 vertexData[] =
	//	{
	//		{  }
	//	};

	//	Diligent::BufferData bufferData;
	//	bufferData.pContext = SRendererContext;
	//	bufferData.DataSize = (8 * 8 + 9 * 9) * sizeof(glm::vec3);
	//	bufferData.pData = vertexData;

	//	SRendererDevice->CreateBuffer(bufferDesc, &bufferData, &_vertexBuffer);

	//	if (!_vertexBuffer)
	//		assert(false && "Failed to create MapTile vertex buffer.");
	//}

	//// Index Data
	//{
	//	Diligent::BufferDesc bufferDesc;
	//	bufferDesc.Usage = Diligent::USAGE_DEFAULT;
	//	bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
	//	bufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
	//	bufferDesc.MiscFlags = Diligent::MISC_BUFFER_FLAG_NONE;
	//	bufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
	//	bufferDesc.Size = 6 * sizeof(uint32_t);
	//	bufferDesc.ElementByteStride = sizeof(uint32_t);

	//	uint32_t indexData[] =
	//	{
	//		0, 1, 2,
	//		2, 3, 1
	//	};

	//	Diligent::BufferData bufferData;
	//	bufferData.pContext = SRendererContext;
	//	bufferData.DataSize = 6 * sizeof(uint32_t);
	//	bufferData.pData = indexData;

	//	SRendererDevice->CreateBuffer(bufferDesc, &bufferData, &_indexBuffer);
	//	if (!_indexBuffer)
	//		assert(false && "Failed to create WorldGrid index buffer.");
	//}
}

void MapTileRenderer::Unload()
{
}

void MapTileRenderer::Render(WorldView* view)
{
	//SRendererContext->SetPipelineState(_pipeline);

	//Diligent::IBuffer* vertexBuffers[] = { _vertexBuffer };
	//uint64_t vertexOffsets[] = { 0 };
	//SRendererContext->SetVertexBuffers(0, 1, vertexBuffers, vertexOffsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);
	//SRendererContext->SetIndexBuffer(_indexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	//SRendererContext->CommitShaderResources(_shaderResources, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	//SRendererContext->DrawIndexed(Diligent::DrawIndexedAttribs{ 6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL });

}
