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

#ifndef APE_COMMANDRESPONSEIMPL_H
#define APE_COMMANDRESPONSEIMPL_H

#include "apeICommandResponse.h"
#include "apeEventManagerImpl.h"
#include "apeReplica.h"

namespace ape
{
	class CommandResponseImpl : public ape::ICommandResponse, public ape::Replica
	{
	public:

		CommandResponseImpl(std::string name, bool replicate, std::string ownerID, bool isHost, ape::CommandResponse::RunMode runMode, std::string userName);

		~CommandResponseImpl();

		std::string getName() const override;

		std::string getCommandString() const override;

		std::string getStdOut() const override;

		std::string getStdErr() const override;

		int getResultCode() const override;

		void setCommandString(std::string cmd) override;

		bool isReplicated() const override;

		void setOwner(std::string ownerID) override;

		std::string getOwner() const override;

		std::string getCreator() const override;

		std::string getUserName() const override;

		std::string run() override;

		void setRunMode(ape::CommandResponse::RunMode mode) override;

		void WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const override;

		RakNet::RM3SerializationResult Serialize(RakNet::SerializeParameters *serializeParameters) override;

		void Deserialize(RakNet::DeserializeParameters *deserializeParameters) override;

	private:

		void shiftCounter();

		void setStdOut(std::string);

		void setStdErr(std::string);

		ape::EventManagerImpl* mpEventManagerImpl;

		ape::ISceneManager* mpSceneManager;

		std::string mName;

		bool mIsReplicated;

		std::string mCreatorID;

		std::string mUserName;

		std::string mCmdString;

		std::string mStdOut;

		std::string mStdErr;

		int mResultCode;

		int mCmdCounter;

		ape::CommandResponse::RunMode mRunMode;
	};
}

#endif
