// OGLESQuery.hpp
// KlayGE OpenGL ES�ڵ������ ʵ���ļ�
// Ver 3.0.0
// ��Ȩ����(C) ������, 2005
// Homepage: http://www.klayge.org
//
// 3.0.0
// ���ν��� (2005.9.27)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESQUERY_HPP
#define _OGLESQUERY_HPP

#pragma once

#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class OGLESConditionalRender : public ConditionalRender
	{
	public:
		OGLESConditionalRender();
		~OGLESConditionalRender();

		void Begin();
		void End();

		void BeginConditionalRender();
		void EndConditionalRender();

		bool AnySamplesPassed();

	private:
		GLuint query_;
	};
}

#endif		// _OGLQUERY_HPP
