// InputFactory.hpp
// KlayGE ����������󹤳� ͷ�ļ�
// Ver 3.1.0
// ��Ȩ����(C) ������, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// ������NullObject (2005.10.29)
//
// 2.0.0
// ���ν��� (2003.8.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _INPUTFACTORY_HPP
#define _INPUTFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>
#include <boost/noncopyable.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API InputFactory
	{
	public:
		virtual ~InputFactory()
			{ }

		static InputFactoryPtr NullObject();

		virtual std::wstring const & Name() const = 0;
		InputEngine& InputEngineInstance();

	private:
		virtual InputEnginePtr DoMakeInputEngine() = 0;

	protected:
		InputEnginePtr ie_;
	};

	template <typename InputEngineType>
	class ConcreteInputFactory : boost::noncopyable, public InputFactory
	{
	public:
		ConcreteInputFactory(std::wstring const & name)
				: name_(name)
			{ }

		std::wstring const & Name() const
			{ return name_; }

	private:
		InputEnginePtr DoMakeInputEngine()
		{
			return MakeSharedPtr<InputEngineType>();
		}

	private:
		std::wstring const name_;
	};
}

#endif			// _INPUTFACTORY_HPP
