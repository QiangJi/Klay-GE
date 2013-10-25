// OGLESRenderStateObject.hpp
// KlayGE OpenGL ES��Ⱦ״̬������ ͷ�ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERSTATEOBJECT_HPP
#define _OGLESRENDERSTATEOBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

namespace KlayGE
{
	class OGLESRasterizerStateObject : public RasterizerStateObject
	{
	public:
		explicit OGLESRasterizerStateObject(RasterizerStateDesc const & desc);

		void Active();
		void ForceDefaultState();

	private:
		GLenum ogl_front_face_;
	};

	class OGLESDepthStencilStateObject : public DepthStencilStateObject
	{
	public:
		explicit OGLESDepthStencilStateObject(DepthStencilStateDesc const & desc);

		void Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref);
		void ForceDefaultState();

	private:
		GLboolean ogl_depth_write_mask_;
		GLenum ogl_depth_func_;
		GLenum ogl_front_stencil_func_;
		GLenum ogl_front_stencil_fail_;
		GLenum ogl_front_stencil_depth_fail_;
		GLenum ogl_front_stencil_pass_;
		GLenum ogl_back_stencil_func_;
		GLenum ogl_back_stencil_fail_;
		GLenum ogl_back_stencil_depth_fail_;
		GLenum ogl_back_stencil_pass_;
	};

	class OGLESBlendStateObject : public BlendStateObject
	{
	public:
		explicit OGLESBlendStateObject(BlendStateDesc const & desc);

		void Active(Color const & blend_factor, uint32_t sample_mask);
		void ForceDefaultState();

	private:
		GLenum ogl_blend_op_;
		GLenum ogl_blend_op_alpha_;
		GLenum ogl_src_blend_;
		GLenum ogl_dest_blend_;
		GLenum ogl_src_blend_alpha_;
		GLenum ogl_dest_blend_alpha_;
	};

	class OGLESSamplerStateObject : public SamplerStateObject
	{
	public:
		explicit OGLESSamplerStateObject(SamplerStateDesc const & desc);

		void Active(TexturePtr const & texture);

	private:
		GLenum ogl_addr_mode_u_;
		GLenum ogl_addr_mode_v_;
		GLenum ogl_addr_mode_w_;
		GLenum ogl_min_filter_;
		GLenum ogl_mag_filter_;
	};
}

#endif			// _OGLESRENDERSTATEOBJECT_HPP
