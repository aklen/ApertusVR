#ifndef APE_ICONFIGMANAGER_H
#define APE_ICONFIGMANAGER_H

#ifdef _WIN32
	#ifdef BUILDING_APE_CONFIGMANAGER_DLL
		#define APE_CONFIGMANAGER_DLL_EXPORT __declspec(dllexport)
	#else
		#define APE_CONFIGMANAGER_DLL_EXPORT __declspec(dllimport)
	#endif
#else
	#define APE_CONFIGMANAGER_DLL_EXPORT
#endif

#include <functional>
#include "apeEvent.h"
#include "apeSingleton.h"
#include "apeConfigNode.h"
#include <string>

namespace ape
{
    class APE_CONFIGMANAGER_DLL_EXPORT IConfigManager : public Singleton<IConfigManager>
    {
    protected:
        virtual ~IConfigManager() {};

    public:
        virtual bool loadJson(const std::string& filePath, ConfigNode& config) = 0;
    };
}

#endif
