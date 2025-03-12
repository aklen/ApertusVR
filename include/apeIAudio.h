#ifndef APE_IAUDIO_H
#define APE_IAUDIO_H

#include <string>
#include <vector>
#include "apeEntity.h"

#define APE_AUDIO_CHUNK_SIZE_128KB       128 * 1024
#define APE_AUDIO_CHUNK_SIZE_256KB       256 * 1024
#define APE_AUDIO_CHUNK_SIZE_512KB       512 * 1024
#define APE_AUDIO_CHUNK_SIZE_1MB    1 * 1024 * 1024
#define APE_AUDIO_CHUNK_SIZE_2MB    2 * 1024 * 1024
#define APE_AUDIO_CHUNK_SIZE_4MB    4 * 1024 * 1024
#define APE_AUDIO_CHUNK_SIZE_8MB    8 * 1024 * 1024

namespace ape
{
    class IAudio : public ape::Entity
    {
    protected:
        IAudio(std::string name, bool replicate, std::string ownerID) 
            : ape::Entity(name, ape::Entity::AUDIO, replicate, ownerID) {}

        virtual ~IAudio() {}

    public:
        virtual std::vector<uint8_t> getLastChunkData() = 0;
        virtual void appendAudioData(const std::vector<uint8_t>& newAudioData) = 0;
        virtual void setOwner(std::string ownerID) = 0;
        virtual std::string getOwner() = 0;
    };

    typedef std::shared_ptr<ape::IAudio> AudioSharedPtr;
    typedef std::weak_ptr<ape::IAudio> AudioWeakPtr;
}

#endif // APE_IAUDIO_H
