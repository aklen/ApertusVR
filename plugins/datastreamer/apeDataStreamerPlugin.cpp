#include "apeDataStreamerPlugin.h"

ape::DataStreamerPlugin::DataStreamerPlugin()
{
    APE_LOG_FUNC_ENTER();
    mpCoreConfig = ape::ICoreConfig::getSingletonPtr();
    mpSceneManager = ape::ISceneManager::getSingletonPtr();
    mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
    mpEventManagerImpl->connectEvent(ape::Event::Group::AUDIO, std::bind(&DataStreamerPlugin::eventCallback, this, std::placeholders::_1));
    mChunkSize = 4 * 1024; // Default chunk size
    APE_LOG_FUNC_LEAVE();
}

ape::DataStreamerPlugin::~DataStreamerPlugin()
{
    APE_LOG_FUNC_ENTER();
    if (mAudioFile.is_open())
    {
        mAudioFile.close();
    }
    APE_LOG_FUNC_LEAVE();
}

void ape::DataStreamerPlugin::Init()
{
    APE_LOG_FUNC_ENTER();

    // // create an audio entity
    // if (auto audio = std::static_pointer_cast<ape::IAudio>(mpSceneManager->createEntity("audio_test", ape::Entity::AUDIO, true, mpCoreConfig->getNetworkGUID()).lock())) {
    //     int channels = audio->getChannels();
    //     APE_LOG_DEBUG("[DataStreamerPlugin]::Init() Audio channels: " << channels);

    //     // load the first chunk of the audio file
    //     audio->setFilePath("/Users/aklen/Music/Ableton/Projects/647 Project/export/647.mp3");
    //     bool firstLoaded = audio->loadNextAudioChunk(APE_AUDIO_CHUNK_SIZE_1MB);
    //     APE_LOG_DEBUG("[DataStreamerPlugin]::Init() First chunk loaded: " << firstLoaded);
    // }

    APE_LOG_FUNC_LEAVE();
}

void ape::DataStreamerPlugin::Run()
{
    APE_LOG_FUNC_ENTER();
    // while (true)
    // {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(20));
    // }
    APE_LOG_FUNC_LEAVE();
}

void ape::DataStreamerPlugin::Step()
{
    APE_LOG_FUNC_ENTER();
    APE_LOG_FUNC_LEAVE();
}

void ape::DataStreamerPlugin::Stop()
{
    APE_LOG_FUNC_ENTER();
    APE_LOG_FUNC_LEAVE();
}

void ape::DataStreamerPlugin::Suspend()
{
    APE_LOG_FUNC_ENTER();
    APE_LOG_FUNC_LEAVE();
}

void ape::DataStreamerPlugin::Restart()
{
    APE_LOG_FUNC_ENTER();
    APE_LOG_FUNC_LEAVE();
}

void ape::DataStreamerPlugin::eventCallback(const ape::Event& event)
{
    if (event.type == ape::Event::Type::AUDIO_CHUNK_REQUEST)
    {
        APE_LOG_DEBUG("[DataStreamerPlugin] Received request to load next chunk.");
        loadNextChunk();
    }
}

void ape::DataStreamerPlugin::loadFirstChunk()
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mAudioFile.is_open())
    {
        mAudioFile.close();
    }

    mAudioFile.open(mFilePath, std::ios::binary);
    if (!mAudioFile)
    {
        APE_LOG_ERROR("[DataStreamerPlugin] Error: Failed to open file: " << mFilePath);
        return;
    }

    loadNextChunk();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    loadNextChunk();
}

void ape::DataStreamerPlugin::loadNextChunk()
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (!mAudioFile.is_open() || mAudioFile.eof())
    {
        APE_LOG_DEBUG("[DataStreamerPlugin] End of file reached.");
        return;
    }

    std::vector<uint8_t> buffer(mChunkSize);
    mAudioFile.read(reinterpret_cast<char*>(buffer.data()), mChunkSize);
    size_t bytesRead = mAudioFile.gcount();
    buffer.resize(bytesRead);

    if (auto audio = mAudio.lock())
    {
        audio->appendAudioData(buffer);
    }
}