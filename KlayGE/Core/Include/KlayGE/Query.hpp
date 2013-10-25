// Query.hpp
// KlayGE ��ѯ������ ʵ���ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ������ConditionalRender::AnySamplesPassed (2010.4.3)
//
// 3.8.0
// ����ConditionalRender (2008.10.11)
//
// 3.0.0
// ���ν��� (2005.10.18)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _QUERY_HPP
#define _QUERY_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API Query
	{
	public:
		virtual ~Query()
		{
		}

		static QueryPtr NullObject();

		virtual void Begin() = 0;
		virtual void End() = 0;
	};

	class KLAYGE_CORE_API OcclusionQuery : public Query
	{
	public:
		virtual ~OcclusionQuery()
		{
		}

		virtual uint64_t SamplesPassed() = 0;
	};

	class KLAYGE_CORE_API ConditionalRender : public Query
	{
	public:
		virtual ~ConditionalRender()
		{
		}

		virtual void BeginConditionalRender() = 0;
		virtual void EndConditionalRender() = 0;

		virtual bool AnySamplesPassed() = 0;
	};
}

#endif		// _QUERY_HPP
