// DSMusicBuffer.cpp
// KlayGE DirectSound���ֻ������� ʵ���ļ�
// Ver 2.0.4
// ��Ȩ����(C) ������, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.0.4
// ����timeSetEventʵ�� (2004.3.28)
//
// 2.0.0
// ���ν��� (2003.10.4)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/DSound/DSAudio.hpp>

namespace KlayGE
{
	// ���캯��������һ������������ʽ���ŵĻ�����
	/////////////////////////////////////////////////////////////////////////////////
	DSMusicBuffer::DSMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds, float volume)
					: MusicBuffer(dataSource),
						writePos_(0),
						played_(false), stopped_(true)
	{
		WAVEFORMATEX wfx(WaveFormatEx(dataSource));
		fillSize_	= wfx.nAvgBytesPerSec / PreSecond;
		fillCount_	= bufferSeconds * PreSecond;

		bool const mono(1 == wfx.nChannels);

		shared_ptr<IDirectSound> const & dsound = checked_cast<DSAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance())->DSound();

		// ���� DirectSound ��������Ҫ��������ʹ�ý�����־��
		// ��Ϊʹ��̫�಻��Ҫ�ı�־��Ӱ��Ӳ����������
		DSBUFFERDESC dsbd;
		std::memset(&dsbd, 0, sizeof(dsbd));
		dsbd.dwSize = sizeof(dsbd);
		dsbd.dwFlags = DSBCAPS_CTRLVOLUME;
		if (mono)
		{
			dsbd.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
			dsbd.guid3DAlgorithm	= GUID_NULL;
		}
		dsbd.dwBufferBytes		= fillSize_ * fillCount_;
		dsbd.lpwfxFormat		= &wfx;

		// DirectSoundֻ�ܲ���PCM���ݡ�������ʽ���ܲ��ܹ�����
		IDirectSoundBuffer* buffer;
		TIF(dsound->CreateSoundBuffer(&dsbd, &buffer, nullptr));
		buffer_ = MakeCOMPtr(buffer);

		if (mono)
		{
			IDirectSound3DBuffer* ds3DBuffer;
			buffer_->QueryInterface(IID_IDirectSound3DBuffer,
				reinterpret_cast<void**>(&ds3DBuffer));
			ds3DBuffer_ = MakeCOMPtr(ds3DBuffer);
		}

		this->Position(float3::Zero());
		this->Velocity(float3::Zero());
		this->Direction(float3::Zero());

		this->Volume(volume);

		this->Reset();
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	DSMusicBuffer::~DSMusicBuffer()
	{
		this->Stop();
	}

	void DSMusicBuffer::LoopUpdateBuffer()
	{
		unique_lock<mutex> lock(play_mutex_);
		while (!played_)
		{
			play_cond_.wait(lock);
		}
		played_ = false;

		while (!stopped_)
		{
			// ����������
			uint8_t* lockedBuffer;			// ָ�򻺳����������ڴ��ָ��
			DWORD lockedBufferSize;		// �������ڴ��С
			TIF(buffer_->Lock(fillSize_ * writePos_, fillSize_,
				reinterpret_cast<void**>(&lockedBuffer), &lockedBufferSize,
				nullptr, nullptr, 0));

			std::vector<uint8_t> data(fillSize_);
			data.resize(dataSource_->Read(&data[0], fillSize_));

			if (data.empty())
			{
				if (loop_)
				{
					stopped_ = false;
					this->Reset();
				}
				else
				{
					stopped_ = true;
				}
			}
			else
			{
				std::copy(data.begin(), data.end(), lockedBuffer);

				std::fill_n(lockedBuffer + data.size(), lockedBufferSize - data.size(), 0);
			}

			// ����������
			buffer_->Unlock(lockedBuffer, lockedBufferSize, nullptr, 0);

			// �γɻ��λ�����
			++ writePos_;
			writePos_ %= fillCount_;

			Sleep(1000 / PreSecond);
		}
	}

	// ��������λ�Ա��ڴ�ͷ����
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoReset()
	{
		this->writePos_	= 0;

		dataSource_->Reset();

		// ����������
		uint8_t* lockedBuffer;			// ָ�򻺳����������ڴ��ָ��
		DWORD lockedBufferSize;		// �������ڴ��С
		TIF(buffer_->Lock(0, fillSize_ * fillCount_,
			reinterpret_cast<void**>(&lockedBuffer), &lockedBufferSize, nullptr, nullptr, 0));

		std::vector<uint8_t> data(fillSize_ * fillCount_);
		data.resize(dataSource_->Read(&data[0], fillSize_ * fillCount_));

		if (data.empty())
		{
			// �����Ƶ���ݿհף��þ������
			std::fill_n(lockedBuffer, lockedBufferSize, 0);
		}
		else
		{
			// �������Դ�Ȼ�����С��������Ƶ������仺����
			std::copy(data.begin(), data.end(), lockedBuffer);

			// ʣ�µ������ÿհ����
			std::fill_n(lockedBuffer + data.size(), lockedBufferSize - data.size(), 0);
		}

		// ����������
		buffer_->Unlock(lockedBuffer, lockedBufferSize, nullptr, 0);

		buffer_->SetCurrentPosition(0);
	}

	// ������Ƶ��
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoPlay(bool loop)
	{
		play_thread_ = Context::Instance().ThreadPool()(bind(&DSMusicBuffer::LoopUpdateBuffer, this));

		loop_ = loop;

		stopped_ = false;
		{
			unique_lock<mutex> lock(play_mutex_);
			played_ = true;
		}
		play_cond_.notify_one();

		buffer_->Play(0, 0, DSBPLAY_LOOPING);
	}

	// ֹͣ������Ƶ��
	////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoStop()
	{
		if (!stopped_)
		{
			stopped_ = true;
			play_thread_();
		}

		buffer_->Stop();
	}

	// ��黺�����Ƿ��ڲ���
	/////////////////////////////////////////////////////////////////////////////////
	bool DSMusicBuffer::IsPlaying() const
	{
		if (buffer_)
		{
			DWORD status;
			buffer_->GetStatus(&status);
			return ((status & DSBSTATUS_PLAYING) != 0);
		}

		return false;
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Volume(float vol)
	{
		buffer_->SetVolume(LinearGainToDB(vol));
	}


	// ��ȡ��Դλ��
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSMusicBuffer::Position() const
	{
		float3 ret(float3::Zero());

		if (ds3DBuffer_)
		{
			D3DVECTOR v;
			ds3DBuffer_->GetPosition(&v);
			ret = float3(v.x, v.y, v.z);
		}

		return ret;
	}

	// ������Դλ��
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Position(float3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetPosition(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	// ��ȡ��Դ�ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSMusicBuffer::Velocity() const
	{
		float3 ret(float3::Zero());

		if (ds3DBuffer_)
		{
			D3DVECTOR v;
			ds3DBuffer_->GetVelocity(&v);
			ret = float3(v.x, v.y, v.z);
		}

		return ret;
	}

	// ������Դ�ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Velocity(float3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetVelocity(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	// ��ȡ��Դ����
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSMusicBuffer::Direction() const
	{
		float3 ret(float3::Zero());

		if (ds3DBuffer_)
		{
			D3DVECTOR v;
			ds3DBuffer_->GetConeOrientation(&v);
			ret = float3(v.x, v.y, v.z);
		}

		return ret;
	}

	// ������Դ����
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Direction(float3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetConeOrientation(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}
}
