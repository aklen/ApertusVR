#include "apeAudioImpl.h"
#include <zlib.h> // CRC32

uint32_t calculateCRC32(const std::vector<uint8_t>& data)
{
    return crc32(0L, data.data(), data.size());
}

ape::AudioImpl::AudioImpl(std::string name, bool replicate, std::string ownerID, bool isHost)
    : ape::IAudio(name, replicate, ownerID), ape::Replica("Audio", name, ownerID, isHost)
{
    mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
    mpSceneManager = ape::ISceneManager::getSingletonPtr();
    mAudioChunks = std::deque<std::vector<uint8_t>>();
    mMaxChunks = 2;
    modified = false;
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
    modified = true;

    APE_LOG_DEBUG("AudioImpl::appendAudioData() chunk added (size: " << newAudioData.size() << "), firing event...");
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
    if (!modified)
        return RakNet::RM3SR_DO_NOT_SERIALIZE;

    APE_LOG_DEBUG("AudioImpl::Serialize() called");
    RakNet::VariableDeltaSerializer::SerializationContext serializationContext;
    serializeParameters->pro[0].reliability = RELIABLE_ORDERED;
    mVariableDeltaSerializer.BeginIdenticalSerialize(&serializationContext, serializeParameters->whenLastSerialized == 0, &serializeParameters->outputBitstream[0]);

    // save the name of the audio
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mName.c_str()));

    // save the number of audio chunks
    size_t audioChunkCount = mAudioChunks.size();
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, audioChunkCount);

    // save each audio chunk
    for (const auto& chunk : mAudioChunks)
    {
        size_t chunkSize = chunk.size();
        mVariableDeltaSerializer.SerializeVariable(&serializationContext, chunkSize);

        for (const auto& byte : chunk)
        {
            mVariableDeltaSerializer.SerializeVariable(&serializationContext, byte);
        }

        // calculate the CRC32 hash of the chunk
        uint32_t chunkHash = calculateCRC32(chunk);
        APE_LOG_DEBUG("AudioImpl::Serialize() chunk size: " << chunkSize << ", CRC32: " << chunkHash);
    }

    // save the maximum number of chunks
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, mMaxChunks);

    // save the current playing chunk index
    mVariableDeltaSerializer.SerializeVariable(&serializationContext, mPlayingChunkIndex);

    modified = false;
    mVariableDeltaSerializer.EndSerialize(&serializationContext);
    return RakNet::RM3SR_BROADCAST_IDENTICALLY_FORCE_SERIALIZATION;
}

void ape::AudioImpl::Deserialize(RakNet::DeserializeParameters* deserializeParameters)
{
    APE_LOG_DEBUG("AudioImpl::Deserialize() called");
    RakNet::VariableDeltaSerializer::DeserializationContext deserializationContext;
    mVariableDeltaSerializer.BeginDeserialize(&deserializationContext, &deserializeParameters->serializationBitstream[0]);

    // load the name of the audio
    RakNet::RakString name;
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, name))
    {
        mName = name.C_String();
    }

    // load the number of audio chunks
    size_t audioChunkCount = 0;
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, audioChunkCount))
    {
        if (audioChunkCount > 0)  // only clear the chunks if we have some to load
        {
            APE_LOG_DEBUG("AudioImpl::Deserialize() clearing audio chunks (before: " << mAudioChunks.size() << ")");
            mAudioChunks.clear();
        }
        else {
            APE_LOG_WARNING("AudioImpl::Deserialize() no audio chunks to load");
        }

        for (size_t i = 0; i < audioChunkCount; i++)
        {
            size_t chunkSize = 0;
            if (!mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, chunkSize))
            {
                APE_LOG_ERROR("AudioImpl::Deserialize() ERROR: Failed to deserialize chunk size!");
                return;
            }
         
            std::vector<uint8_t> chunk(chunkSize);
            for (size_t j = 0; j < chunkSize; j++)
            {
                if (!mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, chunk[j]))
                {
                    APE_LOG_ERROR("AudioImpl::Deserialize() ERROR: Failed to deserialize chunk data at index " << j);
                    return;
                }
            }

            uint32_t chunkHash = calculateCRC32(chunk);
            APE_LOG_DEBUG("AudioImpl::Deserialize() chunk size: " << chunk.size() << ", CRC32: " << chunkHash);

            mAudioChunks.push_back(chunk);
        }
        mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHUNK_LOAD));
    }

    // load the maximum number of chunks
    size_t oldMaxChunks = mMaxChunks;
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mMaxChunks))
    {
        if (oldMaxChunks != mMaxChunks) {
            mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHUNK_MAX));
        }
    }

    // load the current playing chunk index
    size_t newPlayingChunkIndex = 0;
    if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, newPlayingChunkIndex))
    {
        if (newPlayingChunkIndex != mPlayingChunkIndex) {
            mPlayingChunkIndex = newPlayingChunkIndex;
            mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::AUDIO_CHUNK_INDEX));
        }
    }

    mVariableDeltaSerializer.EndDeserialize(&deserializationContext);
}
