/*******************************************************************************
 * Copyright 2023 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#pragma once

#include "Runtime/Launch/Resources/Version.h"

#define XESS_ENGINE_VERSION_EQU(Major, Minor) \
	(ENGINE_MAJOR_VERSION == Major && ENGINE_MINOR_VERSION == Minor)

#define XESS_ENGINE_VERSION_GEQ(Major, Minor) \
	((ENGINE_MAJOR_VERSION == Major && ENGINE_MINOR_VERSION >= Minor) || ENGINE_MAJOR_VERSION > Major)

#define XESS_ENGINE_VERSION_LSS(Major, Minor) \
	((ENGINE_MAJOR_VERSION == Major && ENGINE_MINOR_VERSION < Minor) || ENGINE_MAJOR_VERSION < Major)

#define XESS_ENGINE_WITH_XEFG_API_MIN 0x1  // Minimum APIs used by XeFG
#define XESS_ENGINE_WITH_XEFG_API_SVE_POST_RENDER_VIEW_RDG 0x2  // ISceneViewExtension::PostRenderView_RenderThread with FRDGBuilder to support secondary upscale
#define XESS_ENGINE_WITH_XEFG_API_ALL (XESS_ENGINE_WITH_XEFG_API_MIN | XESS_ENGINE_WITH_XEFG_API_SVE_POST_RENDER_VIEW_RDG)

#ifndef XESS_ENGINE_WITH_XEFG_PATCH
	// Set to combination of XESS_ENGINE_WITH_XEFG_API_* if not set in Engine patch, only needed for Unreal version below 5.2
	#define XESS_ENGINE_WITH_XEFG_PATCH 0
#endif

// TODO(sunzhuoshi): support Unreal 4.26
#if XESS_ENGINE_VERSION_GEQ(5, 2)
	// Unreal 5.2 and above: all APIs available
	#define XESS_ENGINE_WITH_XEFG_API XESS_ENGINE_WITH_XEFG_API_ALL
#elif XESS_ENGINE_VERSION_GEQ(5, 0)
	// Unreal 5.0, 5.1: APIs depend on engine patch
	#if XESS_ENGINE_WITH_XEFG_PATCH
		#define XESS_ENGINE_WITH_XEFG_API (XESS_ENGINE_WITH_XEFG_API_SVE_POST_RENDER_VIEW_RDG | XESS_ENGINE_WITH_XEFG_PATCH)
	#else
		#define XESS_ENGINE_WITH_XEFG_API 0
	#endif
#else
	// Unreal 4: only APIs defined in the engine patch
	#define XESS_ENGINE_WITH_XEFG_API XESS_ENGINE_WITH_XEFG_PATCH
#endif

// Ensure minimum API flag is always present if XESS_ENGINE_WITH_XEFG_API is true
#if XESS_ENGINE_WITH_XEFG_API && !(XESS_ENGINE_WITH_XEFG_API & XESS_ENGINE_WITH_XEFG_API_MIN)
	#error "Invalid XESS_ENGINE_WITH_XEFG_API, XESS_ENGINE_WITH_XEFG_API_MIN is a MUST have in XESS_ENGINE_WITH_XEFG_API"
#endif

#if XESS_ENGINE_VERSION_LSS(5, 5)
#define RDG_EVENT_SCOPE_STAT(...)  // Introduced in Unreal 5.5
#endif
