// D3D11VideoMode.hpp
// KlayGE D3D11��ʾģʽ ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2008.9.21)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11VIDEOMODE_HPP
#define _D3D11VIDEOMODE_HPP

#pragma once

#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	// ������ʾģʽ��Ϣ
	/////////////////////////////////////////////////////////////////////////////////
	class D3D11VideoMode
	{
	public:
		D3D11VideoMode();
		D3D11VideoMode(uint32_t width, uint32_t height, DXGI_FORMAT format);

		uint32_t Width() const;
		uint32_t Height() const;
		DXGI_FORMAT Format() const;
		uint32_t ColorDepth() const;

	private:
		uint32_t		width_;
		uint32_t		height_;
		DXGI_FORMAT		format_;
	};

	bool operator<(D3D11VideoMode const & lhs, D3D11VideoMode const & rhs);
	bool operator==(D3D11VideoMode const & lhs, D3D11VideoMode const & rhs);
}

#endif			// _D3D11VIDEOMODE_HPP
