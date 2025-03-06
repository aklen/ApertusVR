#ifndef APE_GSTREAMERPLUGIN_H
#define APE_GSTREAMERPLUGIN_H

#include <chrono>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <vector>
#include "apePluginAPI.h"
#include "apeIEventManager.h"
#include "apeEventManagerImpl.h"
#include "apeILogManager.h"
#include "apeICoreConfig.h"
#include <gst/gst.h>

#define THIS_PLUGINNAME "apeGStreamerPlugin"

namespace ape
{
	class apeGStreamerPlugin : public ape::IPlugin
	{
	public:
		apeGStreamerPlugin();
		~apeGStreamerPlugin();

		void Init() override;
		void Run() override;
		void Step() override;
		void Stop() override;
		void Suspend() override;
		void Restart() override;

		void PlayAudio(const std::string& uri);  // Play a file or URL
		void StopAudio(bool force = false);  // Stop the current playback
		void PauseAudio();  // Pause the current playback
		void ResumeAudio();  // Resume the current playback

	private:
		ape::IEventManager* mpEventManager;
		ape::EventManagerImpl* mpEventManagerImpl;
		ape::ICoreConfig* mpCoreConfig;
		void eventCallBack(const ape::Event& event);

		GstElement* pipeline;
		std::atomic<bool> running;
		std::thread gstThread;

		static void OnBusMessage(GstBus* bus, GstMessage* msg, gpointer data);
		void GStreamerMainLoop();
	};

	APE_PLUGIN_FUNC ape::IPlugin* CreateapeGStreamerPlugin()
	{
		return new ape::apeGStreamerPlugin;
	}

	APE_PLUGIN_FUNC void DestroyapeGStreamerPlugin(ape::IPlugin *plugin)
	{
		delete (ape::apeGStreamerPlugin*)plugin;
	}

	APE_PLUGIN_DISPLAY_NAME(THIS_PLUGINNAME);

	APE_PLUGIN_ALLOC()
	{
		APE_LOG_DEBUG(THIS_PLUGINNAME << "_CREATE");
		apeRegisterPlugin(THIS_PLUGINNAME, CreateapeGStreamerPlugin, DestroyapeGStreamerPlugin);
		return 0;
	}
}

#endif
