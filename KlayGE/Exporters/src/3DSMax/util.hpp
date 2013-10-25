// util.hpp
// KlayGE ʵ�ú��� ͷ�ļ�
// Ver 2.5.0
// ��Ȩ����(C) ������, 2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// ���ν��� (2005.5.1)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _UTIL_HPP
#define _UTIL_HPP

#include <string>

namespace KlayGE
{
	std::string tstr_to_str(std::basic_string<TCHAR> const & tstr);

	bool is_mesh(INode* node);
	bool is_bone(INode* node);
}

#endif		// _UTIL_HPP
