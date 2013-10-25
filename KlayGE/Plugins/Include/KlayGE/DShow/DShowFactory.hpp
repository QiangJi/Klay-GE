// DShowFactory.hpp
// KlayGE DirectShow����������󹤳� ͷ�ļ�
// Ver 3.4.0
// ��Ȩ����(C) ������, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// ���ν��� (2006.7.15)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DSHOWFACTORY_HPP
#define _DSHOWFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_DSHOW_SE_SOURCE				// Build dll
		#define KLAYGE_DSHOW_SE_API __declspec(dllexport)
	#else										// Use dll
		#define KLAYGE_DSHOW_SE_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_DSHOW_SE_API
#endif // KLAYGE_HAS_DECLSPEC

extern "C"
{
	KLAYGE_DSHOW_SE_API void MakeShowFactory(KlayGE::ShowFactoryPtr& ptr);
}

#endif			// _DSHOWFACTORY_HPP
