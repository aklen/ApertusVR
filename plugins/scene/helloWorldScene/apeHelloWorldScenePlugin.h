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

#ifndef APE_HELLOWORLDSCENEPLUGIN_H
#define APE_HELLOWORLDSCENEPLUGIN_H

#include "apeICamera.h"
#include "apeICommand.h"
#include "apeICommandResponse.h"
#include "apeIConeGeometry.h"
#include "apeICoreConfig.h"
#include "apeIEventManager.h"
#include "apeIFileGeometry.h"
#include "apeIFileMaterial.h"
#include "apeIIndexedFaceSetGeometry.h"
#include "apeIIndexedLineSetGeometry.h"
#include "apeILight.h"
#include "apeILogManager.h"
#include "apeIManualMaterial.h"
#include "apeINode.h"
#include "apeIPlaneGeometry.h"
#include "apeIPointCloud.h"
#include "apeISceneManager.h"
#include "apeISceneNetwork.h"
#include "apeISphereGeometry.h"
#include "apeITextGeometry.h"
#include "apeITubeGeometry.h"
#include "apeInterpolator.h"
#include "apePluginAPI.h"
#include "apeUtils.h"
#include <chrono>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#define THIS_PLUGINNAME "apeHelloWorldScenePlugin"

namespace ape
{
	class apeHelloWorldScenePlugin : public ape::IPlugin
	{
	private:
		ape::IEventManager* mpEventManager;

		ape::ISceneManager* mpSceneManager;

		ape::ICoreConfig* mpCoreConfig;

		ape::CommandWeakPtr mCommand;

		std::string mGuid;

		std::string mUserName;

		bool mIsHost;

		bool mIsStopped;

		std::mutex stopMutex;

		void eventCallBack(const ape::Event& event);

	public:
		apeHelloWorldScenePlugin();

		~apeHelloWorldScenePlugin();

		void Init() override;

		void Run() override;

		void Step() override;

		void Stop() override;

		void Suspend() override;

		void Restart() override;
	};

	APE_PLUGIN_FUNC ape::IPlugin* CreateapeHelloWorldScenePlugin()
	{
		return new ape::apeHelloWorldScenePlugin;
	}

	APE_PLUGIN_FUNC void DestroyapeHelloWorldScenePlugin(ape::IPlugin* plugin)
	{
		delete (ape::apeHelloWorldScenePlugin*)plugin;
	}

	APE_PLUGIN_DISPLAY_NAME(THIS_PLUGINNAME);

	APE_PLUGIN_ALLOC()
	{
		APE_LOG_DEBUG(THIS_PLUGINNAME << "_CREATE");
		apeRegisterPlugin(THIS_PLUGINNAME, CreateapeHelloWorldScenePlugin, DestroyapeHelloWorldScenePlugin);
		return 0;
	}
}

#endif
