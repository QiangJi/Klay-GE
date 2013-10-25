// OGLESRenderView.cpp
// KlayGE OpenGL ES 2��Ⱦ��ͼ�� ʵ���ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <boost/assert.hpp>

#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESFrameBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESRenderView.hpp>

namespace KlayGE
{
	OGLESRenderView::OGLESRenderView()
		: tex_(0), fbo_(0)
	{
	}

	OGLESRenderView::~OGLESRenderView()
	{
	}

	void OGLESRenderView::ClearDepth(float depth)
	{
		this->DoClear(GL_DEPTH_BUFFER_BIT, Color(), depth, 0);
	}

	void OGLESRenderView::ClearStencil(int32_t stencil)
	{
		this->DoClear(GL_STENCIL_BUFFER_BIT, Color(), 0, stencil);
	}

	void OGLESRenderView::ClearDepthStencil(float depth, int32_t stencil)
	{
		uint32_t flags = 0;
		if (IsDepthFormat(pf_))
		{
			flags |= GL_DEPTH_BUFFER_BIT;
		}
		if (IsStencilFormat(pf_))
		{
			flags |= GL_STENCIL_BUFFER_BIT;
		}

		this->DoClear(flags, Color(), depth, stencil);
	}

	void OGLESRenderView::DoClear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(fbo_);

		DepthStencilStateDesc const & ds_desc = re.CurDSSObj()->GetDesc();
		BlendStateDesc const & blend_desc = re.CurBSObj()->GetDesc();

		if (flags & GL_COLOR_BUFFER_BIT)
		{
			if (blend_desc.color_write_mask[0] != CMASK_All)
			{
				glColorMask(true, true, true, true);
			}
		}
		if (flags & GL_DEPTH_BUFFER_BIT)
		{
			if (!ds_desc.depth_write_mask)
			{
				glDepthMask(GL_TRUE);
			}
		}
		if (flags & GL_STENCIL_BUFFER_BIT)
		{
			if (!ds_desc.front_stencil_write_mask)
			{
				glStencilMaskSeparate(GL_FRONT, GL_TRUE);
			}
			if (!ds_desc.back_stencil_write_mask)
			{
				glStencilMaskSeparate(GL_BACK, GL_TRUE);
			}
		}

		if (glloader_GLES_VERSION_3_0())
		{
			if (flags & GL_COLOR_BUFFER_BIT)
			{
				glClearBufferfv(GL_COLOR, index_, &clr[0]);
			}

			if ((flags & GL_DEPTH_BUFFER_BIT) && (flags & GL_STENCIL_BUFFER_BIT))
			{
				glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
			}
			else
			{
				if (flags & GL_DEPTH_BUFFER_BIT)
				{
					glClearBufferfv(GL_DEPTH, 0, &depth);
				}
				else
				{
					if (flags & GL_STENCIL_BUFFER_BIT)
					{
						GLint s = stencil;
						glClearBufferiv(GL_STENCIL, 0, &s);
					}
				}
			}
		}
		else
		{
			if (flags & GL_COLOR_BUFFER_BIT)
			{
				re.ClearColor(clr.r(), clr.g(), clr.b(), clr.a());
			}
			if (flags & GL_DEPTH_BUFFER_BIT)
			{
				re.ClearDepth(depth);
			}
			if (flags & GL_STENCIL_BUFFER_BIT)
			{
				re.ClearStencil(stencil);
			}
			if (flags != 0)
			{
				glClear(flags);
			}
		}

		if (flags & GL_COLOR_BUFFER_BIT)
		{
			if (blend_desc.color_write_mask[0] != CMASK_All)
			{
				glColorMask((blend_desc.color_write_mask[0] & CMASK_Red) != 0,
						(blend_desc.color_write_mask[0] & CMASK_Green) != 0,
						(blend_desc.color_write_mask[0] & CMASK_Blue) != 0,
						(blend_desc.color_write_mask[0] & CMASK_Alpha) != 0);
			}
		}
		if (flags & GL_DEPTH_BUFFER_BIT)
		{
			if (!ds_desc.depth_write_mask)
			{
				glDepthMask(GL_FALSE);
			}
		}
		if (flags & GL_STENCIL_BUFFER_BIT)
		{
			if (!ds_desc.front_stencil_write_mask)
			{
				glStencilMaskSeparate(GL_FRONT, GL_FALSE);
			}
			if (!ds_desc.back_stencil_write_mask)
			{
				glStencilMaskSeparate(GL_BACK, GL_FALSE);
			}
		}

		re.BindFramebuffer(old_fbo);
	}
	
	void OGLESRenderView::DoDiscardColor()
	{
		if (glloader_GLES_EXT_discard_framebuffer())
		{
			GLenum attachment;
			if (fbo_ != 0)
			{
				attachment = GL_COLOR_ATTACHMENT0 + index_;
			}
			else
			{
				attachment = GL_COLOR_EXT;
			}

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			GLuint old_fbo = re.BindFramebuffer();
			re.BindFramebuffer(fbo_);
			glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, &attachment);

			re.BindFramebuffer(old_fbo);
		}
		else
		{
			this->ClearColor(Color(0, 0, 0, 0));
		}
	}

	void OGLESRenderView::DoDiscardDepthStencil()
	{
		if (glloader_GLES_EXT_discard_framebuffer())
		{
			GLenum attachments[2];
			if (fbo_ != 0)
			{
				attachments[0] = GL_DEPTH_ATTACHMENT;
				attachments[1] = GL_STENCIL_ATTACHMENT;
			}
			else
			{
				attachments[0] = GL_DEPTH_EXT;
				attachments[1] = GL_STENCIL_EXT;
			}

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			GLuint old_fbo = re.BindFramebuffer();
			re.BindFramebuffer(fbo_);

			glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, attachments);

			re.BindFramebuffer(old_fbo);
		}
		else
		{
			this->ClearDepthStencil(1, 0);
		}
	}


	OGLESScreenColorRenderView::OGLESScreenColorRenderView(uint32_t width, uint32_t height, ElementFormat pf)
	{
		width_ = width;
		height_ = height;
		pf_ = pf;
	}

	void OGLESScreenColorRenderView::ClearColor(Color const & clr)
	{
		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLESScreenColorRenderView::ClearDepth(float /*depth*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESScreenColorRenderView::ClearStencil(int32_t /*stencil*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESScreenColorRenderView::ClearDepthStencil(float /*depth*/, int32_t /*stencil*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESScreenColorRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESScreenColorRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		index_ = att - FrameBuffer::ATT_Color0;

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLESScreenColorRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}


	OGLESScreenDepthStencilRenderView::OGLESScreenDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;
	}

	void OGLESScreenDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESScreenDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLESScreenDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		index_ = 0;

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLESScreenDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}


	OGLESTexture1DRenderView::OGLESTexture1DRenderView(Texture& texture_1d, int array_index, int level)
		: texture_1d_(*checked_cast<OGLESTexture1D*>(&texture_1d)),
			array_index_(array_index), level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d.Type());

		if (array_index > 0)
		{
			THR(errc::function_not_supported);
		}

		uint32_t const channels = NumComponents(texture_1d.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_texture_rg())))
		{
			THR(errc::function_not_supported);
		}

		tex_ = texture_1d_.GLTexture();

		width_ = texture_1d_.Width(level);
		height_ = 1;
		pf_ = texture_1d_.Format();
	}

	void OGLESTexture1DRenderView::ClearColor(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			glBindTexture(texture_1d_.GLType(), tex_);

			std::vector<Color> mem_clr(width_, clr);
			glTexSubImage2D(texture_1d_.GLType(), level_, 0, 0, width_, 1, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLESTexture1DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTexture1DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;
		fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_1d_.GLType(), tex_, level_);

		re.BindFramebuffer(0);
	}

	void OGLESTexture1DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_1d_.GLType(), 0, 0);

		re.BindFramebuffer(0);
	}


	OGLESTexture2DRenderView::OGLESTexture2DRenderView(Texture& texture_2d, int array_index, int level)
		: texture_2d_(*checked_cast<OGLESTexture2D*>(&texture_2d)),
			array_index_(array_index), level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d.Type());

		if (array_index > 0)
		{
			THR(errc::function_not_supported);
		}

		uint32_t const channels = NumComponents(texture_2d.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_texture_rg())))
		{
			THR(errc::function_not_supported);
		}

		tex_ = texture_2d_.GLTexture();

		width_ = texture_2d_.Width(level);
		height_ = texture_2d_.Height(level);
		pf_ = texture_2d_.Format();
	}

	void OGLESTexture2DRenderView::ClearColor(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			glBindTexture(texture_2d_.GLType(), tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(texture_2d_.GLType(), level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLESTexture2DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTexture2DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;
		fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_2d_.GLType(), tex_, level_);

		re.BindFramebuffer(0);
	}

	void OGLESTexture2DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, texture_2d_.GLType(), 0, 0);

		re.BindFramebuffer(0);
	}


	OGLESTexture3DRenderView::OGLESTexture3DRenderView(Texture& texture_3d, int array_index, uint32_t slice, int level)
		: texture_3d_(*checked_cast<OGLESTexture3D*>(&texture_3d)),
			slice_(slice), level_(level), copy_to_tex_(0)
	{
		UNREF_PARAM(array_index);

		BOOST_ASSERT(Texture::TT_3D == texture_3d.Type());
		BOOST_ASSERT(texture_3d_.Depth(level) > slice);
		BOOST_ASSERT(0 == array_index);

		uint32_t const channels = NumComponents(texture_3d.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_texture_rg())))
		{
			THR(errc::function_not_supported);
		}

		tex_ = texture_3d_.GLTexture();

		width_ = texture_3d_.Width(level);
		height_ = texture_3d_.Height(level);
		pf_ = texture_3d_.Format();
	}

	OGLESTexture3DRenderView::~OGLESTexture3DRenderView()
	{
		if (2 == copy_to_tex_)
		{
			glDeleteTextures(1, &tex_2d_);
		}
	}

	void OGLESTexture3DRenderView::ClearColor(Color const & clr)
	{
		BOOST_ASSERT(fbo_ != 0);

		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLESTexture3DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTexture3DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		if (0 == copy_to_tex_)
		{
			glFramebufferTexture3DOES(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D_OES, tex_, level_, slice_);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (GL_FRAMEBUFFER_COMPLETE == status)
			{
				glGenTextures(1, &tex_2d_);
				glBindTexture(GL_TEXTURE_2D, tex_2d_);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_,
					0, GL_RGBA, GL_FLOAT, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				copy_to_tex_ = 2;
			}
			else
			{
				copy_to_tex_ = 1;
			}
		}

		if (1 == copy_to_tex_)
		{
			glFramebufferTexture3DOES(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_3D_OES, tex_, level_, slice_);
		}
		else
		{
			BOOST_ASSERT(2 == copy_to_tex_);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_2D, tex_2d_, 0);
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture3DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		BOOST_ASSERT(copy_to_tex_ != 0);
		if (1 == copy_to_tex_)
		{
			glFramebufferTexture3DOES(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_3D_OES, 0, 0, 0);
		}
		else
		{
			BOOST_ASSERT(2 == copy_to_tex_);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_2D, 0, 0);

			this->CopyToSlice(att);
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture3DRenderView::OnUnbind(FrameBuffer& /*fb*/, uint32_t att)
	{
		BOOST_ASSERT(copy_to_tex_ != 0);
		if (2 == copy_to_tex_)
		{
			this->CopyToSlice(att);
		}
	}

	void OGLESTexture3DRenderView::CopyToSlice(uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		UNREF_PARAM(att);

		if (glloader_GLES_VERSION_3_0())
		{
			glBindTexture(GL_TEXTURE_3D, tex_);
			glCopyTexSubImage3D(GL_TEXTURE_3D, level_, 0, 0, slice_, 0, 0, width_, height_);
		}
		else
		{
			glBindTexture(GL_TEXTURE_3D_OES, tex_);
			glCopyTexSubImage3DOES(GL_TEXTURE_3D_OES, level_, 0, 0, slice_, 0, 0, width_, height_);
		}
	}


	OGLESTextureCubeRenderView::OGLESTextureCubeRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: texture_cube_(*checked_cast<OGLESTextureCube*>(&texture_cube)),
			face_(face), level_(level)
	{
		UNREF_PARAM(array_index);

		BOOST_ASSERT(Texture::TT_Cube == texture_cube.Type());
		BOOST_ASSERT(0 == array_index);

		uint32_t const channels = NumComponents(texture_cube.Format());
		if (((1 == channels) || (2 == channels)) && (!(glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_texture_rg())))
		{
			THR(errc::function_not_supported);
		}

		tex_ = texture_cube_.GLTexture();

		width_ = texture_cube_.Width(level);
		height_ = texture_cube_.Height(level);
		pf_ = texture_cube_.Format();
	}

	void OGLESTextureCubeRenderView::ClearColor(Color const & clr)
	{
		if (fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X,
				level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLESTextureCubeRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTextureCubeRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
				face, tex_, level_);

		re.BindFramebuffer(0);
	}

	void OGLESTextureCubeRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
				face, 0, 0);

		re.BindFramebuffer(0);
	}


	OGLESDepthStencilRenderView::OGLESDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: target_type_(0), array_index_(0), level_(-1),
			sample_count_(sample_count), sample_quality_(sample_quality)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;

		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLESMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		switch (internalFormat)
		{
		case GL_DEPTH_COMPONENT:
			internalFormat = GL_DEPTH_COMPONENT16;
			break;

		case GL_DEPTH_STENCIL_OES:
			internalFormat = GL_DEPTH24_STENCIL8_OES;
			break;

		default:
			break;
		}

		glGenRenderbuffers(1, &rbo_);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
		glRenderbufferStorage(GL_RENDERBUFFER,
								internalFormat, width_, height_);
	}

	OGLESDepthStencilRenderView::OGLESDepthStencilRenderView(Texture& texture, int array_index, int level)
		: target_type_(checked_cast<OGLESTexture2D*>(&texture)->GLType()), array_index_(array_index), level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture.Type());
		BOOST_ASSERT(IsDepthFormat(texture.Format()));

		if (array_index > 0)
		{
			THR(errc::function_not_supported);
		}

		width_ = texture.Width(level);
		height_ = texture.Height(level);
		pf_ = texture.Format();

		tex_ = checked_cast<OGLESTexture2D*>(&texture)->GLTexture();
	}

	OGLESDepthStencilRenderView::~OGLESDepthStencilRenderView()
	{
		glDeleteRenderbuffers(1, &rbo_);
	}

	void OGLESDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLESDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		index_ = 0;

		fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		if (level_ < 0)
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_DEPTH_ATTACHMENT,
									GL_RENDERBUFFER, rbo_);
			if (IsStencilFormat(pf_))
			{
				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_STENCIL_ATTACHMENT,
									GL_RENDERBUFFER, rbo_);
			}

			re.BindFramebuffer(0);
		}
		else
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			BOOST_ASSERT(GL_TEXTURE_2D == target_type_);

			re.BindFramebuffer(fbo_);

			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_DEPTH_ATTACHMENT, target_type_, tex_, level_);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_STENCIL_ATTACHMENT, target_type_, tex_, level_);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLESDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);
		UNREF_PARAM(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		BOOST_ASSERT(fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());
		if (level_ < 0)
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(fbo_);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_DEPTH_ATTACHMENT,
									GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_STENCIL_ATTACHMENT,
									GL_RENDERBUFFER, 0);

			re.BindFramebuffer(0);
		}
		else
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			BOOST_ASSERT(GL_TEXTURE_2D == target_type_);

			re.BindFramebuffer(fbo_);

			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_DEPTH_ATTACHMENT, target_type_, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_STENCIL_ATTACHMENT, target_type_, 0, 0);
			}

			re.BindFramebuffer(0);
		}
	}


	OGLESTextureCubeDepthStencilRenderView::OGLESTextureCubeDepthStencilRenderView(Texture& texture_cube, int array_index, Texture::CubeFaces face, int level)
		: texture_cube_(*checked_cast<OGLESTextureCube*>(&texture_cube)),
			face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube.Type());
		BOOST_ASSERT(IsDepthFormat(texture_cube.Format()));

		if (array_index > 0)
		{
			THR(errc::function_not_supported);
		}

		width_ = texture_cube.Width(level);
		height_ = texture_cube.Height(level);
		pf_ = texture_cube.Format();

		tex_ = checked_cast<OGLESTextureCube*>(&texture_cube)->GLTexture();
	}

	void OGLESTextureCubeDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTextureCubeDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLESTextureCubeDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		index_ = 0;

		fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		if (IsDepthFormat(pf_))
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT, face, tex_, level_);
		}
		if (IsStencilFormat(pf_))
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_STENCIL_ATTACHMENT, face, tex_, level_);
		}

		re.BindFramebuffer(0);
	}

	void OGLESTextureCubeDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		UNREF_PARAM(fb);
		UNREF_PARAM(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(fbo_);

		if (IsDepthFormat(pf_))
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT, face, 0, 0);
		}
		if (IsStencilFormat(pf_))
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_STENCIL_ATTACHMENT, face, 0, 0);
		}

		re.BindFramebuffer(0);
	}
}
