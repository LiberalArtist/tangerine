
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

#include "lua_sdf.h"
#if EMBED_LUA
#include "sdf_model.h"

#include <string.h>
#include <random>
#include <algorithm>

#include "sdf_evaluator.h"
#include "colors.h"
#include "tangerine.h"


std::default_random_engine RNGesus;


SDFNode* GetSDFNode(lua_State* L, int Arg)
{
	SDFNode** Node = (SDFNode**)luaL_checkudata(L, Arg, "tangerine.sdf");
	return *Node;
}


LuaModel* GetSDFModel(lua_State* L, int Arg)
{
	LuaModel** Model = (LuaModel**)luaL_checkudata(L, Arg, "tangerine.model");
	return *Model;
}


int WrapSDFNode(lua_State* L, SDFNode* NewNode)
{
	SDFNode** Wrapper = (SDFNode**)lua_newuserdata(L, sizeof(SDFNode*));
	luaL_getmetatable(L, "tangerine.sdf");
	lua_setmetatable(L, -2);
	*Wrapper = NewNode;
	NewNode->Hold();
	return 1;
}


int LuaMoveInner(lua_State* L, float X, float Y, float Z)
{
	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	NewNode->Move(glm::vec3(X, Y, Z));
	return WrapSDFNode(L, NewNode);
}


int LuaMoveX(lua_State* L)
{
	float X = (float)luaL_checknumber(L, 2);
	return LuaMoveInner(L, X, 0, 0);
}


int LuaMoveY(lua_State* L)
{
	float Y = (float)luaL_checknumber(L, 2);
	return LuaMoveInner(L, 0, Y, 0);
}


int LuaMoveZ(lua_State* L)
{
	float Z = (float)luaL_checknumber(L, 2);
	return LuaMoveInner(L, 0, 0, Z);
}


int LuaMove(lua_State* L)
{
	int NextArg = 2;
	glm::vec3 Offset = GetVec3(L, NextArg);

	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	NewNode->Move(Offset);
	return WrapSDFNode(L, NewNode);
}


int LuaAlign(lua_State* L)
{
	int NextArg = 2;
	glm::vec3 Anchors = GetVec3(L, NextArg);

	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	SDF::Align(NewNode, Anchors);
	return WrapSDFNode(L, NewNode);
}


int LuaRotateInner(lua_State* L, float X, float Y, float Z, float W = 1.0)
{
	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	NewNode->Rotate(glm::quat(W, X, Y, Z));
	return WrapSDFNode(L, NewNode);
}


int LuaRotateX(lua_State* L)
{
	float Degrees = (float)luaL_checknumber(L, 2);
	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	SDF::RotateX(NewNode, Degrees);
	return WrapSDFNode(L, NewNode);
}


int LuaRotateY(lua_State* L)
{
	float Degrees = (float)luaL_checknumber(L, 2);
	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	SDF::RotateY(NewNode, Degrees);
	return WrapSDFNode(L, NewNode);
}


int LuaRotateZ(lua_State* L)
{
	float Degrees = (float)luaL_checknumber(L, 2);
	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	SDF::RotateZ(NewNode, Degrees);
	return WrapSDFNode(L, NewNode);
}


int LuaRotate(lua_State* L)
{
	glm::quat Quat(
		(float)luaL_checknumber(L, 5),
		(float)luaL_checknumber(L, 2),
		(float)luaL_checknumber(L, 3),
		(float)luaL_checknumber(L, 4));
	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	NewNode->Rotate(Quat);
	return WrapSDFNode(L, NewNode);
}


int LuaScale(lua_State* L)
{
	float Scale = (float)luaL_checknumber(L, 2);
	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	NewNode->Scale(Scale);
	return WrapSDFNode(L, NewNode);
}


int LuaFlate(lua_State* L)
{
	float Radius = (float)luaL_checknumber(L, 2) * .5;
	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = SDF::Flate(Node, Radius);
	return WrapSDFNode(L, NewNode);
}


template <bool Force>
int LuaPaint(lua_State* L)
{
	glm::vec3 Color;
	if (!lua_isnumber(L, 2) && lua_isstring(L, 2))
	{
		const char* ColorString = luaL_checklstring(L, 2, nullptr);
		StatusCode Result = ParseColor(ColorString, Color);
	}
	else
	{
		int NextArg = 2;
		Color = GetVec3(L, NextArg);
	}

	SDFNode* Node = GetSDFNode(L, 1);
	SDFNode* NewNode = Node->Copy();
	NewNode->ApplyMaterial(Color, Force);
	return WrapSDFNode(L, NewNode);
}


int LuaSphere(lua_State* L)
{
	float Radius = luaL_checknumber(L, 1) * .5;
	SDFNode* NewNode = SDF::Sphere(Radius);
	return WrapSDFNode(L, NewNode);
}


int LuaEllipsoid(lua_State* L)
{
	int NextArg = 1;
	glm::vec3 Radipodes = GetVec3(L, NextArg) * glm::vec3(0.5);
	SDFNode* NewNode = SDF::Ellipsoid(Radipodes.x, Radipodes.y, Radipodes.z);
	return WrapSDFNode(L, NewNode);
}


int LuaBox(lua_State* L)
{
	int NextArg = 1;
	glm::vec3 Extent = GetVec3(L, NextArg) * glm::vec3(0.5);
	SDFNode* NewNode = SDF::Box(Extent.x, Extent.y, Extent.z);
	return WrapSDFNode(L, NewNode);
}


int LuaCube(lua_State* L)
{
	float Extent = luaL_checknumber(L, 1) * .5;
	SDFNode* NewNode = SDF::Box(Extent, Extent, Extent);
	return WrapSDFNode(L, NewNode);
}


int LuaTorus(lua_State* L)
{
	float MajorRadius = luaL_checknumber(L, 1) * .5;
	float MinorRadius = luaL_checknumber(L, 2) * .5;
	SDFNode* NewNode = SDF::Torus(MajorRadius - MinorRadius, MinorRadius);
	return WrapSDFNode(L, NewNode);
}


int LuaCylinder(lua_State* L)
{
	float Radius = luaL_checknumber(L, 1) * .5;
	float Extent = luaL_checknumber(L, 2) * .5;
	SDFNode* NewNode = SDF::Cylinder(Radius, Extent);
	return WrapSDFNode(L, NewNode);
}


int LuaPlane(lua_State* L)
{
	int NextArg = 1;
	glm::vec3 Normal = GetVec3(L, NextArg);
	SDFNode* NewNode = SDF::Plane(Normal.x, Normal.y, Normal.z);
	return WrapSDFNode(L, NewNode);
}


int LuaCone(lua_State* L)
{
	float Radius = luaL_checknumber(L, 1) * .5;
	float Height = luaL_checknumber(L, 2);
	SDFNode* NewNode = SDF::Cone(Radius, Height);
	return WrapSDFNode(L, NewNode);
}


int LuaConinder(lua_State* L)
{
	int NextArg = 1;
	// RadiusL, RadiusH, and Height
	glm::vec3 Params = GetVec3(L, NextArg) * glm::vec3(.5, .5, 1.0);
	SDFNode* NewNode = SDF::Coninder(Params.x, Params.y, Params.z);
	return WrapSDFNode(L, NewNode);
}


enum class SetFamily : uint32_t
{
	Union,
	Inter,
	Diff
};


template<SetFamily Family>
SDFNode* Operator(SDFNode* LHS, SDFNode* RHS)
{
	if (Family == SetFamily::Union)
	{
		return SDF::Union(LHS, RHS);
	}
	else if (Family == SetFamily::Inter)
	{
		return SDF::Inter(LHS, RHS);
	}
	else
	{
		return SDF::Diff(LHS, RHS);
	}
}


template<SetFamily Family>
SDFNode* Operator(float Threshold, SDFNode* LHS, SDFNode* RHS)
{
	if (Family == SetFamily::Union)
	{
		return SDF::BlendUnion(Threshold, LHS, RHS);
	}
	else if (Family == SetFamily::Inter)
	{
		return SDF::BlendInter(Threshold, LHS, RHS);
	}
	else
	{
		return SDF::BlendDiff(Threshold, LHS, RHS);
	}
}


template<SetFamily Family>
int LuaOperator(lua_State* L)
{
	SDFNode** LHS = (SDFNode**)luaL_checkudata(L, 1, "tangerine.sdf");
	SDFNode** RHS = (SDFNode**)luaL_checkudata(L, 2, "tangerine.sdf");
	SDFNode* NewNode = Operator<Family>(*LHS, *RHS);

	int Args = lua_gettop(L);
	for (int i = 3; i <= Args; ++i)
	{
		SDFNode** Next = (SDFNode**)luaL_checkudata(L, i, "tangerine.sdf");
		NewNode = Operator<Family>(NewNode, *Next);
	}

	return WrapSDFNode(L, NewNode);
}


template<SetFamily Family>
int LuaBlendOperator(lua_State* L)
{
	float Threshold = (float)luaL_checknumber(L, -1);
	lua_pop(L, 1);

	SDFNode** LHS = (SDFNode**)luaL_checkudata(L, 1, "tangerine.sdf");
	SDFNode** RHS = (SDFNode**)luaL_checkudata(L, 2, "tangerine.sdf");
	SDFNode* NewNode = Operator<Family>(Threshold, *LHS, *RHS);

	int Args = lua_gettop(L);
	for (int i = 3; i <= Args; ++i)
	{
		SDFNode** Next = (SDFNode**)luaL_checkudata(L, i, "tangerine.sdf");
		NewNode = Operator<Family>(Threshold, NewNode, *Next);
	}

	return WrapSDFNode(L, NewNode);
}


int LuaEval(lua_State* L)
{
	SDFNode* Node = GetSDFNode(L, 1);
	int NextArg = 2;
	glm::vec3 Point = GetVec3(L, NextArg);

	float Distance = Node->Eval(Point);
	lua_pushnumber(L, Distance);
	return 1;
}


int LuaGradient(lua_State* L)
{
	SDFNode* Node = GetSDFNode(L, 1);
	int NextArg = 2;
	glm::vec3 Point = GetVec3(L, NextArg);
	glm::vec3 Gradient = Node->Gradient(Point);
	CreateVec(L, Gradient);
	return 1;
}


int LuaPickColor(lua_State* L)
{
	SDFNode* Node = GetSDFNode(L, 1);
	int NextArg = 2;
	glm::vec3 Point = GetVec3(L, NextArg);
	glm::vec4 Color = Node->Sample(Point);
	CreateVec(L, Color);
	return 1;
}


template <bool Magnet>
int LuaRayCast(lua_State* L)
{
	SDFNode* Node = GetSDFNode(L, 1);

	int NextArg = 2;
	glm::vec3 Origin = GetVec3(L, NextArg);
	glm::vec3 Direction = GetVec3(L, NextArg);

	if (Magnet)
	{
		Direction = glm::normalize(Direction - Origin);
	}

	int MaxIterations = 100;
	float Epsilon = 0.001;

	int Args = lua_gettop(L);
	if (Args >= 8)
	{
		MaxIterations = (int)luaL_checkinteger(L, 8);
	}
	if (Args >= 9)
	{
		Epsilon = (float)luaL_checknumber(L, 9);
	}

	RayHit RayHit = Node->RayMarch(Origin, Direction, MaxIterations, Epsilon);

	if (RayHit.Hit)
	{
		CreateVec(L, RayHit.Position);
		return 1;
	}
	else
	{
		lua_pushnil(L);
		return 1;
	}
}


int LuaPivotTowards(lua_State* L)
{
	SDFNode* Node = GetSDFNode(L, 1);

	float MaxDist = (float)luaL_checknumber(L, 2);
	float Margin = (float)luaL_checknumber(L, 3);
	float MaxAngle = glm::radians((float)luaL_checknumber(L, 4));

	int NextArg = 5;
	glm::vec3 Pivot = GetVec3(L, NextArg);
	glm::vec3 Heading = GetVec3(L, NextArg);
	glm::vec3 Down = GetVec3(L, NextArg);

	glm::vec3 Tail;
	glm::vec3 Axis;

	// Normalize the heading.
	{
		float HeadingLen = glm::length(Heading);
		if (HeadingLen == 0.0 || glm::isnan(HeadingLen) || glm::isinf(HeadingLen))
		{
			// The heading is invalid, so return the pivot and a zero heading.
			Tail = Pivot;
			Heading = glm::vec3(0.0, 0.0, 0.0);
			goto finish;
		}
		else
		{
			Heading /= HeadingLen;
		}
	}

	// Normalize the down vector.
	{
		float DownLen = glm::length(Down);
		if (DownLen == 0.0 || glm::isnan(DownLen) || glm::isinf(DownLen))
		{
			// Down vector is invalid, so just use negative z.
			Down = glm::vec3(0, 0, -1);
		}
		else
		{
			Down /= DownLen;
		}
	}

	// Set the start position.
	Tail = Heading * MaxDist + Pivot;

	// Try to find a pivot axis vector
	Axis = glm::cross(Heading, Down);
	{
		float AxisLen = glm::length(Axis);
		if (AxisLen == 0.0)
		{
			// Are we already at the rest position?
			if (glm::distance(Down, Heading) <= 0.001)
			{
				Heading = Down;
				goto finish;
			}
			else
			{
				// The normal is antiparallel to the down vector, so just pick a random pivot.
				while (AxisLen == 0.0)
				{
					std::uniform_real_distribution<double> Roll(-1.0, std::nextafter(1.0, DBL_MAX));
					Axis = glm::cross(glm::normalize(glm::vec3(Roll(RNGesus), Roll(RNGesus), Roll(RNGesus))), Down);
					AxisLen = glm::length(Axis);
				}
			}
		}
	}

	// The rotation axis is probably valid now :D
	Axis = glm::normalize(Axis);
	{
		float Dist;
		const glm::vec3 Start = Tail;
		float RemainingMaxAngle = MaxAngle;
		const float Fnord = MaxDist * MaxDist * 2;
		while (true)
		{
			Dist = Node->Eval(Tail);
			if (Dist <= Margin)
			{
				break;
			}

			Dist = glm::min(glm::abs(Dist), MaxDist);

			float Cosine = glm::abs((Fnord - Dist * Dist) / Fnord);
			float MaybeAngle = glm::acos(Cosine);
			float MaxDownward = glm::abs(glm::acos(glm::dot(glm::normalize(Tail - Pivot), Down)));
			float Angle = glm::min(RemainingMaxAngle, glm::min(MaxDownward, MaybeAngle));
			RemainingMaxAngle -= Angle;

			glm::quat Rotation = glm::angleAxis(Angle, Axis);
			Tail = glm::rotate(Rotation, Tail - Pivot) + Pivot;

			if (Angle <= glm::radians(0.5))
			{
				break;
			}
		}

		const float Cosine = glm::min(glm::abs(glm::dot(glm::normalize(Tail - Pivot), Heading)), 1.0f);
		const float MaybeAngle = glm::acos(Cosine);
		const float FinalAngle = glm::min(MaxAngle, MaybeAngle);
		glm::quat Rotation = glm::angleAxis(FinalAngle, Axis);
		Heading = glm::rotate(Rotation, Heading);
		Tail = Heading * MaxDist + Pivot;
	}

finish:
	CreateVec(L, Tail);
	CreateVec(L, Heading);

	return 2;
}


int LuaClearColor(lua_State* L)
{
	glm::vec3 Color;
	if (!lua_isnumber(L, 1) && lua_isstring(L, 1))
	{
		const char* ColorString = luaL_checklstring(L, 1, nullptr);
		StatusCode Result = ParseColor(ColorString, Color);
	}
	else
	{
		int NextArg = 1;
		Color = GetVec3(L, NextArg);
	}
	SetClearColor(Color);
	return 0;
}


int LuaOutliner(lua_State* L)
{
	const bool OutlinerState = lua_toboolean(L, 1);
	SetOutline(OutlinerState);
	return 0;
}


int LuaFixedCamera(lua_State* L)
{
	int NextArg = 1;
	glm::vec3 Origin = GetVec3(L, NextArg);
	glm::vec3 Center = GetVec3(L, NextArg);
	glm::vec3 Up = GetVec3(L, NextArg);
	SetFixedCamera(Origin, Center, Up);
	return 0;
}


int LuaRandomSeed(lua_State* L)
{
	lua_Integer Seed = luaL_checkinteger(L, 1);
	RNGesus.seed(Seed);
	return 0;
}


int LuaRandom(lua_State* L)
{
	if (lua_gettop(L) == 0)
	{
		// Return a random float between 0.0 and 1.0, inclusive.
		std::uniform_real_distribution<double> Dist(0.0, std::nextafter(1.0, DBL_MAX));
		lua_Number Random = (lua_Number)Dist(RNGesus);
		lua_pushnumber(L, Random);
		return 1;
	}
	else if (lua_gettop(L) >= 2 && lua_isinteger(L, 1) && lua_isinteger(L, 2))
	{
		// Return a random integer between Low and High, inclusive.
		lua_Integer Low = luaL_checkinteger(L, 1);
		lua_Integer High = luaL_checkinteger(L, 2);
		std::uniform_int_distribution<lua_Integer> Dist(Low, High);
		lua_pushinteger(L, Dist(RNGesus));
		return 1;
	}
	else
	{
		// Return a random float between Low and High, inclusive.
		lua_Number Low = luaL_checknumber(L, 1);
		lua_Number High = luaL_checknumber(L, 2);
		std::uniform_real_distribution<double> Dist(Low, std::nextafter(High, DBL_MAX));
		lua_Number Random = (lua_Number)Dist(RNGesus);
		lua_pushnumber(L, Random);
		return 1;
	}
}


int LuaShuffleSequence(lua_State* L)
{
	lua_Integer Count = luaL_checkinteger(L, 1);
	if (Count > 0)
	{
		std::vector<int> Deck;
		Deck.reserve(Count);
		for (int i = 1; i <= Count; ++i)
		{
			Deck.push_back(i);
		}
		std::shuffle(Deck.begin(), Deck.end(), RNGesus);
		lua_createtable(L, Count, 0);
		int i = 1;
		for (int& Index : Deck)
		{
			lua_pushinteger(L, Index);
			lua_rawseti(L, -2, i++);
		}
		return 1;
	}
	return 0;
}


int LuaInstanceModel(lua_State* L)
{
	SDFNode* Evaluator = GetSDFNode(L, 1);
	LuaModel* NewModel = new LuaModel(L, Evaluator, 0.25);

	LuaModel** Wrapper = (LuaModel**)lua_newuserdata(L, sizeof(LuaModel*));
	luaL_getmetatable(L, "tangerine.model");
	lua_setmetatable(L, -2);
	*Wrapper = NewModel;
	NewModel->Hold();
	return 1;
}


const luaL_Reg LuaSDFType[] = \
{
	{ "move", LuaMove },
	{ "move_x", LuaMoveX },
	{ "move_y", LuaMoveY },
	{ "move_z", LuaMoveZ },

	{ "align", LuaAlign },

	{ "rotate", LuaRotate },
	{ "rotate_x", LuaRotateX },
	{ "rotate_y", LuaRotateY },
	{ "rotate_z", LuaRotateZ },

	{ "scale", LuaScale },
	{ "flate", LuaFlate },

	{ "paint", LuaPaint<false> },
	{ "paint_over", LuaPaint<true> },

	{ "sphere", LuaSphere },
	{ "ellipsoid", LuaEllipsoid },
	{ "box", LuaBox },
	{ "cube", LuaCube },
	{ "torus", LuaTorus },
	{ "cylinder", LuaCylinder },
	{ "plane", LuaPlane },
	{ "cone", LuaCone },
	{ "coninder", LuaConinder },
	{ "cylicone", LuaConinder },

	{ "union", LuaOperator<SetFamily::Union> },
	{ "inter", LuaOperator<SetFamily::Inter> },
	{ "diff", LuaOperator<SetFamily::Diff> },

	{ "blend_union", LuaBlendOperator<SetFamily::Union> },
	{ "blend_inter", LuaBlendOperator<SetFamily::Inter> },
	{ "blend_diff", LuaBlendOperator<SetFamily::Diff> },

	{ "eval", LuaEval },
	{ "gradient", LuaGradient },
	{ "pick_color", LuaPickColor },
	{ "ray_cast", LuaRayCast<false> },
	{ "magnet", LuaRayCast<true> },
	{ "pivot_towards", LuaPivotTowards },

	{ "set_bg", LuaClearColor },
	{ "set_outline", LuaOutliner },
	{ "set_fixed_camera", LuaFixedCamera },

	{ "random_seed", LuaRandomSeed },
	{ "random", LuaRandom },
	{ "shuffle_sequence", LuaShuffleSequence },

	{ "instance", LuaInstanceModel },

	{ NULL, NULL }
};


int IndexSDFNode(lua_State* L)
{
	SDFNode** Self = (SDFNode**)luaL_checkudata(L, 1, "tangerine.sdf");
	const char* Key = luaL_checklstring(L, 2, nullptr);

	for (const luaL_Reg& Reg : LuaSDFType)
	{
		if (Reg.name && strcmp(Reg.name, Key) == 0)
		{
			lua_pushcfunction(L, Reg.func);
			break;
		}
		else if (!Reg.name)
		{
			lua_pushnil(L);
		}
	}

	return 1;
}


int SDFNodeGC(lua_State* L)
{
	SDFNode** Self = (SDFNode**)luaL_checkudata(L, 1, "tangerine.sdf");
	if (Self)
	{
		(*Self)->Release();
	}
	return 0;
}


const luaL_Reg LuaSDFMeta[] = \
{
	{ "__index", IndexSDFNode },
	{ "__gc", SDFNodeGC },
	{ NULL, NULL }
};


LuaModel::LuaModel(lua_State* L, SDFNode* InEvaluator, const float VoxelSize)
	: SDFModel(InEvaluator, VoxelSize)
	, Env(LuaEnvironment::GetScriptEnvironment(L))
{
	for (int i = 0; i < MOUSE_EVENTS; ++i)
	{
		MouseCallbackRefs[i] = LUA_REFNIL;
	}
}


LuaModel::~LuaModel()
{
	for (int i = 0; i < MOUSE_EVENTS; ++i)
	{
		luaL_unref(Env->L, LUA_REGISTRYINDEX, MouseCallbackRefs[i]);
	}
}


int LuaHideModel(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	Self->Visible = false;
	return 1;
};


int LuaShowModel(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	Self->Visible = true;
	return 1;
};


int LuaModelMove(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	int NextArg = 2;
	glm::vec3 Offset = GetVec3(L, NextArg);
	Self->Transform.Move(Offset);
	lua_pop(L, 3);
	return 1;
}


int LuaModelMoveX(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	glm::vec3 Offset(
		(float)luaL_checknumber(L, 2),
		0.0,
		0.0);
	Self->Transform.Move(Offset);
	lua_pop(L, 1);
	return 1;
}


int LuaModelMoveY(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	glm::vec3 Offset(
		0.0,
		(float)luaL_checknumber(L, 2),
		0.0);
	Self->Transform.Move(Offset);
	lua_pop(L, 1);
	return 1;
}


int LuaModelMoveZ(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	glm::vec3 Offset(
		0.0,
		0.0,
		(float)luaL_checknumber(L, 2));
	Self->Transform.Move(Offset);
	lua_pop(L, 1);
	return 1;
}


int LuaModelRotate(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	glm::quat Quat(
		(float)luaL_checknumber(L, 5),
		(float)luaL_checknumber(L, 2),
		(float)luaL_checknumber(L, 3),
		(float)luaL_checknumber(L, 4));
	Self->Transform.Rotate(Quat);
	lua_pop(L, 4);
	return 1;
}


int LuaModelRotateX(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	float Degrees = (float)luaL_checknumber(L, 2);
	float R = glm::radians(Degrees) * .5;
	float S = sin(R);
	float C = cos(R);
	Self->Transform.Rotate(glm::quat(C, S, 0, 0));
	lua_pop(L, 1);
	return 1;
}


int LuaModelRotateY(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	float Degrees = (float)luaL_checknumber(L, 2);
	float R = glm::radians(Degrees) * .5;
	float S = sin(R);
	float C = cos(R);
	Self->Transform.Rotate(glm::quat(C, 0, S, 0));
	lua_pop(L, 1);
	return 1;
}


int LuaModelRotateZ(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	float Degrees = (float)luaL_checknumber(L, 2);
	float R = glm::radians(Degrees) * .5;
	float S = sin(R);
	float C = cos(R);
	Self->Transform.Rotate(glm::quat(C, 0, 0, S));
	lua_pop(L, 1);
	return 1;
}


int LuaModelResetTransform(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	Self->Transform.Reset();
	return 1;
}


void LuaModel::SetMouseEventCallback(int EventIndex)
{
	lua_State* L = Env->L;
	int& CallbackRef = MouseCallbackRefs[EventIndex];
	luaL_unref(L, LUA_REGISTRYINDEX, CallbackRef);
	const int EventFlag = MOUSE_FLAG(EventIndex);

	if (lua_isnil(L, 2))
	{
		lua_pop(L, 1);
		CallbackRef = LUA_REFNIL;
		MouseListenFlags = MouseListenFlags & ~EventFlag;
	}
	else
	{
		luaL_checktype(L, 2, LUA_TFUNCTION);
		CallbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
		MouseListenFlags = MouseListenFlags | EventFlag;
	}
}


template<int EventIndex>
int SetOnMouseEvent(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	Self->SetMouseEventCallback(EventIndex);
	return 1;
}


void LuaModel::OnMouseEvent(MouseEvent& Event, bool Picked)
{
	lua_State* L = Env->L;
	int& CallbackRef = MouseCallbackRefs[Event.Type];
	int ErrorStatus = LUA_OK;
	switch (Event.Type)
	{
	case MOUSE_DOWN:
	case MOUSE_UP:
		if (CallbackRef != LUA_REFNIL)
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, CallbackRef);

			lua_createtable(L, 0, 5);
			{
				lua_pushboolean(L, true);
				lua_setfield(L, -2, Event.Type == MOUSE_DOWN ? "mouse_down" : "mouse_up");

				lua_pushinteger(L, Event.Button);
				lua_setfield(L, -2, "button");

				lua_pushinteger(L, Event.Clicks);
				lua_setfield(L, -2, "clicks");

				lua_pushboolean(L, Picked);
				lua_setfield(L, -2, "picked");

				if (Event.AnyHit)
				{
					CreateVec(L, Event.Cursor);
				}
				else
				{
					lua_pushnil(L);
				}
				lua_setfield(L, -2, "cursor");
			}
			ErrorStatus = lua_pcall(L, 1, 0, 0);
		}
		break;

	default:
		break;
	}

	if (!Env->HandleError(ErrorStatus))
	{
		luaL_unref(L, LUA_REGISTRYINDEX, CallbackRef);
		CallbackRef = LUA_REFNIL;
		const int EventFlag = MOUSE_FLAG(Event.Type);
		MouseListenFlags = MouseListenFlags & ~EventFlag;
	}
}


const luaL_Reg LuaModelType[] = \
{
	{ "hide", LuaHideModel },
	{ "show", LuaShowModel },

	{ "move", LuaModelMove },
	{ "move_x", LuaModelMoveX },
	{ "move_y", LuaModelMoveY },
	{ "move_z", LuaModelMoveZ },

	{ "rotate", LuaModelRotate },
	{ "rotate_x", LuaModelRotateX },
	{ "rotate_y", LuaModelRotateY },
	{ "rotate_z", LuaModelRotateZ },

	{ "reset_transform", LuaModelResetTransform },

	{ "on_mouse_down" , SetOnMouseEvent<MOUSE_DOWN> },
	{ "on_mouse_up" , SetOnMouseEvent<MOUSE_UP> },

	{ NULL, NULL }
};


int IndexSDFModel(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	const char* Key = luaL_checklstring(L, 2, nullptr);

	for (const luaL_Reg& Reg : LuaModelType)
	{
		if (Reg.name && strcmp(Reg.name, Key) == 0)
		{
			lua_pushcfunction(L, Reg.func);
			break;
		}
		else if (!Reg.name)
		{
			lua_pushnil(L);
		}
	}

	return 1;
}


int SDFModelGC(lua_State* L)
{
	LuaModel* Self = GetSDFModel(L, 1);
	Self->Release();
	return 0;
}


const luaL_Reg LuaModelMeta[] = \
{
	{ "__index", IndexSDFModel },
	{ "__gc", SDFModelGC },
	{ NULL, NULL }
};


int LuaOpenSDF(lua_State* L)
{
	luaL_newmetatable(L, "tangerine.model");
	luaL_setfuncs(L, LuaModelMeta, 0);

	luaL_newmetatable(L, "tangerine.sdf");
	luaL_setfuncs(L, LuaSDFMeta, 0);
	luaL_newlib(L, LuaSDFType);
	return 1;
}

#endif // EMBED_LUA
