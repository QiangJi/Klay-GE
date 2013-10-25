// RenderDeviceCaps.hpp
// KlayGE ��Ⱦ�豸������ ͷ�ļ�
// Ver 2.8.0
// ��Ȩ����(C) ������, 2005
// Homepage: http://www.klayge.org
//
// 2.8.0
// ���ν��� (2005.7.17)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERDEVICECAPS_HPP
#define _RENDERDEVICECAPS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ElementFormat.hpp>

namespace KlayGE
{
	enum TessellationMethod
	{
		TM_Hardware,
		TM_Instanced,
		TM_No
	};

	struct RenderDeviceCaps
	{
		uint8_t max_shader_model;

		uint32_t max_texture_width;
		uint32_t max_texture_height;
		uint32_t max_texture_depth;
		uint32_t max_texture_cube_size;
		uint32_t max_texture_array_length;
		uint8_t max_vertex_texture_units;
		uint8_t max_pixel_texture_units;
		uint8_t max_geometry_texture_units;
		uint8_t max_simultaneous_rts;
		uint8_t max_simultaneous_uavs;
		uint8_t max_vertex_streams;
		uint8_t max_texture_anisotropy;

		bool is_tbdr;

		bool hw_instancing_support;
		bool instance_id_support;
		bool stream_output_support;
		bool alpha_to_coverage_support;
		bool primitive_restart_support;
		bool multithread_rendering_support;
		bool multithread_res_creating_support;
		bool mrt_independent_bit_depths_support;
		bool standard_derivatives_support;
		bool logic_op_support;

		bool gs_support;
		bool cs_support;
		bool hs_support;
		bool ds_support;

		TessellationMethod tess_method;

		function<bool(ElementFormat)> vertex_format_support;
		function<bool(ElementFormat)> texture_format_support;
		function<bool(ElementFormat, uint32_t, uint32_t)> rendertarget_format_support;
	};
}

#endif			// _RENDERDEVICECAPS_HPP
