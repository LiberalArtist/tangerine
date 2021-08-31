
// Copyright 2021 Aeva Palecek
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
// See the License for the specific language governing permissionsand
// limitations under the License.

// This file is imported by GLSL and C++ sources.

#define DIV_UP(X, Y) ((X + Y - 1) / Y)
#define TILE_SIZE_X 8
#define TILE_SIZE_Y 8

#define USE_COVERAGE_SEARCH 0
#define VISUALIZE_TRACING_ERROR 0
#define VISUALIZE_CLUSTER_COVERAGE 1