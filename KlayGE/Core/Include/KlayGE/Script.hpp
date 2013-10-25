/**
 * @file Script.hpp
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

#ifndef _SCRIPT_HPP
#define _SCRIPT_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <vector>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

namespace KlayGE
{
	typedef std::vector<boost::any> AnyDataListType;

	class KLAYGE_CORE_API ScriptModule
	{
	private:
		template <typename tuple_type, int N>
		struct Tuple2Vector
		{
			static AnyDataListType Do(tuple_type const & t)
			{
				AnyDataListType ret = Tuple2Vector<tuple_type, N - 1>::Do(t);
				ret.push_back(get<N - 1>(t));
				return ret;
			}
		};

		template <typename tuple_type>
		struct Tuple2Vector<tuple_type, 1>
		{
			static AnyDataListType Do(tuple_type const & t)
			{
				return AnyDataListType(1, get<0>(t));
			}
		};

	public:
		ScriptModule();
		virtual ~ScriptModule();

		template <typename TupleType>
		boost::any Call(std::string const & func_name, TupleType const & t)
		{
			AnyDataListType v(Tuple2Vector<TupleType, tuple_size<TupleType>::value>::Do(t));
			return Call(func_name, v);
		}

		virtual boost::any Value(std::string const & name);
		virtual boost::any Call(std::string const & func_name, const AnyDataListType& args);
		virtual boost::any RunString(std::string const & script);
	};

	typedef shared_ptr<ScriptModule> ScriptModulePtr;

	// ʵ�ֽű�����Ĺ���
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API ScriptEngine : boost::noncopyable
	{
	public:
		ScriptEngine();
		virtual ~ScriptEngine();

		static ScriptEnginePtr NullObject();

		// ����һ���µĽű�ģ��
		virtual ScriptModulePtr CreateModule(std::string const & name);
	};
}

#endif		// _SCRIPT_HPP
