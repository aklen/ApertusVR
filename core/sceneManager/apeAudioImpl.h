#ifndef APE_AUDIOIMPL_H
#define APE_AUDIOIMPL_H

#include "apeIAudio.h"
#include "apeISceneManager.h"
#include "apeEventManagerImpl.h"
#include "apeReplica.h"
#include <mutex>

namespace ape
{
    class AudioImpl : public ape::IAudio, public ape::Replica
    {
    public:
        AudioImpl(std::string name, bool replicate, std::string ownerID, bool isHost);

        ~AudioImpl();

        std::vector<uint8_t> getAudioData() override;

        void setAudioData(const std::vector<uint8_t>& data) override;

        void appendAudioData(const std::vector<uint8_t>& newAudioData) override;

        int getSampleRate() override;

        void setSampleRate(int sampleRate) override;

        int getChannels() override;

        void setChannels(int channels) override;

        void setStreaming(bool streaming) override;

        bool isStreaming() override;

        void setOwner(std::string ownerID) override;

        std::string getOwner() override;

        void WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const override;

        RakNet::RM3SerializationResult Serialize(RakNet::SerializeParameters *serializeParameters) override;

        void Deserialize(RakNet::DeserializeParameters *deserializeParameters) override;

    private:
        ape::EventManagerImpl* mpEventManagerImpl;
        ape::ISceneManager* mpSceneManager;

        std::mutex mMutex;

        std::vector<uint8_t> mAudioData;
        bool mStreaming;

        int mSampleRate;
        int mChannels;
        int mMaxBufferSize;
    };
}

#endif
