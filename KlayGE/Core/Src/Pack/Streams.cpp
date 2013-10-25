/**
 * @file Streams.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ResLoader.hpp>

#include <boost/assert.hpp>

#include <CPP/Common/MyWindows.h>

#include "Streams.hpp"

namespace KlayGE
{
	void CInStream::Attach(ResIdentifierPtr const & is)
	{
		BOOST_ASSERT(*is);

		is_ = is;
		is_->seekg(0, std::ios_base::end);
		stream_size_ = is_->tellg();
		is_->seekg(0, std::ios_base::beg);
	}

	STDMETHODIMP CInStream::Read(void *data, uint32_t size, uint32_t* processedSize)
	{
		is_->read(data, size);
		if (processedSize)
		{
			*processedSize = static_cast<uint32_t>(is_->gcount());
		}

		return *is_ ? S_OK: E_FAIL;
	}

	STDMETHODIMP CInStream::Seek(int64_t offset, uint32_t seekOrigin, uint64_t* newPosition)
	{
		std::ios_base::seekdir way;
		switch (seekOrigin)
		{
		case 0:
			way = std::ios_base::beg;
			break;

		case 1:
			way = std::ios_base::cur;
			break;

		case 2:
			way = std::ios_base::end;
			break;

		default:
			return STG_E_INVALIDFUNCTION;
		}

		is_->seekg(static_cast<std::istream::off_type>(offset), way);
		if (newPosition)
		{
			*newPosition = is_->tellg();
		}

		return *is_ ? S_OK: E_FAIL;
	}

	STDMETHODIMP CInStream::GetSize(uint64_t* size)
	{
		*size = stream_size_;
		return S_OK;
	}


	//////////////////////////
	// COutStream

	void COutStream::Attach(shared_ptr<std::ostream> const & os)
	{
		os_ = os;
	}

	STDMETHODIMP COutStream::Write(const void *data, uint32_t size, uint32_t* processedSize)
	{
		os_->write(static_cast<char const *>(data), size);
		if (processedSize)
		{
			*processedSize = size;
		}

		return *os_ ? S_OK: E_FAIL;
	}

	STDMETHODIMP COutStream::Seek(int64_t offset, uint32_t seekOrigin, uint64_t* newPosition)
	{
		std::ios_base::seekdir way;
		switch (seekOrigin)
		{
		case 0:
			way = std::ios_base::beg;
			break;

		case 1:
			way = std::ios_base::cur;
			break;

		case 2:
			way = std::ios_base::end;
			break;

		default:
			return STG_E_INVALIDFUNCTION;
		}

		os_->seekp(static_cast<std::ofstream::off_type>(offset), way);
		if (newPosition)
		{
			*newPosition = os_->tellp();
		}

		return *os_ ? S_OK: E_FAIL;
	}

	STDMETHODIMP COutStream::SetSize(uint64_t /*newSize*/)
	{
		return E_NOTIMPL;
	}
}
