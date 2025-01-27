
// Copyright 2022 Aeva Palecek
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "../shaders/defines.h"
#include "sdf_evaluator.h"
#include "gl_boilerplate.h"
#include "gl_async.h"


// This object tracks a buffer containing the bytecode that is used to render part of a model's
// evaluator, and the voxels that will draw this program.  This bytecode buffer is used by both
// the shader interpreter and the compiled shader.
struct ProgramBuffer
{
	struct VoxelUpload
	{
		glm::vec4 Center;
		glm::vec4 Extent;

		VoxelUpload(const AABB& Bounds)
		{
			Extent = glm::vec4((Bounds.Max - Bounds.Min) * glm::vec3(0.5), 0.0);
			Center = glm::vec4(Extent.xyz + Bounds.Min, 0.0);
		}
	};

	std::vector<float> Params;
	std::vector<VoxelUpload> Voxels;
	Buffer ParamsBuffer;
	Buffer VoxelsBuffer;
	Buffer DrawsBuffer;

	ProgramBuffer(uint32_t ShaderIndex, uint32_t SubtreeIndex, size_t ParamCount, const float* InParams, const std::vector<AABB>& InVoxels);
	void Release();
};


// This object represents all ProgramBuffers that share the same symbolic behavior.
// This is used to access related ProgramBuffers for rendering, and is the interface
// for accessing the shader that is needed to draw the programs when the interpreter
// is not in use.
struct ProgramTemplate
{
	int LeafCount;
	std::string DebugName;
	std::string PrettyTree;
	std::string DistSource;

	std::shared_ptr<ShaderEnvelope> Compiled;

	TimingQuery DepthQuery;

	std::vector<ProgramBuffer> ProgramVariants;

	ProgramTemplate(ProgramTemplate&& Old);
	ProgramTemplate(std::string InDebugName, std::string InPrettyTree, std::string InDistSource, int InLeafCount);
	void StartCompile();
	ShaderProgram* GetCompiledShader();
	void Reset();
	void Release();
};
