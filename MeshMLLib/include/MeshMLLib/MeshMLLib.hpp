/**
 * @file MeshMLLib.hpp
 * @author Rui Wang, Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of MeshMLLib, a subproject of KlayGE
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

#ifndef _MESHMLLIB_MESHMLLIB_HPP
#define _MESHMLLIB_MESHMLLIB_HPP

#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include <KFL/PreDeclare.hpp>
#include <KFL/Vector.hpp>
#include <KFL/Quaternion.hpp>
#include <KFL/Matrix.hpp>

namespace KlayGE
{
	class MeshMLObj
	{
	public:
		enum VertexExportSetting
		{
			VES_None = 0,
			VES_Normal = 0x1,
			VES_TangentQuat = 0x2,
			VES_Texcoord = 0x4
		};

		enum UserExportSetting
		{
			UES_None = 0,
			UES_CombineMeshes = 0x1,
			UES_SortMeshes = 0x2,
			UES_All = 0xFF
		};

	public:
		explicit MeshMLObj(float unit_scale);

		void NumFrames(int nf)
		{
			num_frames_ = nf;
		}
		int NumFrames() const
		{
			return num_frames_;
		}

		void FrameRate(int fr)
		{
			frame_rate_ = fr;
		}
		int FrameRate() const
		{
			return frame_rate_;
		}

		int AllocJoint();
		void SetJoint(int joint_id, std::string const & joint_name, int parent_id,
			float4x4 const & bind_mat);
		void SetJoint(int joint_id, std::string const & joint_name, int parent_id,
			Quaternion const & bind_quat, float3 const & bind_pos);
		void SetJoint(int joint_id, std::string const & joint_name, int parent_id,
			Quaternion const & bind_real, Quaternion const & bind_dual);

		int AllocMaterial();
		void SetMaterial(int mtl_id, float3 const & ambient, float3 const & diffuse,
			float3 const & specular, float3 const & emit, float opacity, float shininess);
		int AllocTextureSlot(int mtl_id);
		void SetTextureSlot(int mtl_id, int slot_id, std::string const & type, std::string const & name);

		int AllocMesh();
		void SetMesh(int mesh_id, int material_id, std::string const & name);
		int AllocVertex(int mesh_id);
		void SetVertex(int mesh_id, int vertex_id, float3 const & pos, float3 const & normal,
			int texcoord_components, std::vector<float3> const & texcoords);
		void SetVertex(int mesh_id, int vertex_id, float3 const & pos,
			float3 const & tangent, float3 const & binormal, float3 const & normal,
			int texcoord_components, std::vector<float3> const & texcoords);
		void SetVertex(int mesh_id, int vertex_id, float3 const & pos, Quaternion const & tangent_quat,
			int texcoord_components, std::vector<float3> const & texcoords);
		int AllocJointBinding(int mesh_id, int vertex_id);
		void SetJointBinding(int mesh_id, int vertex_id, int binding_id,
			int joint_id, float weight);
		int AllocTriangle(int mesh_id);
		void SetTriangle(int mesh_id, int triangle_id, int index0, int index1, int index2);

		int AllocKeyframes();
		void SetKeyframes(int kfs_id, int joint_id);
		int AllocKeyframe(int kfs_id);
		void SetKeyframe(int kfs_id, int kf_id, int frame_id, float4x4 const & bind_mat);
		void SetKeyframe(int kfs_id, int kf_id, int frame_id, Quaternion const & bind_quat, float3 const & bind_pos);
		void SetKeyframe(int kfs_id, int kf_id, int frame_id, Quaternion const & bind_real, Quaternion const & bind_dual);

		int AllocAction();
		void SetAction(int action_id, std::string const & name, int start_frame, int end_frame);

		void WriteMeshML(std::ostream& os,
			int vertex_export_settings = VES_TangentQuat | VES_Texcoord, int user_export_settings = UES_SortMeshes,
			std::string const & encoding = std::string());

	private:
		typedef std::pair<std::string, std::string> TextureSlot;
		typedef std::pair<int, float> JointBinding;

		struct Material
		{
			float3 ambient;
			float3 diffuse;
			float3 specular;
			float3 emit;
			float opacity;
			float shininess;
			std::vector<TextureSlot> texture_slots;

			bool operator==(Material const & rhs) const;
		};
		
		struct Vertex
		{
			float3 position;
			float3 normal;
			Quaternion tangent_quat;
			int texcoord_components;
			std::vector<float3> texcoords;
			std::vector<JointBinding> binds;
		};

		struct Triangle
		{
			int vertex_index[3];
		};

		struct Mesh
		{
			int material_id;
			std::string name;
			std::vector<Vertex> vertices;
			std::vector<Triangle> triangles;
		};

		struct Joint
		{
			std::string name;
			int parent_id;
			Quaternion bind_real;
			Quaternion bind_dual;
			float bind_scale;
		};

		struct Keyframes
		{
			int joint_id;

			std::vector<int> frame_ids;
			std::vector<Quaternion> bind_reals;
			std::vector<Quaternion> bind_duals;
			std::vector<float> bind_scales;

			std::pair<std::pair<Quaternion, Quaternion>, float> Frame(float frame) const;
		};

		struct AnimationAction
		{
			std::string name;
			int start_frame;
			int end_frame;
		};

		void OptimizeJoints();
		void OptimizeMaterials();
		void OptimizeMeshes(int user_export_settings);

		void WriteJointChunk(std::ostream& os);
		void WriteMaterialChunk(std::ostream& os);
		void WriteMeshChunk(std::ostream& os, int vertex_export_settings);
		void WriteKeyframeChunk(std::ostream& os);
		void WriteAABBKeyframeChunk(std::ostream& os);
		void WriteActionChunk(std::ostream& os);

		void MatrixToDQ(float4x4 const & mat, Quaternion& real, Quaternion& dual) const;
		void UpdateJoints(int frame, std::vector<Quaternion>& bind_reals, std::vector<Quaternion>& bind_duals) const;

		struct MaterialIDSortOp
		{
			bool operator()(Mesh const & lhs, Mesh const & rhs) const
			{
				return lhs.material_id < rhs.material_id;
			}
		};

	private:
		float unit_scale_;

		int num_frames_;
		int frame_rate_;

		std::map<int, Joint> joints_;
		std::vector<Material> materials_;
		std::vector<Mesh> meshes_;
		std::vector<Keyframes> keyframes_;
		std::vector<AnimationAction> actions_;
	};
}

#endif  // _MESHMLLIB_MESHMLLIB_HPP
