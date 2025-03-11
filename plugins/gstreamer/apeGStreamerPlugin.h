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
#include <atomic>
#include <string>
#include "apePluginAPI.h"
#include "apeIEventManager.h"
#include "apeEventManagerImpl.h"
#include "apeISceneManager.h"
#include "apeILogManager.h"
#include "apeIConfigManager.h"
#include "apeICoreConfig.h"
#include "apeIAudio.h"
#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <gobject/gsignal.h>

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

		// URI-based playback (playbin)
		void PlayAudio(const std::string& uri);

		// Chunk-based playback (appsrc → decodebin → audioconvert → audioresample → autoaudiosink)
		void PlayAudioChunk(const std::vector<uint8_t>& audioData);

		void StopAudio(bool force = false);  // Stop the current playback
		void PauseAudio();  // Pause the current playback
		void ResumeAudio();  // Resume the current playback

		std::string GetCurrentAudioEntityId();

		GstElement* getAppSrc();
		ape::ISceneManager* getSceneManager();
		GstElement* GetPipelineUri();
		GstElement* GetPipelineChunk();

		void StopPipeline(GstElement* pipeline);
		void RemovePiplineBusWatch(GstElement* pipeline);
		void ShutdownPipeline(GstElement* pipeline);
		void UnrefPipeline(GstElement*& pipeline);
		void DestroyPipeline(GstElement* pipeline);
		void StopAppSrc(GstElement* appsrc);

	private:
		ape::ICoreConfig* mpCoreConfig;
		ape::IConfigManager* mpConfigManager;
		ape::IEventManager* mpEventManager;
		ape::EventManagerImpl* mpEventManagerImpl;
		ape::ISceneManager* mpSceneManager;
		void eventCallBack(const ape::Event& event);
		ConfigNode mConfig;

		GstElement* pipeline_uri;  // URI alapú lejátszáshoz
        GstElement* pipeline_chunk;  // Chunk alapú lejátszáshoz
		GstElement* appsrc;
		
		std::atomic<bool> running;
		std::thread gstThread;

		std::string mCurrentAudioEntityId;

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
