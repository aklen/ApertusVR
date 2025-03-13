# RakNet Serialization & Debugging Guide

## Introduction
RakNet is a powerful networking library that provides object replication and serialization for multiplayer applications. However, its built-in optimizations, such as delta compression and variable delta serialization, can sometimes introduce unexpected behavior when dealing with **large data chunks** like audio or video streaming.

This document provides an in-depth look into **RakNet serialization and deserialization**, including debugging tips and a crucial trick that can force RakNet to always send full data instead of relying on delta compression.

---

## 1. RakNet Serialization Overview
RakNet uses **variable delta serialization** to optimize bandwidth usage. This means that **only the differences** between the last sent data and the current state will be transmitted.

### **How Serialization Works in RakNet**
When an object needs to be synchronized across the network:
1. `Serialize()` is called on the host.
2. RakNet checks what has changed compared to the last transmission.
3. If only part of the data has changed, RakNet sends only that portion.
4. On the guest, `Deserialize()` reconstructs the object based on the received data.

**This is efficient, but can cause issues when transmitting large chunks of data!**

---

## 2. Common Issues & Debugging Serialization
During debugging, I encountered an issue where **not all data chunks were being received properly on the guest side**. After investigating, here are the key pain points and how to solve them.

### **Problem 1: Incomplete Data Reception**
**Symptoms:**
- The host was correctly serializing and logging large audio chunks (e.g., 1MB chunks with CRC32 verification).
- The guest was failing to deserialize some chunks, logging errors such as:
  
  ```
  ERROR: AudioImpl::Deserialize() ERROR: Failed to deserialize chunk count!
  ```
- `RakNet::Deserialize()` sometimes received fewer bytes than expected.

**Root Cause:**
RakNet was **only sending changes** due to its delta compression feature, meaning that not all data chunks were being sent on every frame.

### **Solution: Force RakNet to Send Full Data**
To **force RakNet to always send the full serialized data**, add this line at the beginning of `Serialize()`:

```cpp
serializeParameters->whenLastSerialized = 0;
```

This tells RakNet **to ignore previous serialized states** and treat every serialization as if it's happening for the first time. As a result, it sends **all data** rather than relying on delta updates.

### **Example Fix in `Serialize()`**
```cpp
RakNet::RM3SerializationResult ape::AudioImpl::Serialize(RakNet::SerializeParameters* serializeParameters)
{
    if (!modified)
        return RakNet::RM3SR_DO_NOT_SERIALIZE;

    APE_LOG_DEBUG("AudioImpl::Serialize() called");

    // Force full serialization
    serializeParameters->whenLastSerialized = 0;
    
    RakNet::VariableDeltaSerializer::SerializationContext serializationContext;
    serializeParameters->pro[0].reliability = RELIABLE_ORDERED;
    mVariableDeltaSerializer.BeginIdenticalSerialize(&serializationContext, serializeParameters->whenLastSerialized == 0, &serializeParameters->outputBitstream[0]);

    // Serialize variables...

    modified = false;
    mVariableDeltaSerializer.EndSerialize(&serializationContext);
    return RakNet::RM3SR_BROADCAST_IDENTICALLY_FORCE_SERIALIZATION;
}
```

---

## 3. Debugging RakNet Serialization
Here are some essential debugging tips to verify **if the data is correctly serialized and deserialized.**

### **Step 1: Log Total Serialized Data**
After `Serialize()`, log the total number of bytes serialized:

```cpp
APE_LOG_DEBUG("AudioImpl::Serialize() Total serialized bytes: "
              << serializeParameters->outputBitstream[0].GetNumberOfBytesUsed());
```

If the **guest receives fewer bytes than this**, RakNet is dropping part of the data (likely due to delta compression).

### **Step 2: Log Chunk Data Before and After Writing**
To track **where a problem occurs**, log the offset in the bitstream before and after writing:

```cpp
APE_LOG_DEBUG("AudioImpl::Serialize() Before writing chunk count | Offset: "
              << serializeParameters->outputBitstream[0].GetNumberOfBytesUsed());
```
```cpp
APE_LOG_DEBUG("AudioImpl::Serialize() After writing chunk count | Offset: "
              << serializeParameters->outputBitstream[0].GetNumberOfBytesUsed());
```

This ensures that all variables are correctly written into the bitstream.

### **Step 3: Log Offsets in `Deserialize()`**
On the guest side, log read offsets **before and after** reading each variable:

```cpp
APE_LOG_DEBUG("AudioImpl::Deserialize() Before reading chunk count | Read Offset: "
              << deserializeParameters->serializationBitstream[0].GetReadOffset());
```
```cpp
APE_LOG_DEBUG("AudioImpl::Deserialize() After reading chunk count | Read Offset: "
              << deserializeParameters->serializationBitstream[0].GetReadOffset());
```

If the **offsets do not match the expected positions**, then RakNet is skipping or misinterpreting data.

### **Step 4: Validate CRC32 of Each Chunk**
To confirm **if a chunk is being received correctly**, log its CRC32 hash after deserialization:

```cpp
APE_LOG_DEBUG("AudioImpl::Deserialize() chunk size: " << chunk.size()
              << ", CRC32: " << calculateCRC32(chunk));
```

If the CRC32 on the **guest differs from the host**, **then the chunk is getting corrupted during transmission.**

---

## 4. Summary of Key Fixes
| Problem | Solution |
|---------|----------|
| Missing data in `Deserialize()` | Set `serializeParameters->whenLastSerialized = 0` in `Serialize()` |
| RakNet skipping variables | Log bitstream offsets before/after serialization & deserialization |
| Chunks received incorrectly | Validate CRC32 after deserialization |

With these techniques, we can **force full data transmission**, debug RakNet's serialization process, and ensure **large chunks (like audio) are properly sent and received.** 🚀

