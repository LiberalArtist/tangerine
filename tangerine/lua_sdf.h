
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

#include "embedding.h"
#if EMBED_LUA
#include "lua_env.h"
#include "lua_vec.h"
#include "sdf_model.h"
#include <lua/lua.hpp>

struct LuaModel : public SDFModel
{
	LuaEnvironment* Env = nullptr;
	int MouseCallbackRefs[MOUSE_EVENTS];

	LuaModel(lua_State* L, SDFNode* InEvaluator, const float VoxelSize);
	virtual ~LuaModel();
	virtual void OnMouseEvent(MouseEvent& Event, bool Picked);

	void SetMouseEventCallback(int EventFlag);
};


int LuaOpenSDF(struct lua_State* L);


#endif // EMBED_LUA
