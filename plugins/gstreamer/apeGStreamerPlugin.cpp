#include "apeGStreamerPlugin.h"
#include "apeUtils.h"

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

    APE_LOG_DEBUG("[GStreamerPlugin]::on_need_data() Check if more data needed...");
    std::string currentAudioEntityId = plugin->GetCurrentAudioEntityId();

    if (plugin->IsHost()) {
        APE_LOG_DEBUG("[GStreamerPlugin]::on_need_data() Host GStreamer, requesting more data...");
        // if (auto audio = std::static_pointer_cast<ape::IAudio>(plugin->getSceneManager()->getEntity(currentAudioEntityId).lock())) {
            if (plugin->loadNextAudioChunk(APE_AUDIO_CHUNK_SIZE_1MB)) {
                APE_LOG_DEBUG("[GStreamerPlugin]::on_need_data() Loaded next chunk.");
            }
            else {
                APE_LOG_DEBUG("[GStreamerPlugin]::on_need_data() No more chunks, sending EOS.");
                GstBus* bus = gst_element_get_bus(plugin->GetPipelineChunk());
                GstMessage* eos_msg = gst_message_new_eos(GST_OBJECT(src));
                gst_bus_post(bus, eos_msg);
                gst_object_unref(bus);
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
	mpEventManager = ape::IEventManager::getSingletonPtr();
    mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
    mpEventManager->connectEvent(ape::Event::Group::AUDIO, std::bind(&apeGStreamerPlugin::eventCallBack, this, std::placeholders::_1));
    mpSceneManager = ape::ISceneManager::getSingletonPtr();
    mCurrentAudioEntityId = "";
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
    APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Received event: " << event.subjectName);

    if (event.type == ape::Event::Type::AUDIO_CREATE) {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Audio entity created: " << event.subjectName);
    }
    else if (event.type == ape::Event::Type::AUDIO_DELETE) {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Audio entity deleted: " << event.subjectName);
    }
    else if (event.type == ape::Event::Type::AUDIO_PLAYBACK_STATE) {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Audio playback state changed: " << event.subjectName);
    }
    else if (event.type == ape::Event::Type::AUDIO_DATA) {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Audio data changed: " << event.subjectName);
    }
    else if (event.type == ape::Event::Type::AUDIO_SAMPLE_RATE) {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Audio sample rate changed: " << event.subjectName);
    }
    else if (event.type == ape::Event::Type::AUDIO_CHANNELS) {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Audio channels changed: " << event.subjectName);
    }
    else if (event.type == ape::Event::Type::AUDIO_STREAMING) {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Audio streaming changed: " << event.subjectName);
    }
    else if (event.type == ape::Event::Type::AUDIO_CHUNK_LOAD) {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Audio chunk position changed: " << event.subjectName);
        if (auto audio = std::static_pointer_cast<ape::IAudio>(mpSceneManager->getEntity(event.subjectName).lock())) {
            PlayAudioChunk(audio->getLastChunkData());
        }
    }
    else {
        APE_LOG_DEBUG("[GStreamerPlugin]::eventCallBack() Unknown event type: " << event.subjectName);
    }
}

void ape::apeGStreamerPlugin::Init()
{
	APE_LOG_FUNC_ENTER();

    if (!mIsHost) {
        APE_LOG_DEBUG("[GStreamerPlugin]::Init() Not host, skipping initialization.");
        return;
    }

    // std::this_thread::sleep_for(std::chrono::seconds(15));

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

        // create an audio entity
        mCurrentAudioEntityId = "audio_" + audioFileName;
        APE_LOG_DEBUG("[GStreamerPlugin]::Init() Creating audio entity: " << mCurrentAudioEntityId);

        if (auto audio = std::static_pointer_cast<ape::IAudio>(mpSceneManager->createEntity(mCurrentAudioEntityId, ape::Entity::AUDIO, true, mpCoreConfig->getNetworkGUID()).lock())) {
            // load the first chunk of the audio file
            APE_LOG_DEBUG("[GStreamerPlugin]::Init() Audio Entity created, loading first audio chunk...");
            bool firstLoaded = loadNextAudioChunk(APE_AUDIO_CHUNK_SIZE_1MB);
            APE_LOG_DEBUG("[GStreamerPlugin]::Init() First chunk loaded: " << firstLoaded);
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
            APE_LOG_DEBUG("[GStreamerPlugin]::loadNextAudioChunk() Chunk added, size: " << buffer.size());
        }
    }

    return bytesRead > 0;
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
            APE_LOG_DEBUG("[GStreamerPlugin]::Play() Changing state to PLAYING...");
            gst_element_set_state(pipeline_chunk, GST_STATE_PLAYING);
            running = true;

            mpEventManagerImpl->fireEvent(ape::Event("PlaybackStarted", ape::Event::Type::AUDIO_PLAYBACK_STATE));
            APE_LOG_DEBUG("[GStreamerPlugin]::Play() Playback started.");
        }
        else {
            APE_LOG_DEBUG("[GStreamerPlugin]::Play() Pipeline already playing.");
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
