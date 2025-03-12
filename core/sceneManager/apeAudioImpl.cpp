#include "apeAudioImpl.h"

ape::AudioImpl::AudioImpl(std::string name, bool replicate, std::string ownerID, bool isHost)
    : ape::IAudio(name, replicate, ownerID), ape::Replica("Audio", name, ownerID, isHost)
{
    mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
    mpSceneManager = ape::ISceneManager::getSingletonPtr();
    mAudioChunks = std::deque<std::vector<uint8_t>>();
    mMaxChunks = 2;
}

ape::AudioImpl::~AudioImpl()
{
}

std::vector<uint8_t> ape::AudioImpl::getLastChunkData()
{
    // std::lock_guard<std::mutex> lock(mMutex);
    return mAudioChunks.empty() ? std::vector<uint8_t>{} : mAudioChunks.back();
}

void ape::AudioImpl::appendAudioData(const std::vector<uint8_t>& newAudioData)
{
    // std::lock_guard<std::mutex> lock(mMutex);

    // if we have reached the maximum number of chunks, remove the oldest one
    if (mAudioChunks.size() >= mMaxChunks)
    {
        mAudioChunks.pop_front();
    }

    mAudioChunks.push_back(newAudioData);
    mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHUNK_LOAD));
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
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, mAudioChunks);
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, mMaxChunks);
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, mPlayingChunkIndex);
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
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mAudioChunks))
    {
        mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHUNK_LOAD));
    }
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mMaxChunks))
    {
        mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHUNK_MAX));
    }
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mPlayingChunkIndex))
    {
        mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHUNK_INDEX));
    }
    mVariableDeltaSerializer.EndDeserialize(&deserializationContext);
}
