// DShow.hpp
// KlayGE DirectShow ���������� ͷ�ļ�
// Ver 2.0.0
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// ���ν��� (2003.9.3)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////

#ifndef _DSHOW_HPP
#define _DSHOW_HPP

#pragma once

#include <windows.h>
#include <control.h>
#include <d3d9.h>
#include <strmif.h>
#include <vmr9.h>

#include <string>

#include <boost/noncopyable.hpp>

#include <KlayGE/Show.hpp>

namespace KlayGE
{
	class DShowEngine : boost::noncopyable, public ShowEngine
	{
	public:
		DShowEngine();
		~DShowEngine();

		bool IsComplete();

		void Load(std::string const & fileName);
		TexturePtr PresentTexture();

		ShowState State(long msTimeout = -1);

	private:
		shared_ptr<IGraphBuilder>	graph_;
		shared_ptr<IBaseFilter>      filter_;
		shared_ptr<IMediaControl>	media_control_;
		shared_ptr<IMediaEvent>		media_event_;
		shared_ptr<IVMRSurfaceAllocator9> vmr_allocator_;

	private:
		void Init();
		void Free();

		void DoPlay();
		void DoStop();
		void DoPause();
	};
}

#endif		// _DSHOW_HPP
