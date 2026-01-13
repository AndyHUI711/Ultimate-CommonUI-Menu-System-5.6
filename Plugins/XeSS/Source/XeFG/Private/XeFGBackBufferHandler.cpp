/*******************************************************************************
 * Copyright 2024 Intel Corporation
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

#include "XeFGBackBufferHandler.h"

#if XESS_ENGINE_WITH_XEFG_API
#include "Framework/Application/SlateApplication.h"
#include "RenderGraphBuilder.h"
#include "Rendering/SlateRenderer.h"
#include "RHICommandList.h"
#include "SceneRendering.h"
#include "ShaderParameterMacros.h"
#include "XeFGModule.h"
#include "XeSSPrePass.h"
#include "XeFGRHI.h"

static TAutoConsoleVariable<int32> CVarXeFGTagUI(
	TEXT("r.XeFG.TagUI"),
	1,
	TEXT("[default: 1] Tag UI color and alpha."),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<float> CVarXeFGTagUIAlphaThreshold(
	TEXT("r.XeFG.TagUIAlphaThreshold"),
	0.0,
	TEXT("[default: 0.0] UI extraction pass alpha threshold value."),
	ECVF_RenderThreadSafe);

BEGIN_SHADER_PARAMETER_STRUCT(FXeFGExtractUIShaderParameters, )
SHADER_PARAMETER_RDG_TEXTURE(Texture2D, UIColorAndAlpha)
RDG_TEXTURE_ACCESS(DummyOutput, ERHIAccess::UAVCompute)
END_SHADER_PARAMETER_STRUCT()

FXeFGViewInfo::FXeFGViewInfo(const FViewInfo& ViewInfo)
{
	UnscaledViewRect = ViewInfo.UnscaledViewRect;
}

FXeFGBackBufferHandler::FXeFGBackBufferHandler(FXeFGRHI* InXeFGRHI)
	: XeFGRHI(InXeFGRHI)
{
	check(FSlateApplication::IsInitialized());
	FDelegateHandle OnBackBufferReadyToPresentHandle = FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().AddRaw(this,
		&FXeFGBackBufferHandler::OnBackBufferReadyToPresent
	);
	// Remove here, for it does not work inside destructor
	FSlateApplication::Get().OnPreShutdown().AddLambda([OnBackBufferReadyToPresentHandle] {
		FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().Remove(OnBackBufferReadyToPresentHandle);
	});
}

FXeFGBackBufferHandler::~FXeFGBackBufferHandler()
{
}

void FXeFGBackBufferHandler::OnBackBufferReadyToPresent(SWindow& InWindow, const XeSSUnreal::XTextureRHIRef& InBackBuffer)
{
	// Only game window supported now
	if (InWindow.GetType() != EWindowType::GameWindow)
	{
		return;
	}
	if (ViewInfos.Num() == 0)
	{
		// Error log will be seen if XeFG is enabled: Error: XeFG: Invalid argument. Missing resource of type XEFG_RES_BACK_BUFFER
		// But no better way to remove it.
		return;
	}
	// Fetch the first view info and empty all
	const FIntRect ViewRect = ViewInfos[0].UnscaledViewRect;
	ViewInfos.Empty();
	if (!XeFGRHI->IsXeFGEnabled() || !XeFGRHI->IfGameViewportUsesXeFGSwapChain())
	{
		return;
	}
	float AlphaThreshold = CVarXeFGTagUIAlphaThreshold.GetValueOnRenderThread();
	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
	FRDGBuilder GraphBuilder(RHICmdList);
	FXeFGExtractUIShaderParameters* PassParameters = GraphBuilder.AllocParameters<FXeFGExtractUIShaderParameters>();
	FXeFGRHI* LocalXeFGRHI = XeFGRHI;
	FRDGTexture* UITexture = nullptr;
	if (CVarXeFGTagUI->GetBool())
	{
		UITexture = AddXeFGExtractUIPass(GraphBuilder, AlphaThreshold, InBackBuffer);
	}
	else
	{
		// HACK to generate black only texture
		// 2.0f is a value larger than maximum alpha value (1.0f)
		UITexture = AddXeFGExtractUIPass(GraphBuilder, 2.0f, InBackBuffer);
	}
	PassParameters->UIColorAndAlpha = UITexture;
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("XeFG::Tag UI"),
		PassParameters,
		ERDGPassFlags::Compute | ERDGPassFlags::Raster | ERDGPassFlags::SkipRenderPass | ERDGPassFlags::NeverCull,
		[LocalXeFGRHI, PassParameters, ViewRect](FRHICommandListImmediate& RHICmdList)
		{
			PassParameters->UIColorAndAlpha->MarkResourceAsUsed();
			FRHITexture* UITexture = PassParameters->UIColorAndAlpha->GetRHI();
			uint32 FrameID = GFrameNumberRenderThread;

			RHICmdList.EnqueueLambda(
				[LocalXeFGRHI, UITexture, ViewRect, FrameID](FRHICommandListImmediate& Cmd)
				{
					LocalXeFGRHI->TagUI(FrameID, UITexture, ViewRect, Cmd);
					LocalXeFGRHI->TagBackBuffer(FrameID, UITexture, ViewRect, Cmd);
				}
			);
		}
	);
	GraphBuilder.Execute();
}

void FXeFGBackBufferHandler::AddViewInfo(const FViewInfo& ViewInfo)
{
	ViewInfos.Empty();  // Workaround for window mode switch, due to only one view is supported now
	ViewInfos.Add(FXeFGViewInfo(ViewInfo));
}
#endif
