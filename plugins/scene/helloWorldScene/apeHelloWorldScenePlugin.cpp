#include "apeHelloWorldScenePlugin.h"

ape::apeHelloWorldScenePlugin::apeHelloWorldScenePlugin()
{
	APE_LOG_FUNC_ENTER();
	mpCoreConfig = ape::ICoreConfig::getSingletonPtr();
	mpEventManager = ape::IEventManager::getSingletonPtr();
	mpEventManager->connectEvent(ape::Event::Group::NODE, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_TEXT, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpSceneManager = ape::ISceneManager::getSingletonPtr();
	mCounter = 0;
	mIsHost = mpCoreConfig->getNetworkConfig().participant == SceneNetwork::ParticipantType::HOST;
	APE_LOG_FUNC_LEAVE();
}

ape::apeHelloWorldScenePlugin::~apeHelloWorldScenePlugin()
{
	APE_LOG_FUNC_ENTER();
	mpEventManager->disconnectEvent(ape::Event::Group::CAMERA, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->disconnectEvent(ape::Event::Group::NODE, std::bind(&apeHelloWorldScenePlugin::eventCallBack, this, std::placeholders::_1));
	APE_LOG_FUNC_LEAVE();
}

void ape::apeHelloWorldScenePlugin::eventCallBack(const ape::Event& event)
{
	if (event.group == ape::Event::Group::GEOMETRY_TEXT) {
		if (event.type == ape::Event::Type::GEOMETRY_TEXT_CAPTION) {
			if (auto text = std::static_pointer_cast<ape::ITextGeometry>(mpSceneManager->getEntity(event.subjectName).lock())) {
				APE_LOG_DEBUG(text->getCaption());
			}
		}
	}
}

void ape::apeHelloWorldScenePlugin::Init()
{
	APE_LOG_FUNC_ENTER();
	if (mIsHost) {
		mTextNode = mpSceneManager->createNode("helloWorldNode", true, mpCoreConfig->getNetworkGUID());
		if (auto textNode = mTextNode.lock())
		{
			mTextEntity = mpSceneManager->createEntity("helloWorldText", ape::Entity::GEOMETRY_TEXT, true, mpCoreConfig->getNetworkGUID());
			if (auto userNameText = std::static_pointer_cast<ape::ITextGeometry>(mTextEntity.lock()))
			{
				userNameText->setCaption("helloWorld");
				userNameText->setParentNode(textNode);
			}
		}
	}
	APE_LOG_FUNC_LEAVE();
}

void ape::apeHelloWorldScenePlugin::Run()
{
	APE_LOG_FUNC_ENTER();
	if (mIsHost) {
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			mCounter++;
			if (auto userNameText = std::static_pointer_cast<ape::ITextGeometry>(mTextEntity.lock())) {
				userNameText->setCaption("helloWorld " + std::to_string(mCounter));
			}
		}
	}
	else {
		while (true)
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
