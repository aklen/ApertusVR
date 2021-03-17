#include "apeHelloWorldScenePlugin.h"

ape::apeHelloWorldScenePlugin::apeHelloWorldScenePlugin()
{
	APE_LOG_FUNC_ENTER();
	mpCoreConfig = ape::ICoreConfig::getSingletonPtr();
	mpEventManager = ape::IEventManager::getSingletonPtr();
	mpEventManager->connectEvent(ape::Event::Group::NODE, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_TEXT, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::COMMAND, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::COMMAND_RESPONSE, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpSceneManager = ape::ISceneManager::getSingletonPtr();
	mIsHost = mpCoreConfig->getNetworkConfig().participant == SceneNetwork::ParticipantType::HOST;
	mGuid = mpCoreConfig->getNetworkGUID();
	mUserName = mpCoreConfig->getNetworkConfig().userName;
	mIsStopped = false;
	APE_LOG_FUNC_LEAVE();
}

ape::apeHelloWorldScenePlugin::~apeHelloWorldScenePlugin()
{
	APE_LOG_FUNC_ENTER();
	mpEventManager->disconnectEvent(ape::Event::Group::NODE, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->disconnectEvent(ape::Event::Group::GEOMETRY_TEXT, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->disconnectEvent(ape::Event::Group::COMMAND, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->disconnectEvent(ape::Event::Group::COMMAND_RESPONSE, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	APE_LOG_FUNC_LEAVE();
}

void ape::apeHelloWorldScenePlugin::eventCallBack(const ape::Event& event)
{
	if (!mIsHost)
	{
		std:: cout << std::endl;
		APE_LOG_DEBUG("eventCallback: group: " << event.group << ", type: " << event.type << ", subjectName:" << event.subjectName);
	}

	if (event.group == ape::Event::Group::COMMAND)
	{
		if (auto command = std::static_pointer_cast<ape::ICommand>(mpSceneManager->getCommand(event.subjectName).lock()))
		{
			switch(event.type)
			{
				case ape::Event::Type::COMMAND_CREATE:
				{
					if (!mIsHost)
					{
						APE_LOG_DEBUG("COMMAND_CREATE " << event.subjectName);
					}
					break;
				}
				case ape::Event::Type::COMMAND_CMDSTR:
				{
					if (!mIsHost)
					{
						std::string userToRun = command->getUserToRun();
						std::string cmdStr = command->getCommandString();

						APE_LOG_DEBUG("COMMAND_CMDSTR " << "g user to run: '" << userToRun << "'");
						APE_LOG_DEBUG("COMMAND_CMDSTR " << "g cmd: '" << cmdStr << "'");

						if (userToRun == mUserName || userToRun == "")
						{
							APE_LOG_DEBUG("COMMAND_CMDSTR " << command->getName() << " cmdStr: '" << cmdStr << "'");
							if (cmdStr.length() > 0)
							{
								mpSceneManager->deleteCommand(command->getName());
								auto commandResp = mpSceneManager->createCommandResponse(event.subjectName + "commandResponseHash" + mUserName, true, "none", ape::CommandResponse::RunMode::GUEST, mUserName);
								if (auto cr = commandResp.lock())
								{
									cr->setCommandString(cmdStr);
									cr->run();
								}
							}
						}
					}
					break;
				}
				case ape::Event::Type::COMMAND_RUNMODE:
				{
					if (!mIsHost)
					{
						APE_LOG_DEBUG("COMMAND_RUNMODE " << event.subjectName);
					}
					break;
				}
				case ape::Event::Type::COMMAND_DELETE:
				{
					if (!mIsHost)
					{
						APE_LOG_DEBUG("COMMAND_DELETEE " << event.subjectName);
					}
					break;
				}
				default:
					APE_LOG_DEBUG("unknown");
			}
		}
	}
	else if (event.group == ape::Event::Group::COMMAND_RESPONSE)
	{
//		APE_LOG_DEBUG("COMMAND_RESPONSE Event: " << event.type);
		if (auto cmdResp = std::static_pointer_cast<ape::ICommandResponse>(mpSceneManager->getCommandResponse(event.subjectName).lock()))
		{
			switch(event.type)
			{
				case ape::Event::Type::COMMAND_RESPONSE_CREATE:
				{
//					APE_LOG_DEBUG("COMMAND_RESPONSE_CREATE " << cmdResp->getName());
					break;
				}
				case ape::Event::Type::COMMAND_RESPONSE_STDOUT:
				{
					std::string stdout = cmdResp->getStdOut();
					if (mIsHost)
					{
						std::cout << cmdResp->getUserName() << "> " << stdout << std::endl;
					}
					else
					{
						APE_LOG_DEBUG("COMMAND_RESPONSE_STDOUT " << cmdResp->getName() << " stdOut: '" << stdout << "'");
					}
					mpSceneManager->deleteCommandResponse(cmdResp->getName());
				}
				case ape::Event::Type::COMMAND_RESPONSE_STDERR:
				{
					std::string stderr = cmdResp->getStdErr();
					if (mIsHost)
					{
						if (stderr.length() > 0)
							std::cout << cmdResp->getUserName() << " err> " << cmdResp->getStdErr() << std::endl;
					}
					else
					{
						APE_LOG_DEBUG("COMMAND_RESPONSE_STDERR " << cmdResp->getName() << " stdErr: '" << stderr << "'");
					}
				}
				default:
//					APE_LOG_DEBUG("unknown");
					break;
			}
		}
	}
}

void ape::apeHelloWorldScenePlugin::Init()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeHelloWorldScenePlugin::Run()
{
	APE_LOG_FUNC_ENTER();
	int c = 0;
	if (mIsHost)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		std::string inputCmd;
		std::string userToRun;
		while (true)
		{
			// std::cout << std::endl << ape::utils::getCurrentPath();
			std::cout << std::endl;
			std::cout << COLOR_LIGHT_BLUE << mUserName << COLOR_LIGHT_YELLOW << "$ " << COLOR_TERM;
			std::getline(std::cin, inputCmd);
			std::vector<std::string> splittedUserCmd = ape::utils::stringSplit(inputCmd, '>', true);
			if (splittedUserCmd.size() == 1)
			{
				userToRun = "";
				inputCmd = splittedUserCmd.at(0);
			}
			else if (splittedUserCmd.size() == 2)
			{
				userToRun = splittedUserCmd.at(0);
				inputCmd = splittedUserCmd.at(1);
			}

			c++;
			if (c > 9)
			{
				c = 0;
			}
			std::ostringstream sstr;
			sstr << c << "commandStr" << mUserName;
			auto commandWkPtr = mpSceneManager->createCommand(sstr.str(), true, "none", ape::Command::RunMode::GUEST, userToRun);
			if (auto command = commandWkPtr.lock())
			{
				// command->setOwner(mpCoreConfig->getNetworkGUID());
				command->setRunMode(ape::Command::RunMode::GUEST);
				command->setCommandString(inputCmd);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
	else
	{
		while (!mIsStopped)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	APE_LOG_FUNC_LEAVE();
}

void ape::apeHelloWorldScenePlugin::Step()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeHelloWorldScenePlugin::Stop()
{
	APE_LOG_FUNC_ENTER();
	stopMutex.lock();
	mIsStopped = true;
	stopMutex.unlock();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeHelloWorldScenePlugin::Suspend()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::apeHelloWorldScenePlugin::Restart()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}
