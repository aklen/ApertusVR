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

#include "apeUtils.h"
#include "apeCommandResponseImpl.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include "subprocess.h"

using namespace subprocess;

std::string& leftTrim(std::string& str, std::string chars)
{
	str.erase(0, str.find_first_not_of(chars));
	return str;
}

std::string& rightTrim(std::string& str, std::string chars)
{
	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

std::string& trimString(std::string& str, std::string chars)
{
	return leftTrim(rightTrim(str, chars), chars);
}

std::string removeNewLineEnding(std::string str)
{
	if (str[str.length() - 1] == '\n')
	{
		str.erase(str.length() - 1);
	}
	return str;
}

std::string combineCounter(std::string str, int counter)
{
	std::ostringstream sstr;
	sstr << counter << str;
	return sstr.str();
}

ape::CommandResponseImpl::CommandResponseImpl(std::string name, bool replicate, std::string ownerID, bool isHost, ape::CommandResponse::RunMode runMode, std::string userName) : ape::Replica("CommandResponse", name, ownerID, isHost)
{
	mpEventManagerImpl = ((ape::EventManagerImpl*)ape::IEventManager::getSingletonPtr());
	mpSceneManager = ape::ISceneManager::getSingletonPtr();
	mName = name;
	mIsReplicated = replicate;
	mCreatorID = ownerID;
	mUserName = userName;
	mCmdString = "";
	mStdOut = "";
	mStdErr = "";
	mResultCode = -1;
	mCmdCounter = 0;
	mRunMode = runMode;
}

ape::CommandResponseImpl::~CommandResponseImpl()
{

}

std::string ape::CommandResponseImpl::getName() const
{
	return mName;
}

std::string ape::CommandResponseImpl::getCommandString() const
{
	if (mCmdString.length() == 0) {
		return "";
	}
	return mCmdString.substr(1);
}

std::string ape::CommandResponseImpl::getStdOut() const
{
	if (mStdOut.length() == 0) {
		return "";
	}
	return mStdOut.substr(1);
}

std::string ape::CommandResponseImpl::getStdErr() const
{
	if (mStdErr.length() == 0) {
		return "";
	}
	return mStdErr.substr(1);
}

int ape::CommandResponseImpl::getResultCode() const
{
	return mResultCode;
}

void ape::CommandResponseImpl::setCommandString(std::string cmd)
{
	shiftCounter();
	mCmdString = combineCounter(cmd, mCmdCounter);
}

void ape::CommandResponseImpl::setStdOut(std::string str)
{
	mStdOut = combineCounter(str, mCmdCounter);
}

void ape::CommandResponseImpl::setStdErr(std::string str)
{
	mStdErr = combineCounter(str, mCmdCounter);
}

void ape::CommandResponseImpl::shiftCounter()
{
	mCmdCounter = mCmdCounter + 1;
	if (mCmdCounter == 9) {
		mCmdCounter = 0;
	}
}

std::string ape::CommandResponseImpl::run()
{
	if (mRunMode == ape::CommandResponse::RunMode::NONE)
	{
		return "";
	}

	bool runModeOk = false;
	if (mRunMode == ape::CommandResponse::RunMode::BOTH)
	{
		runModeOk = true;
	}
	else
	{
		runModeOk = (mRunMode == ape::CommandResponse::RunMode::GUEST && !isHost()) || (mRunMode == ape::CommandResponse::RunMode::HOST && isHost());
	}

	if (!runModeOk)
	{
		APE_LOG_ERROR("Command run mode is not ok");
		return "";
	}

	if (getCommandString().length() == 0)
	{
		APE_LOG_ERROR("Command string must not be empty");
		return "";
	}

	std::vector<std::string> splittedCmd = ape::utils::stringSplit(getCommandString(), '|', true);
	for (int i = 0; i < splittedCmd.size(); i++)
	{
		splittedCmd[i] = trimString(splittedCmd[i], " ");
	}

	try
	{
		auto res = pipeline(splittedCmd);
		auto outBuffer = res.first;
		auto errBuffer = res.second;

		if (errBuffer.length > 0)
		{
			setStdErr(removeNewLineEnding(std::string(errBuffer.buf.begin(), errBuffer.buf.end())));
		}
		else if (outBuffer.length > 0)
		{
			setStdOut(removeNewLineEnding(std::string(outBuffer.buf.begin(), outBuffer.buf.end())));
		}
	}
	catch (const std::exception& e)
	{
		APE_LOG_ERROR("Catched error: " << e.what());
		setStdErr(e.what());
	}
	APE_LOG_DEBUG("mStdOut: " << mStdOut);

	return mStdOut;
}

void ape::CommandResponseImpl::setRunMode(ape::CommandResponse::RunMode mode)
{
	mRunMode = mode;
}

bool ape::CommandResponseImpl::isReplicated() const
{
	return mIsReplicated;
}

void ape::CommandResponseImpl::setOwner(std::string ownerID)
{
	mOwnerID = ownerID;
}

std::string ape::CommandResponseImpl::getOwner() const
{
	return mOwnerID;
}

std::string ape::CommandResponseImpl::getCreator() const
{
	return mCreatorID;
}

std::string ape::CommandResponseImpl::getUserName() const
{
	return mUserName;
}

void ape::CommandResponseImpl::WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const
{
	allocationIdBitstream->Write(mObjectType);
	allocationIdBitstream->Write(RakNet::RakString(mName.c_str()));
	allocationIdBitstream->Write(RakNet::RakString(mOwnerID.c_str()));
	allocationIdBitstream->Write(RakNet::RakString(mUserName.c_str()));
	allocationIdBitstream->Write(mRunMode);
}

RakNet::RM3SerializationResult ape::CommandResponseImpl::Serialize(RakNet::SerializeParameters *serializeParameters)
{
	RakNet::VariableDeltaSerializer::SerializationContext serializationContext;
	serializeParameters->pro[0].reliability = RELIABLE_ORDERED;
	mVariableDeltaSerializer.BeginIdenticalSerialize(&serializationContext, serializeParameters->whenLastSerialized == 0, &serializeParameters->outputBitstream[0]);
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mCmdString.c_str()));
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mStdOut.c_str()));
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mStdErr.c_str()));
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, mResultCode);
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, mCmdCounter);
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, mRunMode);
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mUserName.c_str()));
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mCreatorID.c_str()));
	mVariableDeltaSerializer.SerializeVariable(&serializationContext, RakNet::RakString(mOwnerID.c_str()));
	mVariableDeltaSerializer.EndSerialize(&serializationContext);
	return RakNet::RM3SR_BROADCAST_IDENTICALLY_FORCE_SERIALIZATION;
}

void ape::CommandResponseImpl::Deserialize(RakNet::DeserializeParameters *deserializeParameters)
{
	// begin deserialization
	RakNet::VariableDeltaSerializer::DeserializationContext deserializationContext;
	mVariableDeltaSerializer.BeginDeserialize(&deserializationContext, &deserializeParameters->serializationBitstream[0]);

	// deserialize cmdString
	RakNet::RakString cmdString;
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, cmdString))
	{
		mCmdString = cmdString.C_String();
		mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::COMMAND_RESPONSE_CMDSTR));
	}

	// deserialize stdOut
	RakNet::RakString stdOut;
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, stdOut))
	{
		mStdOut = stdOut.C_String();
		mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::COMMAND_RESPONSE_STDOUT));
	}

	// deserialize stdErr
	RakNet::RakString stdErr;
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, stdErr))
	{
		mStdErr = stdErr.C_String();
		mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::COMMAND_RESPONSE_STDERR));
	}

	// deserialize resultCode
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mResultCode))
	{
		mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::COMMAND_RESPONSE_RESULTCODE));
	}

	// deserialize cmdCounter
	mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mCmdCounter);

	// deserialize runMode
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, mRunMode))
	{
		mpEventManagerImpl->fireEvent(ape::Event(mName, ape::Event::Type::COMMAND_RESPONSE_RUNMODE));
	}

	// deserialize userName
	RakNet::RakString userName;
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, userName))
	{
		mUserName = userName.C_String();
	}

	// deserialize creatorID
	RakNet::RakString creatorID;
	if (mVariableDeltaSerializer.DeserializeVariable(&deserializationContext, creatorID))
	{
		mCreatorID = creatorID.C_String();
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
