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

#include <chrono>
#include <random>
#include <thread>
#include "apePlatform.h"
#include "apeSystem.h"
#include "apeEventManagerImpl.h"
#include "apeLogManagerImpl.h"
#include "apePluginManagerImpl.h"
#include "apeSceneManagerImpl.h"
#include "apeCoreConfigImpl.h"
#include "apeConfigManagerImpl.h"

ape::PluginManagerImpl* gpPluginManagerImpl;
ape::EventManagerImpl* gpEventManagerImpl;
ape::LogManagerImpl* gpLogManagerImpl;
ape::SceneManagerImpl* gpSceneManagerImpl;
ape::CoreConfigImpl* gpCoreConfigImpl;
ape::ConfigManagerImpl* gpConfigManagerImpl;

void ape::System::Start(const char* configFolderPath, bool isBlocking, std::function<void()> userThreadFunction, int step_interval)
{
	gpLogManagerImpl = new LogManagerImpl();
	APE_LOG("ApertusVR - Your open source AR/VR engine for science, education and industry");
	APE_LOG("Build Target Platform: " << APE_PLATFORM_STRING);
	APE_LOG("-----------------------------------------------------------------------------");
	gpCoreConfigImpl = new CoreConfigImpl(std::string(configFolderPath));
	gpConfigManagerImpl = new ConfigManagerImpl();
	gpEventManagerImpl = new EventManagerImpl();
	gpSceneManagerImpl = new SceneManagerImpl();
	gpPluginManagerImpl = new PluginManagerImpl();

	gpPluginManagerImpl->CreatePlugins();
	gpPluginManagerImpl->InitAndRunPlugins();

	if (userThreadFunction)
		gpPluginManagerImpl->registerUserThreadFunction(userThreadFunction);

	if (isBlocking)
		gpPluginManagerImpl->joinThreads();
	else
		gpPluginManagerImpl->detachThreads();
	APE_LOG_TRACE("ape::System::Start() after joinThreads() || detachThreads()");

	std::this_thread::sleep_for(std::chrono::milliseconds(step_interval));
	gpPluginManagerImpl->callStepFunc();
}

void ape::System::Stop()
{
	std::stringstream ssOut;
	APE_LOG_INFO("ape::System::Stop() stopping plugins...");
	gpPluginManagerImpl->StopPlugins();
	APE_LOG_INFO("ape::System::Stop() plugins stopped");

	if (gpPluginManagerImpl)
	{
		APE_LOG_TRACE("PluginManager deleting...");
		delete gpPluginManagerImpl;
		gpPluginManagerImpl = nullptr;
		APE_LOG_INFO("PluginManager deleted");
	}

	if (gpEventManagerImpl)
	{
		APE_LOG_TRACE("EventManager deleting...");
		delete gpEventManagerImpl;
		gpEventManagerImpl = nullptr;
		APE_LOG_INFO("EventManager deleted");
	}

	if (gpSceneManagerImpl)
	{
		APE_LOG_TRACE("SceneManager deleting...");
		delete gpSceneManagerImpl;
		gpSceneManagerImpl = nullptr;
		APE_LOG_INFO("SceneManager deleted");
	}
	
	if (gpCoreConfigImpl)
	{
		APE_LOG_TRACE("CoreConfig deleting...");
		delete gpCoreConfigImpl;
		gpCoreConfigImpl = nullptr;
		APE_LOG_INFO("CoreConfig deleted");
	}

	if (gpConfigManagerImpl)
	{
		APE_LOG_TRACE("ConfigManager deleting...");
		delete gpConfigManagerImpl;
		gpConfigManagerImpl = nullptr;
		APE_LOG_INFO("ConfigManager deleted");
	}

	if (gpLogManagerImpl)
	{
		APE_LOG_TRACE("LogManager deleting...");
		delete gpLogManagerImpl;
		gpLogManagerImpl = nullptr;
		ssOut << "INFO:  LogManager deleted" << std::endl;
	}

	ssOut << "INFO:  All systems stopped and destroyed";
	std::cout << ssOut.str() << std::endl;
}
