/**
 * @file SkyBox.cpp
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
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/SkyBox.hpp>

namespace KlayGE
{
	RenderableSkyBox::RenderableSkyBox()
		: RenderableHelper(L"SkyBox")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEffectPtr effect = SyncLoadRenderEffect("SkyBox.fxml");
		if (deferred_effect_)
		{
			this->BindDeferredEffect(effect);
			depth_tech_ = effect->TechniqueByName("DepthSkyBoxTech");
			gbuffer_rt0_tech_ = effect->TechniqueByName("GBufferSkyBoxRT0Tech");
			gbuffer_rt1_tech_ = effect->TechniqueByName("GBufferSkyBoxRT1Tech");
			gbuffer_mrt_tech_ = effect->TechniqueByName("GBufferSkyBoxMRTTech");
			special_shading_tech_ = effect->TechniqueByName("SkyBoxTech");
			this->Technique(gbuffer_rt0_tech_);

			effect_attrs_ |= EA_SpecialShading;
		}
		else
		{
			this->Technique(effect->TechniqueByName("SkyBoxTech"));
		}

		float3 xyzs[] =
		{
			float3(1.0f, 1.0f, 1.0f),
			float3(1.0f, -1.0f, 1.0f),
			float3(-1.0f, 1.0f, 1.0f),
			float3(-1.0f, -1.0f, 1.0f),
		};

		ElementInitData init_data;
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleStrip);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[4]);
		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
	}

	void RenderableSkyBox::Technique(RenderTechniquePtr const & tech)
	{
		technique_ = tech;
		skybox_cube_tex_ep_ = technique_->Effect().ParameterByName("skybox_tex");
		skybox_Ccube_tex_ep_ = technique_->Effect().ParameterByName("skybox_C_tex");
		skybox_compressed_ep_ = technique_->Effect().ParameterByName("skybox_compressed");
		depth_far_ep_ = technique_->Effect().ParameterByName("depth_far");
		inv_mvp_ep_ = technique_->Effect().ParameterByName("inv_mvp");
	}

	void RenderableSkyBox::CubeMap(TexturePtr const & cube)
	{
		*skybox_cube_tex_ep_ = cube;
		*skybox_compressed_ep_ = static_cast<int32_t>(0);
	}

	void RenderableSkyBox::CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
	{
		*skybox_cube_tex_ep_ = y_cube;
		*skybox_Ccube_tex_ep_ = c_cube;
		*skybox_compressed_ep_ = static_cast<int32_t>(1);
	}

	void RenderableSkyBox::Pass(PassType type)
	{
		switch (type)
		{
		case PT_OpaqueDepth:
			technique_ = depth_tech_;
			break;

		case PT_OpaqueGBufferRT0:
			technique_ = gbuffer_rt0_tech_;
			break;

		case PT_OpaqueGBufferRT1:
			technique_ = gbuffer_rt1_tech_;
			break;

		case PT_OpaqueGBufferMRT:
			technique_ = gbuffer_mrt_tech_;
			break;

		case PT_OpaqueSpecialShading:
			technique_ = special_shading_tech_;
			break;

		default:
			break;
		}
	}

	void RenderableSkyBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		*depth_far_ep_ = camera.FarPlane();

		float4x4 rot_view = camera.ViewMatrix();
		rot_view(3, 0) = 0;
		rot_view(3, 1) = 0;
		rot_view(3, 2) = 0;
		*inv_mvp_ep_ = MathLib::inverse(rot_view * camera.ProjMatrix());
	}
}
