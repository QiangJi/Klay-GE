// OCTreeFactory.hpp
// KlayGE OCTree���������� ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2008.10.17)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OCTREEFACTORY_HPP
#define _OCTREEFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_OCTREE_SM_SOURCE				// Build dll
		#define KLAYGE_OCTREE_SM_API __declspec(dllexport)
	#else										// Use dll
		#define KLAYGE_OCTREE_SM_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_OCTREE_SM_API
#endif // KLAYGE_HAS_DECLSPEC

extern "C"
{
	KLAYGE_OCTREE_SM_API void MakeSceneManager(KlayGE::SceneManagerPtr& ptr);
}

#endif			// _OCTREEFACTORY_HPP
