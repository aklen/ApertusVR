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

#ifndef APEUTILS_H
#define APEUTILS_H

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace ape::utils
{

	std::vector<std::string> stringSplit(std::string input, char splitChar, bool keepInputOnNoSplit = false)
	{
		std::stringstream test;
		test << input;
		std::string segment;
		std::vector<std::string> seglist;

		while (std::getline(test, segment, splitChar))
		{
			seglist.push_back(segment);
		}
		if (keepInputOnNoSplit && seglist.size() == 0)
		{
			seglist.push_back(input);
		}

		return seglist;
	}

	std::string getCurrentPath()
	{
		return std::filesystem::current_path();
	}

} // namespace ape::utils

#endif
