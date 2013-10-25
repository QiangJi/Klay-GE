// RenderEngine.cpp
// KlayGE ��Ⱦ������ ʵ���ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2003-2009
// Homepage: http://www.klayge.org
//
// 3.10.0
// ������Dispatch (2009.12.22)
//
// 3.9.0
// ֧��Stream Output (2009.5.14)
//
// 3.6.0
// ȥ����RenderTarget��ֱ��ʹ��FrameBuffer (2007.6.20)
//
// 3.3.0
// ͳһ��RenderState (2006.5.21)
//
// 2.8.0
// ����StencilBuffer��ز��� (2005.7.20)
//
// 2.7.1
// ViewMatrix��ProjectionMatrix��Ϊconst (2005.7.10)
//
// 2.4.0
// ������NumPrimitivesJustRendered��NumVerticesJustRendered (2005.3.21)
//
// 2.0.3
// �Ż���RenderEffect������ (2004.2.16)
// ȥ����VO_2D (2004.3.1)
// ȥ����SoftwareBlend (2004.3.10)
//
// 2.0.0
// ���ν���(2003.10.1)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <sstream>

#include <KlayGE/RenderEngine.hpp>

namespace KlayGE
{
	class NullRenderEngine : public RenderEngine
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Render Engine");
			return name;
		}

		bool RequiresFlipping() const
		{
			return false;
		}

		void ForceFlush()
		{
		}

		void ScissorRect(uint32_t /*x*/, uint32_t /*y*/, uint32_t /*width*/, uint32_t /*height*/)
		{
		}

		bool FullScreen() const
		{
			return false;
		}

		void FullScreen(bool /*fs*/)
		{
		}

	private:
		void DoCreateRenderWindow(std::string const & /*name*/, RenderSettings const & /*settings*/)
		{
		}

		void DoBindFrameBuffer(FrameBufferPtr const & /*fb*/)
		{
		}

		void DoBindSOBuffers(RenderLayoutPtr const & /*rl*/)
		{
		}

		void DoRender(RenderTechnique const & /*tech*/, RenderLayout const & /*rl*/)
		{
		}

		void DoDispatch(RenderTechnique const & /*tech*/, uint32_t /*tgx*/, uint32_t /*tgy*/, uint32_t /*tgz*/)
		{
		}

		void DoResize(uint32_t /*width*/, uint32_t /*height*/)
		{
		}
	};

	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderEngine()
		: numPrimitivesJustRendered_(0),
			numVerticesJustRendered_(0),
			cur_front_stencil_ref_(0),
			cur_back_stencil_ref_(0),
			cur_blend_factor_(1, 1, 1, 1),
			cur_sample_mask_(0xFFFFFFFF),
			motion_frames_(0),
			stereo_method_(STM_None), stereo_separation_(0),
			fb_stage_(0)
	{
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::~RenderEngine()
	{
	}

	// ���ؿն���
	/////////////////////////////////////////////////////////////////////////////////
	RenderEnginePtr RenderEngine::NullObject()
	{
		static RenderEnginePtr obj = MakeSharedPtr<NullRenderEngine>();
		return obj;
	}

	void RenderEngine::BeginFrame()
	{
	}

	void RenderEngine::BeginPass()
	{
	}

	void RenderEngine::EndFrame()
	{
		this->BindFrameBuffer(default_frame_buffers_[0]);
	}

	void RenderEngine::EndPass()
	{
	}

	// ������Ⱦ����
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::CreateRenderWindow(std::string const & name, RenderSettings& settings)
	{
		stereo_separation_ = settings.stereo_separation;
		this->DoCreateRenderWindow(name, settings);
		this->CheckConfig(settings);
		RenderDeviceCaps const & caps = this->DeviceCaps();

		screen_frame_buffer_ = cur_frame_buffer_;

		uint32_t const screen_width = screen_frame_buffer_->Width();
		uint32_t const screen_height = screen_frame_buffer_->Height();
		uint32_t const render_width = settings.width;
		uint32_t const render_height = settings.height;

		hdr_enabled_ = settings.hdr;
		if (settings.hdr)
		{
			hdr_pp_ = MakeSharedPtr<HDRPostProcess>(settings.fft_lens_effects);
			skip_hdr_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");
		}

		ppaa_enabled_ = settings.ppaa ? 1 : 0;
		gamma_enabled_ = settings.gamma;
		color_grading_enabled_ = settings.color_grading;
		if (settings.ppaa || settings.color_grading || settings.gamma)
		{
			for (size_t i = 0; i < 12; ++ i)
			{
				std::ostringstream iss;
				iss << "PostToneMapping" << i;
				ldr_pps_[i] = SyncLoadPostProcess("PostToneMapping.ppml", iss.str());
			}

			ldr_pp_ = ldr_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}

		bool need_resize = ((render_width != screen_width) || (render_height != screen_height));
		{
			resize_pps_[0] = SyncLoadPostProcess("Resizer.ppml", "bilinear");
			resize_pps_[1] = MakeSharedPtr<BicubicFilteringPostProcess>();

			float const scale_x = static_cast<float>(screen_width) / render_width;
			float const scale_y = static_cast<float>(screen_height) / render_height;

			float2 pos_scale;
			if (scale_x < scale_y)
			{
				pos_scale.x() = 1;
				pos_scale.y() = (scale_x * render_height) / screen_height;
			}
			else
			{
				pos_scale.x() = (scale_y * render_width) / screen_width;
				pos_scale.y() = 1;
			}

			for (size_t i = 0; i < 2; ++ i)
			{
				resize_pps_[i]->SetParam(0, pos_scale);
			}
		}

		for (int i = 0; i < 4; ++ i)
		{
			default_frame_buffers_[i] = screen_frame_buffer_;
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderViewPtr ds_view;
		if (hdr_pp_ || ldr_pp_ || (settings.stereo_method != STM_None))
		{
			if (caps.texture_format_support(EF_D24S8) || caps.texture_format_support(EF_D16))
			{
				ElementFormat fmt;
				if (caps.texture_format_support(settings.depth_stencil_fmt))
				{
					fmt = settings.depth_stencil_fmt;
				}
				else
				{
					BOOST_ASSERT(caps.texture_format_support(EF_D16));

					fmt = EF_D16;
				}
				ds_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);
			}
			else
			{
				ElementFormat fmt;
				if (caps.rendertarget_format_support(settings.depth_stencil_fmt, 1, 0))
				{
					fmt = settings.depth_stencil_fmt;
				}
				else
				{
					BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

					fmt = EF_D16;
				}
				ds_view = rf.Make2DDepthStencilRenderView(render_width, render_height, fmt, 1, 0);
			}
		}

		if (settings.stereo_method != STM_None)
		{
			mono_frame_buffer_ = rf.MakeFrameBuffer();
			mono_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			ElementFormat fmt;
			if (caps.texture_format_support(settings.color_fmt) && caps.rendertarget_format_support(settings.color_fmt, 1, 0))
			{
				fmt = settings.color_fmt;
			}
			else
			{
				if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
				{
					fmt = EF_ABGR8;
				}
				else
				{
					BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));

					fmt = EF_ARGB8;
				}
			}

			mono_tex_ = rf.MakeTexture2D(screen_width, screen_height, 1, 1,
				fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			mono_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*mono_tex_, 0, 1, 0));

			default_frame_buffers_[0] = default_frame_buffers_[1]
				= default_frame_buffers_[2] = mono_frame_buffer_;

			overlay_frame_buffer_ = rf.MakeFrameBuffer();
			overlay_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			overlay_tex_ = rf.MakeTexture2D(screen_width, screen_height, 1, 1,
				fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			overlay_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*overlay_tex_, 0, 1, 0));

			RenderViewPtr screen_size_ds_view;
			if (need_resize)
			{
				screen_size_ds_view = rf.Make2DDepthStencilRenderView(screen_width, screen_height, ds_view->Format(), 1, 0);
			}
			else
			{
				screen_size_ds_view = ds_view;
			}
			overlay_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, screen_size_ds_view);
		}
		else
		{
			if (need_resize)
			{
				resize_frame_buffer_ = rf.MakeFrameBuffer();
				resize_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

				ElementFormat fmt;
				if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
				{
					fmt = EF_ABGR8;
				}
				else
				{
					BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));

					fmt = EF_ARGB8;
				}

				resize_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1,
					fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				resize_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*resize_tex_, 0, 1, 0));

				default_frame_buffers_[0] = default_frame_buffers_[1]
					= default_frame_buffers_[2] = resize_frame_buffer_;
			}
		}
		
		if (ldr_pp_)
		{
			ldr_frame_buffer_ = rf.MakeFrameBuffer();
			ldr_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			ElementFormat fmt;
			if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));

				fmt = EF_ARGB8;
			}
			ElementFormat fmt_srgb = MakeSRGB(fmt);
			if (caps.texture_format_support(fmt_srgb) && caps.rendertarget_format_support(fmt_srgb, 1, 0))
			{
				fmt = fmt_srgb;
			}

			ldr_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			ldr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ldr_tex_, 0, 1, 0));
			ldr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

			default_frame_buffers_[0] = default_frame_buffers_[1] = ldr_frame_buffer_;
		}

		if (hdr_pp_)
		{
			hdr_frame_buffer_ = rf.MakeFrameBuffer();
			hdr_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			bool non_fp_tex;
			ElementFormat fmt;
			if (caps.texture_format_support(EF_B10G11R11F) && caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
			{
				fmt = EF_B10G11R11F;
				non_fp_tex = false;
			}
			else if (caps.texture_format_support(EF_ABGR16F) && caps.rendertarget_format_support(EF_ABGR16F, 1, 0))
			{
				fmt = EF_ABGR16F;
				non_fp_tex = false;
			}
			else if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt = EF_ABGR8;
				non_fp_tex = true;
			}
			else
			{
				BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));

				fmt = EF_ARGB8;
				non_fp_tex = true;
			}
			if (non_fp_tex)
			{
				ElementFormat fmt_srgb = MakeSRGB(fmt);
				if (caps.rendertarget_format_support(fmt_srgb, 1, 0))
				{
					fmt = fmt_srgb;
				}
			}
			hdr_tex_ = rf.MakeTexture2D(render_width, render_height, 4, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			hdr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0, 1, 0));
			hdr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

			default_frame_buffers_[0] = hdr_frame_buffer_;
		}

		this->BindFrameBuffer(default_frame_buffers_[0]);
		this->Stereo(settings.stereo_method);
	}

	void RenderEngine::CheckConfig(RenderSettings& /*settings*/)
	{
	}

	// ���õ�ǰ��Ⱦ״̬����
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::SetStateObjects(RasterizerStateObjectPtr const & rs_obj,
		DepthStencilStateObjectPtr const & dss_obj, uint16_t front_stencil_ref, uint16_t back_stencil_ref,
		BlendStateObjectPtr const & bs_obj, Color const & blend_factor, uint32_t sample_mask)
	{
		if (cur_rs_obj_ != rs_obj)
		{
			rs_obj->Active();
			cur_rs_obj_ = rs_obj;
		}

		if ((cur_dss_obj_ != dss_obj) || (cur_front_stencil_ref_ != front_stencil_ref) || (cur_back_stencil_ref_ != back_stencil_ref))
		{
			dss_obj->Active(front_stencil_ref, back_stencil_ref);
			cur_dss_obj_ = dss_obj;
			cur_front_stencil_ref_ = front_stencil_ref;
			cur_back_stencil_ref_ = back_stencil_ref;
		}

		if ((cur_bs_obj_ != bs_obj) || (cur_blend_factor_ != blend_factor) || (cur_sample_mask_ != sample_mask))
		{
			bs_obj->Active(blend_factor, sample_mask);
			cur_bs_obj_ = bs_obj;
			cur_blend_factor_ = blend_factor;
			cur_sample_mask_ = sample_mask;
		}
	}

	// ���õ�ǰ��ȾĿ��
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::BindFrameBuffer(FrameBufferPtr const & fb)
	{
		FrameBufferPtr new_fb;
		if (!fb)
		{
			new_fb = this->DefaultFrameBuffer();
		}
		else
		{
			new_fb = fb;
		}

		if ((fb != new_fb) || (fb && fb->Dirty()))
		{
			if (cur_frame_buffer_)
			{
				cur_frame_buffer_->OnUnbind();
			}

			cur_frame_buffer_ = new_fb;
			cur_frame_buffer_->OnBind();

			this->DoBindFrameBuffer(cur_frame_buffer_);
		}
	}

	// ��ȡ��ǰ��ȾĿ��
	/////////////////////////////////////////////////////////////////////////////////
	FrameBufferPtr const & RenderEngine::CurFrameBuffer() const
	{
		return cur_frame_buffer_;
	}

	// ��ȡĬ����ȾĿ��
	/////////////////////////////////////////////////////////////////////////////////
	FrameBufferPtr const & RenderEngine::DefaultFrameBuffer() const
	{
		return default_frame_buffers_[fb_stage_];
	}

	// ��ȡ��Ļ��ȾĿ��
	/////////////////////////////////////////////////////////////////////////////////
	FrameBufferPtr const & RenderEngine::ScreenFrameBuffer() const
	{
		return screen_frame_buffer_;
	}

	FrameBufferPtr const & RenderEngine::OverlayFrameBuffer() const
	{
		return overlay_frame_buffer_;
	}

	void RenderEngine::BindSOBuffers(RenderLayoutPtr const & rl)
	{
		so_buffers_ = rl;
		this->DoBindSOBuffers(rl);
	}

	// ��Ⱦһ��vb
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::Render(RenderTechnique const & tech, RenderLayout const & rl)
	{
		this->DoRender(tech, rl);
	}

	void RenderEngine::Dispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		this->DoDispatch(tech, tgx, tgy, tgz);
	}

	// �ϴ�Render()����Ⱦ��ͼԪ��
	/////////////////////////////////////////////////////////////////////////////////
	size_t RenderEngine::NumPrimitivesJustRendered()
	{
		size_t const ret = numPrimitivesJustRendered_;
		numPrimitivesJustRendered_ = 0;
		return ret;
	}

	// �ϴ�Render()����Ⱦ�Ķ�����
	/////////////////////////////////////////////////////////////////////////////////
	size_t RenderEngine::NumVerticesJustRendered()
	{
		size_t const ret = numVerticesJustRendered_;
		numVerticesJustRendered_ = 0;
		return ret;
	}

	// ��ȡ��Ⱦ�豸����
	/////////////////////////////////////////////////////////////////////////////////
	RenderDeviceCaps const & RenderEngine::DeviceCaps() const
	{
		return caps_;
	}

	void RenderEngine::GetCustomAttrib(std::string const & /*name*/, void* /*value*/)
	{
	}

	void RenderEngine::SetCustomAttrib(std::string const & /*name*/, void* /*value*/)
	{
	}

	void RenderEngine::Resize(uint32_t width, uint32_t height)
	{
		uint32_t const old_screen_width = default_frame_buffers_[3]->Width();
		uint32_t const old_screen_height = default_frame_buffers_[3]->Height();
		uint32_t const old_render_width = default_frame_buffers_[0]->Width();
		uint32_t const old_render_height = default_frame_buffers_[0]->Height();
		uint32_t const new_screen_width = width;
		uint32_t const new_screen_height = height;
		uint32_t const new_render_width = new_screen_width * old_render_width / old_screen_width;
		uint32_t const new_render_height = new_screen_height * old_render_height / old_screen_height;
		if ((old_screen_width != new_screen_width) || (old_screen_height != new_screen_height))
		{
			RenderSettings const & settings = Context::Instance().Config().graphics_cfg;
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			RenderViewPtr ds_view;
			if (hdr_pp_ || ldr_pp_ || (stereo_method_ != STM_None))
			{
				if (caps.texture_format_support(EF_D24S8) || caps.texture_format_support(EF_D16))
				{
					ElementFormat fmt = ds_tex_->Format();
					ds_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
					ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);
				}
				else
				{
					ElementFormat fmt;
					if (caps.rendertarget_format_support(settings.depth_stencil_fmt, 1, 0))
					{
						fmt = settings.depth_stencil_fmt;
					}
					else
					{
						BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

						fmt = EF_D16;
					}
					ds_view = rf.Make2DDepthStencilRenderView(new_render_width, new_render_height, fmt, 1, 0);
				}
			}

			if (stereo_method_ != STM_None)
			{
				ElementFormat fmt = mono_tex_->Format();
				mono_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				mono_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*mono_tex_, 0, 1, 0));
			}
			else
			{
				bool need_resize = ((new_render_width != new_screen_width) || (new_render_height != new_screen_height));
				if (need_resize)
				{
					ElementFormat fmt = resize_tex_->Format();
					resize_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
					resize_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*resize_tex_, 0, 1, 0));

					float const scale_x = static_cast<float>(new_screen_width) / new_render_width;
					float const scale_y = static_cast<float>(new_screen_height) / new_render_height;

					float2 pos_scale;
					if (scale_x < scale_y)
					{
						pos_scale.x() = 1;
						pos_scale.y() = (scale_x * new_render_height) / new_screen_height;
					}
					else
					{
						pos_scale.x() = (scale_y * new_render_width) / new_screen_width;
						pos_scale.y() = 1;
					}

					for (size_t i = 0; i < 2; ++ i)
					{
						resize_pps_[i]->SetParam(0, pos_scale);
					}
				}
			}
			if (ldr_pp_)
			{
				ElementFormat fmt = ldr_tex_->Format();
				ldr_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				ldr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ldr_tex_, 0, 1, 0));
				ldr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
			}
			if (hdr_pp_)
			{
				ElementFormat fmt = hdr_tex_->Format();
				hdr_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 4, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
				hdr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0, 1, 0));
				hdr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
			}

			pp_chain_dirty_ = true;

			this->DoResize(new_render_width, new_render_height);
		}
		else
		{
			this->DoResize(old_render_width, old_render_height);
		}
	}

	void RenderEngine::PostProcess(bool skip)
	{
		if (pp_chain_dirty_)
		{
			this->AssemblePostProcessChain();
			pp_chain_dirty_ = false;
		}

		fb_stage_ = 1;

		if (hdr_enabled_)
		{
			if (skip)
			{
				skip_hdr_pp_->Apply();
			}
			else
			{
				hdr_tex_->BuildMipSubLevels();
				hdr_pp_->Apply();
			}
		}
		else
		{
			if (skip_hdr_pp_)
			{
				skip_hdr_pp_->Apply();
			}
		}

		fb_stage_ = 2;

		if (ppaa_enabled_ || gamma_enabled_ || color_grading_enabled_)
		{
			if (skip)
			{
				ldr_pps_[0]->Apply();
			}
			else
			{
				ldr_pp_->Apply();
			}
		}
		else
		{
			if (ldr_pps_[0])
			{
				ldr_pps_[0]->Apply();
			}
		}

		fb_stage_ = 3;

		uint32_t const screen_width = default_frame_buffers_[3]->Width();
		uint32_t const screen_height = default_frame_buffers_[3]->Height();
		uint32_t const render_width = default_frame_buffers_[0]->Width();
		uint32_t const render_height = default_frame_buffers_[0]->Height();
		bool need_resize = ((render_width != screen_width) || (render_height != screen_height));
		if ((STM_None == stereo_method_) && need_resize)
		{
			float const scale_x = static_cast<float>(screen_width) / render_width;
			float const scale_y = static_cast<float>(screen_height) / render_height;

			if (!MathLib::equal(scale_x, scale_y))
			{
				this->DefaultFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
			}

			if ((scale_x > 2) || (scale_y > 2))
			{
				resize_pps_[1]->Apply();
			}
			else
			{
				resize_pps_[0]->Apply();
			}
		}

		this->DefaultFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

		fb_stage_ = 0;
	}

	void RenderEngine::HDREnabled(bool hdr)
	{
		if (hdr_pp_)
		{
			pp_chain_dirty_ = true;
			hdr_enabled_ = hdr;
		}
	}

	void RenderEngine::PPAAEnabled(int aa)
	{
		if (ldr_pp_)
		{
			pp_chain_dirty_ = true;
			ppaa_enabled_ = aa;
			ldr_pp_ = ldr_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}
	}

	void RenderEngine::GammaEnabled(bool gamma)
	{
		if (ldr_pp_)
		{
			pp_chain_dirty_ = true;
			gamma_enabled_ = gamma;
			ldr_pp_ = ldr_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}
	}

	void RenderEngine::ColorGradingEnabled(bool cg)
	{
		if (ldr_pp_)
		{
			pp_chain_dirty_ = true;
			color_grading_enabled_ = cg;
			ldr_pp_ = ldr_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}
	}

	void RenderEngine::Refresh()
	{
		FrameBuffer& fb = *this->ScreenFrameBuffer();
		if (Context::Instance().AppInstance().MainWnd()->Active())
		{
			Context::Instance().SceneManagerInstance().Update();
			fb.SwapBuffers();
		}
	}

	void RenderEngine::Stereoscopic()
	{
		if (stereo_method_ != STM_None)
		{
			fb_stage_ = 3;

			this->BindFrameBuffer(screen_frame_buffer_);
			if (stereoscopic_pp_)
			{
				stereoscopic_pp_->SetParam(0, stereo_separation_);
				stereoscopic_pp_->SetParam(1, screen_frame_buffer_->GetViewport()->camera->NearPlane());
			}
			if (stereo_method_ != STM_LCDShutter)
			{
				stereoscopic_pp_->Render();
			}
			else
			{
				this->StereoscopicForLCDShutter(1);
				this->StereoscopicForLCDShutter(0);

				this->BindFrameBuffer(screen_frame_buffer_);
			}

			fb_stage_ = 0;
		}
	}

	void RenderEngine::StereoscopicForLCDShutter(int32_t /*eye*/)
	{
	}

	void RenderEngine::Stereo(StereoMethod method)
	{
		stereo_method_ = method;
		if (stereo_method_ != STM_None)
		{
			std::string pp_name;
			switch (stereo_method_)
			{
			case STM_ColorAnaglyph_RedCyan:
				pp_name = "stereoscopic_red_cyan";
				break;

			case STM_ColorAnaglyph_YellowBlue:
				pp_name = "stereoscopic_yellow_blue";
				break;

			case STM_ColorAnaglyph_GreenRed:
				pp_name = "stereoscopic_green_red";
				break;

			case STM_HorizontalInterlacing:
				pp_name = "stereoscopic_hor_interlacing";
				break;

			case STM_VerticalInterlacing:
				pp_name = "stereoscopic_ver_interlacing";
				break;

			case STM_Horizontal:
				pp_name = "stereoscopic_horizontal";
				break;

			case STM_Vertical:
				pp_name = "stereoscopic_vertical";
				break;

			case STM_LCDShutter:
				pp_name = "stereoscopic_lcd_shutter";
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}

			stereoscopic_pp_ = SyncLoadPostProcess("Stereoscopic.ppml", pp_name);
		}

		pp_chain_dirty_ = true;
	}

	void RenderEngine::AssemblePostProcessChain()
	{
		if (ldr_pp_)
		{
			for (size_t i = 0; i < 12; ++ i)
			{
				ldr_pps_[i]->OutputPin(0, TexturePtr());
			}
		}				
		if (hdr_pp_)
		{
			hdr_pp_->OutputPin(0, TexturePtr());
			skip_hdr_pp_->OutputPin(0, TexturePtr());
		}

		if (stereo_method_ != STM_None)
		{
			if (stereoscopic_pp_)
			{
				if (ldr_pp_)
				{
					for (size_t i = 0; i < 12; ++ i)
					{
						ldr_pps_[i]->OutputPin(0, mono_tex_);
					}
				}
				if (hdr_pp_)
				{
					hdr_pp_->OutputPin(0, mono_tex_);
					skip_hdr_pp_->OutputPin(0, mono_tex_);
				}

				stereoscopic_pp_->InputPin(0, mono_tex_);
				stereoscopic_pp_->InputPin(1, ds_tex_);
				stereoscopic_pp_->InputPin(2, overlay_tex_);
			}
		}
		else
		{
			uint32_t const screen_width = default_frame_buffers_[3]->Width();
			uint32_t const screen_height = default_frame_buffers_[3]->Height();
			uint32_t const render_width = default_frame_buffers_[0]->Width();
			uint32_t const render_height = default_frame_buffers_[0]->Height();
			bool need_resize = ((render_width != screen_width) || (render_height != screen_height));
			if (need_resize)
			{
				if (ldr_pp_)
				{
					for (size_t i = 0; i < 12; ++ i)
					{
						ldr_pps_[i]->OutputPin(0, resize_tex_);
					}
				}
				if (hdr_pp_)
				{
					hdr_pp_->OutputPin(0, resize_tex_);
					skip_hdr_pp_->OutputPin(0, resize_tex_);
				}

				for (size_t i = 0; i < 2; ++ i)
				{
					resize_pps_[i]->InputPin(0, resize_tex_);
				}
			}
		}

		if (ldr_pp_)
		{
			if (hdr_pp_)
			{
				hdr_pp_->OutputPin(0, ldr_tex_);
				skip_hdr_pp_->OutputPin(0, ldr_tex_);
			}

			for (size_t i = 0; i < 12; ++ i)
			{
				ldr_pps_[i]->InputPin(0, ldr_tex_);
			}
		}

		if (hdr_pp_)
		{
			hdr_pp_->InputPin(0, hdr_tex_);
			skip_hdr_pp_->InputPin(0, hdr_tex_);
		}
	}
}
