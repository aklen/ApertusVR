#ifndef APE_AUDIOIMPL_H
#define APE_AUDIOIMPL_H

#include "apeIAudio.h"
#include "apeISceneManager.h"
#include "apeEventManagerImpl.h"
#include "apeReplica.h"
#include <mutex>
#include <deque>
#include <vector>
#include <fstream>

namespace ape
{
    class AudioImpl : public ape::IAudio, public ape::Replica
    {
    public:
        AudioImpl(std::string name, bool replicate, std::string ownerID, bool isHost);
        ~AudioImpl();

        std::vector<uint8_t> getLastChunkData() override;
        void appendAudioData(const std::vector<uint8_t>& newAudioData) override;

        void setOwner(std::string ownerID) override;
        std::string getOwner() override;
        void WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const override;
        RakNet::RM3SerializationResult Serialize(RakNet::SerializeParameters *serializeParameters) override;
        void Deserialize(RakNet::DeserializeParameters *deserializeParameters) override;

    private:
        ape::EventManagerImpl* mpEventManagerImpl;
        ape::ISceneManager* mpSceneManager;

        std::mutex mMutex;
        std::deque<std::vector<uint8_t>> mAudioChunks;
        size_t mMaxChunks; // Maximum number of chunks to keep in memory
        size_t mPlayingChunkIndex; // Current position in the audio data
    };
}

#endif
