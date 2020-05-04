/*MIT License

Copyright (c) 2018 MTA SZTAKI

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#ifndef APE_OPENXRPLUGIN_H
#define APE_OPENXRPLUGIN_H

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <d3d11.h>
#include "openxr/openxr.h"
#include "openxr/openxr_platform.h"
#include "apePluginAPI.h"
#include "apeIEventManager.h"
#include "apeILogManager.h"
#include "apeISceneManager.h"
#include "apeICoreConfig.h"
#include "apeMatrix4.h"
#include "apeICamera.h"
#include "apeIConeGeometry.h"
#include "apeIFileGeometry.h"
#include "apeIFileMaterial.h"
#include "apeIIndexedFaceSetGeometry.h"
#include "apeIManualMaterial.h"
#include "apeIManualMaterial.h"
#include "apeIManualTexture.h"
#include "apeINode.h"
#include "apeITextGeometry.h"
#include "apeUserInputMacro.h"

#define FOR_EACH_EXTENSION_FUNCTION(_)           \
    _(xrCreateSpatialAnchorMSFT)                 \
    _(xrCreateSpatialAnchorSpaceMSFT)            \
    _(xrDestroySpatialAnchorMSFT)                \
    _(xrConvertWin32PerformanceCounterToTimeKHR) \
    _(xrConvertTimeToWin32PerformanceCounterKHR) \
    _(xrGetD3D11GraphicsRequirementsKHR)         \
    _(xrGetVisibilityMaskKHR)

#define GET_INSTANCE_PROC_ADDRESS(name) (void)xrGetInstanceProcAddr(instance, #name, (PFN_xrVoidFunction*)((PFN_##name*)(&result.name)));
#define DEFINE_PROC_MEMBER(name) PFN_##name name;

struct XrExtTable {
	FOR_EACH_EXTENSION_FUNCTION(DEFINE_PROC_MEMBER);
};

inline XrExtTable xrCreateExtensionTable(XrInstance instance) {
	XrExtTable result = {};
	FOR_EACH_EXTENSION_FUNCTION(GET_INSTANCE_PROC_ADDRESS);
	return result;
}

#define DeclarePrivateType(name) struct _ ## name; typedef struct _ ## name *name;

#define THIS_PLUGINNAME "apeOpenXRPlugin"

namespace ape
{
	class OpenXRPlugin : public ape::IPlugin
	{
	public:
		DeclarePrivateType(tex_t);
		enum tex_format_ 
		{
			tex_format_rgba32 = 0,
			tex_format_rgba32_linear,
			tex_format_rgba64,
			tex_format_rgba128,
			tex_format_depthstencil,
			tex_format_depth32,
			tex_format_depth16,
		};
		struct swapchain_t 
		{
			XrSwapchain handle;
			int32_t width;
			int32_t height;
			std::vector<XrSwapchainImageD3D11KHR> surface_images;
			std::vector<tex_t> surface_data;
		};
	private:
		ape::IEventManager* mpEventManager;

		ape::ISceneManager* mpSceneManager;

		ape::ICoreConfig* mpCoreConfig;

		ape::UserInputMacro* mpApeUserInputMacro;

		XrInstance mOpenXRInstance;

		XrExtTable mOpenXRExtensions;

		XrSession mOpenXRSession;

		XrSpace mOpenXRAppSpace;

		XrTime mOpenXRTime;

		bool mOpenXRDepthLsr;

		XrFormFactor mOpenXRAppConfigForm;

		XrSystemId mOpenXRSystemID;

		XrViewConfigurationType mOpenXRAppConfigView;

		XrEnvironmentBlendMode mOpenXRBlend;

		XrReferenceSpaceType mOpenXRRefSpace;

		XrSpace mOpenXRHeadSpace;

		std::vector<XrViewConfigurationView> mOpenXRConfigViews;

		std::vector<ape::Matrix4> mOpenXRViewPointView;

		std::vector<ape::Matrix4> mOpenXRViewPointProjection;

		swapchain_t mOpenXRSwapchains;

		XrSessionState mOpenXRSessionState;

		bool mIsOpenXRRunning;

		void eventCallBack(const ape::Event& event);

		void openXRPreferredExtensions(uint32_t &out_extension_count, const char **out_extensions);

		XrReferenceSpaceType openXRPreferredSpace();

		void openXRPreferredFormat(DXGI_FORMAT &out_pixel_format, tex_format_ &out_depth_format);

		bool openXRLocValid(XrSpaceLocation &loc);

		void openXRPollEvents();

		void openXRPollActions();

		void openXRRenderFrame();

	public:
		OpenXRPlugin();

		~OpenXRPlugin();

		void Init() override;

		void Run() override;

		void Step() override;

		void Stop() override;

		void Suspend() override;

		void Restart() override;
	};

	APE_PLUGIN_FUNC ape::IPlugin* CreateapeOpenXRPlugin()
	{
		return new ape::OpenXRPlugin;
	}

	APE_PLUGIN_FUNC void DestroyapeOpenXRPlugin(ape::IPlugin *plugin)
	{
		delete (ape::OpenXRPlugin*)plugin;
	}

	APE_PLUGIN_DISPLAY_NAME(THIS_PLUGINNAME);

	APE_PLUGIN_ALLOC()
	{
		APE_LOG_DEBUG(THIS_PLUGINNAME << "_CREATE");
		apeRegisterPlugin(THIS_PLUGINNAME, CreateapeOpenXRPlugin, DestroyapeOpenXRPlugin);
		return 0;
	}
}
#endif
