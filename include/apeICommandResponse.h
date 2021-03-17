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

#ifndef APE_ICOMMANDRESPONSE_H
#define APE_ICOMMANDRESPONSE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ape.h"

namespace ape
{
	namespace CommandResponse
	{
		enum RunMode
		{
			HOST,
			GUEST,
			BOTH,
			NONE
		};
	}

	class ICommandResponse
	{
	protected:
		virtual ~ICommandResponse() {};

	public:
		virtual std::string getName() const = 0;

		virtual std::string getCommandString() const = 0;

		virtual std::string getStdOut() const = 0;

		virtual std::string getStdErr() const = 0;

		virtual int getResultCode() const = 0;

		virtual void setCommandString(std::string cmd) = 0;

		virtual bool isReplicated() const = 0;

		virtual void setOwner(std::string ownerID) = 0;

		virtual std::string getOwner() const = 0;

		virtual std::string getCreator() const = 0;

		virtual std::string getUserName() const = 0;

		virtual std::string run() = 0;

		virtual void setRunMode(ape::CommandResponse::RunMode mode) = 0;
	};
}

#endif
