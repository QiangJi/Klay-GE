// ShowFactory.hpp
// KlayGE ����������󹤳� ͷ�ļ�
// Ver 3.4.0
// ��Ȩ����(C) ������, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// ���ν��� (2006.7.15)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _SHOWFACTORY_HPP
#define _SHOWFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>
#include <boost/noncopyable.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ShowFactory
	{
	public:
		virtual ~ShowFactory()
			{ }

		static ShowFactoryPtr NullObject();

		virtual std::wstring const & Name() const = 0;
		ShowEngine& ShowEngineInstance();

	private:
		virtual ShowEnginePtr MakeShowEngine() = 0;

	protected:
		ShowEnginePtr se_;
	};

	template <typename ShowEngineType>
	class ConcreteShowFactory : boost::noncopyable, public ShowFactory
	{
	public:
		ConcreteShowFactory(std::wstring const & name)
				: name_(name)
			{ }

		std::wstring const & Name() const
			{ return name_; }

	private:
		ShowEnginePtr MakeShowEngine()
		{
			return MakeSharedPtr<ShowEngineType>();
		}

	private:
		std::wstring const name_;
	};
}

#endif			// _SHOWFACTORY_HPP
