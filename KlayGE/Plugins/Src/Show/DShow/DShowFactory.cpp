// DShowFactory.cpp
// KlayGE DirectShow����������󹤳� ʵ���ļ�
// Ver 3.4.0
// ��Ȩ����(C) ������, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// ���ν��� (2006.7.15)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/ShowFactory.hpp>

#include <KlayGE/DShow/DShow.hpp>
#include <KlayGE/DShow/DShowFactory.hpp>

void MakeShowFactory(KlayGE::ShowFactoryPtr& ptr)
{
	ptr = KlayGE::MakeSharedPtr<KlayGE::ConcreteShowFactory<KlayGE::DShowEngine> >(L"DirectShow Show Factory");
}
