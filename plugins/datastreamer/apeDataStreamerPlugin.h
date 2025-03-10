#ifndef APE_DATASTREAMERPLUGIN_H
#define APE_DATASTREAMERPLUGIN_H

#include "apePluginAPI.h"
#include "apeICoreConfig.h"
#include "apeEventManagerImpl.h"
#include "apeISceneManager.h"
#include "apeIAudio.h"
#include <fstream>
#include <vector>
#include <string>
#include <mutex>
#include <thread>

#define THIS_PLUGINNAME "apeDataStreamerPlugin"

namespace ape
{
    class DataStreamerPlugin : public ape::IPlugin
    {
    public:
        DataStreamerPlugin();

        ~DataStreamerPlugin();

        void Init() override;

        void Run() override;

        void Step() override;

        void Stop() override;

        void Suspend() override;

        void Restart() override;

    private:
        void eventCallback(const ape::Event& event);

        void loadFirstChunk();

        void loadNextChunk();

        std::ifstream mAudioFile;
        std::mutex mMutex;
        ape::AudioWeakPtr mAudio;
        size_t mChunkSize;
        std::string mFilePath;

        ape::ISceneManager* mpSceneManager;
        ape::EventManagerImpl* mpEventManagerImpl;
        ape::ICoreConfig* mpCoreConfig;
    };

    APE_PLUGIN_FUNC ape::IPlugin* CreateDataStreamerPlugin()
	{
		return new ape::DataStreamerPlugin;
	}

	APE_PLUGIN_FUNC void DestroyDataStreamerPlugin(ape::IPlugin *plugin)
	{
		delete (ape::DataStreamerPlugin*)plugin;
	}

	APE_PLUGIN_DISPLAY_NAME(THIS_PLUGINNAME);

	APE_PLUGIN_ALLOC()
	{
		APE_LOG_DEBUG(THIS_PLUGINNAME << "_CREATE");
		apeRegisterPlugin(THIS_PLUGINNAME, CreateDataStreamerPlugin, DestroyDataStreamerPlugin);
		return 0;
	}
}

#endif // APE_DATASTREAMERPLUGIN_H