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

#include "CoreMinimal.h"
#include "XeSSCommonMacros.h"

#if XESS_ENGINE_WITH_XEFG_API
#include "XeSSCommonSceneViewExtensionBase.h"
#include "ScreenPass.h"

class FXeFGRHI;
class FXeFGBackBufferHandler;

class FXeFGViewExtension : public FXeSSCommonSceneViewExtensionBase
{
public:
	FXeFGViewExtension(const FAutoRegister& AutoRegister, class FXeFGRHI* InXeFGRHI, class FXeFGBackBufferHandler* InXeFGBackBufferHandler)
		: FXeSSCommonSceneViewExtensionBase(AutoRegister)
		, XeFGRHI(InXeFGRHI)
		, XeFGBackBufferHandler(InXeFGBackBufferHandler)
		{}
	bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override;
#if XESS_ENGINE_WITH_XEFG_API & XESS_ENGINE_WITH_XEFG_API_SVE_POST_RENDER_VIEW_RDG
	void PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override;
#else
	void SubscribeToPostProcessingPass_Unified(EPostProcessingPass Pass, const FSceneView* InOptionalView, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled) override;
	FScreenPassTexture PostProcessPassAfterVisualizeDepthOfField_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs);
#endif
#if XESS_ENGINE_VERSION_EQU(5, 4)
	void PreRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override;
#endif
private:
	FXeFGRHI* XeFGRHI;
	FXeFGBackBufferHandler* XeFGBackBufferHandler;
};
#endif
