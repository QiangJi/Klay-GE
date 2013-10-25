// D3D11RenderEngine.hpp
// KlayGE D3D11��Ⱦ������ ͷ�ļ�
// Ver 3.11.0
// ��Ȩ����(C) ������, 2009-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Remove TexelToPixelOffset (2010.9.26)
//
// 3.10.0
// ������DXGI 1.1 (2010.2.8)
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11RENDERENGINE_HPP
#define _D3D11RENDERENGINE_HPP

#pragma once

#include <KFL/Vector.hpp>
#include <KFL/Color.hpp>

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <D3D11Shader.h>

#include <vector>
#include <set>
#include <map>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/D3D11/D3D11AdapterList.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>

namespace KlayGE
{
	class D3D11AdapterList;
	class D3D11Adapter;

	class D3D11RenderEngine : public RenderEngine
	{
	public:
		D3D11RenderEngine();
		~D3D11RenderEngine();

		std::wstring const & Name() const;

		bool RequiresFlipping() const
		{
			return true;
		}

		IDXGIFactory1Ptr const & DXGIFactory() const;
		ID3D11DevicePtr const & D3DDevice() const;
		ID3D11DeviceContextPtr const & D3DDeviceImmContext() const;
		bool IsD3D11_1() const;
		D3D_FEATURE_LEVEL DeviceFeatureLevel() const;
		void D3DDevice(ID3D11DevicePtr const & device, ID3D11DeviceContextPtr const & imm_ctx, D3D_FEATURE_LEVEL feature_level);

		void ForceFlush();

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		bool FullScreen() const;
		void FullScreen(bool fs);

		std::string const & VertexShaderProfile() const
		{
			return vs_profile_;
		}
		std::string const & PixelShaderProfile() const
		{
			return ps_profile_;
		}
		std::string const & GeometryShaderProfile() const
		{
			return gs_profile_;
		}
		std::string const & ComputeShaderProfile() const
		{
			return cs_profile_;
		}
		std::string const & HullShaderProfile() const
		{
			return hs_profile_;
		}
		std::string const & DomainShaderProfile() const
		{
			return ds_profile_;
		}

		void RSSetState(ID3D11RasterizerStatePtr const & ras);
		void OMSetDepthStencilState(ID3D11DepthStencilStatePtr const & ds, uint16_t stencil_ref);
		void OMSetBlendState(ID3D11BlendStatePtr const & bs, Color const & blend_factor, uint32_t sample_mask);
		void VSSetShader(ID3D11VertexShaderPtr const & shader);
		void PSSetShader(ID3D11PixelShaderPtr const & shader);
		void GSSetShader(ID3D11GeometryShaderPtr const & shader);
		void CSSetShader(ID3D11ComputeShaderPtr const & shader);
		void HSSetShader(ID3D11HullShaderPtr const & shader);
		void DSSetShader(ID3D11DomainShaderPtr const & shader);
		void SetShaderResources(ShaderObject::ShaderType st, std::vector<tuple<void*, uint32_t, uint32_t> > const & srvsrcs, std::vector<ID3D11ShaderResourceViewPtr> const & srvs);
		void SetSamplers(ShaderObject::ShaderType st, std::vector<ID3D11SamplerStatePtr> const & samplers);
		void SetConstantBuffers(ShaderObject::ShaderType st, std::vector<ID3D11BufferPtr> const & cbs);
		void RSSetViewports(UINT NumViewports, D3D11_VIEWPORT const * pViewports);
		
		void ResetRenderStates();
		void DetachSRV(void* rtv_src, uint32_t rt_first_subres, uint32_t rt_num_subres);

		ID3D11InputLayoutPtr const & CreateD3D11InputLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> const & elems, size_t signature, std::vector<uint8_t> const & vs_code);

		HRESULT D3D11CreateDevice(IDXGIAdapter* pAdapter,
								D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
								D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
								ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) const;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HRESULT D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
								D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
								LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs) const;
		HRESULT D3DReflect(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector) const;
		HRESULT D3DStripShader(LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags, ID3DBlob** ppStrippedBlob) const;
#endif

	private:
		void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings);
		void DoBindFrameBuffer(FrameBufferPtr const & fb);
		void DoBindSOBuffers(RenderLayoutPtr const & rl);
		void DoRender(RenderTechnique const & tech, RenderLayout const & rl);
		void DoDispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz);
		void DoResize(uint32_t width, uint32_t height);

		void FillRenderDeviceCaps();

		virtual void StereoscopicForLCDShutter(int32_t eye) KLAYGE_OVERRIDE;

		bool VertexFormatSupport(ElementFormat elem_fmt);
		bool TextureFormatSupport(ElementFormat elem_fmt);
		bool RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality);

		virtual void CheckConfig(RenderSettings& settings) KLAYGE_OVERRIDE;

	private:
		D3D11AdapterList const & D3DAdapters() const;
		D3D11AdapterPtr const & ActiveAdapter() const;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HMODULE mod_dxgi_;
		HMODULE mod_d3d11_;
		HMODULE mod_d3dcompiler_;

		typedef HRESULT (WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);
		typedef HRESULT (WINAPI *D3D11CreateDeviceFunc)(IDXGIAdapter* pAdapter,
								D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
								D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
								ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
		typedef HRESULT (WINAPI *D3DCompileFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
								D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
								LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
		typedef HRESULT (WINAPI *D3DReflectFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector);
		typedef HRESULT (WINAPI *D3DStripShaderFunc)(LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags, ID3DBlob** ppStrippedBlob);

		CreateDXGIFactory1Func DynamicCreateDXGIFactory1_;
		D3D11CreateDeviceFunc DynamicD3D11CreateDevice_;
		D3DCompileFunc DynamicD3DCompile_;
		D3DReflectFunc DynamicD3DReflect_;
		D3DStripShaderFunc DynamicD3DStripShader_;
#endif

		// Direct3D rendering device
		// Only created after top-level window created
		IDXGIFactory1Ptr	gi_factory_;
		ID3D11DevicePtr		d3d_device_;
		ID3D11DeviceContextPtr d3d_imm_ctx_;
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
		bool is_d3d_11_1_;
#endif
		D3D_FEATURE_LEVEL d3d_feature_level_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D11AdapterList adapterList_;

		ID3D11RasterizerStatePtr rasterizer_state_cache_;
		ID3D11DepthStencilStatePtr depth_stencil_state_cache_;
		uint16_t stencil_ref_cache_;
		ID3D11BlendStatePtr blend_state_cache_;
		Color blend_factor_cache_;
		uint32_t sample_mask_cache_;
		ID3D11VertexShaderPtr vertex_shader_cache_;
		ID3D11PixelShaderPtr pixel_shader_cache_;
		ID3D11GeometryShaderPtr geometry_shader_cache_;
		ID3D11ComputeShaderPtr compute_shader_cache_;
		ID3D11HullShaderPtr hull_shader_cache_;
		ID3D11DomainShaderPtr domain_shader_cache_;
		RenderLayout::topology_type topology_type_cache_;
		ID3D11InputLayoutPtr input_layout_cache_;
		D3D11_VIEWPORT viewport_cache_;
		uint32_t num_so_buffs_;
		std::vector<ID3D11Buffer*> vb_cache_;
		std::vector<UINT> vb_stride_cache_;
		std::vector<UINT> vb_offset_cache_;
		ID3D11BufferPtr ib_cache_;

		array<std::vector<tuple<void*, uint32_t, uint32_t> >, ShaderObject::ST_NumShaderTypes> shader_srvsrc_cache_;
		array<std::vector<ID3D11ShaderResourceView*>, ShaderObject::ST_NumShaderTypes> shader_srv_ptr_cache_;
		array<std::vector<ID3D11SamplerState*>, ShaderObject::ST_NumShaderTypes> shader_sampler_ptr_cache_;
		array<std::vector<ID3D11Buffer*>, ShaderObject::ST_NumShaderTypes> shader_cb_ptr_cache_;
		array<std::vector<ID3D11ShaderResourceViewPtr>, ShaderObject::ST_NumShaderTypes> shader_srv_cache_;
		array<std::vector<ID3D11SamplerStatePtr>, ShaderObject::ST_NumShaderTypes> shader_sampler_cache_;
		array<std::vector<ID3D11BufferPtr>, ShaderObject::ST_NumShaderTypes> shader_cb_cache_;

		unordered_map<size_t, ID3D11InputLayoutPtr> input_layout_bank_;

		std::string vs_profile_, ps_profile_, gs_profile_, cs_profile_, hs_profile_, ds_profile_;

		enum StereoMethod
		{
			SM_None,
			SM_DXGI,
			SM_NV3DVision,
			SM_AMDQuadBuffer
		};

		StereoMethod stereo_method_;
		FrameBufferPtr stereo_nv_3d_vision_fb_;
		TexturePtr stereo_nv_3d_vision_tex_;

		std::set<ElementFormat> vertex_format_;
		std::set<ElementFormat> texture_format_;
		std::map<ElementFormat, std::vector<std::pair<uint32_t, uint32_t> > > rendertarget_format_;
	};

	typedef shared_ptr<D3D11RenderEngine> D3D11RenderEnginePtr;
}

#endif			// _D3D11RENDERENGINE_HPP
