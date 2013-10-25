// OGLTexture1D.cpp
// KlayGE OpenGL 1D������ ʵ���ļ�
// Ver 3.9.0
// ��Ȩ����(C) ������, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// ֧��GL_NV_copy_image (2009.8.5)
//
// 3.8.0
// ͨ��GL_EXT_framebuffer_blit����CopyTexture (2008.10.12)
//
// 3.6.0
// ��pbo���� (2007.3.13)
//
// 3.2.0
// ���ν��� (2006.4.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "OpenGL32.lib")
#endif

namespace KlayGE
{
	OGLTexture1D::OGLTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
					: OGLTexture(TT_1D, array_size, sample_count, sample_quality, access_hint)
	{
		format_ = format;

		if (0 == numMipMaps)
		{
			num_mip_maps_ = 1;
			uint32_t w = width;
			while (w != 1)
			{
				++ num_mip_maps_;

				w = std::max<uint32_t>(1U, w / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		widths_.resize(num_mip_maps_);
		{
			uint32_t w = width;
			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				widths_[level] = w;

				w = std::max<uint32_t>(1U, w / 2);
			}
		}

		uint32_t texel_size = NumFormatBytes(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		pbos_.resize(array_size * num_mip_maps_);
		glGenBuffers(static_cast<GLsizei>(pbos_.size()), &pbos_[0]);

		if (sample_count <= 1)
		{
			glGenTextures(1, &texture_);
			glBindTexture(target_type_, texture_);
			glTexParameteri(target_type_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(target_type_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
			{
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					uint32_t const w = widths_[level];

					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
					if (IsCompressedFormat(format_))
					{
						int block_size;
						if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
							|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
						{
							block_size = 8;
						}
						else
						{
							block_size = 16;
						}

						GLsizei const image_size = ((w + 3) / 4) * block_size;

						glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, nullptr, GL_STREAM_DRAW);
						re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

						if (array_size > 1)
						{
							if (0 == array_index)
							{
								glCompressedTexImage2D(target_type_, level, glinternalFormat,
									w, array_size, 0, image_size, nullptr);
							}

							glCompressedTexSubImage2D(target_type_, level, 0, array_index, w, 1,
								glformat, image_size, (nullptr == init_data) ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
						}
						else
						{
							glCompressedTexImage1D(target_type_, level, glinternalFormat,
								w, 0, image_size, (nullptr == init_data) ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
						}
					}
					else
					{
						GLsizei const image_size = w * texel_size;

						glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, nullptr, GL_STREAM_DRAW);
						re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

						if (array_size > 1)
						{
							if (0 == array_index)
							{
								glTexImage2D(target_type_, level, glinternalFormat, w, array_size, 0, glformat, gltype, nullptr);
							}

							glTexSubImage2D(target_type_, level, 0, array_index, w, 1,
								glformat, gltype, (nullptr == init_data) ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
						}
						else
						{
							glTexImage1D(target_type_, level, glinternalFormat, w, 0, glformat, gltype,
								(nullptr == init_data) ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
						}
					}
				}
			}
		}
		else
		{
			glGenRenderbuffersEXT(1, &texture_);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, texture_);
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, sample_count, glinternalFormat, width, 1);
		}
	}

	uint32_t OGLTexture1D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return widths_[level];
	}

	void OGLTexture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
		{
			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				this->CopyToSubTexture1D(target,
					array_index, level, 0, target.Width(level),
					array_index, level, 0, this->Width(level));
			}
		}
	}

	void OGLTexture1D::CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width)
	{
		BOOST_ASSERT(type_ == target.Type());
		
		if ((format_ == target.Format()) && !IsCompressedFormat(format_) && glloader_GL_NV_copy_image()
			&& (src_width == dst_width) && (1 == sample_count_))
		{
			OGLTexture& ogl_target = *checked_cast<OGLTexture*>(&target);
			glCopyImageSubDataNV(
				texture_, target_type_, src_level,
				src_x_offset, 0, src_array_index,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, 0, dst_array_index, src_width, 1, 1);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (((sample_count_ > 1) || !re.HackForATI()) && !IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))) && glloader_GL_EXT_framebuffer_blit())
			{
				GLuint fbo_src, fbo_dst;
				re.GetFBOForBlit(fbo_src, fbo_dst);

				GLuint old_fbo = re.BindFramebuffer();

				glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo_src);
				if (array_size_ > 1)
				{
					glFramebufferTextureLayerEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, texture_, src_level, src_array_index);
				}
				else
				{
					if (sample_count_ <= 1)
					{
						glFramebufferTexture1DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target_type_, texture_, src_level);
					}
					else
					{
						glFramebufferRenderbufferEXT(GL_READ_FRAMEBUFFER_EXT,
											GL_COLOR_ATTACHMENT0_EXT,
											GL_RENDERBUFFER_EXT, texture_);
					}
				}

				OGLTexture& ogl_target = *checked_cast<OGLTexture*>(&target);
				glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo_dst);
				if (array_size_ > 1)
				{
					glFramebufferTextureLayerEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, ogl_target.GLTexture(), dst_level, dst_array_index);
				}
				else
				{
					glFramebufferTexture1DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target_type_, ogl_target.GLTexture(), dst_level);
				}

				glBlitFramebufferEXT(src_x_offset, 0, src_x_offset + src_width, 1,
								dst_x_offset, 0, dst_x_offset + dst_width, 1,
								GL_COLOR_BUFFER_BIT, (src_width == dst_width) ? GL_NEAREST : GL_LINEAR);

				re.BindFramebuffer(old_fbo, true);
			}
			else
			{
				if ((src_width == dst_width) && (format_ == target.Format()))
				{
					if (IsCompressedFormat(format_))
					{
						BOOST_ASSERT(0 == (src_x_offset & 0x3));
						BOOST_ASSERT(0 == (dst_x_offset & 0x3));
						BOOST_ASSERT(0 == (src_width & 0x3));
						BOOST_ASSERT(0 == (dst_width & 0x3));

						Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, 0, this->Width(src_level));
						Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, 0, target.Width(dst_level));

						int block_size;
						if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
							|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
						{
							block_size = 8;
						}
						else
						{
							block_size = 16;
						}

						uint8_t const * s = mapper_src.Pointer<uint8_t>() + (src_x_offset / 4 * block_size);
						uint8_t* d = mapper_dst.Pointer<uint8_t>() + (dst_x_offset / 4 * block_size);
						std::memcpy(d, s, src_width / 4 * block_size);
					}
					else
					{
						size_t const format_size = NumFormatBytes(format_);

						Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_width);
						Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_width);
						uint8_t const * s = mapper_src.Pointer<uint8_t>();
						uint8_t* d = mapper_dst.Pointer<uint8_t>();

						std::memcpy(d, s, src_width * format_size);
					}
				}
				else
				{
					this->ResizeTexture1D(target, dst_array_index, dst_level, dst_x_offset, dst_width,
						src_array_index, src_level, src_x_offset, src_width, true);
				}
			}
		}
	}

	void OGLTexture1D::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t /*width*/, void*& data)
	{
		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);
		int block_size;
		if (IsCompressedFormat(format_))
		{
			if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
				|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}
		}
		else
		{
			block_size = 0;
		}

		uint8_t* p;
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (tma)
		{
		case TMA_Read_Only:
		case TMA_Read_Write:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);

				glBindTexture(target_type_, texture_);
				if (IsCompressedFormat(format_))
				{
					glGetCompressedTexImage(target_type_, level, nullptr);
					p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
				}
				else
				{
					glGetTexImage(target_type_, level, gl_format, gl_type, nullptr);
					p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
				}
			}
			break;

		case TMA_Write_Only:
			re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
			re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
			p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
			break;

		default:
			BOOST_ASSERT(false);
			p = nullptr;
			break;
		}

		if (IsCompressedFormat(format_))
		{
			data = p + (x_offset / 4 * block_size);
		}
		else
		{
			data = p + x_offset * texel_size;
		}
	}

	void OGLTexture1D::Unmap1D(uint32_t array_index, uint32_t level)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (last_tma_)
		{
		case TMA_Read_Only:
			re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			break;

		case TMA_Write_Only:
		case TMA_Read_Write:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				GLsizei image_size = 0;
				if (IsCompressedFormat(format_))
				{
					int block_size;
					if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
						|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
					{
						block_size = 8;
					}
					else
					{
						block_size = 16;
					}

					image_size = ((widths_[level] + 3) / 4) * block_size;
				}

				glBindTexture(target_type_, texture_);

				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				if (IsCompressedFormat(format_))
				{
					if (array_size_ > 1)
					{
						glCompressedTexSubImage2D(target_type_, level, 0, array_index,
							widths_[level], 1, gl_format, image_size, nullptr);
					}
					else
					{
						glCompressedTexSubImage1D(target_type_, level, 0,
							widths_[level], gl_format, image_size, nullptr);
					}
				}
				else
				{
					if (array_size_ > 1)
					{
						glTexSubImage2D(target_type_, level, 0, array_index, widths_[level], 1,
							gl_format, gl_type, nullptr);
					}
					else
					{
						glTexSubImage1D(target_type_, level, 0, widths_[level],
							gl_format, gl_type, nullptr);
					}
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}
}
