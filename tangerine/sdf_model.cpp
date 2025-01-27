
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

#include "sdf_model.h"
#include "profiling.h"


std::vector<SDFModel*> LiveModels;


std::vector<SDFModel*>& GetLiveModels()
{
	return LiveModels;
}


void UnloadAllModels()
{
	for (SDFModel* Model : LiveModels)
	{
		delete Model;
	}
	LiveModels.clear();
}


void GetIncompleteModels(std::vector<SDFModel*>& Incomplete)
{
	Incomplete.clear();
	Incomplete.reserve(LiveModels.size());

	for (SDFModel* Model : LiveModels)
	{
		if (Model->HasPendingShaders())
		{
			Incomplete.push_back(Model);
		}
	}
}


void GetRenderableModels(std::vector<SDFModel*>& Renderable)
{
	Renderable.clear();
	Renderable.reserve(LiveModels.size());

	for (SDFModel* Model : LiveModels)
	{
		if (Model->HasCompleteShaders())
		{
			Renderable.push_back(Model);
		}
	}
}


bool MatchEvent(SDFModel& Model, MouseEvent& Event)
{
	return (Model.MouseListenFlags & MOUSE_FLAG(Event.Type)) == MOUSE_FLAG(Event.Type);
}


bool DeliverMouseMove(glm::vec3 Origin, glm::vec3 RayDir, int MouseX, int MouseY)
{
	bool ReturnToSender = true;

	// TODO

	return ReturnToSender;
}


bool DeliverMouseButton(MouseEvent Event)
{
	bool ReturnToSender = true;

	float Nearest = std::numeric_limits<float>::infinity();
	SDFModel* NearestMatch = nullptr;
	std::vector<SDFModel*> MouseUpRecipients;

	const bool Press = Event.Type == MOUSE_DOWN;
	const bool Release = Event.Type == MOUSE_UP;

	for (SDFModel* Model : LiveModels)
	{
		// TODO:
		// I'm unsure if this is how I want to handle mouse button event routing.
		// Obviously the most useful form is an env registers a mouse down event on
		// one of its models to find when the model is clicked, double clicked, or to
		// start listening to mouse move.
		// I'm pretty sure listening for a global mouse up is also useful, because if
		// the down event us used to start some interaction state machine, we probably
		// want to be able to use the up event to terminate the machine even if the
		// model is occluded.  This part is fine.
		// Quesiton is, is it useful for models to register a global mouse down event?
		// I'm guessing "maybe yes maybe" - for example you click on a model in a pallet,
		// then click on the board to place an instance of it, like a paint program.
		// Likewise it would be useful for some models to opt out of blocking the ray
		// queries, and for others to be able to block the ray queries without registering
		// a handler.
		// This is probably fine at least until the events can be forwarded back to
		// the script envs.

		if (MatchEvent(*Model, Event))
		{
			if (Release)
			{
				MouseUpRecipients.push_back(Model);
			}
			if (Model->Visible)
			{
				RayHit Query = Model->RayMarch(Event.RayOrigin, Event.RayDir);
				if (Query.Hit && Query.Travel < Nearest)
				{
					Nearest = Query.Travel;
					NearestMatch = Model;
					Event.AnyHit = true;
					Event.Cursor = Query.Position;
				}
			}
		}
	}

	if (Press && NearestMatch)
	{
		ReturnToSender = false;
		NearestMatch->OnMouseEvent(Event, true);
	}

	if (MouseUpRecipients.size() > 0)
	{
		ReturnToSender = false;
		for (SDFModel* Recipient : MouseUpRecipients)
		{
			Recipient->OnMouseEvent(Event, Recipient == NearestMatch);
		}
	}

	return ReturnToSender;
}


bool DeliverMouseScroll(glm::vec3 Origin, glm::vec3 RayDir, int ScrollX, int ScrollY)
{
	bool ReturnToSender = true;

	// TODO

	return ReturnToSender;
}


RayHit SDFModel::RayMarch(glm::vec3 RayStart, glm::vec3 RayDir, int MaxIterations, float Epsilon)
{
	glm::vec3 RelativeOrigin = Transform.ApplyInverse(RayStart);
	glm::mat3 Rotation = (glm::mat3)Transform.LastFoldInverse;
	glm::vec3 RelativeRayDir = Rotation * RayDir;
	return Evaluator->RayMarch(RelativeOrigin, RelativeRayDir, 1000);
}


bool SDFModel::HasPendingShaders()
{
	return PendingShaders.size() > 0;
}


bool SDFModel::HasCompleteShaders()
{
	return CompiledTemplates.size() > 0;
}


void SDFModel::CompileNextShader()
{
	BeginEvent("Compile Shader");

	size_t TemplateIndex = PendingShaders.back();
	PendingShaders.pop_back();

	ProgramTemplate& ProgramFamily = ProgramTemplates[TemplateIndex];
	ProgramFamily.StartCompile();
	if (ProgramFamily.ProgramVariants.size() > 0)
	{
		CompiledTemplates.push_back(&ProgramFamily);
	}

	EndEvent();
}


SDFModel::SDFModel(SDFNode* InEvaluator, const float VoxelSize)
{
	InEvaluator->Hold();
	Evaluator = InEvaluator;
	Compile(VoxelSize);

	TransformBuffer.DebugName = "Instance Transforms Buffer";

	LiveModels.push_back(this);
}


SDFModel::~SDFModel()
{
	Assert(RefCount == 0);

	for (int i = 0; i < LiveModels.size(); ++i)
	{
		if (LiveModels[i] == this)
		{
			LiveModels.erase(LiveModels.begin() + i);
			break;
		}
	}

	// If the Evaluator is nullptr, then this SDFModel was moved into a new instance, and less stuff needs to be deleted.
	if (Evaluator)
	{
		Evaluator->Release();

		for (ProgramTemplate& ProgramFamily : ProgramTemplates)
		{
			ProgramFamily.Release();
		}
	}

	ProgramTemplates.clear();
	ProgramTemplateSourceMap.clear();
	PendingShaders.clear();
	CompiledTemplates.clear();
}
