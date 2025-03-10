#include "apeAudioImpl.h"

ape::AudioImpl::AudioImpl(std::string name, bool replicate, std::string ownerID, bool isHost)
    : ape::IAudio(name, replicate, ownerID), ape::Replica("Audio", name, ownerID, isHost)
{
    mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
    mpSceneManager = ape::ISceneManager::getSingletonPtr();
    mAudioData = std::vector<uint8_t>();
    mSampleRate = 44100; // Default sample rate
    mChannels = 2; // Default stereo
    mMaxBufferSize = 4 * 1024 * 1024; // Default is 4 MB
    mFilePosition = 0;
    mDataSize = 0;
}

ape::AudioImpl::~AudioImpl()
{
}

std::vector<uint8_t> ape::AudioImpl::getAudioData()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mAudioData;
}

void ape::AudioImpl::setAudioData(const std::vector<uint8_t>& audioData)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mAudioData = audioData;
    mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_DATA));
}

void ape::AudioImpl::appendAudioData(const std::vector<uint8_t>& newAudioData)
{
    std::lock_guard<std::mutex> lock(mMutex);
    APE_LOG_DEBUG("[AudioImpl]::appendAudioData() Appending " << newAudioData.size() << " bytes of audio data.");

    // append new audio data to the end of the buffer
    mAudioData.insert(mAudioData.end(), newAudioData.begin(), newAudioData.end());

    // if buffer size exceeds the maximum allowed size, remove the oldest data
    if (mAudioData.size() > mMaxBufferSize)
    {
        size_t excess = mAudioData.size() - mMaxBufferSize;
        mAudioData.erase(mAudioData.begin(), mAudioData.begin() + excess);
    }

    mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_DATA));
}

void ape::AudioImpl::setFilePath(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(mMutex);
    APE_LOG_DEBUG("[AudioImpl]::setFilePath() Setting file path: " << filePath);
    if (mAudioFile.is_open())
    {
        APE_LOG_DEBUG("[AudioImpl]::setFilePath() Closing previous file.");
        mAudioFile.close();
    }

    mFilePath = filePath;
    mAudioFile.open(filePath, std::ios::binary);
    if (!mAudioFile)
    {
        APE_LOG_ERROR("[AudioImpl]::setFilePath() Error: Failed to open file: " << filePath);
        return;
    }
    APE_LOG_DEBUG("[AudioImpl]::setFilePath() File opened successfully.");

    mFilePosition = 0;
    mAudioFile.seekg(0, std::ios::end);
    mDataSize = mAudioFile.tellg();
    mAudioFile.seekg(0, std::ios::beg);

    APE_LOG_DEBUG("[AudioImpl]::setFilePath() File size: " << mDataSize);
}

bool ape::AudioImpl::loadNextAudioChunk(size_t chunkSize)
{
    std::vector<uint8_t> buffer(chunkSize);

    {
        std::lock_guard<std::mutex> lock(mMutex);
        APE_LOG_DEBUG("[AudioImpl]::loadNextAudioChunk() Loading next audio chunk...");

        if (!mAudioFile.is_open() || mFilePosition >= mDataSize)
        {
            APE_LOG_DEBUG("[AudioImpl]::loadNextAudioChunk() End of file reached.");
            return false;
        }
        APE_LOG_DEBUG("[AudioImpl]::loadNextAudioChunk() File position: " << mFilePosition);

        mAudioFile.seekg(mFilePosition, std::ios::beg);
        mAudioFile.read(reinterpret_cast<char*>(buffer.data()), chunkSize);
        size_t bytesRead = mAudioFile.gcount();
        APE_LOG_DEBUG("[AudioImpl]::loadNextAudioChunk() Read " << bytesRead << " bytes of audio data.");
        buffer.resize(bytesRead);

        mFilePosition += bytesRead;
    } // release lock

    appendAudioData(buffer);
    mLastChunkData = buffer;
    APE_LOG_DEBUG("[AudioImpl]::loadNextAudioChunk() Appended " << buffer.size() << " bytes of audio data.");
    mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHUNK_LOAD));
    return buffer.size() > 0;
}

std::vector<uint8_t> ape::AudioImpl::getLastChunkData()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mLastChunkData;
}

size_t ape::AudioImpl::getCurrentStreamPosition()
{
    return mFilePosition;
}

void ape::AudioImpl::seekTo(size_t newPosition)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (newPosition >= mDataSize)
    {
        APE_LOG_WARNING("[AudioImpl] Warning: Seek position out of range, setting to end of file.");
        newPosition = mDataSize;
    }

    mFilePosition = newPosition;
    mAudioData.clear();
    loadNextAudioChunk(mMaxBufferSize / 2);

    APE_LOG_DEBUG("[AudioImpl] Seeked to new position: " << newPosition);
}

int ape::AudioImpl::getSampleRate()
{
    return mSampleRate;
}

void ape::AudioImpl::setSampleRate(int sampleRate)
{
    mSampleRate = sampleRate;
    mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_SAMPLE_RATE));
}

int ape::AudioImpl::getChannels()
{
    return mChannels;
}

void ape::AudioImpl::setChannels(int channels)
{
    mChannels = channels;
    mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHANNELS));
}

bool ape::AudioImpl::isStreaming() {
    return mStreaming;
}

void ape::AudioImpl::setStreaming(bool streaming) {
    mStreaming = streaming;
    mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_STREAMING));
}

std::string ape::AudioImpl::getOwner() {
    return mOwnerID;
}

void ape::AudioImpl::setOwner(std::string ownerID) {
    mOwnerID = ownerID;
}

void ape::AudioImpl::WriteAllocationID(RakNet::Connection_RM3* destinationConnection, RakNet::BitStream* allocationIdBitstream) const
{
    allocationIdBitstream->Write(mObjectType);
    allocationIdBitstream->Write(RakNet::RakString(mName.c_str()));
    allocationIdBitstream->Write(RakNet::RakString(mOwnerID.c_str()));
}

RakNet::RM3SerializationResult ape::AudioImpl::Serialize(RakNet::SerializeParameters* serializeParameters)
{
    RakNet::VariableDeltaSerializer::SerializationContext serializationContext;
    serializeParameters->pro[0].reliability = RELIABLE_ORDERED;
    mVariableDeltaSerializer.BeginIdenticalSerialize(&serializationContext, serializeParameters->whenLastSerialized == 0, &serializeParameters->outputBitstream[0]);
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mName.c_str()));
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, mSampleRate);
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, mChannels);
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, mAudioData);
    mVariableDeltaSerializer.EndSerialize(&serializationContext);
    return RakNet::RM3SR_BROADCAST_IDENTICALLY_FORCE_SERIALIZATION;
}

void ape::AudioImpl::Deserialize(RakNet::DeserializeParameters* deserializeParameters)
{
    RakNet::VariableDeltaSerializer::DeserializationContext deserializationContext;
    mVariableDeltaSerializer.BeginDeserialize(&deserializationContext, &deserializeParameters->serializationBitstream[0]);
    RakNet::RakString name;
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, name))
    {
        mName = name.C_String();
    }
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mSampleRate))
    {
        mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_SAMPLE_RATE));
    }
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mChannels))
    {
        mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHANNELS));
    }
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mAudioData))
    {
        mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_DATA));
    }
    mVariableDeltaSerializer.EndDeserialize(&deserializationContext);
}
