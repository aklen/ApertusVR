#include "apeAudioSyncImpl.h"

ape::AudioSyncImpl::AudioSyncImpl(std::string name, bool replicate, std::string ownerID, bool isHost)
    : ape::IAudioSync(name, replicate, ownerID), ape::Replica("AudioSync", name, ownerID, isHost), mIsHostMachine(isHost)
{
    mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
    mpSceneManager = ape::ISceneManager::getSingletonPtr();
    mPlaybackTime = std::chrono::milliseconds(0);
    modified = false;
}

ape::AudioSyncImpl::~AudioSyncImpl()
{
}

std::chrono::milliseconds ape::AudioSyncImpl::getPlaybackTime()
{
    // std::lock_guard<std::mutex> lock(mMutex);
    return mPlaybackTime;
}

void ape::AudioSyncImpl::setPlaybackTime(std::chrono::milliseconds time)
{
    if (!mIsHostMachine)
        return;

    std::lock_guard<std::mutex> lock(mMutex);
    mPlaybackTime = time;
    modified = true;
}

std::string ape::AudioSyncImpl::getOwner() {
    return mOwnerID;
}

void ape::AudioSyncImpl::setOwner(std::string ownerID) {
    mOwnerID = ownerID;
}

void ape::AudioSyncImpl::WriteAllocationID(RakNet::Connection_RM3* destinationConnection, RakNet::BitStream* allocationIdBitstream) const
{
    allocationIdBitstream->Write(mObjectType);
    allocationIdBitstream->Write(RakNet::RakString(mName.c_str()));
    allocationIdBitstream->Write(RakNet::RakString(mOwnerID.c_str()));
}

RakNet::RM3SerializationResult ape::AudioSyncImpl::Serialize(RakNet::SerializeParameters* serializeParameters)
{
    if (!modified)
        return RakNet::RM3SR_DO_NOT_SERIALIZE;

    APE_LOG_DEBUG("AudioSyncImpl::Serialize() sending timestamp: " << mPlaybackTime.count());

    serializeParameters->whenLastSerialized = 0; // Force full update
    RakNet::VariableDeltaSerializer::SerializationContext serializationContext;
    serializeParameters->pro[0].reliability = RELIABLE_ORDERED;

    mVariableDeltaSerializer.BeginIdenticalSerialize(&serializationContext, true, &serializeParameters->outputBitstream[0]);
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, mPlaybackTime.count());
    mVariableDeltaSerializer.EndSerialize(&serializationContext);

    modified = false;
    return RakNet::RM3SR_BROADCAST_IDENTICALLY_FORCE_SERIALIZATION;
}


void ape::AudioSyncImpl::Deserialize(RakNet::DeserializeParameters* deserializeParameters)
{
    APE_LOG_DEBUG("AudioSyncImpl::Deserialize() called");

    RakNet::VariableDeltaSerializer::DeserializationContext deserializationContext;
    mVariableDeltaSerializer.BeginDeserialize(&deserializationContext, &deserializeParameters->serializationBitstream[0]);

    int64_t receivedTimestamp = 0;
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, receivedTimestamp))
    {
        APE_LOG_DEBUG("AudioSyncImpl::Deserialize() received timestamp: " << receivedTimestamp << " ms");
        // std::lock_guard<std::mutex> lock(mMutex);
        mPlaybackTime = std::chrono::milliseconds(receivedTimestamp);
        mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_SYNC_PLAYBACK_TIME));
    }

    mVariableDeltaSerializer.EndDeserialize(&deserializationContext);
}
