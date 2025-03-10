#ifndef APE_CONFIGMANAGERIMPL_H
#define APE_CONFIGMANAGERIMPL_H

#ifdef _WIN32
#ifdef BUILDING_APE_CONFIGMANAGER_DLL
#define APE_CONFIGMANAGER_DLL_EXPORT __declspec(dllexport)
#else
#define APE_CONFIGMANAGER_DLL_EXPORT __declspec(dllimport)
#endif
#else
#define APE_CONFIGMANAGER_DLL_EXPORT 
#endif

#include "apeIConfigManager.h"
#include "apeConfigNode.h"
#include "apeILogManager.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <string>

namespace ape
{
    class APE_CONFIGMANAGER_DLL_EXPORT ConfigManagerImpl : public IConfigManager
    {
    public:
        ConfigManagerImpl();
        ~ConfigManagerImpl();

        bool loadJson(const std::string& filePath, ConfigNode& config) override;

    private:
        void parseJsonToConfigNode(const rapidjson::Value& jsonValue, ConfigNode& node, const std::string& key = "");
        ILogManager* mpLogManager;
    };
}

#endif
