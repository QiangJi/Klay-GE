// DSAudio.hpp
// KlayGE DirectSound8�������� ͷ�ļ�
// Ver 2.0.0
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// ���ν��� (2003.10.4)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DSAUDIO_HPP
#define _DSAUDIO_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Thread.hpp>

#include <vector>
#include <windows.h>
#ifdef KLAYGE_COMPILER_GCC
	#define __bcount(size)
	#define __in
	#define __in_ecount(size)
	#define __out
	#define __out_ecount(size)
	#define __out_bcount(size)
	#define __inout
	#define __in_bcount(size)
	#define __in_opt
	#define __in_ecount_opt(size)
	#define __in_bcount_opt(size)
	#define __out_opt
	#define __out_ecount_opt(size)
	#define __out_bcount_opt(size)
	#define __inout_opt
	#define __out_ecount_part_opt(size,length)
	#define __in_xcount_opt(size) 
	#define __deref_out
	#define __deref_out_bcount(size)
	#define __deref_opt_out_bcount(size)
	#define __null
#endif
#include <dsound.h>

#include <boost/noncopyable.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	typedef shared_ptr<IDirectSoundBuffer> IDSBufferPtr;

	WAVEFORMATEX WaveFormatEx(AudioDataSourcePtr const & dataSource);
	long LinearGainToDB(float vol);

	// ����������
	/////////////////////////////////////////////////////////////////////////////////
	class DSSoundBuffer : boost::noncopyable, public SoundBuffer
	{
	public:
		DSSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource, float volume);
		~DSSoundBuffer();

		void Play(bool loop = false);
		void Stop();

		void Volume(float vol);

		bool IsPlaying() const;

		float3 Position() const;
		void Position(float3 const & v);
		float3 Velocity() const;
		void Velocity(float3 const & v);
		float3 Direction() const;
		void Direction(float3 const & v);

	private:
		shared_ptr<IDirectSound3DBuffer> Get3DBufferInterface(std::vector<IDSBufferPtr>::iterator iter);

		void DoReset();
		std::vector<IDSBufferPtr>::iterator FreeSource();

	private:
		std::vector<IDSBufferPtr>		sources_;

		float3		pos_;
		float3		vel_;
		float3		dir_;
	};

	// ���ֻ�����
	/////////////////////////////////////////////////////////////////////////////////
	class DSMusicBuffer : boost::noncopyable, public MusicBuffer
	{
	public:
		DSMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds, float volume);
		~DSMusicBuffer();

		void Volume(float vol);

		bool IsPlaying() const;

		float3 Position() const;
		void Position(float3 const & v);
		float3 Velocity() const;
		void Velocity(float3 const & v);
		float3 Direction() const;
		void Direction(float3 const & v);

	private:
		void LoopUpdateBuffer();

		void DoReset();
		void DoPlay(bool loop);
		void DoStop();

	private:
		IDSBufferPtr	buffer_;
		uint32_t		fillSize_;
		uint32_t		fillCount_;
		uint32_t		writePos_;

		shared_ptr<IDirectSound3DBuffer> ds3DBuffer_;

		bool		loop_;

		bool played_;
		bool stopped_;
		condition_variable play_cond_;
		mutex play_mutex_;
		joiner<void> play_thread_;
	};

	// ������Ƶ����
	/////////////////////////////////////////////////////////////////////////////////
	class DSAudioEngine : boost::noncopyable, public AudioEngine
	{
	public:
		DSAudioEngine();
		~DSAudioEngine();

		shared_ptr<IDirectSound> const & DSound() const
			{ return dsound_; }

		std::wstring const & Name() const;

		float3 GetListenerPos() const;
		void SetListenerPos(float3 const & v);
		float3 GetListenerVel() const;
		void SetListenerVel(float3 const & v);
		void GetListenerOri(float3& face, float3& up) const;
		void SetListenerOri(float3 const & face, float3 const & up);

	private:
		shared_ptr<IDirectSound>				dsound_;
		shared_ptr<IDirectSound3DListener>	ds3dListener_;

		HMODULE mod_dsound_;
		typedef HRESULT (WINAPI *DirectSoundCreateFunc)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
		DirectSoundCreateFunc DynamicDirectSoundCreate_;
	};
}

#endif		// _DS8AUDIO_HPP
