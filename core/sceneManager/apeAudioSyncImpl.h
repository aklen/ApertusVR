#ifndef APE_AUDIOSYNCIMPL_H
#define APE_AUDIOSYNCIMPL_H

#include "apeIAudioSync.h"
#include "apeReplica.h"
#include "apeEventManagerImpl.h"
#include "apeISceneManager.h"
#include <chrono>
#include <mutex>

namespace ape
{
    class AudioSyncImpl : public ape::IAudioSync, public ape::Replica
    {
    public:
        AudioSyncImpl(std::string name, bool replicate, std::string ownerID, bool isHost);
        ~AudioSyncImpl();

        std::chrono::milliseconds getPlaybackTime() override;
        void setPlaybackTime(std::chrono::milliseconds timestamp) override;

        void WriteAllocationID(RakNet::Connection_RM3* destinationConnection, RakNet::BitStream* allocationIdBitstream) const override;
        RakNet::RM3SerializationResult Serialize(RakNet::SerializeParameters* serializeParameters) override;
        void Deserialize(RakNet::DeserializeParameters* deserializeParameters) override;

    private:
        ape::EventManagerImpl* mpEventManagerImpl;
        ape::ISceneManager* mpSceneManager;
        std::chrono::milliseconds mPlaybackTime;
        std::mutex mMutex;
        bool mIsHostMachine;
        bool modified;
    };
}

#endif // APE_AUDIOSYNCIMPL_H
