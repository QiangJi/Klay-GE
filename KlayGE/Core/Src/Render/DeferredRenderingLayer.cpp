// DeferredRenderingLayer.cpp
// KlayGE Deferred Rendering Layer implement file
// Ver 4.0.0
// Copyright(C) Minmin Gong, 2011
// Homepage: http://www.klayge.org
//
// 4.0.0
// First release (2011.8.28)
//
// CHANGE LIST
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SSVOPostProcess.hpp>
#include <KlayGE/SSRPostProcess.hpp>

#include <KlayGE/DeferredRenderingLayer.hpp>

static uint32_t m_min(uint32_t a, uint32_t b)
{
    if(a < b)
        return a;
    else
        return b;
}

namespace KlayGE
{
	int const SM_SIZE = 512;
	
	int const MAX_IL_MIPMAP_LEVELS = 3;

	int const MAX_RSM_MIPMAP_LEVELS = 7; // (log(512)-log(4))/log(2) + 1
	int const BEGIN_RSM_SAMPLING_LIGHT_LEVEL = 5;
	int const SAMPLE_LEVEL_CNT = MAX_RSM_MIPMAP_LEVELS - BEGIN_RSM_SAMPLING_LIGHT_LEVEL;
	int const VPL_COUNT = 64 * ((1UL << (SAMPLE_LEVEL_CNT * 2)) - 1) / (4 - 1);

	float const ESM_SCALE_FACTOR = 300.0f;

#ifdef LIGHT_INDEXED_DEFERRED
	uint32_t const TILE_SIZE = 32;
#endif

	template <typename T>
	void CreateConeMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float radius, float height, uint16_t n)
	{
		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(T());
			vb.back().x() = vb.back().y() = vb.back().z() = 0;
		}

		float outer_radius = radius / cos(PI / n);
		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(T());
			float angle = i * 2 * PI / n;
			vb.back().x() = outer_radius * cos(angle);
			vb.back().y() = outer_radius * sin(angle);
			vb.back().z() = height;
		}

		vb.push_back(T());
		vb.back().x() = vb.back().y() = 0;
		vb.back().z() = height;

		for (int i = 0; i < n; ++ i)
		{
			vb.push_back(T());
			vb.back() = vb[vertex_base + n + i];
		}

		for (uint16_t i = 0; i < n - 1; ++ i)
		{
			ib.push_back(vertex_base + i);
			ib.push_back(vertex_base + n + i + 1);
			ib.push_back(vertex_base + n + i);
		}
		ib.push_back(vertex_base + n - 1);
		ib.push_back(vertex_base + n + 0);
		ib.push_back(vertex_base + n + n - 1);

		for (uint16_t i = 0; i < n - 1; ++ i)
		{
			ib.push_back(vertex_base + 2 * n);
			ib.push_back(vertex_base + 2 * n + 1 + i);
			ib.push_back(vertex_base + 2 * n + 1 + i + 1);
		}
		ib.push_back(vertex_base + 2 * n);
		ib.push_back(vertex_base + 2 * n + 1 + n - 1);
		ib.push_back(vertex_base + 2 * n + 1);
	}

	template <typename T>
	void CreatePyramidMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float radius, float height)
	{
		for (int i = 0; i < 4; ++ i)
		{
			vb.push_back(T());
			vb.back().x() = vb.back().y() = vb.back().z() = 0;
		}

		float outer_radius = radius * sqrt(2.0f);
		vb.push_back(T());
		vb.back().x() = -outer_radius;
		vb.back().y() = -outer_radius;
		vb.back().z() = height;
		vb.push_back(T());
		vb.back().x() = +outer_radius;
		vb.back().y() = -outer_radius;
		vb.back().z() = height;
		vb.push_back(T());
		vb.back().x() = +outer_radius;
		vb.back().y() = +outer_radius;
		vb.back().z() = height;
		vb.push_back(T());
		vb.back().x() = -outer_radius;
		vb.back().y() = +outer_radius;
		vb.back().z() = height;

		vb.push_back(T());
		vb.back().x() = vb.back().y() = 0;
		vb.back().z() = height;

		for (int i = 0; i < 4; ++ i)
		{
			vb.push_back(T());
			vb.back() = vb[vertex_base + 4 + i];
		}

		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 7);

		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 9);
		ib.push_back(vertex_base + 10);
		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 10);
		ib.push_back(vertex_base + 11);
		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 11);
		ib.push_back(vertex_base + 12);
		ib.push_back(vertex_base + 8);
		ib.push_back(vertex_base + 12);
		ib.push_back(vertex_base + 9);
	}

	template <typename T>
	void CreateBoxMesh(std::vector<T>& vb, std::vector<uint16_t>& ib, uint16_t vertex_base, float half_length)
	{
		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = +half_length;
		vb.back().z() = -half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = +half_length;
		vb.back().z() = -half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = -half_length;
		vb.back().z() = -half_length;
		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = -half_length;
		vb.back().z() = -half_length;

		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = +half_length;
		vb.back().z() = +half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = +half_length;
		vb.back().z() = +half_length;
		vb.push_back(T());
		vb.back().x() = +half_length;
		vb.back().y() = -half_length;
		vb.back().z() = +half_length;
		vb.push_back(T());
		vb.back().x() = -half_length;
		vb.back().y() = -half_length;
		vb.back().z() = +half_length;

		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 0);

		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 5);

		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 4);

		ib.push_back(vertex_base + 1);
		ib.push_back(vertex_base + 5);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 1);

		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 2);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 6);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 3);

		ib.push_back(vertex_base + 4);
		ib.push_back(vertex_base + 0);
		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 3);
		ib.push_back(vertex_base + 7);
		ib.push_back(vertex_base + 4);
	}


	DeferredRenderingLayer::DeferredRenderingLayer()
		: active_viewport_(0), ssr_enabled_(true), taa_enabled_(true),
			light_scale_(1), illum_(0), indirect_scale_(1.0f)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		ElementFormat ds_fmt;
		if (caps.texture_format_support(EF_D24S8))
		{
			ds_fmt = EF_D24S8;
			depth_texture_support_ = true;
		}
		else
		{
			ds_fmt = EF_D16;
			if (caps.texture_format_support(EF_D16))
			{				
				depth_texture_support_ = true;
			}
			else
			{
				depth_texture_support_ = false;
			}
		}

		mrt_g_buffer_support_ = (caps.max_simultaneous_rts > 1);
		tex_array_support_ = (caps.max_texture_array_length >= 4);

		for (size_t vpi = 0; vpi < viewports_.size(); ++ vpi)
		{
			PerViewport& pvp = viewports_[vpi];
			if (!depth_texture_support_)
			{
				pvp.pre_depth_buffer = rf.MakeFrameBuffer();
			}
			pvp.g_buffer = rf.MakeFrameBuffer();
			if (!mrt_g_buffer_support_)
			{
				pvp.g_buffer_rt1 = rf.MakeFrameBuffer();
			}
			pvp.lighting_buffer = rf.MakeFrameBuffer();
			pvp.shading_buffer = rf.MakeFrameBuffer();
			pvp.shadowing_buffer = rf.MakeFrameBuffer();
			pvp.projective_shadowing_buffer = rf.MakeFrameBuffer();
			pvp.curr_merged_shading_buffer = rf.MakeFrameBuffer();
			pvp.curr_merged_depth_buffer = rf.MakeFrameBuffer();
			pvp.prev_merged_shading_buffer = rf.MakeFrameBuffer();
			pvp.prev_merged_depth_buffer = rf.MakeFrameBuffer();
#ifdef LIGHT_INDEXED_DEFERRED
			pvp.light_index_buffer = rf.MakeFrameBuffer();
#endif
		}

		{
			rl_cone_ = rf.MakeRenderLayout();
			rl_cone_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateConeMesh(pos, index, 0, 100.0f, 100.0f, 12);
			cone_obb_ = MathLib::compute_obbox(pos.begin(), pos.end());

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_cone_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data),
				make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.data = &index[0];
			rl_cone_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data), EF_R16UI);
		}
		{
			rl_pyramid_ = rf.MakeRenderLayout();
			rl_pyramid_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreatePyramidMesh(pos, index, 0, 100.0f, 100.0f);
			pyramid_obb_ = MathLib::compute_obbox(pos.begin(), pos.end());

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_pyramid_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data),
				make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.data = &index[0];
			rl_pyramid_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data), EF_R16UI);
		}
		{
			rl_box_ = rf.MakeRenderLayout();
			rl_box_->TopologyType(RenderLayout::TT_TriangleList);

			std::vector<float3> pos;
			std::vector<uint16_t> index;
			CreateBoxMesh(pos, index, 0, 100.0f);
			box_obb_ = MathLib::compute_obbox(pos.begin(), pos.end());

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_box_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data),
				make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
			init_data.data = &index[0];
			rl_box_->BindIndexStream(rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data), EF_R16UI);
		}
		{
			rl_quad_ = rf.MakeRenderLayout();
			rl_quad_->TopologyType(RenderLayout::TT_TriangleStrip);

			std::vector<float3> pos;
			std::vector<uint16_t> index;

			pos.push_back(float3(+1, +1, 1));
			pos.push_back(float3(-1, +1, 1));
			pos.push_back(float3(+1, -1, 1));
			pos.push_back(float3(-1, -1, 1));

			ElementInitData init_data;
			init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
			init_data.slice_pitch = 0;
			init_data.data = &pos[0];
			rl_quad_->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data),
				make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
		}

		light_volume_rl_[LightSource::LT_Ambient] = rl_quad_;
		light_volume_rl_[LightSource::LT_Directional] = rl_quad_;
		light_volume_rl_[LightSource::LT_Point] = rl_box_;
		light_volume_rl_[LightSource::LT_Spot] = rl_cone_;
		light_volume_rl_[LightSource::LT_Sun] = rl_quad_;

		g_buffer_effect_ = SyncLoadRenderEffect("GBuffer.fxml");
		dr_effect_ = SyncLoadRenderEffect("DeferredRendering.fxml");

		technique_shadows_[LightSource::LT_Point][0] = dr_effect_->TechniqueByName("DeferredShadowingPointR");
		technique_shadows_[LightSource::LT_Point][1] = dr_effect_->TechniqueByName("DeferredShadowingPointG");
		technique_shadows_[LightSource::LT_Point][2] = dr_effect_->TechniqueByName("DeferredShadowingPointB");
		technique_shadows_[LightSource::LT_Point][3] = dr_effect_->TechniqueByName("DeferredShadowingPointA");
		technique_shadows_[LightSource::LT_Point][4] = dr_effect_->TechniqueByName("DeferredShadowingPoint");
		technique_shadows_[LightSource::LT_Spot][0] = dr_effect_->TechniqueByName("DeferredShadowingSpotR");
		technique_shadows_[LightSource::LT_Spot][1] = dr_effect_->TechniqueByName("DeferredShadowingSpotG");
		technique_shadows_[LightSource::LT_Spot][2] = dr_effect_->TechniqueByName("DeferredShadowingSpotB");
		technique_shadows_[LightSource::LT_Spot][3] = dr_effect_->TechniqueByName("DeferredShadowingSpotA");
		technique_shadows_[LightSource::LT_Spot][4] = dr_effect_->TechniqueByName("DeferredShadowingSpot");
		technique_shadows_[LightSource::LT_Sun][0] = dr_effect_->TechniqueByName("DeferredShadowingSunR");
		technique_shadows_[LightSource::LT_Sun][1] = dr_effect_->TechniqueByName("DeferredShadowingSunG");
		technique_shadows_[LightSource::LT_Sun][2] = dr_effect_->TechniqueByName("DeferredShadowingSunB");
		technique_shadows_[LightSource::LT_Sun][3] = dr_effect_->TechniqueByName("DeferredShadowingSunA");
		technique_shadows_[LightSource::LT_Sun][4] = dr_effect_->TechniqueByName("DeferredShadowingSun");
		technique_lights_[LightSource::LT_Ambient] = dr_effect_->TechniqueByName("DeferredRenderingAmbient");
		technique_lights_[LightSource::LT_Directional] = dr_effect_->TechniqueByName("DeferredRenderingDirectional");
		technique_lights_[LightSource::LT_Point] = dr_effect_->TechniqueByName("DeferredRenderingPoint");
		technique_lights_[LightSource::LT_Spot] = dr_effect_->TechniqueByName("DeferredRenderingSpot");
		technique_lights_[LightSource::LT_Sun] = dr_effect_->TechniqueByName("DeferredRenderingSun");
		technique_light_depth_only_ = dr_effect_->TechniqueByName("DeferredRenderingLightDepthOnly");
		technique_light_stencil_ = dr_effect_->TechniqueByName("DeferredRenderingLightStencil");
		technique_no_lighting_ = dr_effect_->TechniqueByName("NoLightingTech");
		technique_shading_ = dr_effect_->TechniqueByName("ShadingTech");
		technique_merge_shadings_[0] = dr_effect_->TechniqueByName("MergeShadingTech");
		technique_merge_shadings_[1] = dr_effect_->TechniqueByName("MergeShadingAlphaBlendTech");
		technique_merge_depths_[0] = dr_effect_->TechniqueByName("MergeDepthTech");
		technique_merge_depths_[1] = dr_effect_->TechniqueByName("MergeDepthAlphaBlendTech");
		technique_copy_shading_depth_ = dr_effect_->TechniqueByName("CopyShadingDepthTech");
		technique_copy_depth_ = dr_effect_->TechniqueByName("CopyDepthTech");
#ifdef LIGHT_INDEXED_DEFERRED
		technique_draw_light_index_point_ = dr_effect_->TechniqueByName("DrawLightIndexPoint");
		technique_draw_light_index_spot_ = dr_effect_->TechniqueByName("DrawLightIndexSpot");
		technique_light_indexed_deferred_rendering_ambient_ = dr_effect_->TechniqueByName("LightIndexedDeferredRenderingAmbient");
		technique_light_indexed_deferred_rendering_sun_ = dr_effect_->TechniqueByName("LightIndexedDeferredRenderingSun");
		technique_light_indexed_deferred_rendering_directional_ = dr_effect_->TechniqueByName("LightIndexedDeferredRenderingDirectional");
		technique_light_indexed_deferred_rendering_point_shadow_ = dr_effect_->TechniqueByName("LightIndexedDeferredRenderingPointShadow");
		technique_light_indexed_deferred_rendering_point_no_shadow_ = dr_effect_->TechniqueByName("LightIndexedDeferredRenderingPointNoShadow");
		technique_light_indexed_deferred_rendering_spot_shadow_ = dr_effect_->TechniqueByName("LightIndexedDeferredRenderingSpotShadow");
		technique_light_indexed_deferred_rendering_spot_no_shadow_ = dr_effect_->TechniqueByName("LightIndexedDeferredRenderingSpotNoShadow");
#endif

		sm_buffer_ = rf.MakeFrameBuffer();
		ElementFormat fmt;
		if (caps.rendertarget_format_support(EF_R32F, 1, 0))
		{
			fmt = EF_R32F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR32F, 1, 0));

			fmt = EF_ABGR32F;
		}
		sm_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		sm_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*sm_tex_, 0, 1, 0));
		RenderViewPtr sm_depth_view;
		if (depth_texture_support_)
		{
			sm_depth_tex_ = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			sm_depth_view = rf.Make2DDepthStencilRenderView(*sm_depth_tex_, 0, 1, 0);
		}
		else
		{
			sm_depth_view = rf.Make2DDepthStencilRenderView(SM_SIZE, SM_SIZE, EF_D16, 1, 0);
		}
		sm_buffer_->Attach(FrameBuffer::ATT_DepthStencil, sm_depth_view);

		cascaded_sm_buffer_ = rf.MakeFrameBuffer();
		cascaded_sm_tex_ = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		cascaded_sm_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*cascaded_sm_tex_, 0, 1, 0));
		cascaded_sm_buffer_->Attach(FrameBuffer::ATT_DepthStencil,
			rf.Make2DDepthStencilRenderView(SM_SIZE * 2, SM_SIZE * 2, ds_fmt, 1, 0));

		for (uint32_t i = 0; i < blur_sm_2d_texs_.size(); ++ i)
		{
			blur_sm_2d_texs_[i] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		}
		for (uint32_t i = 0; i < blur_sm_cube_texs_.size(); ++ i)
		{
			blur_sm_cube_texs_[i] = rf.MakeTextureCube(SM_SIZE, 1, 1, sm_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		}

		ssvo_pp_ = MakeSharedPtr<SSVOPostProcess>();
		ssvo_blur_pp_ = MakeSharedPtr<BlurPostProcess<SeparableBilateralFilterPostProcess> >(8, 1.0f,
			SyncLoadRenderEffect("SSVO.fxml")->TechniqueByName("SSVOBlurX"),
			SyncLoadRenderEffect("SSVO.fxml")->TechniqueByName("SSVOBlurY"));

		ssr_pp_ = MakeSharedPtr<SSRPostProcess>();

		taa_pp_ = SyncLoadPostProcess("TAA.ppml", "taa");

		if (depth_texture_support_ && mrt_g_buffer_support_)
		{
			rsm_buffer_ = rf.MakeFrameBuffer();

			ElementFormat fmt8;
			if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt8 = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

				fmt8 = EF_ARGB8;
			}

			rsm_texs_[0] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, MAX_RSM_MIPMAP_LEVELS, 1, fmt8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			rsm_texs_[1] = rf.MakeTexture2D(SM_SIZE, SM_SIZE, MAX_RSM_MIPMAP_LEVELS, 1, fmt8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			rsm_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*rsm_texs_[0], 0, 1, 0)); // albedo
			rsm_buffer_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*rsm_texs_[1], 0, 1, 0)); // normal (light space)
			rsm_buffer_->Attach(FrameBuffer::ATT_DepthStencil, sm_depth_view);
			
			copy_to_light_buffer_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBuffer");
			copy_to_light_buffer_i_pp_ = SyncLoadPostProcess("Copy2LightBuffer.ppml", "CopyToLightBufferI");
		}


		sm_filter_pp_ = MakeSharedPtr<LogGaussianBlurPostProcess>(4, true);
		if (depth_texture_support_)
		{
			depth_to_esm_pp_ = SyncLoadPostProcess("DepthToSM.ppml", "DepthToESM");
			depth_to_esm_pp_->InputPin(0, sm_depth_tex_);
			depth_to_esm_pp_->OutputPin(0, sm_tex_);

			depth_to_linear_pp_ = SyncLoadPostProcess("DepthToSM.ppml", "DepthToSM");
		}

		g_buffer_tex_param_ = dr_effect_->ParameterByName("g_buffer_tex");
		g_buffer_1_tex_param_ = dr_effect_->ParameterByName("g_buffer_1_tex");
		depth_tex_param_ = dr_effect_->ParameterByName("depth_tex");
		lighting_tex_param_ = dr_effect_->ParameterByName("lighting_tex");
		shading_tex_param_ = dr_effect_->ParameterByName("shading_tex");
		depth_near_far_invfar_param_ = dr_effect_->ParameterByName("depth_near_far_invfar");
		light_attrib_param_ = dr_effect_->ParameterByName("light_attrib");
		light_color_param_ = dr_effect_->ParameterByName("light_color");
		light_falloff_param_ = dr_effect_->ParameterByName("light_falloff");
		light_view_proj_param_ = dr_effect_->ParameterByName("light_view_proj");
		light_volume_mv_param_ = dr_effect_->ParameterByName("light_volume_mv");
		light_volume_mvp_param_ = dr_effect_->ParameterByName("light_volume_mvp");
		view_to_light_model_param_ = dr_effect_->ParameterByName("view_to_light_model");
		light_pos_es_param_ = dr_effect_->ParameterByName("light_pos_es");
		light_dir_es_param_ = dr_effect_->ParameterByName("light_dir_es");
		projective_map_2d_tex_param_ = dr_effect_->ParameterByName("projective_map_2d_tex");
		projective_map_cube_tex_param_ = dr_effect_->ParameterByName("projective_map_cube_tex");
		shadow_map_2d_tex_param_ = dr_effect_->ParameterByName("shadow_map_2d_tex");
		shadow_map_cube_tex_param_ = dr_effect_->ParameterByName("shadow_map_cube_tex");
		inv_width_height_param_ = dr_effect_->ParameterByName("inv_width_height");
		shadowing_tex_param_ = dr_effect_->ParameterByName("shadowing_tex");
		projective_shadowing_tex_param_ = dr_effect_->ParameterByName("projective_shadowing_tex");
		shadowing_channel_param_ = dr_effect_->ParameterByName("shadowing_channel");
		esm_scale_factor_param_ = dr_effect_->ParameterByName("esm_scale_factor");
		near_q_param_ = dr_effect_->ParameterByName("near_q");
		cascade_intervals_param_ = dr_effect_->ParameterByName("cascade_intervals");
		cascade_scale_bias_param_ = dr_effect_->ParameterByName("cascade_scale_bias");
		num_cascades_param_ = dr_effect_->ParameterByName("num_cascades");
		view_z_to_light_view_param_ = dr_effect_->ParameterByName("view_z_to_light_view");
		if (tex_array_support_)
		{
			cascaded_shadow_map_tex_array_param_ = dr_effect_->ParameterByName("cascaded_shadow_map_tex_array");
		}
		else
		{
			cascaded_shadow_map_texs_param_[0] = dr_effect_->ParameterByName("cascaded_shadow_map_0_tex");
			cascaded_shadow_map_texs_param_[1] = dr_effect_->ParameterByName("cascaded_shadow_map_1_tex");
			cascaded_shadow_map_texs_param_[2] = dr_effect_->ParameterByName("cascaded_shadow_map_2_tex");
			cascaded_shadow_map_texs_param_[3] = dr_effect_->ParameterByName("cascaded_shadow_map_3_tex");
		}
#ifdef LIGHT_INDEXED_DEFERRED
		min_max_depth_tex_param_ = dr_effect_->ParameterByName("min_max_depth_tex");
		lights_color_param_ = dr_effect_->ParameterByName("lights_color");
		lights_pos_es_param_ = dr_effect_->ParameterByName("lights_pos_es");
		lights_dir_es_param_ = dr_effect_->ParameterByName("lights_dir_es");
		lights_falloff_range_param_ = dr_effect_->ParameterByName("lights_falloff_range");
		lights_attrib_param_ = dr_effect_->ParameterByName("lights_attrib");
		lights_shadowing_channel_param_ = dr_effect_->ParameterByName("lights_shadowing_channel");
		lights_aabb_min_param_ = dr_effect_->ParameterByName("lights_aabb_min");
		lights_aabb_max_param_ = dr_effect_->ParameterByName("lights_aabb_max");
		num_lights_param_ = dr_effect_->ParameterByName("num_lights");
		light_index_tex_param_ = dr_effect_->ParameterByName("light_index_tex");
		light_index_tex_width_height_param_ = dr_effect_->ParameterByName("light_index_tex_width_height");
		tile_scale_param_ = dr_effect_->ParameterByName("tile_scale");
		camera_proj_param_ = dr_effect_->ParameterByName("camera_proj");
		tc_to_tile_scale_param_ = dr_effect_->ParameterByName("tc_to_tile_scale");

		depth_to_min_max_pp_ = SyncLoadPostProcess("DepthToSM.ppml", "DepthToMinMax");
		reduce_min_max_pp_ = SyncLoadPostProcess("DepthToSM.ppml", "ReduceMinMax");
#endif

		this->SetCascadedShadowType(CSLT_Auto);
	}

	void DeferredRenderingLayer::SSGIEnabled(uint32_t vp, bool ssgi)
	{
		this->SetupViewportGI(vp, ssgi);
	}

	void DeferredRenderingLayer::SSVOEnabled(uint32_t vp, bool ssvo)
	{
		viewports_[vp].ssvo_enabled = ssvo;
	}

	void DeferredRenderingLayer::SSREnabled(bool ssr)
	{
		ssr_enabled_ = ssr;
	}

	void DeferredRenderingLayer::TemporalAAEnabled(bool taa)
	{
		taa_enabled_ = taa;
	}

	void DeferredRenderingLayer::AddDecal(RenderDecalPtr const & decal)
	{
		decals_.push_back(decal);
	}

	void DeferredRenderingLayer::SetupViewport(uint32_t index, FrameBufferPtr const & fb, uint32_t attrib)
	{
		PerViewport& pvp = viewports_[index];
		pvp.attrib = attrib;
		pvp.frame_buffer = fb;
		pvp.frame_buffer->GetViewport()->camera->JitterMode(true);

		if (fb)
		{
			pvp.attrib |= VPAM_Enabled;
		}

		uint32_t const width = pvp.frame_buffer->GetViewport()->width;
		uint32_t const height = pvp.frame_buffer->GetViewport()->height;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		ElementFormat fmt8;
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt8 = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

			fmt8 = EF_ARGB8;
		}
		ElementFormat depth_fmt;
		if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			depth_fmt = EF_R16F;
		}
		else
		{
			if (caps.rendertarget_format_support(EF_R32F, 1, 0))
			{
				depth_fmt = EF_R32F;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

				depth_fmt = EF_ABGR16F;
			}
		}

		RenderViewPtr ds_view;
		if (depth_texture_support_)
		{
			ElementFormat ds_fmt;
			if (caps.texture_format_support(EF_D24S8))
			{
				ds_fmt = EF_D24S8;
			}
			else
			{
				BOOST_ASSERT(caps.texture_format_support(EF_D16));
				
				ds_fmt = EF_D16;
			}

			pvp.g_buffer_ds_tex = rf.MakeTexture2D(width, height, 1, 1, ds_fmt, 1, 0,  EAH_GPU_Read | EAH_GPU_Write, nullptr);
			ds_view = rf.Make2DDepthStencilRenderView(*pvp.g_buffer_ds_tex, 0, 0, 0);
		}
		else
		{
			ds_view = rf.Make2DDepthStencilRenderView(width, height, EF_D16, 1, 0);
		}

		pvp.g_buffer_rt0_tex = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
		pvp.g_buffer_rt1_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.g_buffer_depth_tex = rf.MakeTexture2D(width, height, MAX_IL_MIPMAP_LEVELS + 1, 1, depth_fmt, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
		pvp.g_buffer_rt0_backup_tex = rf.MakeTexture2D(width, height, 1, 1, fmt8, 1, 0,
			EAH_GPU_Read, nullptr);
#ifdef LIGHT_INDEXED_DEFERRED
		{
			ElementFormat min_max_depth_fmt;
			if (EF_R16F == depth_fmt)
			{
				min_max_depth_fmt = EF_GR16F;
			}
			else if (EF_R32F == depth_fmt)
			{
				min_max_depth_fmt = EF_GR32F;
			}
			else
			{
				min_max_depth_fmt = depth_fmt;
			}

			uint32_t w = std::max(1U, (width + 1) / 2);
			uint32_t h = std::max(1U, (height + 1) / 2);
			for (uint32_t ts = TILE_SIZE; ts > 1; ts /= 2)
			{
				pvp.g_buffer_min_max_depth_texs.push_back(rf.MakeTexture2D(w, h, 1, 1, min_max_depth_fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write, nullptr));
				w = std::max(1U, (w + 1) / 2);
				h = std::max(1U, (h + 1) / 2);
			}
		}
#endif

		RenderViewPtr g_buffer_rt0_view = rf.Make2DRenderView(*pvp.g_buffer_rt0_tex, 0, 1, 0);
		RenderViewPtr g_buffer_rt1_view = rf.Make2DRenderView(*pvp.g_buffer_rt1_tex, 0, 1, 0);
		RenderViewPtr g_buffer_depth_view = rf.Make2DRenderView(*pvp.g_buffer_depth_tex, 0, 1, 0);

		pvp.g_buffer->Attach(FrameBuffer::ATT_Color0, g_buffer_rt0_view);
		pvp.g_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		if (mrt_g_buffer_support_)
		{
			pvp.g_buffer->Attach(FrameBuffer::ATT_Color1, g_buffer_rt1_view);
		}
		else
		{
			pvp.g_buffer_rt1->Attach(FrameBuffer::ATT_Color0, g_buffer_rt1_view);
			pvp.g_buffer_rt1->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}

		if (!depth_texture_support_)
		{
			pvp.pre_depth_buffer->Attach(FrameBuffer::ATT_Color0, g_buffer_depth_view);
			pvp.pre_depth_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}

		this->SetupViewportGI(index, false);

		ElementFormat fmt;
		if (caps.rendertarget_format_support(EF_R32F, 1, 0))
		{
			fmt = EF_R32F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR32F, 1, 0));

			fmt = EF_ABGR32F;
		}
		if (tex_array_support_)
		{
			pvp.blur_cascaded_sm_texs[0] = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 3,
				CascadedShadowLayer::MAX_NUM_CASCADES, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
		}
		else
		{
			for (size_t i = 0; i < pvp.blur_cascaded_sm_texs.size(); ++ i)
			{
				pvp.blur_cascaded_sm_texs[i] = rf.MakeTexture2D(SM_SIZE * 2, SM_SIZE * 2, 3, 1, fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			}
		}

		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

			fmt = EF_ARGB8;
		}
		pvp.shadowing_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.shadowing_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.shadowing_tex, 0, 1, 0));
		
		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

				fmt = EF_ARGB8;
			}
		}
		pvp.projective_shadowing_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.projective_shadowing_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.projective_shadowing_tex, 0, 1, 0));

		pvp.lighting_tex = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.lighting_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.lighting_tex, 0, 1, 0));
		pvp.lighting_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
		{
			fmt = EF_B10G11R11F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}
		pvp.shading_tex = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.curr_merged_shading_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.curr_merged_depth_tex = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.prev_merged_shading_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.prev_merged_depth_tex = rf.MakeTexture2D(width, height, 1, 1, depth_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

		pvp.shading_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.shading_tex, 0, 1, 0));
		pvp.shading_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		pvp.curr_merged_shading_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.curr_merged_shading_tex, 0, 1, 0));
		pvp.curr_merged_shading_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		pvp.curr_merged_depth_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.curr_merged_depth_tex, 0, 1, 0));
		pvp.curr_merged_depth_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		pvp.prev_merged_shading_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.prev_merged_shading_tex, 0, 1, 0));
		pvp.prev_merged_shading_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		pvp.prev_merged_depth_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.prev_merged_depth_tex, 0, 1, 0));
		pvp.prev_merged_depth_buffer->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

		if (caps.rendertarget_format_support(EF_R8, 1, 0))
		{
			fmt = EF_R8;
		}
		else if (caps.rendertarget_format_support(EF_R16F, 1, 0))
		{
			fmt = EF_R16F;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

			fmt = EF_ABGR16F;
		}
		pvp.small_ssvo_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

#ifdef LIGHT_INDEXED_DEFERRED
		if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
		{
			fmt = EF_ABGR8;
		}
		else
		{
			BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

			fmt = EF_ARGB8;
		}
		pvp.light_index_tex = rf.MakeTexture2D((width + (TILE_SIZE - 1)) / TILE_SIZE,
			(height + (TILE_SIZE - 1)) / TILE_SIZE, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
		pvp.light_index_buffer->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*pvp.light_index_tex, 0, 1, 0));
#endif
	}

	void DeferredRenderingLayer::EnableViewport(uint32_t index, bool enable)
	{
		PerViewport& pvp = viewports_[index];
		if (enable)
		{
			pvp.attrib |= VPAM_Enabled;
		}
		else
		{
			pvp.attrib &= ~VPAM_Enabled;
		}
	}

	uint32_t DeferredRenderingLayer::Update(uint32_t pass)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();
		
		if (0 == pass)
		{
			curr_cascade_index_ = -1;

			this->BuildLightList();

			bool has_opaque_objs = false;
			bool has_transparency_back_objs = false;
			bool has_transparency_front_objs = false;
			this->BuildVisibleSceneObjList(has_opaque_objs, has_transparency_back_objs, has_transparency_front_objs);

			this->BuildPassScanList(has_opaque_objs, has_transparency_back_objs, has_transparency_front_objs);
		}

		uint32_t vp_index;
		PassType pass_type;
		int32_t org_no, index_in_pass;
		this->DecomposePassScanCode(vp_index, pass_type, org_no, index_in_pass, pass_scaned_[pass]);

		PassRT const pass_rt = GetPassRT(pass_type);
		PassTargetBuffer const pass_tb = GetPassTargetBuffer(pass_type);
		PassCategory const pass_cat = GetPassCategory(pass_type);

		active_viewport_ = vp_index;

		PerViewport& pvp = viewports_[vp_index];

		if ((pass_cat != PC_Shadowing) && (pass_cat != PC_IndirectLighting) && (pass_cat != PC_Shading))
		{
			typedef KLAYGE_DECLTYPE(visible_scene_objs_) VisibleSceneObjsType;
			KLAYGE_FOREACH(VisibleSceneObjsType::reference deo, visible_scene_objs_)
			{
				deo->Pass(pass_type);
			}
		}

		uint32_t urv;
		switch (pass_cat)
		{
		case PC_Depth:
			// Pre depth for no depth texture support platforms
			this->PreparePVP(pvp);
			this->GenerateDepthBuffer(pvp, pass_tb);
			urv = App3DFramework::URV_Need_Flush | (App3DFramework::URV_Opaque_Only << pass_tb);
			break;

		case PC_GBuffer:
			if (0 == index_in_pass)
			{
				if ((PRT_RT0 == pass_rt) || (PRT_MRT == pass_rt))
				{
					CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
					pvp.shadowing_buffer->GetViewport()->camera = camera;
					pvp.projective_shadowing_buffer->GetViewport()->camera = camera;

					if (depth_texture_support_)
					{
						this->PreparePVP(pvp);

						float q = camera->FarPlane() / (camera->FarPlane() - camera->NearPlane());
						float2 near_q(camera->NearPlane() * q, q);
						depth_to_linear_pp_->SetParam(0, near_q);
					}

					*depth_near_far_invfar_param_ = pvp.depth_near_far_invfar;

					*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
					*depth_tex_param_ = pvp.g_buffer_depth_tex;
					*inv_width_height_param_ = float2(1.0f / pvp.frame_buffer->GetViewport()->width,
						1.0f / pvp.frame_buffer->GetViewport()->height);
					*shadowing_tex_param_ = pvp.shadowing_tex;
					*projective_shadowing_tex_param_ = pvp.projective_shadowing_tex;

					this->GenerateGBuffer(pvp, pass_tb);
				}
				else
				{
					BOOST_ASSERT(PRT_RT1 == pass_rt);

					re.BindFrameBuffer(pvp.g_buffer_rt1);
					pvp.g_buffer_rt1->Attached(FrameBuffer::ATT_Color0)->Discard();
				}
				urv = App3DFramework::URV_Need_Flush | (App3DFramework::URV_Opaque_Only << pass_tb);
			}
			else
			{
				if ((PRT_RT0 == pass_rt) || (PRT_MRT == pass_rt))
				{
					this->PostGenerateGBuffer(pvp);
					if (PTB_Opaque == pass_tb)
					{
						if (indirect_lighting_enabled_ && !(pvp.attrib & VPAM_NoGI))
						{
							pvp.il_layer->UpdateGBuffer(pvp.frame_buffer->GetViewport()->camera);
						}

						if (cascaded_shadow_index_ >= 0)
						{
							CameraPtr const & scene_camera = pvp.frame_buffer->GetViewport()->camera;
							CameraPtr const & light_camera = lights_[cascaded_shadow_index_]->SMCamera(0);

							checked_pointer_cast<SunLightSource>(lights_[cascaded_shadow_index_])->UpdateSMCamera(*scene_camera);

							float const BLUR_FACTOR = 0.2f;
							blur_size_light_space_.x() = BLUR_FACTOR * 0.5f * light_camera->ProjMatrix()(0, 0);
							blur_size_light_space_.y() = BLUR_FACTOR * 0.5f * light_camera->ProjMatrix()(1, 1);
							
							float3 cascade_border(blur_size_light_space_.x(), blur_size_light_space_.y(),
								light_camera->ProjMatrix()(2, 2));
							cascaded_shadow_layer_->NumCascades(pvp.num_cascades);
							if (CSLT_SDSM == cascaded_shadow_layer_->Type())
							{
								checked_pointer_cast<SDSMCascadedShadowLayer>(cascaded_shadow_layer_)->DepthTexture(
									pvp.g_buffer_depth_tex);
							}
							cascaded_shadow_layer_->UpdateCascades(*scene_camera, light_camera->ViewProjMatrix(),
								cascade_border);
						}
					}
				}

				if (PTB_Opaque == pass_tb)
				{
					if (!decals_.empty())
					{
						this->RenderDecals(pvp, (PRT_MRT == pass_rt) ? PT_OpaqueGBufferMRT : PT_OpaqueGBufferRT1);
					}
				}
				urv = App3DFramework::URV_Flushed;
			}
			break;

		case PC_ShadowMap:
			{
				LightSourcePtr const & light = lights_[org_no];
				this->PrepareLightCamera(pvp, light, index_in_pass, pass_type);

				if (index_in_pass > 0)
				{
					this->PostGenerateShadowMap(pvp, org_no, index_in_pass);
				}

				if (((LightSource::LT_Point == light->Type()) && (6 == index_in_pass))
					|| ((LightSource::LT_Spot == light->Type()) && (1 == index_in_pass))
					|| ((LightSource::LT_Sun == light->Type()) && (static_cast<int32_t>(pvp.num_cascades) == index_in_pass)))
				{
					urv = App3DFramework::URV_Flushed;
				}
				else
				{
					urv = App3DFramework::URV_Need_Flush;
					switch (pass_rt)
					{
					case PRT_ShadowMap:
						re.BindFrameBuffer(sm_buffer_);
						sm_buffer_->Attached(FrameBuffer::ATT_Color0)->Discard();
						sm_buffer_->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);
						break;

					case PRT_ShadowMapWODepth:
					case PRT_CascadedShadowMap:
						{
							CameraPtr const & light_camera = sm_buffer_->GetViewport()->camera;
							if (PRT_ShadowMapWODepth == pass_rt)
							{
								re.BindFrameBuffer(sm_buffer_);
							}
							else
							{
								cascaded_sm_buffer_->GetViewport()->camera = light_camera;
								re.BindFrameBuffer(cascaded_sm_buffer_);
							}
							float const far_plane = light_camera->FarPlane();
							re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth,
								Color(far_plane, 0, 0, 0), 1.0f, 0);
						}
						break;

					default:
						BOOST_ASSERT(PRT_ReflectiveShadowMap == pass_rt);

						rsm_buffer_->GetViewport()->camera = sm_buffer_->GetViewport()->camera;
						re.BindFrameBuffer(rsm_buffer_);
						rsm_buffer_->Attached(FrameBuffer::ATT_Color0)->Discard();
						rsm_buffer_->Attached(FrameBuffer::ATT_Color1)->Discard();
						rsm_buffer_->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1.0f, 0);
						urv |= App3DFramework::URV_Opaque_Only;
						break;
					}
				}
			}
			break;

		case PC_IndirectLighting:
			if (depth_texture_support_)
			{
				depth_to_esm_pp_->Apply();
			}
			pvp.il_layer->UpdateRSM(rsm_buffer_->GetViewport()->camera, lights_[org_no]);
			urv = App3DFramework::URV_Flushed;
			break;

		case PC_Shadowing:
			{
				LightSourcePtr const & light = lights_[org_no];
				LightSource::LightType type = light->Type();
				int32_t attr = light->Attrib();

				this->PrepareLightCamera(pvp, light, index_in_pass, pass_type);

				if (LightSource::LT_Point == type)
				{
					*projective_map_cube_tex_param_ = light->ProjectiveTexture();
				}
				else
				{
					*projective_map_2d_tex_param_ = light->ProjectiveTexture();
				}

				*light_attrib_param_ = float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
					attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f,
					attr & LightSource::LSA_NoShadow ? -1.0f : 1.0f, light->ProjectiveTexture() ? 1.0f : -1.0f);

				this->UpdateShadowing(pvp, org_no);

				urv = App3DFramework::URV_Flushed;
			}
			break;

		case PC_Shading:
			{
#ifdef LIGHT_INDEXED_DEFERRED
				if (PTB_Opaque == pass_tb)
				{
					*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
					*light_volume_mv_param_ = pvp.inv_proj;

					re.BindFrameBuffer(pvp.curr_merged_shading_buffer);
					re.Render(*technique_no_lighting_, *rl_quad_);
				}
				std::vector<uint32_t> directional_lights;
				std::vector<uint32_t> point_lights_shadow;
				std::vector<uint32_t> point_lights_no_shadow;
				std::vector<uint32_t> spot_lights_shadow;
				std::vector<uint32_t> spot_lights_no_shadow;
				for (uint32_t li = 0; li < lights_.size(); ++ li)
				{
					LightSourcePtr const & light = lights_[li];
					if (light->Enabled() && pvp.light_visibles[li])
					{
						LightSource::LightType type = light->Type();
						switch (type)
						{
						case LightSource::LT_Ambient:
						case LightSource::LT_Sun:
							{
								this->UpdateLightIndexedLightingAmbientSun(pvp, type, li,
									GetPassCategory(pass_type), index_in_pass, pass_tb);
							}
							break;

						case LightSource::LT_Directional:
							directional_lights.push_back(li);
							break;

						case LightSource::LT_Point:
							if (light->Attrib() & LightSource::LSA_NoShadow)
							{
								point_lights_no_shadow.push_back(li);
							}
							else
							{
								point_lights_shadow.push_back(li);
							}
							break;

						case LightSource::LT_Spot:
							if (light->Attrib() & LightSource::LSA_NoShadow)
							{
								spot_lights_no_shadow.push_back(li);
							}
							else
							{
								spot_lights_shadow.push_back(li);
							}
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
					}
				}

				{
					uint32_t li = 0;
					while (li < directional_lights.size())
					{
						uint32_t nl = m_min(32, (uint32_t)(directional_lights.size() - li));//std::min(static_cast<size_t>(32), directional_lights.size() - li);
						std::vector<uint32_t>::const_iterator iter_beg = directional_lights.begin() + li;
						std::vector<uint32_t>::const_iterator iter_end = iter_beg + nl;
						this->UpdateLightIndexedLightingDirectional(pvp, pass_tb, iter_beg, iter_end);
						li += nl;
					}
				}
				{
					uint32_t li = 0;
					while (li < point_lights_no_shadow.size())
					{
						uint32_t nl = m_min(32, (uint32_t)(point_lights_no_shadow.size() - li));//std::min(static_cast<size_t>(32), point_lights_no_shadow.size() - li);
						std::vector<uint32_t>::iterator iter_beg = point_lights_no_shadow.begin() + li;
						std::vector<uint32_t>::iterator iter_end = iter_beg + nl;
						this->UpdateLightIndexedLightingPointSpot(pvp, iter_beg, iter_end, pass_tb, true, false);
						li += nl;
					}
				}
				{
					uint32_t li = 0;
					while (li < point_lights_shadow.size())
					{
						uint32_t nl = m_min(32, (uint32_t)(point_lights_shadow.size() - li));//std::min(static_cast<size_t>(32), point_lights_shadow.size() - li);
						std::vector<uint32_t>::iterator iter_beg = point_lights_shadow.begin() + li;
						std::vector<uint32_t>::iterator iter_end = iter_beg + nl;
						this->UpdateLightIndexedLightingPointSpot(pvp, iter_beg, iter_end, pass_tb, true, true);
						li += nl;
					}
				}
				{
					uint32_t li = 0;
					while (li < spot_lights_no_shadow.size())
					{
						uint32_t nl = m_min(32, (uint32_t)(spot_lights_no_shadow.size() - li));//std::min(static_cast<size_t>(32), spot_lights_no_shadow.size() - li);
						std::vector<uint32_t>::iterator iter_beg = spot_lights_no_shadow.begin() + li;
						std::vector<uint32_t>::iterator iter_end = iter_beg + nl;
						this->UpdateLightIndexedLightingPointSpot(pvp, iter_beg, iter_end, pass_tb, false, false);
						li += nl;
					}
				}
				{
					uint32_t li = 0;
					while (li < spot_lights_shadow.size())
					{
						uint32_t nl = m_min(32, (uint32_t)(spot_lights_shadow.size() - li));//std::min(static_cast<size_t>(32), spot_lights_shadow.size() - li);
						std::vector<uint32_t>::iterator iter_beg = spot_lights_shadow.begin() + li;
						std::vector<uint32_t>::iterator iter_end = iter_beg + nl;
						this->UpdateLightIndexedLightingPointSpot(pvp, iter_beg, iter_end, pass_tb, false, true);
						li += nl;
					}
				}
#else
				for (uint32_t li = 0; li < lights_.size(); ++ li)
				{
					LightSourcePtr const & light = lights_[li];
					if (light->Enabled() && pvp.light_visibles[li])
					{
						LightSource::LightType type = light->Type();
						int32_t attr = light->Attrib();

						this->PrepareLightCamera(pvp, light, index_in_pass, pass_type);

						*light_attrib_param_ = float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
							attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f,
							attr & LightSource::LSA_NoShadow ? -1.0f : 1.0f, light->ProjectiveTexture() ? 1.0f : -1.0f);
						*light_color_param_ = light->Color();
						*light_falloff_param_ = light->Falloff();

						this->UpdateLighting(pvp, type, li);
					}
				}

				this->UpdateShading(pvp, pass_tb);
#endif

				if (PTB_Opaque == pass_tb)
				{
					this->MergeIndirectLighting(pvp, pass_tb);
					this->MergeSSVO(pvp, pass_tb);
				}
				urv = App3DFramework::URV_Flushed;
			}
			break;

		case PC_SpecialShading:
		default:
			if (0 == index_in_pass)
			{
				if (PTB_Opaque == pass_tb)
				{
					re.BindFrameBuffer(pvp.curr_merged_shading_buffer);
				}
				else
				{
					re.BindFrameBuffer(pvp.shading_buffer);
				}
				urv = App3DFramework::URV_Need_Flush | App3DFramework::URV_Special_Shading_Only
					| (App3DFramework::URV_Opaque_Only << pass_tb);
			}
			else if (1 == index_in_pass)
			{
				this->MergeShadingAndDepth(pvp, pass_tb);
				urv = App3DFramework::URV_Flushed;
			}
			else
			{
				if (has_reflective_objs_ && ssr_enabled_)
				{
					this->AddSSR(pvp);
				}

				if (atmospheric_pp_)
				{
					this->AddAtmospheric(pvp);
				}

				this->AddTAA(pvp);

				std::swap(pvp.curr_merged_shading_buffer, pvp.prev_merged_shading_buffer);
				std::swap(pvp.curr_merged_shading_tex, pvp.prev_merged_shading_tex);
				std::swap(pvp.curr_merged_depth_buffer, pvp.prev_merged_depth_buffer);
				std::swap(pvp.curr_merged_depth_tex, pvp.prev_merged_depth_tex);

				if (has_simple_forward_objs_ && !(pvp.attrib & VPAM_NoSimpleForward))
				{
					typedef KLAYGE_DECLTYPE(visible_scene_objs_) VisibleSceneObjsType;
					KLAYGE_FOREACH(VisibleSceneObjsType::reference deo, visible_scene_objs_)
					{
						deo->Pass(PT_SimpleForward);
					}

					urv = App3DFramework::URV_Need_Flush | App3DFramework::URV_Simple_Forward_Only;
				}
				else
				{
					urv = App3DFramework::URV_Flushed;
				}

				if (pass_scaned_.size() - 1 == pass)
				{
					urv |= App3DFramework::URV_Finished;
				}
			}
			break;
		}

		scene_mgr.SmallObjectThreshold((PC_ShadowMap == pass_cat) ? 0.01f : 0.0f);
		return urv;
	}

	void DeferredRenderingLayer::BuildLightList()
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();

		lights_.clear();
		sm_light_indices_.clear();

		lights_.push_back(MakeSharedPtr<AmbientLightSource>());
		sm_light_indices_.push_back(std::make_pair(-1, 0));

		uint32_t const num_lights = scene_mgr.NumLights();
		uint32_t num_ambient_lights = 0;
		float3 ambient_clr(0, 0, 0);

		projective_light_index_ = -1;
		cascaded_shadow_index_ = -1;
		uint32_t num_sm_lights = 0;
		uint32_t num_sm_2d_lights = 0;
		uint32_t num_sm_cube_lights = 0;
		for (uint32_t i = 0; i < num_lights; ++ i)
		{
			LightSourcePtr const & light = scene_mgr.GetLight(i);
			if (light->Enabled())
			{
				if (LightSource::LT_Ambient == light->Type())
				{
					float4 const & clr = light->Color();
					ambient_clr += float3(clr.x(), clr.y(), clr.z());
					++ num_ambient_lights;
				}
				else
				{
					lights_.push_back(light);

					if (0 == (light->Attrib() & LightSource::LSA_NoShadow))
					{
						switch (light->Type())
						{
						case LightSource::LT_Sun:
							sm_light_indices_.push_back(std::make_pair(0, num_sm_lights));
							++ num_sm_lights;
							cascaded_shadow_index_ = static_cast<int32_t>(i + 1 - num_ambient_lights);
							break;

						case LightSource::LT_Spot:
							if ((projective_light_index_ < 0) && light->ProjectiveTexture())
							{
								projective_light_index_ = static_cast<int32_t>(i + 1 - num_ambient_lights);
								sm_light_indices_.push_back(std::make_pair(0, 4));
							}
							else if ((num_sm_2d_lights < MAX_NUM_SHADOWED_SPOT_LIGHTS)
								&& (num_sm_lights < MAX_NUM_SHADOWED_LIGHTS))
							{
								sm_light_indices_.push_back(std::make_pair(num_sm_2d_lights, num_sm_lights));
								++ num_sm_2d_lights;
								++ num_sm_lights;
							}
							else
							{
								sm_light_indices_.push_back(std::make_pair(-1, 0));
							}
							break;

						case LightSource::LT_Point:
							if ((projective_light_index_ < 0) && light->ProjectiveTexture())
							{
								projective_light_index_ = static_cast<int32_t>(i + 1 - num_ambient_lights);
								sm_light_indices_.push_back(std::make_pair(0, 4));
							}
							else if ((num_sm_cube_lights < MAX_NUM_SHADOWED_POINT_LIGHTS)
								&& (num_sm_lights < MAX_NUM_SHADOWED_LIGHTS))
							{
								sm_light_indices_.push_back(std::make_pair(num_sm_cube_lights, num_sm_lights));
								++ num_sm_cube_lights;
								++ num_sm_lights;
							}
							else
							{
								sm_light_indices_.push_back(std::make_pair(-1, 0));
							}
							break;

						default:
							sm_light_indices_.push_back(std::make_pair(-1, 0));
							break;
						}
					}
					else
					{
						sm_light_indices_.push_back(std::make_pair(-1, 0));
					}
				}
			}
		}
		if (0 == num_ambient_lights)
		{
			ambient_clr = float3(0.1f, 0.1f, 0.1f);
		}
		lights_[0]->Color(ambient_clr);

		indirect_lighting_enabled_ = false;
		if (rsm_buffer_ && (illum_ != 1))
		{
			for (size_t i = 0; i < lights_.size(); ++ i)
			{
				if (lights_[i]->Attrib() & LightSource::LSA_IndirectLighting)
				{
					indirect_lighting_enabled_ = true;
					break;
				}
			}
		}
	}

	void DeferredRenderingLayer::BuildVisibleSceneObjList(bool& has_opaque_objs, bool& has_transparency_back_objs, bool& has_transparency_front_objs)
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();

		has_opaque_objs = false;
		has_reflective_objs_ = false;
		has_simple_forward_objs_ = false;
		visible_scene_objs_.clear();
		uint32_t const num_scene_objs = scene_mgr.NumSceneObjects();
		for (uint32_t i = 0; i < num_scene_objs; ++ i)
		{
			SceneObjectPtr const & so = scene_mgr.GetSceneObject(i);
			if ((0 == (so->Attrib() & SceneObject::SOA_Overlay)) && so->Visible())
			{
				visible_scene_objs_.push_back(so.get());

				has_opaque_objs = true;

				if (so->TransparencyBackFace())
				{
					has_transparency_back_objs = true;
				}
				if (so->TransparencyFrontFace())
				{
					has_transparency_front_objs = true;
				}
				if (so->Reflection())
				{
					has_reflective_objs_ = true;
				}
				if (so->SimpleForward())
				{
					has_simple_forward_objs_ = true;
				}
			}
		}
	}

	void DeferredRenderingLayer::BuildPassScanList(bool has_opaque_objs, bool has_transparency_back_objs, bool has_transparency_front_objs)
	{
		pass_scaned_.clear();
		
		for (uint32_t i = 0; i < lights_.size(); ++ i)
		{
			LightSourcePtr const & light = lights_[i];
			if (light->Enabled())
			{
				this->AppendShadowPassScanCode(i);
			}
		}

		for (uint32_t vpi = 0; vpi < viewports_.size(); ++ vpi)
		{
			PerViewport& pvp = viewports_[vpi];
			if (pvp.attrib & VPAM_Enabled)
			{
				pvp.g_buffer_enables[Opaque_GBuffer] = (pvp.attrib & VPAM_NoOpaque) ? false : has_opaque_objs;
				pvp.g_buffer_enables[TransparencyBack_GBuffer] = (pvp.attrib & VPAM_NoTransparencyBack) ? false : has_transparency_back_objs;
				pvp.g_buffer_enables[TransparencyFront_GBuffer] = (pvp.attrib & VPAM_NoTransparencyFront) ? false : has_transparency_front_objs;

				pvp.light_visibles.resize(lights_.size());
				for (uint32_t li = 0; li < lights_.size(); ++ li)
				{
					LightSourcePtr const & light = lights_[li];
					if (light->Enabled())
					{
						this->CheckLightVisible(vpi, li);
					}
					else
					{
						pvp.light_visibles[li] = false;
					}
				}

				for (uint32_t i = 0; i < Num_GBuffers; ++ i)
				{
					if (pvp.g_buffer_enables[i])
					{
						this->AppendGBufferPassScanCode(vpi, i);
					}
					if (0 == i)
					{
						if (cascaded_shadow_index_ >= 0)
						{
							this->AppendCascadedShadowPassScanCode(vpi, cascaded_shadow_index_);
						}
					}

					if (pvp.g_buffer_enables[i])
					{
						for (uint32_t li = 0; li < lights_.size(); ++ li)
						{
							LightSourcePtr const & light = lights_[li];
							if (light->Enabled() && (0 == (light->Attrib() & LightSource::LSA_NoShadow))
								&& pvp.light_visibles[li])
							{
								this->AppendShadowingPassScanCode(vpi, i, li);
							}
						}

						for (uint32_t li = 0; li < lights_.size(); ++ li)
						{
							LightSourcePtr const & light = lights_[li];
							if (light->Enabled())
							{
								PassTargetBuffer const pass_tb = static_cast<PassTargetBuffer>(i);
								if ((LightSource::LT_Spot == light->Type()) && (PTB_Opaque == pass_tb)
									&& (light->Attrib() & LightSource::LSA_IndirectLighting)
									&& rsm_buffer_ && (illum_ != 1)
									&& pvp.light_visibles[li])
								{
									this->AppendIndirectLightingPassScanCode(vpi, li);
								}
							}
						}

						this->AppendShadingPassScanCode(vpi, i);
					}
				}

				pass_scaned_.push_back(this->ComposePassScanCode(vpi, PT_OpaqueSpecialShading, 0, 2));
			}
		}
	}

	void DeferredRenderingLayer::CheckLightVisible(uint32_t vp_index, uint32_t light_index)
	{
		SceneManager& scene_mgr = Context::Instance().SceneManagerInstance();

		PerViewport& pvp = viewports_[vp_index];
		LightSourcePtr const & light = lights_[light_index];

		float light_scale = std::min(light->Range() * 0.01f, 1.0f) * light_scale_;
		switch (light->Type())
		{
		case LightSource::LT_Spot:
			{
				float4x4 const & inv_light_view = light->SMCamera(0)->InverseViewMatrix();
				float const scale = light->CosOuterInner().w();
				float4x4 mat = MathLib::scaling(scale * light_scale, scale * light_scale, light_scale);
				float4x4 light_model = mat * inv_light_view;
				pvp.light_visibles[light_index] = scene_mgr.OBBVisible(MathLib::transform_obb(cone_obb_, light_model));
			}
			break;

		case LightSource::LT_Point:
			{
				float3 const & p = light->Position();
				float4x4 light_model = MathLib::scaling(light_scale, light_scale, light_scale)
					* MathLib::translation(p);
				pvp.light_visibles[light_index] = scene_mgr.OBBVisible(MathLib::transform_obb(box_obb_, light_model));
			}
			break;

		default:
			pvp.light_visibles[light_index] = true;
			break;
		}
	}

	void DeferredRenderingLayer::AppendGBufferPassScanCode(uint32_t vp_index, uint32_t g_buffer_index)
	{
		PassTargetBuffer ptb = static_cast<PassTargetBuffer>(g_buffer_index);

		if (!depth_texture_support_)
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_None, ptb, PC_Depth), 0, 0));
		}

		if (mrt_g_buffer_support_)
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_MRT, ptb, PC_GBuffer), 0, 0));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_MRT, ptb, PC_GBuffer), 0, 1));
		}
		else
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT0, ptb, PC_GBuffer), 0, 0));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT0, ptb, PC_GBuffer), 0, 1));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT1, ptb, PC_GBuffer), 0, 0));
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
				ComposePassType(PRT_RT1, ptb, PC_GBuffer), 0, 1));
		}
	}

	void DeferredRenderingLayer::AppendShadowPassScanCode(uint32_t light_index)
	{
		PassType shadow_pt;
		if (depth_texture_support_)
		{
			shadow_pt = PT_GenShadowMap;
		}
		else
		{
			shadow_pt = PT_GenShadowMapWODepthTexture;
		}

		LightSourcePtr const & light = lights_[light_index];
		LightSource::LightType type = light->Type();
		int32_t attr = light->Attrib();
		switch (type)
		{
		case LightSource::LT_Spot:
			{
				int sm_seq;
				if (attr & LightSource::LSA_IndirectLighting)
				{
					if (rsm_buffer_ && (illum_ != 1))
					{
						sm_seq = 2;
						shadow_pt = PT_GenReflectiveShadowMap;
					}
					else
					{
						sm_seq = 1;
					}
				}
				else
				{
					if (0 == (attr & LightSource::LSA_NoShadow))
					{
						sm_seq = 1;
					}
					else
					{
						sm_seq = 0;
					}
				}

				if (sm_seq != 0)
				{
					pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, 0));
					pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, 1));
				}
			}
			break;

		case LightSource::LT_Point:
			if (0 == (attr & LightSource::LSA_NoShadow))
			{
				for (int j = 0; j < 7; ++ j)
				{
					pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, j));
				}
			}
			break;

		case LightSource::LT_Sun:
			break;

		default:
			if (0 == (attr & LightSource::LSA_NoShadow))
			{							
				pass_scaned_.push_back(this->ComposePassScanCode(0, shadow_pt, light_index, 0));
			}
			break;
		}
	}

	void DeferredRenderingLayer::AppendCascadedShadowPassScanCode(uint32_t vp_index, uint32_t light_index)
	{
		BOOST_ASSERT(LightSource::LT_Sun == lights_[light_index]->Type());

		PerViewport& pvp = viewports_[vp_index];
		for (uint32_t i = 0; i < pvp.num_cascades + 1; ++ i)
		{
			pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_GenCascadedShadowMap, light_index, i));
		}
	}

	void DeferredRenderingLayer::AppendShadowingPassScanCode(uint32_t vp_index, uint32_t g_buffer_index, uint32_t light_index)
	{
		PassTargetBuffer const pass_tb = static_cast<PassTargetBuffer>(g_buffer_index);
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_Shadowing), light_index, 0));
	}

	void DeferredRenderingLayer::AppendIndirectLightingPassScanCode(uint32_t vp_index, uint32_t light_index)
	{
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index, PT_IndirectLighting, light_index, 0));
	}

	void DeferredRenderingLayer::AppendShadingPassScanCode(uint32_t vp_index, uint32_t g_buffer_index)
	{
		PassTargetBuffer const pass_tb = static_cast<PassTargetBuffer>(g_buffer_index);

		pass_scaned_.push_back(this->ComposePassScanCode(vp_index, 
			ComposePassType(PRT_None, pass_tb, PC_Shading), 0, 0));
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_SpecialShading), 0, 0));
		pass_scaned_.push_back(this->ComposePassScanCode(vp_index,
			ComposePassType(PRT_None, pass_tb, PC_SpecialShading), 0, 1));
	}

	void DeferredRenderingLayer::PreparePVP(PerViewport& pvp)
	{
		CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
		pvp.inv_view = camera->InverseViewMatrix();
		pvp.inv_proj = camera->InverseProjMatrix();
		pvp.proj_to_prev = pvp.inv_proj * pvp.inv_view * pvp.view * pvp.proj;
		pvp.view = camera->ViewMatrix();
		pvp.proj = camera->ProjMatrix();
		pvp.depth_near_far_invfar = float3(camera->NearPlane(), camera->FarPlane(), 1 / camera->FarPlane());
	}

	void DeferredRenderingLayer::GenerateDepthBuffer(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
		pvp.pre_depth_buffer->GetViewport()->camera = camera;

		re.BindFrameBuffer(pvp.pre_depth_buffer);

		float depth = (TransparencyBack_GBuffer == g_buffer_index) ? 0.0f : 1.0f;
		int32_t stencil = (Opaque_GBuffer == g_buffer_index) ? 0 : 128;
		pvp.g_buffer->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
			Color(0, 0, 0, 0), depth, stencil);
	}

	void DeferredRenderingLayer::GenerateGBuffer(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (Opaque_GBuffer == g_buffer_index)
		{
			CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
			pvp.g_buffer->GetViewport()->camera = camera;
			if (!mrt_g_buffer_support_)
			{
				pvp.g_buffer_rt1->GetViewport()->camera = camera;
			}
			pvp.lighting_buffer->GetViewport()->camera = camera;
			pvp.shading_buffer->GetViewport()->camera = camera;
			pvp.curr_merged_shading_buffer->GetViewport()->camera = camera;
			pvp.curr_merged_depth_buffer->GetViewport()->camera = camera;
			pvp.prev_merged_shading_buffer->GetViewport()->camera = camera;
			pvp.prev_merged_depth_buffer->GetViewport()->camera = camera;
		}

		re.BindFrameBuffer(pvp.g_buffer);

		float depth = (TransparencyBack_GBuffer == g_buffer_index) ? 0.0f : 1.0f;
		int32_t stencil = (Opaque_GBuffer == g_buffer_index) ? 0 : 128;
		pvp.g_buffer->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil,
			Color(0, 0, 0, 0), depth, stencil);
	}

	void DeferredRenderingLayer::PostGenerateGBuffer(PerViewport const & pvp)
	{
		pvp.g_buffer_rt0_tex->BuildMipSubLevels();

		if (depth_texture_support_)
		{
			depth_to_linear_pp_->InputPin(0, pvp.g_buffer_ds_tex);
			depth_to_linear_pp_->OutputPin(0, pvp.g_buffer_depth_tex);
			depth_to_linear_pp_->Apply();
		}

		pvp.g_buffer_depth_tex->BuildMipSubLevels();

#ifdef LIGHT_INDEXED_DEFERRED
		this->CreateDepthMinMaxMap(pvp);
#endif
	}

	void DeferredRenderingLayer::RenderDecals(PerViewport const & pvp, PassType pass_type)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		uint32_t const width = pvp.g_buffer_rt0_tex->Width(0);
		uint32_t const height = pvp.g_buffer_rt0_tex->Height(0);
		pvp.g_buffer_rt0_tex->CopyToSubTexture2D(*pvp.g_buffer_rt0_backup_tex, 0, 0,
			0, 0, width, height, 0, 0, 0, 0, width, height);

		re.BindFrameBuffer((PT_OpaqueGBufferRT1 == pass_type) ? pvp.g_buffer_rt1
			: pvp.g_buffer);
		typedef KLAYGE_DECLTYPE(decals_) DecalsType;
		KLAYGE_FOREACH(DecalsType::reference de, decals_)
		{
			de->Pass(pass_type);
			de->Render();
		}
	}

	void DeferredRenderingLayer::PrepareLightCamera(PerViewport const & pvp,
		LightSourcePtr const & light, int32_t index_in_pass, PassType pass_type)
	{
		LightSource::LightType const type = light->Type();
		PassCategory const pass_cat = GetPassCategory(pass_type);

		switch (type)
		{
		case LightSource::LT_Point:
		case LightSource::LT_Spot:
		case LightSource::LT_Sun:
			{
				CameraPtr sm_camera;
				float3 dir_es(0, 0, 0);
				if (LightSource::LT_Spot == type)
				{
					dir_es = MathLib::transform_normal(light->Direction(), pvp.view);
					sm_camera = light->SMCamera(0);
				}
				else if (LightSource::LT_Sun == type)
				{
					dir_es = MathLib::transform_normal(-light->Direction(), pvp.view);
					sm_camera = light->SMCamera(0);
				}
				else
				{
					int32_t face = std::min(index_in_pass, 5);
					std::pair<float3, float3> ad = CubeMapViewVector<float>(static_cast<Texture::CubeFaces>(face));
					dir_es = MathLib::transform_normal(MathLib::transform_quat(ad.first, light->Rotation()), pvp.view);
					sm_camera = light->SMCamera(face);
				}
				float4 light_dir_es_actived = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

				sm_buffer_->GetViewport()->camera = sm_camera;

				if ((LightSource::LT_Sun == type) && (pass_cat != PC_Shadowing) && (pass_cat != PC_Shading))
				{
					curr_cascade_index_ = index_in_pass;
				}
				else
				{
					curr_cascade_index_ = -1;
				}

				*light_view_proj_param_ = pvp.inv_view * sm_camera->ViewProjMatrix();

				float4x4 light_to_view = sm_camera->InverseViewMatrix() * pvp.view;
				float4x4 light_to_proj = light_to_view * pvp.proj;

				if (depth_texture_support_ && (index_in_pass > 0)
					&& ((PT_GenShadowMap == pass_type) || (PT_GenReflectiveShadowMap == pass_type)))
				{
					float q = sm_camera->FarPlane() / (sm_camera->FarPlane() - sm_camera->NearPlane());

					float2 near_q(sm_camera->NearPlane() * q, q);
					depth_to_esm_pp_->SetParam(0, near_q);

					float4x4 inv_sm_proj = sm_camera->InverseProjMatrix();
					depth_to_esm_pp_->SetParam(1, inv_sm_proj);
				}

				float3 const & p = light->Position();
				float3 loc_es = MathLib::transform_coord(p, pvp.view);
				float4 light_pos_es_actived = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

				float light_scale = std::min(light->Range() * 0.01f, 1.0f) * light_scale_;
				switch (type)
				{
				case LightSource::LT_Spot:
					{
						light_pos_es_actived.w() = light->CosOuterInner().x();
						light_dir_es_actived.w() = light->CosOuterInner().y();

						float const scale = light->CosOuterInner().w();
						float4x4 light_model = MathLib::scaling(scale * light_scale, scale * light_scale, light_scale);
						*light_volume_mv_param_ = light_model * light_to_view;
						*light_volume_mvp_param_ = light_model * light_to_proj;
					}
					break;

				case LightSource::LT_Point:
					if ((PC_Shadowing == pass_cat) || (PC_Shading == pass_cat))
					{
						float4x4 light_model = MathLib::scaling(light_scale, light_scale, light_scale)
							* MathLib::to_matrix(light->Rotation()) * MathLib::translation(p);
						*light_volume_mv_param_ = light_model * pvp.view;
						*light_volume_mvp_param_ = light_model * pvp.view * pvp.proj;
						*view_to_light_model_param_ = pvp.inv_view * MathLib::inverse(light_model);
					}
					else
					{
						*light_volume_mv_param_ = light_to_view;
						*light_volume_mvp_param_ = light_to_proj;
					}
					break;

				case LightSource::LT_Sun:
					*light_volume_mv_param_ = pvp.inv_proj;
					*light_volume_mvp_param_ = float4x4::Identity();
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
					
				*light_pos_es_param_ = light_pos_es_actived;
				*light_dir_es_param_ = light_dir_es_actived;
			}
			break;

		case LightSource::LT_Directional:
			{
				float3 dir_es = MathLib::transform_normal(light->Direction(), pvp.view);
				*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);
			}
			*light_volume_mv_param_ = pvp.inv_proj;
			*light_volume_mvp_param_ = float4x4::Identity();
			break;

		case LightSource::LT_Ambient:
			{
				float3 dir_es = MathLib::transform_normal(float3(0, 1, 0), pvp.view);
				*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);
			}
			*light_volume_mv_param_ = pvp.inv_proj;
			*light_volume_mvp_param_ = float4x4::Identity();
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void DeferredRenderingLayer::PostGenerateShadowMap(PerViewport const & pvp, int32_t org_no, int32_t index_in_pass)
	{
		LightSource::LightType type = lights_[org_no]->Type();

		if (depth_texture_support_)
		{
			if (type != LightSource::LT_Sun)
			{
				depth_to_esm_pp_->Apply();
			}
		}

		int2 kernel_size;
		if (LightSource::LT_Sun == type)
		{
			float3 const & scale = cascaded_shadow_layer_->CascadeScales()[index_in_pass - 1];
			float2 blur_kernel_size = blur_size_light_space_ * float2(scale.x(), scale.y()) * static_cast<float>(cascaded_sm_tex_->Width(0));
			kernel_size.x() = MathLib::clamp(static_cast<int32_t>(blur_kernel_size.x() + 0.5f), 1, 4);
			kernel_size.y() = MathLib::clamp(static_cast<int32_t>(blur_kernel_size.y() + 0.5f), 1, 4);
		}
		else
		{
			kernel_size.x() = 4;
			kernel_size.y() = 4;
		}
		PostProcessChainPtr pp_chain = checked_pointer_cast<PostProcessChain>(sm_filter_pp_);
		checked_pointer_cast<SeparableLogGaussianFilterPostProcess>(pp_chain->GetPostProcess(0))->KernelRadius(kernel_size.x());
		checked_pointer_cast<SeparableLogGaussianFilterPostProcess>(pp_chain->GetPostProcess(1))->KernelRadius(kernel_size.y());

		if (LightSource::LT_Sun == type)
		{
			sm_filter_pp_->InputPin(0, cascaded_sm_tex_);
			if (tex_array_support_)
			{
				sm_filter_pp_->OutputPin(0, pvp.blur_cascaded_sm_texs[0], 0, index_in_pass - 1, 0);
			}
			else
			{
				sm_filter_pp_->OutputPin(0, pvp.blur_cascaded_sm_texs[index_in_pass - 1]);
			}
		}
		else
		{
			sm_filter_pp_->InputPin(0, sm_tex_);
			if (LightSource::LT_Point == type)
			{
				sm_filter_pp_->OutputPin(0, blur_sm_cube_texs_[sm_light_indices_[org_no].first], 0, 0, index_in_pass - 1);
			}
			else 
			{
				sm_filter_pp_->OutputPin(0, blur_sm_2d_texs_[sm_light_indices_[org_no].first]);
			}
		}
		checked_pointer_cast<LogGaussianBlurPostProcess>(sm_filter_pp_)->ESMScaleFactor(ESM_SCALE_FACTOR,
			sm_buffer_->GetViewport()->camera);
		sm_filter_pp_->Apply();

		if (LightSource::LT_Sun == type)
		{
			if (tex_array_support_)
			{
				if (static_cast<int32_t>(pvp.num_cascades) == index_in_pass)
				{
					pvp.blur_cascaded_sm_texs[0]->BuildMipSubLevels();
				}
			}
			else
			{
				pvp.blur_cascaded_sm_texs[index_in_pass - 1]->BuildMipSubLevels();
			}
		}
	}

	void DeferredRenderingLayer::UpdateShadowing(PerViewport const & pvp, int32_t org_no)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		
		LightSourcePtr const & light = lights_[org_no];
		CameraPtr sm_camera;
		LightSource::LightType type = light->Type();

		BOOST_ASSERT(0 == (light->Attrib() & LightSource::LSA_NoShadow));

		int32_t light_index = sm_light_indices_[org_no].first;
		int32_t shadowing_channel = sm_light_indices_[org_no].second;
		if (((light_index >= 0) && (0 == (light->Attrib() & LightSource::LSA_NoShadow)))
			|| (LightSource::LT_Sun == type))
		{
			switch (type)
			{
			case LightSource::LT_Spot:
				sm_camera = light->SMCamera(0);
				*shadow_map_2d_tex_param_ = blur_sm_2d_texs_[light_index];
				break;

			case LightSource::LT_Point:
				sm_camera = light->SMCamera(0);
				*shadow_map_cube_tex_param_ = blur_sm_cube_texs_[light_index];
				break;

			case LightSource::LT_Sun:
				{
					sm_camera = lights_[cascaded_shadow_index_]->SMCamera(0);

					*light_view_proj_param_ = pvp.inv_view * sm_camera->ViewProjMatrix();

					std::vector<float4> cascade_scale_bias(pvp.num_cascades);
					for (uint32_t i = 0; i < pvp.num_cascades; ++ i)
					{
						float3 const & scale = cascaded_shadow_layer_->CascadeScales()[i];
						float3 const & bias = cascaded_shadow_layer_->CascadeBiases()[i];
						cascade_scale_bias[i] = float4(scale.x(), scale.y(),
							bias.x(), bias.y());
					}
					*cascade_intervals_param_ = cascaded_shadow_layer_->CascadeIntervals();
					*cascade_scale_bias_param_ = cascade_scale_bias;
					*num_cascades_param_ = static_cast<int32_t>(pvp.num_cascades);

					float4x4 light_view = pvp.inv_view * sm_camera->ViewMatrix();
					*view_z_to_light_view_param_ = light_view.Col(2);
				}
				if (tex_array_support_)
				{
					*cascaded_shadow_map_tex_array_param_ = pvp.blur_cascaded_sm_texs[0];
				}
				else
				{
					for (uint32_t i = 0; i < pvp.num_cascades; ++ i)
					{
						*cascaded_shadow_map_texs_param_[i] = pvp.blur_cascaded_sm_texs[i];
					}
				}
				break;

			default:
				break;
			}
		}

		if (org_no == projective_light_index_)
		{
			re.BindFrameBuffer(pvp.projective_shadowing_buffer);
			pvp.projective_shadowing_buffer->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(1, 1, 1, 1));
		}
		else
		{
			re.BindFrameBuffer(pvp.shadowing_buffer);
			if (shadowing_channel <= 0)
			{
				pvp.shadowing_buffer->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(1, 1, 1, 1));
			}
		}

		if (sm_camera)
		{
			*esm_scale_factor_param_ = ESM_SCALE_FACTOR / (sm_camera->FarPlane() - sm_camera->NearPlane());
		}

		re.Render(*technique_shadows_[type][shadowing_channel], *light_volume_rl_[type]);
	}

	void DeferredRenderingLayer::UpdateLighting(PerViewport const & pvp, LightSource::LightType type, int32_t org_no)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		LightSourcePtr const & light = lights_[org_no];
		int32_t shadowing_channel;
		if (0 == (light->Attrib() & LightSource::LSA_NoShadow))
		{
			shadowing_channel = sm_light_indices_[org_no].second;
		}
		else
		{
			shadowing_channel = -1;
		}
		*shadowing_channel_param_ = shadowing_channel;

		re.BindFrameBuffer(pvp.lighting_buffer);

		RenderLayoutPtr const & rl = light_volume_rl_[type];

		if ((LightSource::LT_Point == type) || (LightSource::LT_Spot == type))
		{
			re.Render(*technique_light_stencil_, *rl);
		}

		re.Render(*technique_lights_[type], *rl);
	}

	void DeferredRenderingLayer::MergeIndirectLighting(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		if ((indirect_lighting_enabled_ && !(pvp.attrib & VPAM_NoGI)) && (illum_ != 1))
		{
			pvp.il_layer->CalcIndirectLighting(pvp.prev_merged_shading_tex, pvp.proj_to_prev);
			this->AccumulateToLightingTex(pvp, g_buffer_index);
		}
	}

	void DeferredRenderingLayer::UpdateShading(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		*lighting_tex_param_ = pvp.lighting_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		if (Opaque_GBuffer == g_buffer_index)
		{
			re.BindFrameBuffer(pvp.curr_merged_shading_buffer);
			re.Render(*technique_no_lighting_, *rl_quad_);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_buffer);
			re.CurFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->Discard();
		}
		re.Render(*technique_shading_, *rl_quad_);
	}

	void DeferredRenderingLayer::MergeSSVO(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		if (pvp.ssvo_enabled && !(pvp.attrib & VPAM_NoSSVO))
		{
			ssvo_pp_->InputPin(0, pvp.g_buffer_rt0_tex);
			ssvo_pp_->InputPin(1, pvp.g_buffer_depth_tex);
			ssvo_pp_->OutputPin(0, pvp.small_ssvo_tex);
			ssvo_pp_->Apply();

			ssvo_blur_pp_->InputPin(0, pvp.small_ssvo_tex);
			ssvo_blur_pp_->InputPin(1, pvp.g_buffer_depth_tex);
			if (Opaque_GBuffer == g_buffer_index)
			{
				ssvo_blur_pp_->OutputPin(0, pvp.curr_merged_shading_tex);
			}
			else
			{
				ssvo_blur_pp_->OutputPin(0, pvp.shading_tex);
			}
			ssvo_blur_pp_->Apply();
		}
	}

	void DeferredRenderingLayer::AddSSR(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.curr_merged_shading_buffer);
		ssr_pp_->InputPin(0, pvp.g_buffer_rt0_tex);
		ssr_pp_->InputPin(1, pvp.g_buffer_rt1_tex);
		ssr_pp_->InputPin(2, pvp.g_buffer_depth_tex);
		ssr_pp_->InputPin(3, pvp.prev_merged_shading_tex);
		ssr_pp_->InputPin(4, pvp.curr_merged_depth_tex);
		ssr_pp_->Apply();
	}

	void DeferredRenderingLayer::AddAtmospheric(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.curr_merged_shading_buffer);
		atmospheric_pp_->SetParam(0, pvp.inv_proj);
		atmospheric_pp_->InputPin(0, pvp.curr_merged_depth_tex);
		atmospheric_pp_->Render();
	}

	void DeferredRenderingLayer::AddTAA(PerViewport const & pvp)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.frame_buffer);
		pvp.frame_buffer->Discard(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth | FrameBuffer::CBM_Stencil);
		{
			*depth_tex_param_ = pvp.curr_merged_depth_tex;

			CameraPtr const & camera = pvp.frame_buffer->GetViewport()->camera;
			float q = camera->FarPlane() / (camera->FarPlane() - camera->NearPlane());
			float2 near_q(camera->NearPlane() * q, q);
			*near_q_param_ = near_q;
		}
		App3DFramework& app = Context::Instance().AppInstance();
		if ((app.FrameTime() < 1.0f / 30) && taa_enabled_)
		{
			taa_pp_->InputPin(0, pvp.curr_merged_shading_tex);
			taa_pp_->InputPin(1, pvp.prev_merged_shading_tex);
			taa_pp_->Render();
			re.Render(*technique_copy_depth_, *rl_quad_);
		}
		else
		{
			*shading_tex_param_ = pvp.curr_merged_shading_tex;
			re.Render(*technique_copy_shading_depth_, *rl_quad_);
		}
	}

	void DeferredRenderingLayer::MergeShadingAndDepth(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		if (g_buffer_index != 0)
		{
			re.BindFrameBuffer(pvp.curr_merged_shading_buffer);
			*shading_tex_param_ = pvp.shading_tex;
			re.Render(*technique_merge_shadings_[g_buffer_index != 0], *rl_quad_);
		}

		re.BindFrameBuffer(pvp.curr_merged_depth_buffer);
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		re.Render(*technique_merge_depths_[g_buffer_index != 0], *rl_quad_);
	}

	void DeferredRenderingLayer::SetupViewportGI(uint32_t vp, bool ssgi_enable)
	{
		PerViewport& pvp = viewports_[vp];
		if (ssgi_enable)
		{
			pvp.il_layer = MakeSharedPtr<SSGILayer>();
		}
		else
		{
			if (rsm_buffer_)
			{
				pvp.il_layer = MakeSharedPtr<MultiResSILLayer>();
			}
			else
			{
				pvp.il_layer.reset();
			}
		}

		if (pvp.il_layer && pvp.g_buffer_rt0_tex && rsm_texs_[0])
		{
			pvp.il_layer->GBuffer(pvp.g_buffer_rt0_tex, pvp.g_buffer_rt1_tex,
				pvp.g_buffer_depth_tex);
			pvp.il_layer->RSM(rsm_texs_[0], rsm_texs_[1], sm_tex_);
		}
	}

	void DeferredRenderingLayer::SetCascadedShadowType(CascadedShadowLayerType type)
	{
		switch (type)
		{
		case CSLT_Auto:
			{
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				RenderDeviceCaps const & caps = re.DeviceCaps();
				if (caps.cs_support && (caps.max_shader_model >= 5))
				{
					cascaded_shadow_layer_ = MakeSharedPtr<SDSMCascadedShadowLayer>();
				}
				else
				{
					cascaded_shadow_layer_ = MakeSharedPtr<PSSMCascadedShadowLayer>();
				}
			}
			break;

		case CSLT_PSSM:
			cascaded_shadow_layer_ = MakeSharedPtr<PSSMCascadedShadowLayer>();
			break;

		case CSLT_SDSM:
		default:
			cascaded_shadow_layer_ = MakeSharedPtr<SDSMCascadedShadowLayer>();
			break;
		}
	}

	void DeferredRenderingLayer::SetViewportCascades(uint32_t vp, uint32_t num_cascades, float pssm_lambda)
	{
		PerViewport& pvp = viewports_[vp];
		pvp.num_cascades = num_cascades;
		if (CSLT_PSSM == cascaded_shadow_layer_->Type())
		{
			checked_pointer_cast<PSSMCascadedShadowLayer>(cascaded_shadow_layer_)->Lambda(pssm_lambda);
		}
	}

	void DeferredRenderingLayer::AccumulateToLightingTex(PerViewport const & pvp, uint32_t g_buffer_index)
	{
		PostProcessPtr const & copy_to_light_buffer_pp = (0 == illum_) ? copy_to_light_buffer_pp_ : copy_to_light_buffer_i_pp_;
		copy_to_light_buffer_pp->SetParam(0, indirect_scale_ * 256 / VPL_COUNT);
		copy_to_light_buffer_pp->SetParam(1, float2(1.0f / pvp.g_buffer_rt0_tex->Width(0), 1.0f / pvp.g_buffer_rt0_tex->Height(0)));
		copy_to_light_buffer_pp->SetParam(2, pvp.depth_near_far_invfar);
		copy_to_light_buffer_pp->SetParam(3, pvp.inv_proj);
		copy_to_light_buffer_pp->InputPin(0, pvp.il_layer->IndirectLightingTex());
		copy_to_light_buffer_pp->InputPin(1, pvp.g_buffer_rt0_tex);
		copy_to_light_buffer_pp->InputPin(2, pvp.g_buffer_rt1_tex);
		copy_to_light_buffer_pp->InputPin(3, pvp.g_buffer_depth_tex);
		if (Opaque_GBuffer == g_buffer_index)
		{
			copy_to_light_buffer_pp->OutputPin(0, pvp.curr_merged_shading_tex);
		}
		else
		{
			copy_to_light_buffer_pp->OutputPin(0, pvp.shading_tex);
		}
		copy_to_light_buffer_pp->Apply();
	}

	void DeferredRenderingLayer::DisplayIllum(int illum)
	{
		illum_ = illum;
	}

	void DeferredRenderingLayer::IndirectScale(float scale)
	{
		indirect_scale_ = scale;
	}

	uint32_t DeferredRenderingLayer::ComposePassScanCode(uint32_t vp_index, PassType pass_type, int32_t org_no, int32_t index_in_pass)
	{
		return (vp_index << 28) | (pass_type << 18) | (org_no << 6) | index_in_pass;
	}

	void DeferredRenderingLayer::DecomposePassScanCode(uint32_t& vp_index, PassType& pass_type, int32_t& org_no, int32_t& index_in_pass, uint32_t code)
	{
		vp_index = code >> 28;				//  4 bits, [31 - 28]
		pass_type = static_cast<PassType>((code >> 18) & 0x03FF);	//  10 bits, [27 - 18]
		org_no = (code >> 6) & 0x0FFF;		// 12 bits, [17 - 6]
		index_in_pass = (code >> 0) & 0x3F;		//  6 bits, [5 -  0]
	}

#ifdef LIGHT_INDEXED_DEFERRED
	void DeferredRenderingLayer::UpdateLightIndexedLightingAmbientSun(PerViewport const & pvp, LightSource::LightType type,
		int32_t org_no, PassCategory pass_cat, int32_t index_in_pass, uint32_t g_buffer_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		LightSourcePtr const & light = lights_[org_no];
		int32_t shadowing_channel;
		if (0 == (light->Attrib() & LightSource::LSA_NoShadow))
		{
			shadowing_channel = sm_light_indices_[org_no].second;
		}
		else
		{
			shadowing_channel = -1;
		}
		*shadowing_channel_param_ = shadowing_channel;

		int32_t attr = light->Attrib();
		*light_attrib_param_ = float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
			attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f,
			attr & LightSource::LSA_NoShadow ? -1.0f : 1.0f, light->ProjectiveTexture() ? 1.0f : -1.0f);
		*light_color_param_ = light->Color();
		*light_falloff_param_ = light->Falloff();

		RenderTechniquePtr tech;
		if (LightSource::LT_Sun == type)
		{
			float3 dir_es = MathLib::transform_normal(-light->Direction(), pvp.view);
			CameraPtr sm_camera = light->SMCamera(0);
			*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

			sm_buffer_->GetViewport()->camera = sm_camera;

			if ((pass_cat != PC_Shadowing) && (pass_cat != PC_Shading))
			{
				curr_cascade_index_ = index_in_pass;
			}
			else
			{
				curr_cascade_index_ = -1;
			}

			*light_view_proj_param_ = pvp.inv_view * sm_camera->ViewProjMatrix();

			float3 const & p = light->Position();
			float3 loc_es = MathLib::transform_coord(p, pvp.view);
			*light_pos_es_param_ = float4(loc_es.x(), loc_es.y(), loc_es.z(), 1);

			tech = technique_light_indexed_deferred_rendering_sun_;
		}
		else
		{
			BOOST_ASSERT(LightSource::LT_Ambient == type);

			float3 dir_es = MathLib::transform_normal(float3(0, 1, 0), pvp.view);
			*light_dir_es_param_ = float4(dir_es.x(), dir_es.y(), dir_es.z(), 0);

			tech = technique_light_indexed_deferred_rendering_ambient_;
		}

		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		if (Opaque_GBuffer == g_buffer_index)
		{
			re.BindFrameBuffer(pvp.curr_merged_shading_buffer);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_buffer);
		}
		re.Render(*tech, *rl_quad_);
	}

	void DeferredRenderingLayer::UpdateLightIndexedLightingDirectional(PerViewport const & pvp,
		uint32_t g_buffer_index, std::vector<uint32_t>::const_iterator iter_beg, std::vector<uint32_t>::const_iterator iter_end)
	{
		std::vector<float4> lights_color;
		std::vector<float4> lights_dir_es;
		std::vector<float4> lights_attrib;
		for (std::vector<uint32_t>::const_iterator iter = iter_beg; iter != iter_end; ++ iter)
		{
			LightSourcePtr const & light = lights_[*iter];
			BOOST_ASSERT(LightSource::LT_Directional == light->Type());
			int32_t attr = light->Attrib();

			lights_color.push_back(light->Color());

			float3 dir_es = MathLib::transform_normal(light->Direction(), pvp.view);
			lights_dir_es.push_back(float4(dir_es.x(), dir_es.y(), dir_es.z(), 0));

			lights_attrib.push_back(float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
				attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f,
				attr & LightSource::LSA_NoShadow ? -1.0f : 1.0f, light->ProjectiveTexture() ? 1.0f : -1.0f));
		}

		*lights_color_param_ = lights_color;
		*lights_dir_es_param_ = lights_dir_es;
		*lights_attrib_param_ = lights_attrib;
		*num_lights_param_ = static_cast<int32_t>(iter_end - iter_beg);

		*light_index_tex_param_ = pvp.light_index_tex;
		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (Opaque_GBuffer == g_buffer_index)
		{
			re.BindFrameBuffer(pvp.curr_merged_shading_buffer);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_buffer);
		}
		re.Render(*technique_light_indexed_deferred_rendering_directional_, *rl_quad_);
	}

	void DeferredRenderingLayer::UpdateLightIndexedLightingPointSpot(PerViewport const & pvp,
		std::vector<uint32_t>::const_iterator iter_beg, std::vector<uint32_t>::const_iterator iter_end,
		uint32_t g_buffer_index, bool is_point, bool with_shadow)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		re.BindFrameBuffer(pvp.light_index_buffer);
		pvp.light_index_buffer->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));

		*min_max_depth_tex_param_ = pvp.g_buffer_min_max_depth_texs.back();

		uint32_t w = pvp.light_index_tex->Width(0);
		uint32_t h = pvp.light_index_tex->Height(0);
		*light_index_tex_width_height_param_ = float4(static_cast<float>(w),
			static_cast<float>(h), 1.0f / w, 1.0f / h);

		w = (pvp.g_buffer_depth_tex->Width(0) + TILE_SIZE - 1) & ~(TILE_SIZE - 1);
		h = (pvp.g_buffer_depth_tex->Height(0) + TILE_SIZE - 1) & ~(TILE_SIZE - 1);
		*tile_scale_param_ = float2(w / (2.0f * TILE_SIZE), h / (2.0f * TILE_SIZE));

		*camera_proj_param_ = pvp.proj;

		std::vector<float4> lights_color;
		std::vector<float4> lights_pos_es;
		std::vector<float4> lights_dir_es;
		std::vector<float4> lights_falloff_range;
		std::vector<float4> lights_attrib;
		int4 lights_shadowing_channel(-1, -1, -1, -1);
		std::vector<float3> lights_aabb_min;
		std::vector<float3> lights_aabb_max;
		for (std::vector<uint32_t>::const_iterator iter = iter_beg; iter != iter_end; ++ iter)
		{
			LightSourcePtr const & light = lights_[*iter];
			LightSource::LightType type = light->Type();
			int32_t attr = light->Attrib();

			BOOST_ASSERT((LightSource::LT_Point == type) || (LightSource::LT_Spot == type));

			lights_color.push_back(light->Color());

			float3 const & p = light->Position();
			float3 loc_es = MathLib::transform_coord(p, pvp.view);
			lights_pos_es.push_back(float4(loc_es.x(), loc_es.y(), loc_es.z(), 1));

			float3 dir_es(0, 0, 0);
			if (LightSource::LT_Spot == type)
			{
				dir_es = MathLib::transform_normal(light->Direction(), pvp.view);
			}
			lights_dir_es.push_back(float4(dir_es.x(), dir_es.y(), dir_es.z(), 0));

			if (LightSource::LT_Spot == type)
			{
				lights_pos_es.back().w() = light->CosOuterInner().x();
				lights_dir_es.back().w() = light->CosOuterInner().y();
			}

			lights_attrib.push_back(float4(attr & LightSource::LSA_NoDiffuse ? 0.0f : 1.0f,
				attr & LightSource::LSA_NoSpecular ? 0.0f : 1.0f,
				attr & LightSource::LSA_NoShadow ? -1.0f : 1.0f, light->ProjectiveTexture() ? 1.0f : -1.0f));

			if (with_shadow)
			{
				BOOST_ASSERT(0 == (light->Attrib() & LightSource::LSA_NoShadow));
				BOOST_ASSERT(iter - iter_beg < 4);
				lights_shadowing_channel[iter - iter_beg] = sm_light_indices_[*iter].second;
			}

			float range = light->Range() * light_scale_;
			AABBox aabb(float3(0, 0, 0), float3(0, 0, 0));
			if (LightSource::LT_Spot == type)
			{
				float4x4 light_to_view = light->SMCamera(0)->InverseViewMatrix() * pvp.view;
				float const scale = light->CosOuterInner().w();
				float4x4 light_model = MathLib::scaling(range * 0.01f * float3(scale, scale, 1));
				float4x4 light_mv = light_model * light_to_view;
				aabb = MathLib::convert_to_aabbox(MathLib::transform_obb(cone_obb_, light_mv));
			}
			lights_falloff_range.push_back(float4(light->Falloff().x(), light->Falloff().y(),
				light->Falloff().z(), range));
			lights_aabb_min.push_back(aabb.Min());
			lights_aabb_max.push_back(aabb.Max());
		}

		*lights_color_param_ = lights_color;
		*lights_pos_es_param_ = lights_pos_es;
		*lights_dir_es_param_ = lights_dir_es;
		*lights_falloff_range_param_ = lights_falloff_range;
		*lights_attrib_param_ = lights_attrib;
		*lights_shadowing_channel_param_ = lights_shadowing_channel;
		*lights_aabb_min_param_ = lights_aabb_min;
		*lights_aabb_max_param_ = lights_aabb_max;
		*num_lights_param_ = static_cast<int32_t>(iter_end - iter_beg);

		RenderTechniquePtr tech;
		if (is_point)
		{
			tech = technique_draw_light_index_point_;
		}
		else
		{
			tech = technique_draw_light_index_spot_;
		}
		re.Render(*tech, *rl_quad_);

		w = pvp.g_buffer_depth_tex->Width(0);
		h = pvp.g_buffer_depth_tex->Height(0);
		*tc_to_tile_scale_param_ = float2(static_cast<float>(w) / ((w + TILE_SIZE - 1) & ~(TILE_SIZE - 1)),
			static_cast<float>(h) / ((h + TILE_SIZE - 1) & ~(TILE_SIZE - 1)));
		*light_index_tex_param_ = pvp.light_index_tex;
		*g_buffer_tex_param_ = pvp.g_buffer_rt0_tex;
		*g_buffer_1_tex_param_ = pvp.g_buffer_rt1_tex;
		*depth_tex_param_ = pvp.g_buffer_depth_tex;
		*light_volume_mv_param_ = pvp.inv_proj;

		if (Opaque_GBuffer == g_buffer_index)
		{
			re.BindFrameBuffer(pvp.curr_merged_shading_buffer);
		}
		else
		{
			re.BindFrameBuffer(pvp.shading_buffer);
		}
		if (is_point)
		{
			if (with_shadow)
			{
				tech = technique_light_indexed_deferred_rendering_point_shadow_;
			}
			else
			{
				tech = technique_light_indexed_deferred_rendering_point_no_shadow_;
			}
		}
		else
		{
			if (with_shadow)
			{
				tech = technique_light_indexed_deferred_rendering_spot_shadow_;
			}
			else
			{
				tech = technique_light_indexed_deferred_rendering_spot_no_shadow_;
			}
		}
		re.Render(*tech, *rl_quad_);
	}

	void DeferredRenderingLayer::CreateDepthMinMaxMap(PerViewport const & pvp)
	{
		uint32_t w = pvp.g_buffer_depth_tex->Width(0);
		uint32_t h = pvp.g_buffer_depth_tex->Height(0);
		depth_to_min_max_pp_->SetParam(0, float2(0.5f / w, 0.5f / h));
		depth_to_min_max_pp_->SetParam(1, float2(static_cast<float>((w + 1) & ~1) / w,
			static_cast<float>((h + 1) & ~1) / h));
		depth_to_min_max_pp_->InputPin(0, pvp.g_buffer_depth_tex);
		depth_to_min_max_pp_->OutputPin(0, pvp.g_buffer_min_max_depth_texs[0]);
		depth_to_min_max_pp_->Apply();

		for (uint32_t i = 1; i < pvp.g_buffer_min_max_depth_texs.size(); ++ i)
		{
			w = pvp.g_buffer_min_max_depth_texs[i - 1]->Width(0);
			h = pvp.g_buffer_min_max_depth_texs[i - 1]->Height(0);
			reduce_min_max_pp_->SetParam(0, float2(0.5f / w, 0.5f / h));
			reduce_min_max_pp_->SetParam(1, float2(static_cast<float>((w + 1) & ~1) / w,
				static_cast<float>((h + 1) & ~1) / h));
			reduce_min_max_pp_->InputPin(0, pvp.g_buffer_min_max_depth_texs[i - 1]);
			reduce_min_max_pp_->OutputPin(0, pvp.g_buffer_min_max_depth_texs[i]);
			reduce_min_max_pp_->Apply();
		}
	}
#endif
}
