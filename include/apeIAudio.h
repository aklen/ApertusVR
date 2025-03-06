#ifndef APE_IAUDIO_H
#define APE_IAUDIO_H

#include <string>
#include <vector>
#include "apeEntity.h"

namespace ape
{
    class IAudio : public ape::Entity
    {
    protected:
        IAudio(std::string name, bool replicate, std::string ownerID) 
            : ape::Entity(name, ape::Entity::AUDIO, replicate, ownerID) {}

        virtual ~IAudio() {}

    public:
        virtual std::vector<uint8_t> getAudioData() = 0;

        virtual void setAudioData(const std::vector<uint8_t>& data) = 0;

        virtual int getSampleRate() = 0;

        virtual void appendAudioData(const std::vector<uint8_t>& newAudioData) = 0;

        virtual void setSampleRate(int sampleRate) = 0;

        virtual int getChannels() = 0;

        virtual void setChannels(int channels) = 0;

        virtual void setStreaming(bool streaming) = 0;

        virtual bool isStreaming() = 0;

        virtual void setOwner(std::string ownerID) = 0;

        virtual std::string getOwner() = 0;
    };

    typedef std::shared_ptr<ape::IAudio> AudioSharedPtr;
    typedef std::weak_ptr<ape::IAudio> AudioWeakPtr;
}

#endif // APE_IAUDIO_H
