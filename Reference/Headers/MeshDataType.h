#pragma once
#include "Engine_Defines.h"

NAMESPACE_(Engine)

NAMESPACE_(MeshDataType)
struct VertexPositionSize;
_NAMESPACE

using namespace MeshDataType;

// Interface template
template <typename VertexType, typename IndexType>
struct IMeshDataType
{
    IMeshDataType()
    {
	    if (sizeof(IndexType) > sizeof(UINT16))
	    {
            IndexFormat = DXGI_FORMAT_R32_UINT;
	    }
        else
        {
            IndexFormat = DXGI_FORMAT_R16_UINT;
        }
    }
    IMeshDataType(IMeshDataType& rhs) = default;
    ~IMeshDataType() = default;

    std::vector<VertexType> Get_typedVertices() { return vecVertexData; }
    std::vector<IndexType> Get_typedIndices() { return vecIndexData; }

    std::vector<VertexType> vecVertexData;
    std::vector<IndexType> vecIndexData; // UINT16 or UINT32

    DXGI_FORMAT IndexFormat;
};

struct MeshDataType::VertexPositionSize
{
    VertexPositionSize() = default;

    VertexPositionSize(const VertexPositionSize&) = default;
    VertexPositionSize& operator=(const VertexPositionSize&) = default;

    VertexPositionSize(VertexPositionSize&&) = default;
    VertexPositionSize& operator=(VertexPositionSize&&) = default;

    VertexPositionSize(XMFLOAT3 const& iposition, XMFLOAT2 const& isize) noexcept
        : position(iposition), size(isize)
    {
    }

    XMFLOAT3 position;
    XMFLOAT2 size;

    static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
    static constexpr unsigned int InputElementCount = 2;
    static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};
_NAMESPACE