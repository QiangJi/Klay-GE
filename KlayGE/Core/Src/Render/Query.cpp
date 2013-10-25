// Query.hpp
// KlayGE ��ѯ������ ͷ�ļ�
// Ver 3.1.0
// ��Ȩ����(C) ������, 2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// ���ν��� (2005.10.29)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class NullQuery : public Query
	{
	public:
		void Begin()
		{
		}
		void End()
		{
		}
	};

	QueryPtr Query::NullObject()
	{
		static QueryPtr obj = MakeSharedPtr<NullQuery>();
		return obj;
	}
}
