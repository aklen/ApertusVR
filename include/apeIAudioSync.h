#ifndef APE_IAUDIOSYNC_H
#define APE_IAUDIOSYNC_H

#include <chrono>
#include "apeEntity.h"

namespace ape
{
    class IAudioSync : public ape::Entity
    {
    protected:
        IAudioSync(std::string name, bool replicate, std::string ownerID)
            : ape::Entity(name, ape::Entity::AUDIO_SYNC, replicate, ownerID) {}
        virtual ~IAudioSync() = default;

    public:
        virtual std::chrono::milliseconds getPlaybackTime() = 0;
        virtual void setPlaybackTime(std::chrono::milliseconds timestamp) = 0;

    };

    typedef std::shared_ptr<ape::IAudioSync> AudioSyncSharedPtr;
    typedef std::weak_ptr<ape::IAudioSync> AudioSyncWeakPtr;
}

#endif // APE_IAUDIOSYNC_H