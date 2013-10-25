// D3D11RenderFactory.cpp
// KlayGE D3D11��Ⱦ������󹤳� ʵ���ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Query.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>

#include <KlayGE/D3D11/D3D11RenderFactory.hpp>
#include <KlayGE/D3D11/D3D11RenderFactoryInternal.hpp>

namespace KlayGE
{
	D3D11RenderFactory::D3D11RenderFactory()
	{
	}

	std::wstring const & D3D11RenderFactory::Name() const
	{
		static std::wstring const name(L"Direct3D11 Render Factory");
		return name;
	}

	TexturePtr D3D11RenderFactory::MakeTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
	{
		return MakeSharedPtr<D3D11Texture1D>(width, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}
	TexturePtr D3D11RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
	{
		return MakeSharedPtr<D3D11Texture2D>(width, height, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}
	TexturePtr D3D11RenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
	{
		return MakeSharedPtr<D3D11Texture3D>(width, height, depth, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}
	TexturePtr D3D11RenderFactory::MakeTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
	{
		return MakeSharedPtr<D3D11TextureCube>(size, numMipMaps, array_size, format, sample_count, sample_quality, access_hint, init_data);
	}

	FrameBufferPtr D3D11RenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<D3D11FrameBuffer>();
	}

	RenderLayoutPtr D3D11RenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<D3D11RenderLayout>();
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData const * init_data, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_VERTEX_BUFFER, init_data, fmt);
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, ElementInitData const * init_data, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_INDEX_BUFFER, init_data, fmt);
	}

	QueryPtr D3D11RenderFactory::MakeOcclusionQuery()
	{
		return MakeSharedPtr<D3D11OcclusionQuery>();
	}

	QueryPtr D3D11RenderFactory::MakeConditionalRender()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model >= 4)
		{
			return MakeSharedPtr<D3D11ConditionalRender>();
		}
		else
		{
			return QueryPtr();
		}
	}

	RenderViewPtr D3D11RenderFactory::Make1DRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, array_index, face, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return this->Make3DRenderView(texture, array_index, slice, 1, level);
	}

	RenderViewPtr D3D11RenderFactory::MakeCubeRenderView(Texture& texture, int array_index, int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make3DRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, array_index, first_slice, num_slices, level);
	}

	RenderViewPtr D3D11RenderFactory::MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer,
		uint32_t width, uint32_t height, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(gbuffer, width, height, pf);
	}

	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(uint32_t width, uint32_t height,
		ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(width, height, pf, sample_count, sample_quality);
	}

	RenderViewPtr D3D11RenderFactory::Make1DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, first_array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, array_index, face, level);
	}
	
	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return this->Make3DDepthStencilRenderView(texture, array_index, slice, 1, level);
	}

	RenderViewPtr D3D11RenderFactory::MakeCubeDepthStencilRenderView(Texture& texture, int array_index, int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, array_index, array_size, level);
	}
	
	RenderViewPtr D3D11RenderFactory::Make3DDepthStencilRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, array_index, first_slice, num_slices, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make1DUnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make2DUnorderedAccessView(Texture& texture, int first_array_index, int array_size, int level)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make2DUnorderedAccessView(Texture& texture, int array_index, Texture::CubeFaces face, int level)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, array_index, face, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make2DUnorderedAccessView(Texture& texture, int array_index, uint32_t slice, int level)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, array_index, slice, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::MakeCubeUnorderedAccessView(Texture& texture, int array_index, int level)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make3DUnorderedAccessView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, array_index, first_slice, num_slices, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::MakeGraphicsBufferUnorderedAccessView(GraphicsBuffer& gbuffer, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(gbuffer, pf);
	}

	ShaderObjectPtr D3D11RenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<D3D11ShaderObject>();
	}

	RenderEnginePtr D3D11RenderFactory::DoMakeRenderEngine()
	{
		return MakeSharedPtr<D3D11RenderEngine>();
	}

	RasterizerStateObjectPtr D3D11RenderFactory::DoMakeRasterizerStateObject(RasterizerStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11RasterizerStateObject>(desc);
	}

	DepthStencilStateObjectPtr D3D11RenderFactory::DoMakeDepthStencilStateObject(DepthStencilStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11DepthStencilStateObject>(desc);
	}

	BlendStateObjectPtr D3D11RenderFactory::DoMakeBlendStateObject(BlendStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11BlendStateObject>(desc);
	}

	SamplerStateObjectPtr D3D11RenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11SamplerStateObject>(desc);
	}
}

void MakeRenderFactory(KlayGE::RenderFactoryPtr& ptr)
{
	ptr = KlayGE::MakeSharedPtr<KlayGE::D3D11RenderFactory>();
}
