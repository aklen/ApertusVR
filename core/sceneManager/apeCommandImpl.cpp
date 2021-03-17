/*MIT License

Copyright (c) 2021 Akos Hamori

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "apeCommandImpl.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

ape::CommandImpl::CommandImpl(std::string name, bool replicate, std::string ownerID, bool isHost, ape::Command::RunMode runMode, std::string userToRun) : ape::Replica("Command", name, ownerID, isHost)
{
	mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
	mpSceneManager = ape::ISceneManager::getSingletonPtr();
	mName = name;
	mIsReplicated = replicate;
	mCreatorID = ownerID;
	mCmdString = "";
	mRunMode = runMode;
	mUserToRun = userToRun;
}

ape::CommandImpl::~CommandImpl()
{

}

std::string ape::CommandImpl::getName() const
{
	return mName;
}

std::string ape::CommandImpl::getCommandString() const
{
	return mCmdString;
}


void ape::CommandImpl::setCommandString(std::string cmd)
{
	mCmdString = cmd;
}

void ape::CommandImpl::setRunMode(ape::Command::RunMode mode)
{
	mRunMode = mode;
}

bool ape::CommandImpl::isReplicated() const
{
	return mIsReplicated;
}

void ape::CommandImpl::setOwner(std::string ownerID)
{
	mOwnerID = ownerID;
}

std::string ape::CommandImpl::getOwner() const
{
	return mOwnerID;
}

std::string ape::CommandImpl::getCreator() const
{
	return mCreatorID;
}

std::string ape::CommandImpl::getUserToRun() const
{
	return mUserToRun;
}

void ape::CommandImpl::setUserToRun(std::string userToRun)
{
	mUserToRun = userToRun;
}

void ape::CommandImpl::WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const
{
	allocationIdBitstream->Write(mObjectType);
	allocationIdBitstream->Write(RakNet::RakString(mName.c_str()));
	allocationIdBitstream->Write(RakNet::RakString(mOwnerID.c_str()));
	allocationIdBitstream->Write(mRunMode);
	allocationIdBitstream->Write(RakNet::RakString(mUserToRun.c_str()));
}

RakNet::RM3SerializationResult ape::CommandImpl::Serialize(RakNet::SerializeParameters *serializeParameters)
{
	RakNet::VariableDeltaSerializer::SerializationContext serializationContext;
	serializeParameters->pro[0].reliability = RELIABLE_ORDERED;
	mVariableDeltaSerializer.BeginIdenticalSerialize(&serializationContext, serializeParameters->whenLastSerialized == 0, &serializeParameters->outputBitstream[0]);
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mCmdString.c_str()));
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, mRunMode);
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mUserToRun.c_str()));
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mOwnerID.c_str()));
	mVariableDeltaSerializer.EndSerialize(&serializationContext);
	return RakNet::RM3SR_BROADCAST_IDENTICALLY_FORCE_SERIALIZATION;
}

void ape::CommandImpl::Deserialize(RakNet::DeserializeParameters *deserializeParameters)
{
	// begin deserialization
	RakNet::VariableDeltaSerializer::DeserializationContext deserializationContext;
	mVariableDeltaSerializer.BeginDeserialize(&deserializationContext, &deserializeParameters->serializationBitstream[0]);

	// deserialize cmdString
	RakNet::RakString cmdString;
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, cmdString))
	{
		mCmdString = cmdString.C_String();
		mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::COMMAND_CMDSTR));
	}

	// deserialize runMode
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mRunMode))
	{
		mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::COMMAND_RUNMODE));
	}

	// deserialize userToRun
	RakNet::RakString userToRun;
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, userToRun))
	{
		mUserToRun = userToRun.C_String();
	}

	// deserialize ownerID
	RakNet::RakString ownerID;
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, ownerID))
	{
		mOwnerID = ownerID.C_String();
	}

	// end deserialization
	mVariableDeltaSerializer.EndDeserialize(&deserializationContext);
}
