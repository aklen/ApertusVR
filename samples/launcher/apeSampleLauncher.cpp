/*MIT License

Copyright (c) 2018 MTA SZTAKI

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

#include "apeSystem.h"
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <atomic>
#include <cstring>

std::atomic<bool> shouldStop = false;
std::atomic<bool> forceKill = false;

void forceKillProcess()
{
    std::cerr << "forceKillProcess() Process is not responding, forcefully terminating..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1)); // small delay to let the message print
    std::_Exit(EXIT_FAILURE); // garanteed to terminate the process
}

void stopHandlerBlocking(int s)
{
	std::cout << "stopHandlerBlocking() Caught signal " << s << ", quitting..." << std::endl;
	ape::System::Stop();
	exit(0);
}

void stopHandlerNonBlocking(int s)
{
	std::cout << "stopHandlerNonBlocking() Caught signal " << s << ", quitting..." << std::endl;
	shouldStop = true;

	// start a watchdog thread to force kill the process if it doesn't stop in time
    std::thread watchdog([]() {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (!forceKill) // if the main thread is still running
        {
            forceKillProcess();
        }
    });

    watchdog.detach(); // detach the watchdog thread
}

void blockingMode(const char* configFilePath)
{
	std::cout << "Starting in blocking mode" << std::endl;
	signal(SIGINT, stopHandlerBlocking);
	ape::System::Start(configFilePath, true); // interrupts main thread when joining plugin threads
	ape::System::Stop(); // only called if stopHandlerBlocking() is not called
}

void nonBlockingMode(const char* configFilePath)
{
    std::cout << "Starting in non-blocking mode" << std::endl;
    signal(SIGINT, stopHandlerNonBlocking);
    ape::System::Start(configFilePath, false);
    while (!shouldStop)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // try to stop the system
    ape::System::Stop();
    
    forceKill = true; // if the main thread is still running, force kill it
}

int main(int argc, char** argv)
{
	if (argc < 2 || argc > 3)
	{
		std::cout << "Usage example: apeSampleLauncher configFilePath:string [blockingmode:bool - optional, default: false]" << std::endl;
		exit(1);
	}

	if (argc == 2 || (argc == 3 && strcmp(argv[2], "false") == 0))
		nonBlockingMode(argv[1]);
	else
		blockingMode(argv[1]);

	return 0;
}
