#include "apeGStreamerPlugin.h"

ape::apeGStreamerPlugin::apeGStreamerPlugin()
 : pipeline(nullptr), running(false)
{
	APE_LOG_FUNC_ENTER();
	mpCoreConfig = ape::ICoreConfig::getSingletonPtr();
	mpEventManager = ape::IEventManager::getSingletonPtr();
    mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
    // subscribe to events here
    gst_init(nullptr, nullptr);
	APE_LOG_FUNC_LEAVE();
}

ape::apeGStreamerPlugin::~apeGStreamerPlugin()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeGStreamerPlugin::eventCallBack(const ape::Event& event)
{
}

void ape::apeGStreamerPlugin::Init()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeGStreamerPlugin::Run()
{
	APE_LOG_FUNC_ENTER();
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	APE_LOG_FUNC_LEAVE();
}

void ape::apeGStreamerPlugin::Step()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeGStreamerPlugin::Stop()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeGStreamerPlugin::Suspend()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeGStreamerPlugin::Restart()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeGStreamerPlugin::PlayAudio(const std::string& uri) {
    std::string cleanedUri = uri;
    if (cleanedUri.empty()) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Play() Invalid file path: " << cleanedUri);
        return;
    }

    APE_LOG_DEBUG("[GStreamerPlugin]::Play() Original URI: " << uri);
    APE_LOG_DEBUG("[GStreamerPlugin]::Play() Cleaned URI: " << cleanedUri);

    pipeline = gst_parse_launch(("playbin uri=" + cleanedUri).c_str(), nullptr);
    if (!pipeline) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Play() Failed to create pipeline!");
        return;
    }

    APE_LOG_DEBUG("[GStreamerPlugin]::Play() Setting up GStreamer bus...");
    GstBus* bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)OnBusMessage, this);
    gst_object_unref(bus);
    APE_LOG_DEBUG("[GStreamerPlugin]::Play() Bus watch added.");

    // start playback in a separate thread
    std::thread([this] {
        APE_LOG_DEBUG("[GStreamerPlugin]::Play() Changing state to PLAYING...");
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        running = true;

        mpEventManagerImpl->fireEvent(ape::Event("PlaybackStarted", ape::Event::Type::AUDIO_PLAYBACK_STATE));
        APE_LOG_DEBUG("[GStreamerPlugin]::Play() Playback started.");
    }).detach();
}

void ape::apeGStreamerPlugin::PauseAudio() {
    if (pipeline && running) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Pause() Pausing playback...");
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
    }
}

void ape::apeGStreamerPlugin::ResumeAudio() {
    if (pipeline && running) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Resume() Resuming playback...");
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    }
}

void ape::apeGStreamerPlugin::StopAudio(bool force) {
    if (!running && !force) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Stop called, but playback is already stopped.");
        return;
    }

    APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Stopping playback...");
    running = false; // Mark playback as stopped

    if (pipeline) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Changing state to NULL...");
        gst_element_set_state(pipeline, GST_STATE_NULL); // Stop the pipeline
        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Pipeline state changed to NULL.");


        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Removing bus watch...");
        GstBus* bus = gst_element_get_bus(pipeline);
        gst_bus_remove_watch(bus);
        gst_object_unref(bus);
        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Bus watch removed.");


        // Send a final message to the pipeline
        gst_element_post_message(pipeline, gst_message_new_application(GST_OBJECT(pipeline), gst_structure_new_empty("shutdown")));


        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Unref'ing pipeline...");
        gst_object_unref(pipeline); // Unref the pipeline
        pipeline = nullptr; // Set the pipeline to null, prevent double-free
        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Pipeline stopped and freed.");


        mpEventManagerImpl->fireEvent(ape::Event("PlaybackStopped", ape::Event::Type::AUDIO_PLAYBACK_STATE));
    }

    APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Checking if GStreamer thread should join...");
    if (gstThread.joinable()) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Joining playback thread...");
        gstThread.join();
        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Playback thread joined.");
    } else {
        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() No thread to join.");
    }
    APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Playback stopped.");
}

void ape::apeGStreamerPlugin::OnBusMessage(GstBus* bus, GstMessage* msg, gpointer data) {
    ape::apeGStreamerPlugin* plugin = static_cast<ape::apeGStreamerPlugin*>(data);

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            APE_LOG_DEBUG("[GStreamerPlugin]::OnBusMessage End of stream reached!");
            plugin->StopAudio();
            break;
        case GST_MESSAGE_ERROR: {
            GError* err;
            gchar* debug;
            gst_message_parse_error(msg, &err, &debug);
            APE_LOG_DEBUG("[GStreamerPlugin]::OnBusMessage Error: " << err->message);
            // TODO: trigger an error event here
            g_error_free(err);
            g_free(debug);
            plugin->StopAudio();
            break;
        }
        case GST_MESSAGE_STATE_CHANGED: {
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(plugin->pipeline)) { 
                // Get old and new states
                GstState old_state, new_state, pending;
                gst_message_parse_state_changed(msg, &old_state, &new_state, &pending);

                // Log state change
                APE_LOG_DEBUG("[GStreamerPlugin]::OnBusMessage State changed from "
                              << gst_element_state_get_name(old_state) << " to "
                              << gst_element_state_get_name(new_state));
            }
            break;
        }
        default:
            break;
    }
}
