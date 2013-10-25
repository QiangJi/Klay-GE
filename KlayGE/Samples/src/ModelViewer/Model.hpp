#ifndef _MODEL_HPP
#define _MODEL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>

class DetailedSkinnedMesh;

class DetailedSkinnedModel : public KlayGE::SkinnedModel
{
	friend class DetailedSkinnedMesh;

public:
	explicit DetailedSkinnedModel(std::wstring const & name);

	void BuildModelInfo();

	void SetTime(float time);

	void VisualizeLighting();
	void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index);
	void VisualizeTexture(int slot);

	void LineMode(bool line_mode);
	void SmoothMesh(bool smooth);
	void SetTessFactor(KlayGE::int32_t tess_factor);

	KlayGE::RenderEffectPtr const & Effect() const
	{
		return effect_;
	}

private:
	KlayGE::RenderEffectPtr effect_;

	KlayGE::RenderTechniquePtr depth_techs_[3][2][2][2];	
	KlayGE::RenderTechniquePtr depth_alpha_test_techs_[3][2][2][2];	
	KlayGE::RenderTechniquePtr depth_alpha_blend_back_techs_[3][2][2][2];	
	KlayGE::RenderTechniquePtr depth_alpha_blend_front_techs_[3][2][2][2];	
	KlayGE::RenderTechniquePtr gbuffer_rt0_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_alpha_test_rt0_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_back_rt0_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_front_rt0_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_rt1_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_alpha_test_rt1_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_back_rt1_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_front_rt1_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_mrt_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_alpha_test_mrt_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_back_mrt_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr gbuffer_alpha_blend_front_mrt_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr special_shading_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr special_shading_alpha_blend_back_techs_[3][2][2][2];
	KlayGE::RenderTechniquePtr special_shading_alpha_blend_front_techs_[3][2][2][2];
};

class DetailedSkinnedMesh : public KlayGE::SkinnedMesh
{
public:
	DetailedSkinnedMesh(KlayGE::RenderModelPtr const & model, std::wstring const & name);

	void BuildMeshInfo();

	void OnRenderBegin();
	void Render();

	void VisualizeLighting();
	void VisualizeVertex(KlayGE::VertexElementUsage usage, KlayGE::uint8_t usage_index);
	void VisualizeTexture(int slot);

	void LineMode(bool line_mode);
	void SmoothMesh(bool smooth);
	void SetTessFactor(KlayGE::int32_t tess_factor);

private:
	void UpdateTech();

private:
	float tess_factor_;
	int visualize_;
	bool line_mode_;
	bool smooth_mesh_;
	bool has_skinned_;

	KlayGE::RenderLayoutPtr mesh_rl_;
	KlayGE::RenderLayoutPtr point_rl_;
	KlayGE::RenderLayoutPtr skinned_rl_;
	KlayGE::RenderLayoutPtr tess_pattern_rl_;
	KlayGE::GraphicsBufferPtr skinned_pos_vb_;
	KlayGE::GraphicsBufferPtr skinned_tex_vb_;
	KlayGE::GraphicsBufferPtr skinned_tangent_vb_;
	KlayGE::GraphicsBufferPtr bindable_ib_;
};

#endif		// _MODEL_HPP
