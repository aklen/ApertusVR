### Audio Streaming Synchronization Testing and Optimization in ApertusVR

The following steps outline how to test and optimize the audio streaming synchronization in **ApertusVR** using **RakNet**. The goal is to ensure that audio data is properly transmitted between instances while minimizing latency and maximizing stability.

#### 1. Peer-to-Peer Testing
- Launch two separate **ApertusVR** instances: one as the **host** and one as the **client**.
- Instantiate an `AudioImpl` object on the **host** and append audio data using `appendAudioData()`.
- Verify that the **client** receives and updates the replicated `AudioImpl` instance correctly.

#### 2. RakNet Packet Size and Fragmentation Testing
- RakNet enforces **MTU (Maximum Transmission Unit) limits** (~1200-1400 bytes for UDP).
- If the `AudioImpl` transmits data exceeding this limit, RakNet automatically fragments it.
- Monitor network traffic using **Wireshark** or **RakNet PacketLogger** to ensure no packets are lost.
- Adjust the chunk size of `appendAudioData()` to optimize transmission efficiency.

#### 3. Measuring Synchronization Latency
- Measure the time between calling `appendAudioData()` on the **host** and receiving the updated audio data on the **client**.
- Use `RakNet::GetTimeMS()` for precise timestamp logging.
- Experiment with sending **larger** audio chunks less frequently vs. **smaller** chunks more frequently to find the optimal balance.

#### 4. Optimizing for Real-Time Streaming
- Try switching from **RELIABLE_ORDERED** to **UNRELIABLE_SEQUENCED** mode to reduce latency while allowing minor packet loss.
- Experiment with different buffer sizes (`mMaxBufferSize`) to maintain a balance between **low latency** and **stable playback**.
- Implement a simple packet loss recovery mechanism if necessary.

#### 5. GStreamer Integration
- The byte stream from `AudioImpl` should be fed into **GStreamer** for playback.
- Use `appsrc` in GStreamer to push incoming audio data dynamically.
- Ensure that **buffer underflows** do not occur by maintaining an adequate buffer size.
