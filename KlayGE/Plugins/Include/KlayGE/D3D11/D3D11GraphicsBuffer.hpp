// D3D11GraphicsBuffer.hpp
// KlayGE D3D11ͼ�λ������� ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11GRAPHICSBUFFER_HPP
#define _D3D11GRAPHICSBUFFER_HPP

#pragma once

#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11GraphicsBuffer : public GraphicsBuffer
	{
	public:
		D3D11GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags, ElementInitData const * init_data, ElementFormat fmt);

		ID3D11BufferPtr const & D3DBuffer() const
		{
			return buffer_;
		}

		ID3D11ShaderResourceViewPtr const & D3DShaderResourceView() const
		{
			return d3d_sr_view_;
		}

		ID3D11UnorderedAccessViewPtr const & D3DUnorderedAccessView() const
		{
			return d3d_ua_view_;
		}

		void CopyToBuffer(GraphicsBuffer& rhs);

	protected:
		void GetD3DFlags(D3D11_USAGE& usage, UINT& cpu_access_flags, UINT& bind_flags, UINT& misc_flags);

	private:
		void DoResize();
		void CreateBuffer(D3D11_SUBRESOURCE_DATA const * subres_init);

		void* Map(BufferAccess ba);
		void Unmap();

	private:
		ID3D11DevicePtr d3d_device_;
		ID3D11DeviceContextPtr d3d_imm_ctx_;
		ID3D11BufferPtr buffer_;
		ID3D11ShaderResourceViewPtr d3d_sr_view_;
		ID3D11UnorderedAccessViewPtr d3d_ua_view_;

		uint32_t bind_flags_;
		uint32_t hw_buf_size_;
		ElementFormat fmt_as_shader_res_;
	};
	typedef shared_ptr<D3D11GraphicsBuffer> D3D11GraphicsBufferPtr;
}

#endif			// _D3D11GRAPHICSBUFFER_HPP
