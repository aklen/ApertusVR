#include "apeAudioImpl.h"

ape::AudioImpl::AudioImpl(std::string name, bool replicate, std::string ownerID, bool isHost)
    : ape::IAudio(name, replicate, ownerID), ape::Replica("Audio", name, ownerID, isHost)
{
    mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
    mpSceneManager = ape::ISceneManager::getSingletonPtr();
    mAudioData = std::vector<uint8_t>();
    mSampleRate = 44100; // Default sample rate
    mChannels = 2;       // Default stereo
    mMaxBufferSize = 4 * 1024 * 1024; // 4 MB
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
