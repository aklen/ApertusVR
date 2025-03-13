#include "apeGStreamerPlugin.h"
#include "apeUtils.h"

const char* EventGroupToString(ape::Event::Group group) {
    switch (group) {
        case ape::Event::Group::AUDIO: return "AUDIO";
        case ape::Event::Group::AUDIO_SYNC: return "AUDIO_SYNC";
        default: return "OTHER";
    }
}

const std::string EventTypeToString(ape::Event::Type type) {
    std::string result = "";
    switch (type) {
        case ape::Event::Type::AUDIO_CREATE:
            result = "AUDIO_CREATE";
            break;
        case ape::Event::Type::AUDIO_DELETE: 
            result = "AUDIO_DELETE";
            break;
        case ape::Event::Type::AUDIO_PLAYBACK_STATE: 
            result = "AUDIO_PLAYBACK_STATE";
            break;
        case ape::Event::Type::AUDIO_DATA:
            result = "AUDIO_DATA";
            break;
        case ape::Event::Type::AUDIO_DATA_SIZE:
            result = "AUDIO_DATA_SIZE";
            break;
        case ape::Event::Type::AUDIO_SAMPLE_RATE:
            result = "AUDIO_SAMPLE_RATE";
            break;
        case ape::Event::Type::AUDIO_CHANNELS:
            result = "AUDIO_CHANNELS";
            break;
        case ape::Event::Type::AUDIO_CHUNK_LOAD:
            result = "AUDIO_CHUNK_LOAD";
            break;
        case ape::Event::Type::AUDIO_CHUNK_REQUEST:
            result = "AUDIO_CHUNK_REQUEST";
            break;
        case ape::Event::Type::AUDIO_CHUNK_MAX:
            result = "AUDIO_CHUNK_MAX";
            break;
        case ape::Event::Type::AUDIO_CHUNK_INDEX:
            result = "AUDIO_CHUNK_INDEX";
            break;
        case ape::Event::Type::AUDIO_SEEK_POSITION:
            result = "AUDIO_SEEK_POSITION";
            break;
        case ape::Event::Type::AUDIO_STREAMING:
            result = "AUDIO_STREAMING";
            break;
        case ape::Event::Type::AUDIO_END_OF_STREAM:
            result = "AUDIO_END_OF_STREAM";
            break;
        case ape::Event::Type::AUDIO_SYNC_CREATE:
            result = "AUDIO_SYNC_CREATE";
            break;
        case ape::Event::Type::AUDIO_SYNC_PLAYBACK_TIME:
            result = "AUDIO_SYNC_PLAYBACK_TIME";
            break;
        default:
            result = "OTHER (" + std::to_string(type) + ")";
            break;
    }
    return result;
}

static void on_pad_added(GstElement* src, GstPad* new_pad, gpointer data)
{
    GstElement* sink = GST_ELEMENT(data);
    GstPad* sink_pad = gst_element_get_static_pad(sink, "sink");

    if (gst_pad_is_linked(sink_pad)) {
        g_object_unref(sink_pad);
        return;
    }

    GstPadLinkReturn ret = gst_pad_link(new_pad, sink_pad);
    if (GST_PAD_LINK_FAILED(ret)) {
        g_printerr("Type is '%s' but link failed.\n", GST_PAD_NAME(new_pad));
    } else {
        g_print("Link succeeded (type '%s').\n", GST_PAD_NAME(new_pad));
    }

    g_object_unref(sink_pad);
}

static void on_need_data(GstElement* src, guint size, gpointer user_data)
{
    ape::apeGStreamerPlugin* plugin = static_cast<ape::apeGStreamerPlugin*>(user_data);
    if (!plugin) return;

    GstState state;
    gst_element_get_state(plugin->GetPipelineChunk(), &state, nullptr, GST_CLOCK_TIME_NONE);
    if (state != GST_STATE_PLAYING) {
        APE_LOG_DEBUG("[GStreamerPlugin]::on_need_data() Ignoring need-data, pipeline not in PLAYING state.");
        return;
    }

    if (plugin->IsHost()) {
        // APE_LOG_DEBUG("[GStreamerPlugin]::on_need_data() Check if more data needed...");
        // std::string currentAudioEntityId = plugin->GetCurrentAudioEntityId();

        APE_LOG_DEBUG("[GStreamerPlugin]::on_need_data() Host GStreamer, requesting more data...");
        // if (auto audio = std::static_pointer_cast<ape::IAudio>(plugin->getSceneManager()->getEntity(currentAudioEntityId).lock())) {
            if (plugin->loadNextAudioChunk(APE_AUDIO_CHUNK_SIZE_1MB)) {
                APE_LOG_DEBUG("[GStreamerPlugin]::on_need_data() Loaded next chunk.");
            }
            else {
                APE_LOG_DEBUG("[GStreamerPlugin]::on_need_data() No more chunks, sending EOS.");

                // old method
                // GstBus* bus = gst_element_get_bus(plugin->GetPipelineChunk());
                // GstMessage* eos_msg = gst_message_new_eos(GST_OBJECT(src));
                // gst_bus_post(bus, eos_msg);
                // gst_object_unref(bus);

                // new method
                plugin->getEventManager()->fireEvent(ape::Event(plugin->GetCurrentAudioEntityId(), ape::Event::Type::AUDIO_END_OF_STREAM));
            }
        // }
    }
}

ape::apeGStreamerPlugin::apeGStreamerPlugin()
 : pipeline_uri(nullptr), pipeline_chunk(nullptr), running(false)
{
	APE_LOG_FUNC_ENTER();
    mpCoreConfig = ape::ICoreConfig::getSingletonPtr();
    mpConfigManager = ape::IConfigManager::getSingletonPtr();
	mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
    mpEventManagerImpl->connectEvent(ape::Event::Group::AUDIO, std::bind(&apeGStreamerPlugin::eventCallBack, this, std::placeholders::_1));
    mpEventManagerImpl->connectEvent(ape::Event::Group::AUDIO_SYNC, std::bind(&apeGStreamerPlugin::eventCallBack, this, std::placeholders::_1));
    mpSceneManager = ape::ISceneManager::getSingletonPtr();
    mCurrentAudioEntityId = "";
    mCurrentAudioSyncEntityId = "";
    mIsHost = mpCoreConfig->getNetworkConfig().participant == SceneNetwork::ParticipantType::HOST;

    // GStreamer initialization
    gst_init(nullptr, nullptr);

    // initialize pipeline for URI-based audio playback
    {
        pipeline_uri = gst_element_factory_make("playbin", "uri-pipeline");
        if (!pipeline_uri) {
            APE_LOG_DEBUG("[GStreamerPlugin]::Constructor() Failed to create URI pipeline!");
            return;
        }

        GstBus* bus = gst_element_get_bus(pipeline_uri);
        gst_bus_add_watch(bus, (GstBusFunc)OnBusMessage, this);
        gst_object_unref(bus);
    }

    // initialize pipeline for chunk-based audio playback
    {
        pipeline_chunk = gst_pipeline_new("audio-pipeline");
        appsrc = gst_element_factory_make("appsrc", "audio-source");
        GstElement* decodebin = gst_element_factory_make("decodebin", "decoder");
        GstElement* audioconvert = gst_element_factory_make("audioconvert", "converter");
        GstElement* audioresample = gst_element_factory_make("audioresample", "resampler");
        GstElement* autoaudiosink = gst_element_factory_make("autoaudiosink", "audio-output");

        if (!pipeline_chunk || !appsrc || !decodebin || !audioconvert || !audioresample || !autoaudiosink) {
            APE_LOG_DEBUG("[GStreamerPlugin]::Constructor() Failed to create elements!");
            return;
        }

        gst_bin_add_many(GST_BIN(pipeline_chunk), appsrc, decodebin, audioconvert, audioresample, autoaudiosink, nullptr);
        gst_element_link_many(appsrc, decodebin, nullptr);
        gst_element_link_many(audioconvert, audioresample, autoaudiosink, nullptr);

        g_signal_connect(decodebin, "pad-added", G_CALLBACK(on_pad_added), audioconvert);
        g_signal_connect(appsrc, "need-data", G_CALLBACK(on_need_data), this);

        GstBus* bus = gst_element_get_bus(pipeline_chunk);
        gst_bus_add_watch(bus, (GstBusFunc)OnBusMessage, this);
        gst_object_unref(bus);
    }

	APE_LOG_FUNC_LEAVE();
}

ape::apeGStreamerPlugin::~apeGStreamerPlugin()
{
	APE_LOG_FUNC_ENTER();

    StopAudio(true);
    StopAppSrc(appsrc);
    DestroyPipeline(pipeline_chunk);
    DestroyPipeline(pipeline_uri);
    gst_deinit();

    APE_LOG_DEBUG("[GStreamerPlugin]::Destructor() Playback stopped.");
	APE_LOG_FUNC_LEAVE();
}

void ape::apeGStreamerPlugin::eventCallBack(const ape::Event& event)
{
    APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Received event>"
                  << " group: " << EventGroupToString(event.group)
                  << " type: " << EventTypeToString(event.type)
                  << " subject: " << event.subjectName);

    if (event.type == ape::Event::Type::AUDIO_CHUNK_LOAD) {
        if (auto audio = std::static_pointer_cast<ape::IAudio>(mpSceneManager->getEntity(event.subjectName).lock())) {
            PlayAudioChunk(audio->getLastChunkData());
        }
    }
    else if (event.type == ape::Event::Type::AUDIO_END_OF_STREAM) {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Received event> EOS.");
        GstBus* bus = gst_element_get_bus(GetPipelineChunk());
        GstMessage* eos_msg = gst_message_new_eos(GST_OBJECT(appsrc));
        gst_bus_post(bus, eos_msg);
        gst_object_unref(bus);
    }
    else if (event.type == ape::Event::Type::AUDIO_SYNC_CREATE) {
        if (auto audioSync = std::static_pointer_cast<ape::IAudioSync>(mpSceneManager->getEntity(event.subjectName).lock())) {
            mAudioSync = audioSync;
        }
    }
    else {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Ignoring event.");
    }
}

void ape::apeGStreamerPlugin::Init()
{
	APE_LOG_FUNC_ENTER();

    if (!mIsHost) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Init() Not host, skipping initialization.");
        return;
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    // plugin config
    if (mpConfigManager->loadJson(mpCoreConfig->getConfigFolderPath() + "/" + THIS_PLUGINNAME + ".json", mConfig)) {
        // mConfig.print();

        std::string audioFilePath = mConfig["audio"].getString("filePath");
        APE_LOG_DEBUG("[GStreamerPlugin]::Init() File path: " << audioFilePath);

        std::string audioFileName = ape::utils::getFileNameFromPath(audioFilePath);
        APE_LOG_DEBUG("[GStreamerPlugin]::Init() File name: " << audioFileName);

        std::string source = mConfig["audio"].getString("source");
        APE_LOG_DEBUG("[GStreamerPlugin]::Init() Source: " << source);

        // open the audio file
        std::lock_guard<std::mutex> lock(mAudioFileMutex);
        mAudioFile.open(audioFilePath, std::ios::binary);
        if (!mAudioFile) {
            APE_LOG_ERROR("[GStreamerPlugin]::Init() Failed to open file: " << audioFilePath);
            return;
        }

        // get the size of the audio file
        mAudioFile.seekg(0, std::ios::end);
        mAudioDataSize = mAudioFile.tellg();
        mAudioFile.seekg(0, std::ios::beg);
        mAudioFilePosition = 0;
        APE_LOG_DEBUG("[GStreamerPlugin]::Init() File opened successfully. Size: " << mAudioDataSize);

        // create an audio sync entity
        mCurrentAudioSyncEntityId = "audiosync_" + audioFileName;
        APE_LOG_DEBUG("[GStreamerPlugin]::Init() Creating AudioSync entity: " << mCurrentAudioSyncEntityId);
        if (auto audioSync = std::static_pointer_cast<ape::IAudioSync>(mpSceneManager->createEntity(mCurrentAudioSyncEntityId, ape::Entity::AUDIO_SYNC, true, mpCoreConfig->getNetworkGUID()).lock()))
        {
            APE_LOG_DEBUG("[GStreamerPlugin]::Init() AudioSync entity created.");
        }
        else {
            APE_LOG_ERROR("[GStreamerPlugin]::Init() Failed to create AudioSync entity!");
        }

        // create an audio entity
        mCurrentAudioEntityId = "audio_" + audioFileName;
        APE_LOG_DEBUG("[GStreamerPlugin]::Init() Creating audio entity: " << mCurrentAudioEntityId);
        if (auto audio = std::static_pointer_cast<ape::IAudio>(mpSceneManager->createEntity(mCurrentAudioEntityId, ape::Entity::AUDIO, true, mpCoreConfig->getNetworkGUID()).lock())) {
            // load the first chunk of the audio file
            APE_LOG_DEBUG("[GStreamerPlugin]::Init() Audio Entity created, loading first audio chunk...");
            bool firstLoaded = loadNextAudioChunk(APE_AUDIO_CHUNK_SIZE_1MB);
            APE_LOG_DEBUG("[GStreamerPlugin]::Init() First chunk loaded: " << firstLoaded);
        }
        else {
            APE_LOG_ERROR("[GStreamerPlugin]::Init() Failed to create Audio entity!");
        }
    }

	APE_LOG_FUNC_LEAVE();
}

bool ape::apeGStreamerPlugin::loadNextAudioChunk(size_t chunkSize)
{
    APE_LOG_DEBUG("[GStreamerPlugin]::loadNextAudioChunk() Loading next audio chunk...");
    if (!mIsHost) {
        APE_LOG_DEBUG("[GStreamerPlugin]::loadNextAudioChunk() Not host, skipping.");
        return false;
    }

    std::vector<uint8_t> buffer(chunkSize);
    size_t bytesRead = 0;

    {
        // std::lock_guard<std::mutex> lock(mAudioFileMutex);
        APE_LOG_DEBUG("[GStreamerPlugin]::loadNextAudioChunk() Checking file position...");

        if (!mAudioFile.is_open() || mAudioFilePosition >= mAudioDataSize)
        {
            APE_LOG_DEBUG("[GStreamerPlugin]::loadNextAudioChunk() End of file reached.");
            return false;
        }
        APE_LOG_DEBUG("[GStreamerPlugin]::loadNextAudioChunk() Reading file...");
        mAudioFile.seekg(mAudioFilePosition, std::ios::beg);
        mAudioFile.read(reinterpret_cast<char*>(buffer.data()), chunkSize);
        bytesRead = mAudioFile.gcount();
        APE_LOG_DEBUG("[GStreamerPlugin]::loadNextAudioChunk() Bytes read: " << bytesRead);
        buffer.resize(bytesRead);
        mAudioFilePosition += bytesRead;
        APE_LOG_DEBUG("[GStreamerPlugin]::loadNextAudioChunk() New file position: " << mAudioFilePosition);
    }

    if (bytesRead > 0)
    {
        APE_LOG_DEBUG("[GStreamerPlugin]::loadNextAudioChunk() Adding chunk to audio entity...");
        if (auto audio = std::static_pointer_cast<ape::IAudio>(mpSceneManager->getEntity(mCurrentAudioEntityId).lock())) {
            audio->appendAudioData(buffer);
        }
    }

    return bytesRead > 0;
}

void ape::apeGStreamerPlugin::Run()
{
	APE_LOG_FUNC_ENTER();
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

        bool isHostSyncActive = mIsHost && mAudioSync;
        bool isGuestSyncActive = !mIsHost && mAudioSync;

        GstState state;
        gst_element_get_state(pipeline_chunk, &state, nullptr, GST_CLOCK_TIME_NONE);
        
        bool isPlaying = (state == GST_STATE_PLAYING);

        APE_LOG_DEBUG("[GStreamerPlugin]::Run() Guest> GStreamer state: " << gst_element_state_get_name(state) 
                    << ", isPlaying: " << isPlaying
                    << ", isHost: " << mIsHost
                    << ", mAudioSync: " << (mAudioSync ? "true" : "false")
                    << ", isHostSyncActive: " << isHostSyncActive
                    << ", isGuestSyncActive: " << isGuestSyncActive);

        // Host sends playback time to clients
        if (isHostSyncActive && isPlaying)
        {
            std::chrono::milliseconds playbackTime = getCurrentGStreamerPlaybackTime();
            mAudioSync->setPlaybackTime(playbackTime);
            APE_LOG_DEBUG("[GStreamerPlugin]::Run() Host> Updated AudioSync timestamp: " << playbackTime.count() << " ms");
        }
        else if (isHostSyncActive)
        {
            APE_LOG_DEBUG("[GStreamerPlugin]::Run() Host> Skipping sync update, GStreamer is not in PLAYING state.");
        }

        // Clients adjust playback to host's timestamp
        if (isGuestSyncActive && isPlaying)
        {
            std::chrono::milliseconds receivedTime = mAudioSync->getPlaybackTime();
            APE_LOG_DEBUG("[GStreamerPlugin]::Run() Guest> Adjusting playback to received timestamp: " 
                        << receivedTime.count() << " ms");
            adjustGStreamerPlayback(receivedTime);
        }
        else if (isGuestSyncActive)
        {
            APE_LOG_DEBUG("[GStreamerPlugin]::Run() Guest> Skipping sync adjustment, GStreamer is not in PLAYING state.");
        }
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

    pipeline_uri = gst_element_factory_make("playbin", "pipeline");
    g_object_set(pipeline_uri, "uri", uri.c_str(), nullptr);

    std::thread([this] {
        APE_LOG_DEBUG("[GStreamerPlugin]::Play() Changing state to PLAYING...");
        gst_element_set_state(pipeline_uri, GST_STATE_PLAYING);
        running = true;

        mpEventManagerImpl->fireEvent(ape::Event("PlaybackStarted", ape::Event::Type::AUDIO_PLAYBACK_STATE));
        APE_LOG_DEBUG("[GStreamerPlugin]::Play() Playback started.");
    }).detach();
}

void ape::apeGStreamerPlugin::PlayAudioChunk(const std::vector<uint8_t>& audioData)
{
    APE_LOG_DEBUG("[GStreamerPlugin]::PlayAudioChunk() Playing audio chunk...");

    if (!pipeline_chunk) {
        APE_LOG_DEBUG("[GStreamerPlugin]::PlayAudioChunk() Pipeline not initialized!");
        return;
    }

    if (!appsrc) {
        APE_LOG_DEBUG("[GStreamerPlugin]::PlayAudioChunk() appsrc is not initialized!");
        return;
    }

    GstBuffer* buffer = gst_buffer_new_allocate(nullptr, audioData.size(), nullptr);
    
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memcpy(map.data, audioData.data(), audioData.size());
    gst_buffer_unmap(buffer, &map);

    GstFlowReturn ret = GST_FLOW_OK;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);

    if (ret != GST_FLOW_OK) {
        APE_LOG_ERROR("[GStreamerPlugin]::PlayAudioChunk() Error pushing buffer to appsrc!");
    }

    // std::thread([this] {
        GstState state;
        gst_element_get_state(pipeline_chunk, &state, nullptr, GST_CLOCK_TIME_NONE);
        if (state != GST_STATE_PLAYING) {
            APE_LOG_DEBUG("[GStreamerPlugin]::PlayAudioChunk() Changing state to PLAYING...");
            gst_element_set_state(pipeline_chunk, GST_STATE_PLAYING);
            running = true;

            mpEventManagerImpl->fireEvent(ape::Event("PlaybackStarted", ape::Event::Type::AUDIO_PLAYBACK_STATE));
            APE_LOG_DEBUG("[GStreamerPlugin]::PlayAudioChunk() Playback started.");
        }
        else {
            APE_LOG_DEBUG("[GStreamerPlugin]::PlayAudioChunk() Nothing to do, pipeline already playing.");
        }
    // }).detach();
}

void ape::apeGStreamerPlugin::PauseAudio() {
    APE_LOG_DEBUG("[GStreamerPlugin]::Pause() Pausing playback...");
    if (pipeline_uri && running) {
        gst_element_set_state(pipeline_uri, GST_STATE_PAUSED);
    }
    if (pipeline_chunk && running) {
        gst_element_set_state(pipeline_chunk, GST_STATE_PAUSED);
    }
}

void ape::apeGStreamerPlugin::ResumeAudio() {
    APE_LOG_DEBUG("[GStreamerPlugin]::Resume() Resuming playback...");
    if (pipeline_uri && running) {
        gst_element_set_state(pipeline_uri, GST_STATE_PLAYING);
    }
    if (pipeline_chunk && running) {
        gst_element_set_state(pipeline_chunk, GST_STATE_PLAYING);
    }
}

void ape::apeGStreamerPlugin::StopAudio(bool force) {
    if (!running && !force) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Stop called, but playback is already stopped.");
        return;
    }

    APE_LOG_DEBUG("[GStreamerPlugin]::Stop() Stopping playback...");
    running = false; // Mark playback as stopped

    // URI
    StopPipeline(pipeline_uri);

    // CHUNK
    StopAppSrc(appsrc);
    StopPipeline(pipeline_chunk);

    mpEventManagerImpl->fireEvent(ape::Event("PlaybackStopped", ape::Event::Type::AUDIO_PLAYBACK_STATE));
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
            GstElement* srcElement = GST_ELEMENT(GST_MESSAGE_SRC(msg));

            GstState old_state, new_state, pending;
            gst_message_parse_state_changed(msg, &old_state, &new_state, &pending);

            std::string srcName = gst_element_get_name(srcElement);

            if (plugin->pipeline_uri && srcElement == plugin->pipeline_uri) {
                APE_LOG_DEBUG("[GStreamerPlugin]::OnBusMessage (URI Playback) State changed: " 
                              << gst_element_state_get_name(old_state) << " → "
                              << gst_element_state_get_name(new_state));
            }
            else if (plugin->pipeline_chunk && srcElement == plugin->pipeline_chunk) {
                APE_LOG_DEBUG("[GStreamerPlugin]::OnBusMessage (Chunk Playback) State changed: " 
                              << gst_element_state_get_name(old_state) << " → "
                              << gst_element_state_get_name(new_state));
            }
            else {
                APE_LOG_DEBUG("[GStreamerPlugin]::OnBusMessage (Unknown Source: " << srcName << ") State changed: " 
                              << gst_element_state_get_name(old_state) << " → "
                              << gst_element_state_get_name(new_state));
            }

            break;
        }
        default:
            break;
    }
}

GstElement* ape::apeGStreamerPlugin::getAppSrc()
{
    return appsrc;
}

ape::ISceneManager* ape::apeGStreamerPlugin::getSceneManager()
{
    return mpSceneManager;
}

ape::EventManagerImpl* ape::apeGStreamerPlugin::getEventManager()
{
    return mpEventManagerImpl;
}

GstElement* ape::apeGStreamerPlugin::GetPipelineUri()
{
    return pipeline_uri;
}
		
GstElement* ape::apeGStreamerPlugin::GetPipelineChunk()
{
    return pipeline_chunk;
}

void ape::apeGStreamerPlugin::StopPipeline(GstElement* pipeline)
{
    if (!pipeline) {
        APE_LOG_DEBUG("[GStreamerPlugin]::StopPipeline() Pipeline is null.");
        return;
    }
    APE_LOG_DEBUG("[GStreamerPlugin]::StopPipeline() Changing state to NULL...");
    gst_element_set_state(pipeline, GST_STATE_NULL); // Stop the pipeline
    APE_LOG_DEBUG("[GStreamerPlugin]::StopPipeline() Pipeline state changed to NULL.");
}

void ape::apeGStreamerPlugin::RemovePiplineBusWatch(GstElement* pipeline)
{
    if (!pipeline) {
        APE_LOG_DEBUG("[GStreamerPlugin]::RemovePiplineBusWatch() Pipeline is null.");
        return;
    }

    APE_LOG_DEBUG("[GStreamerPlugin]::RemovePiplineBusWatch() Removing bus watch...");
    GstBus* bus = gst_element_get_bus(pipeline);
    gst_bus_remove_watch(bus);
    gst_object_unref(bus);
    APE_LOG_DEBUG("[GStreamerPlugin]::RemovePiplineBusWatch() Bus watch removed.");
}

void ape::apeGStreamerPlugin::ShutdownPipeline(GstElement* pipeline)
{
    if (!pipeline) {
        APE_LOG_DEBUG("[GStreamerPlugin]::ShutdownPipeline() Pipeline is null.");
        return;
    }

    APE_LOG_DEBUG("[GStreamerPlugin]::ShutdownPipeline() Sending EOS to pipeline...");
    // gst_element_send_event(pipeline, gst_event_new_eos()); // Send EOS to the pipeline
    gst_element_post_message(pipeline, gst_message_new_application(GST_OBJECT(pipeline), gst_structure_new_empty("shutdown")));
    APE_LOG_DEBUG("[GStreamerPlugin]::ShutdownPipeline() EOS sent.");
}

void ape::apeGStreamerPlugin::UnrefPipeline(GstElement*& pipeline)
{
    if (!pipeline) {
        APE_LOG_DEBUG("[GStreamerPlugin]::UnrefPipeline() Pipeline is null.");
        return;
    }

    APE_LOG_DEBUG("[GStreamerPlugin]::UnrefPipeline() Unref'ing pipeline...");
    gst_object_unref(pipeline); // Unref the pipeline
    pipeline = nullptr; // Set the pipeline to null, prevent double-free
    APE_LOG_DEBUG("[GStreamerPlugin]::UnrefPipeline() Pipeline unref'd.");
}

void ape::apeGStreamerPlugin::DestroyPipeline(GstElement* pipeline) {
    if (!pipeline) {
        APE_LOG_DEBUG("[GStreamerPlugin]::DestroyPipeline() Pipeline is null.");
        return;
    }

    StopPipeline(pipeline);
    RemovePiplineBusWatch(pipeline);
    ShutdownPipeline(pipeline);
    UnrefPipeline(pipeline);
}

void ape::apeGStreamerPlugin::StopAppSrc(GstElement* appsrc)
{
    if (!appsrc) {
        APE_LOG_DEBUG("[GStreamerPlugin]::DestroyAppSrc() appsrc is null.");
        return;
    }

    APE_LOG_DEBUG("[GStreamerPlugin]::Stop() CHUNK: Stopping appsrc...");

    // Flushing pipeline (prevents hanging)
    gst_element_send_event(appsrc, gst_event_new_flush_start());
    gst_element_send_event(appsrc, gst_event_new_flush_stop(TRUE));

    // deactivate appsrc pad
    GstPad* appsrc_pad = gst_element_get_static_pad(appsrc, "src");
    if (appsrc_pad) {
        gst_pad_set_active(appsrc_pad, FALSE);
        gst_object_unref(appsrc_pad);
    }

    // send EOS to appsrc
    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "end-of-stream", &ret);
    if (ret != GST_FLOW_OK) {
        APE_LOG_WARNING("[GStreamerPlugin]::Stop() CHUNK: Error sending EOS to appsrc!");
    }
}

std::string ape::apeGStreamerPlugin::GetCurrentAudioEntityId()
{
    return mCurrentAudioEntityId;
}

bool ape::apeGStreamerPlugin::IsHost()
{
    return mIsHost;
}

std::chrono::milliseconds ape::apeGStreamerPlugin::getCurrentGStreamerPlaybackTime()
{
    gint64 position = GST_CLOCK_TIME_NONE;
    if (pipeline_chunk) {
        gst_element_query_position(pipeline_chunk, GST_FORMAT_TIME, &position);
    }
    return std::chrono::milliseconds(GST_TIME_AS_MSECONDS(position));
}

void ape::apeGStreamerPlugin::seekGStreamerPlayback(std::chrono::milliseconds receivedTime)
{
    gst_element_seek_simple(pipeline_chunk, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, receivedTime.count() * GST_MSECOND);
    APE_LOG_DEBUG("[GStreamerPlugin]::seekGStreamerPlayback() Seeking to: " << receivedTime.count() << " ms");
}

void ape::apeGStreamerPlugin::adjustGStreamerPlayback(std::chrono::milliseconds receivedTime)
{
    gint64 position = GST_CLOCK_TIME_NONE;
    gst_element_query_position(pipeline_chunk, GST_FORMAT_TIME, &position);

    std::chrono::milliseconds currentTime(GST_TIME_AS_MSECONDS(position));
    std::chrono::milliseconds diff = receivedTime - currentTime;

    APE_LOG_DEBUG("[GStreamerPlugin]::adjustGStreamerPlayback() Current Time: " << currentTime.count() 
                  << " ms, Received Time: " << receivedTime.count() 
                  << " ms, Diff: " << diff.count() << " ms");

    // if the difference is too large, seek to the received time
    constexpr int SEEK_THRESHOLD = 1500; // 1.5 seconds difference
    static std::chrono::steady_clock::time_point lastSeekTime = std::chrono::steady_clock::now();

    if (std::abs(diff.count()) > SEEK_THRESHOLD) 
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastSeekTime).count() > 5) // at least 5 seconds between seeks
        {
            seekGStreamerPlayback(receivedTime);
            lastSeekTime = now;
            return;
        }
    }

    // if the difference is small, adjust the playback speed
    constexpr int SMALL_DIFF_THRESHOLD = 100; // threshold for small differences
    constexpr double SPEED_ADJUST_FACTOR = 10000.0; // factor for adjusting playback speed

    double rate = 1.0;
    if (std::abs(diff.count()) > SMALL_DIFF_THRESHOLD) 
    {
        rate = 1.0 + (static_cast<double>(diff.count()) / SPEED_ADJUST_FACTOR);
        rate = std::clamp(rate, 0.9, 1.1); // clamp to 0.9 - 1.1
    }

    gst_element_seek(pipeline_chunk, rate, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                     GST_SEEK_TYPE_SET, currentTime.count() * GST_MSECOND,
                     GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

    APE_LOG_DEBUG("[GStreamerPlugin]::adjustGStreamerPlayback() Adjusted playback speed to: " << rate);
}
