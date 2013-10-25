// export_main.cpp
// KlayGE 3DSMax�����ӿ��� ʵ���ļ�
// Ver 2.5.0
// ��Ȩ����(C) ������, 2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// ���ν��� (2005.5.1)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable: 4100 4238 4239 4244 4245 4512)
#include <max.h>
#pragma warning(pop)

#include "meshml.hpp"
#include "export_main.hpp"

KlayGE::meshml_class_desc mcd;

HINSTANCE dll_instance;

BOOL APIENTRY DllMain(HINSTANCE module, DWORD /*ul_reason_for_call*/, LPVOID /*reserved*/)
{
	dll_instance = module;
	return TRUE;
}

extern "C" int LibNumberClasses()
{
	return 1;
}

extern "C" ClassDesc* LibClassDesc(int i)
{
	switch (i)
	{
	case 0:
		return &mcd;

	default:
		return 0;
	}
}

extern "C" const TCHAR* LibDescription()
{
	return TEXT("MeshML Max File Export Plugin");
}

extern "C" ULONG LibVersion()
{
	return VERSION_3DSMAX;
}
