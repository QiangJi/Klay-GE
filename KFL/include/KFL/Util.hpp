/**
 * @file Util.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
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

#ifndef _KFL_UTIL_HPP
#define _KFL_UTIL_HPP

#pragma once

#include <KFL/PreDeclare.hpp>

#include <string>
#include <functional>

#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>

#define UNREF_PARAM(x) (void)(x)

#include <KFL/Log.hpp>

#define KFL_STRINGIZE(X) KFL_DO_STRINGIZE(X)
#define KFL_DO_STRINGIZE(X) #X

#define KFL_JOIN(X, Y) KFL_DO_JOIN(X, Y)
#define KFL_DO_JOIN(X, Y) KFL_DO_JOIN2(X, Y)
#define KFL_DO_JOIN2(X, Y) X##Y

namespace KlayGE
{
	// ���n bitΪ1
	inline uint32_t
	SetMask(uint32_t n)
		{ return 1UL << n; }
	template <uint32_t n>
	struct Mask
	{
		enum { value = 1UL << n };
	};

	// ȡ���еĵ� n bit
	inline uint32_t
	GetBit(uint32_t x, uint32_t n)
		{ return (x >> n) & 1; }
	// �����еĵ� n bitΪ1
	inline uint32_t
	SetBit(uint32_t x, uint32_t n)
		{ return x | SetMask(n); }

	// ȡ���ֽ�
	inline uint16_t
	LO_U8(uint16_t x)
		{ return static_cast<uint16_t>(x & 0xFF); }
	// ȡ���ֽ�
	inline uint16_t
	HI_U8(uint16_t x)
		{ return static_cast<uint16_t>((x & 0xFF) >> 8); }

	// ȡ����
	inline uint32_t
	LO_U16(uint32_t x)
		{ return x & 0xFFFF; }
	// ȡ����
	inline uint32_t
	HI_U16(uint32_t x)
		{ return (x & 0xFFFF) >> 16; }

	// �ߵ��ֽڽ���
	inline uint16_t
	HI_LO_SwapU8(uint16_t x)
		{ return static_cast<uint16_t>((LO_U8(x) << 8) | HI_U8(x)); }
	// �ߵ��ֽ���
	inline uint32_t
	HI_LO_SwapU16(uint32_t x)
		{ return (LO_U16(x) << 16) | HI_U16(x); }

	// ���nλ����1������
	inline uint32_t
	MakeMask(uint32_t n)
		{ return (1UL << (n + 1)) - 1; }

	// ����FourCC����
	template <unsigned char ch0, unsigned char ch1, unsigned char ch2, unsigned char ch3>
	struct MakeFourCC
	{
		enum { value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) };
	};

	// Unicode����, ����string, wstring֮���ת��
	std::string& Convert(std::string& strDest, std::string const & strSrc);
	std::string& Convert(std::string& strDest, std::wstring const & wstrSrc);
	std::wstring& Convert(std::wstring& wstrDest, std::string const & strSrc);
	std::wstring& Convert(std::wstring& wstrDest, std::wstring const & wstrSrc);

	// ��ͣ������
	void Sleep(uint32_t ms);

	// Endian��ת��
	template <int size>
	void EndianSwitch(void* p);

	template <>
	void EndianSwitch<2>(void* p);
	template <>
	void EndianSwitch<4>(void* p);
	template <>
	void EndianSwitch<8>(void* p);

	template <int size>
	void NativeToBigEndian(void* p)
	{
	#ifdef KLAYGE_LITTLE_ENDIAN
		EndianSwitch<size>(p);
	#else
		UNREF_PARAM(p);
	#endif
	}
	template <int size>
	void NativeToLittleEndian(void* p)
	{
	#ifdef KLAYGE_LITTLE_ENDIAN
		UNREF_PARAM(p);
	#else
		EndianSwitch<size>(p);
	#endif
	}

	template <int size>
	void BigEndianToNative(void* p)
	{
		NativeToBigEndian<size>(p);
	}
	template <int size>
	void LittleEndianToNative(void* p)
	{
		NativeToLittleEndian<size>(p);
	}


	template <typename To, typename From>
	inline To
	checked_cast(From p)
	{
		BOOST_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
		return static_cast<To>(p);
	}

	template <typename To, typename From>
	inline shared_ptr<To>
	checked_pointer_cast(shared_ptr<From> const & p)
	{
		BOOST_ASSERT(dynamic_pointer_cast<To>(p) == static_pointer_cast<To>(p));
		return static_pointer_cast<To>(p);
	}

	uint32_t LastError();

#ifdef KLAYGE_IDENTITY_SUPPORT
	template <typename arg_type>
	struct identity : public std::unary_function<arg_type, arg_type>
	{
		arg_type const & operator()(arg_type const & x) const
		{
			return x;
		}
	};
#else
	using std::identity;
#endif		// KLAYGE_IDENTITY_SUPPORT

#ifdef KLAYGE_SELECT1ST2ND_SUPPORT
	template <typename pair_type>
	struct select1st : public std::unary_function<pair_type, typename pair_type::first_type>
	{
		typename pair_type::first_type const & operator()(pair_type const & x) const
		{
			return x.first;
		}
	};

	template <typename pair_type>
	struct select2nd : public std::unary_function<pair_type, typename pair_type::second_type>
	{
		typename pair_type::second_type const & operator()(pair_type const & x) const
		{
			return x.second;
		}
	};
#else
	using std::select1st;
	using std::select2nd;
#endif		// KLAYGE_SELECT1ST2ND_SUPPORT

#ifdef KLAYGE_PROJECT1ST2ND_SUPPORT
	template <typename arg1_type, typename arg2_type>
	struct project1st : public std::binary_function<arg1_type, arg2_type, arg1_type>
	{
		arg1_type operator()(arg1_type const & x, arg2_type const & /*y*/) const
		{
			return x;
		}
	};

	template <typename arg1_type, typename arg2_type>
	struct project2nd : public std::binary_function<arg1_type, arg2_type, arg2_type>
	{
		arg2_type operator()(arg1_type const & /*x*/, arg2_type const & y) const
		{
			return y;
		}
	};
#else
	using std::project1st;
	using std::project2nd;
#endif		// KLAYGE_PROJECT1ST2ND_SUPPORT

	std::string ReadShortString(ResIdentifierPtr const & res);
	void WriteShortString(std::ostream& os, std::string const & str);

	template <typename T>
	inline shared_ptr<T> MakeSharedPtr()
	{
		return shared_ptr<T>(new T, boost::checked_deleter<T>());
	}

	template <typename T, typename A1>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1)
	{
		return shared_ptr<T>(new T(a1), boost::checked_deleter<T>());
	}

	template <typename T, typename A1>
	inline shared_ptr<T> MakeSharedPtr(A1& a1)
	{
		return shared_ptr<T>(new T(a1), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2)
	{
		return shared_ptr<T>(new T(a1, a2), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2)
	{
		return shared_ptr<T>(new T(a1, a2), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3)
	{
		return shared_ptr<T>(new T(a1, a2, a3), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3)
	{
		return shared_ptr<T>(new T(a1, a2, a3), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7,
		A8& a8)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7,
		A8& a8, A9& a9)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9, typename A10>
	inline shared_ptr<T> MakeSharedPtr(A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4,
		A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9, A10 const & a10)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), boost::checked_deleter<T>());
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
		typename A7, typename A8, typename A9, typename A10>
	inline shared_ptr<T> MakeSharedPtr(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7,
		A8& a8, A9& a9, A10& a10)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), boost::checked_deleter<T>());
	}
}

#endif		// _KFL_UTIL_HPP
