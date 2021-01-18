#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"
#include "apeFilamentRenderCppPlugin.h"
#ifdef __APPLE__
#include <sys/stat.h>
#include "NativeWindowHelper.h"
#endif
static const filament::math::float2 TRIANGLE_VERTICES[3] = { {1, 0}, {-0.5, 0.866}, {-0.5, -0.866} };
static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

ape::FilamentRenderCppPlugin::FilamentRenderCppPlugin( )
{
	APE_LOG_FUNC_ENTER();
	mpSceneManager = ape::ISceneManager::getSingletonPtr();
	mpCoreConfig = ape::ICoreConfig::getSingletonPtr();
	mEventDoubleQueue = ape::DoubleQueue<Event>();
	mpEventManager = ape::IEventManager::getSingletonPtr();
	mpEventManager->connectEvent(ape::Event::Group::NODE, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::LIGHT, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::CAMERA, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_FILE, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_TEXT, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_PLANE, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_BOX, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_CYLINDER, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_CONE, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_TUBE, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_SPHERE, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_TORUS, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_INDEXEDFACESET, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_INDEXEDLINESET, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_CLONE, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::MATERIAL_FILE, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::MATERIAL_MANUAL, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::TEXTURE_MANUAL, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::TEXTURE_FILE, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::TEXTURE_UNIT, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::GEOMETRY_RAY, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::SKY, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::WATER, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpEventManager->connectEvent(ape::Event::Group::POINT_CLOUD, std::bind(&FilamentRenderCppPlugin::eventCallBack, this, std::placeholders::_1));
	mpCoreConfig = ape::ICoreConfig::getSingletonPtr();
	mFilamentRenderCppPluginConfig = ape::FilamentRenderCppPluginConfig();
	mpFilamentEngine = nullptr;
	mpGltfAssetLoader = nullptr;
	mpFilamentRenderer = nullptr;
	mpFilamentSwapChain = nullptr;
	mpFilamentCamera = nullptr;
	mpFilamentView = nullptr;
	mpFilamentScene = nullptr;
	mpFilamentLightManagerBuilder = nullptr;
	mpFilamentNameComponentManager = nullptr;
	mpFilamentMaterialProvider = nullptr;
	mpFilamentTransformManager = nullptr;
	mpFilamentLoadedAssets = std::map<std::string, gltfio::FilamentAsset*>();
	mpFilamentTransforms = std::map<std::string, filament::TransformManager::Instance>();
    parseJson();
    initFilament();
	APE_LOG_FUNC_LEAVE();
}

ape::FilamentRenderCppPlugin::~FilamentRenderCppPlugin()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::FilamentRenderCppPlugin::eventCallBack(const ape::Event& event)
{
	mEventDoubleQueue.push(event);
}

void ape::FilamentRenderCppPlugin::processEventDoubleQueue()
{
	mEventDoubleQueue.swap();
	while (!mEventDoubleQueue.emptyPop())
	{
		ape::Event event = mEventDoubleQueue.front();
		try
		{
		if (event.group == ape::Event::Group::NODE)
		{
			if (auto node = mpSceneManager->getNode(event.subjectName).lock())
			{
				std::string nodeName = node->getName();
				if (event.type == ape::Event::Type::NODE_CREATE)
				{
					auto filamentEntity = mpFilamentEntityManager->create();
					mpFilamentTransformManager->create(filamentEntity);
					auto filamentTransform = mpFilamentTransformManager->getInstance(filamentEntity);
					mpFilamentTransforms[nodeName] = filamentTransform;
				}
				else 
				{
					switch (event.type)
					{
					case ape::Event::Type::NODE_PARENTNODE:
					{
						if (auto parentNode = node->getParentNode().lock())
						{
							mpFilamentTransformManager->setParent(mpFilamentTransforms[nodeName], mpFilamentTransforms[parentNode->getName()]);
						}
					}
						break;
					case ape::Event::Type::NODE_DETACH:
					{
						mpFilamentTransformManager->setParent(mpFilamentTransforms[nodeName], filament::TransformManager::Instance());
					}
						break;
					case ape::Event::Type::NODE_POSITION:
					{
						auto nodeTransform = node->getModelMatrix().transpose();
						auto filamentTransform = filament::math::mat4f(
							nodeTransform[0][0], nodeTransform[0][1], nodeTransform[0][2], nodeTransform[0][3],
							nodeTransform[1][0], nodeTransform[1][1], nodeTransform[1][2], nodeTransform[1][3],
							nodeTransform[2][0], nodeTransform[2][1], nodeTransform[2][2], nodeTransform[2][3],
							nodeTransform[3][0], nodeTransform[3][1], nodeTransform[3][2], nodeTransform[3][3]);
						mpFilamentTransformManager->setTransform(mpFilamentTransforms[nodeName], filamentTransform);
					}
						break;
					case ape::Event::Type::NODE_ORIENTATION:
					{
						auto nodeTransform = node->getModelMatrix().transpose();
						auto filamentTransform = filament::math::mat4f(
							nodeTransform[0][0], nodeTransform[0][1], nodeTransform[0][2], nodeTransform[0][3],
							nodeTransform[1][0], nodeTransform[1][1], nodeTransform[1][2], nodeTransform[1][3],
							nodeTransform[2][0], nodeTransform[2][1], nodeTransform[2][2], nodeTransform[2][3],
							nodeTransform[3][0], nodeTransform[3][1], nodeTransform[3][2], nodeTransform[3][3]);
						mpFilamentTransformManager->setTransform(mpFilamentTransforms[nodeName], filamentTransform);
					}
						break;
					case ape::Event::Type::NODE_SCALE:
						;
						break;
					case ape::Event::Type::NODE_CHILDVISIBILITY:
						;
						break;
					case ape::Event::Type::NODE_VISIBILITY:
					{
						;
					}
						break;
					case ape::Event::Type::NODE_FIXEDYAW:
						;
						break;
					case ape::Event::Type::NODE_INHERITORIENTATION:
						;
						break;
					case ape::Event::Type::NODE_INITIALSTATE:
						;
						break;
					case ape::Event::Type::NODE_SHOWBOUNDINGBOX:
						;
						break;
					case ape::Event::Type::NODE_HIDEBOUNDINGBOX:
						;
						break;
					}
				}
			}
			else if (event.type == ape::Event::Type::NODE_DELETE)
			{
				;
			}
		}
		else if (event.group == ape::Event::Group::GEOMETRY_FILE)
		{
			if (auto geometryFile = std::static_pointer_cast<ape::IFileGeometry>(mpSceneManager->getEntity(event.subjectName).lock()))
			{
				std::string geometryName = geometryFile->getName();
				std::string fileName = geometryFile->getFileName();
				std::string parentNodeName = "";
				if (auto parentNode = geometryFile->getParentNode().lock())
					parentNodeName = parentNode->getName();
				switch (event.type)
				{
				case ape::Event::Type::GEOMETRY_FILE_CREATE:
					;
					break;
				case ape::Event::Type::GEOMETRY_FILE_PARENTNODE:
				{
					auto filamentAssetRootEntity = mpFilamentLoadedAssets[fileName]->getRoot();
					auto filamentAssetRootTransform = mpFilamentTransformManager->getInstance(filamentAssetRootEntity);
					mpFilamentTransformManager->setParent(filamentAssetRootTransform, mpFilamentTransforms[parentNodeName]);
				}
					break;
				case ape::Event::Type::GEOMETRY_FILE_FILENAME:
				{
                    
					if (fileName.find_first_of(".") != std::string::npos)
					{
						std::string fileExtension = fileName.substr(fileName.find_last_of("."));
						if (fileExtension == ".mesh")
						{
							;
						}
						if (fileExtension == ".glb")
						{
							;
						}
						if (fileExtension == ".gltf")
						{
							std::stringstream filePath;
							std::size_t found = fileName.find(":");
							if (found != std::string::npos)
							{
								filePath << fileName;
							}
							else
							{
								std::string separator = "../";
								found = fileName.find(separator);
								if (found != std::string::npos)
								{
									struct stat info;
									if (stat(fileName.c_str(), &info) == -1)
									{
										auto found_it = std::find_end(fileName.begin(), fileName.end(), separator.begin(), separator.end());
										size_t foundPos = found_it - fileName.begin();
										std::stringstream resourceLocationPath;
										resourceLocationPath << APE_SOURCE_DIR << fileName.substr(foundPos + 2);
										filePath << resourceLocationPath.str();
									}
									else
									{
										filePath << fileName;
									}
								}
								else
								{
									std::stringstream resourceLocationPath;
									resourceLocationPath << APE_SOURCE_DIR << fileName;
									filePath << resourceLocationPath.str();
								}
							}
							std::ifstream in(filePath.str().c_str(), std::ifstream::ate | std::ifstream::binary);
							long contentSize = static_cast<long>(in.tellg());
							if (contentSize <= 0) 
							{
								APE_LOG_DEBUG("Unable to open " << filePath.str());
							}
							else
							{
								APE_LOG_DEBUG(filePath.str() << " was opened");
							}
							std::ifstream inBin(filePath.str().c_str(), std::ifstream::binary | std::ifstream::in);
							std::vector<uint8_t> buffer(static_cast<unsigned long>(contentSize));
							if (!inBin.read((char*)buffer.data(), contentSize))
							{
								APE_LOG_DEBUG("Unable to read " << filePath.str());
							}
							else
							{
								APE_LOG_DEBUG(filePath.str() << " was read");
							}
							auto asset = mpGltfAssetLoader->createAssetFromJson(buffer.data(), buffer.size());
							buffer.clear();
							buffer.shrink_to_fit();
							if (!asset)
							{
								APE_LOG_DEBUG("Unable to parse " << filePath.str());
							}
							else
							{
								APE_LOG_DEBUG(filePath.str() << " was parsed");
								gltfio::ResourceConfiguration resourceConfiguration;
								resourceConfiguration.engine = mpFilamentEngine;
								auto resourceLocation = filePath.str();
								resourceConfiguration.gltfPath = resourceLocation.c_str();
								resourceConfiguration.normalizeSkinningWeights = true;
								resourceConfiguration.recomputeBoundingBoxes = false;
								auto filamentResourceLoader = new gltfio::ResourceLoader(resourceConfiguration);
								if (filamentResourceLoader->loadResources(asset))
								{
									APE_LOG_DEBUG("resources load OK");
									mpFilamentScene->addEntities(asset->getEntities(), asset->getEntityCount());
									mpFilamentLoadedAssets[fileName] = asset;
								}
								else
								{
									APE_LOG_DEBUG("resources load FAILED");
								}
								delete filamentResourceLoader;
							}
						}
					}
				}
					break;
				case ape::Event::Type::GEOMETRY_FILE_MERGESUBMESHES:
				{
					;
				}
				break;
				case ape::Event::Type::GEOMETRY_FILE_EXPORT:
				{
					;
				}
				break;
				case ape::Event::Type::GEOMETRY_FILE_MATERIAL:
				{
					;
				}
				break;
				case ape::Event::Type::GEOMETRY_FILE_VISIBILITY:
				{
					;
				}
				break;
				}
			}
			else if (event.type == ape::Event::Type::GEOMETRY_FILE_DELETE)
			{
				;
			}
		}
		else if (event.group == ape::Event::Group::LIGHT)
		{
			if (auto light = std::static_pointer_cast<ape::ILight>(mpSceneManager->getEntity(event.subjectName).lock()))
			{
				if (event.type == ape::Event::Type::LIGHT_CREATE)
				{
					;
				}
				else 
				{
					switch (event.type)
					{
					case ape::Event::Type::LIGHT_ATTENUATION:
						break;
					case ape::Event::Type::LIGHT_DIFFUSE:
						break;
					case ape::Event::Type::LIGHT_DIRECTION:
						break;
					case ape::Event::Type::LIGHT_POWER:
						break;
					case ape::Event::Type::LIGHT_SPECULAR:
						break;
					case ape::Event::Type::LIGHT_SPOTRANGE:
						break;
					case ape::Event::Type::LIGHT_TYPE:
						break;
					case ape::Event::Type::LIGHT_PARENTNODE:
					{
						;
					}
					break;
					case ape::Event::Type::LIGHT_DELETE:
						;
						break;
					}
				}
			}
		}
		else if (event.group == ape::Event::Group::CAMERA)
		{
			if (auto camera = std::static_pointer_cast<ape::ICamera>(mpSceneManager->getEntity(event.subjectName).lock()))
			{
				if (event.type == ape::Event::Type::CAMERA_CREATE)
				{
					auto filamentEntity = mpFilamentEntityManager->create();
					mpFilamentTransformManager->create(filamentEntity);
					auto filamentTransform = mpFilamentTransformManager->getInstance(filamentEntity);
					mpFilamentTransforms[camera->getName()] = filamentTransform;
					mpFilamentCamera = mpFilamentEngine->createCamera(filamentEntity);
					for (int i = 0; i < mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList.size(); i++)
					{
						for (int j = 0; j < mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].viewportList.size(); j++)
						{
							for (int k = 0; k < mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].viewportList[j].cameras.size(); k++)
							{
								auto cameraSetting = mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].viewportList[j].cameras[k];
								if (cameraSetting.name == camera->getName())
								{
									float aspectRatio = mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].viewportList[j].width / mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].viewportList[j].height;
									camera->setWindow(mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].name);
									camera->setFocalLength(1.0f);
									camera->setNearClipDistance(cameraSetting.nearClip);
									camera->setFarClipDistance(cameraSetting.farClip);
									camera->setFOVy(cameraSetting.fovY.toRadian());
									//mpFilamentCamera->setProjection(cameraSetting.fovY.degree, aspectRatio, cameraSetting.nearClip, cameraSetting.farClip);
								}
							}
						}
					}
				}
				else
				{
					switch (event.type)
					{
					case ape::Event::Type::CAMERA_WINDOW:
					{
						for (int i = 0; i < mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList.size(); i++)
						{
							for (int j = 0; j < mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].viewportList.size(); j++)
							{
								auto renderWindowSetting = mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i];
								auto viewportSetting = mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].viewportList[j];
								for (int k = 0; k < mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].viewportList[j].cameras.size(); k++)
								{
									auto cameraSetting = mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList[i].viewportList[j].cameras[k];
									if (cameraSetting.name == camera->getName())
									{
										int zorder = viewportSetting.zOrder;
										float width = (float)viewportSetting.width / (float)renderWindowSetting.width;
										float height = (float)viewportSetting.height / (float)renderWindowSetting.height;
										float left = (float)viewportSetting.left / (float)renderWindowSetting.width;
										float top = (float)viewportSetting.top / (float)renderWindowSetting.height;
                                       
										if (mpFilamentView)
										{
											APE_LOG_DEBUG("filamentViewport: " << "zorder: " << zorder << " left: " << left << " top: " << top << " width: " << width << " height: " << height);
											mpFilamentView->setCamera(mpFilamentCamera);
											mpFilamentView->setViewport({ viewportSetting.left, viewportSetting.top, (unsigned int)viewportSetting.width, (unsigned int)viewportSetting.height });}
											mpFilamentView->setScene(mpFilamentScene);
											filament::View::AmbientOcclusionOptions ambientOcclusionOptions;
											ambientOcclusionOptions.upsampling = filament::View::QualityLevel::HIGH;
											mpFilamentView->setAmbientOcclusionOptions(ambientOcclusionOptions);
									}
								}
							}
						}
					}
					break;
					case ape::Event::Type::CAMERA_PARENTNODE:
					{
						if (auto parentNode = camera->getParentNode().lock())
						{
							mpFilamentTransformManager->setParent(mpFilamentTransforms[camera->getName()], mpFilamentTransforms[parentNode->getName()]);
						}
					}
					break;
					case ape::Event::Type::CAMERA_DELETE:
						;
						break;
					case ape::Event::Type::CAMERA_FOCALLENGTH:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_ASPECTRATIO:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_AUTOASPECTRATIO:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_FOVY:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_FRUSTUMOFFSET:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_FARCLIP:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_NEARCLIP:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_PROJECTION:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_PROJECTIONTYPE:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_ORTHOWINDOWSIZE:
					{
						;
					}
					break;
					case ape::Event::Type::CAMERA_VISIBILITY:
					{
						;
					}
					break;
					}
				}
			}
		}
		}
		catch (std::exception exp)
		{
			APE_LOG_DEBUG("");
		}
		mEventDoubleQueue.pop();
	}
}
void ape::FilamentRenderCppPlugin:: Filament(){
    mpFilamentEngine = filament::Engine::create(filament::Engine::Backend::OPENGL);
    #ifdef _WIN32
    mpFilamentSwapChain = mpFilamentEngine->createSwapChain(mpCoreConfig->getWindowConfig().handle);
    #else
    mpFilamentSwapChain = mpFilamentEngine->createSwapChain(getNSViewHandle(mpCoreConfig->getWindowConfig().handle));
    #endif
    mpFilamentRenderer = mpFilamentEngine->createRenderer();
    mpFilamentScene = mpFilamentEngine->createScene();
    mpFilamentEntityManager = &utils::EntityManager::get();
    mpFilamentTransformManager = &mpFilamentEngine->getTransformManager();
    mpFilamentMaterialProvider = gltfio::createMaterialGenerator(mpFilamentEngine);
    mpFilamentNameComponentManager = new utils::NameComponentManager(*mpFilamentEntityManager);
    mFilamentSunlight = mpFilamentEntityManager->create();
    mpFilamentLightManagerBuilder = new filament::LightManager::Builder(filament::LightManager::Type::SUN);
    mpFilamentLightManagerBuilder->color(filament::Color::toLinear<filament::ACCURATE>({ 0.98, 0.92, 0.89 }));
    mpFilamentLightManagerBuilder->intensity(100000.0f);
    filament::math::float3 sunlightDirection = { 0.6, -1.0, -0.8 };
    mpFilamentLightManagerBuilder->direction(sunlightDirection);
    mpFilamentLightManagerBuilder->castShadows(true);
    mpFilamentLightManagerBuilder->sunAngularRadius(1.9);
    mpFilamentLightManagerBuilder->sunHaloSize(10.0);
    mpFilamentLightManagerBuilder->sunHaloFalloff(80.0);
    mpFilamentLightManagerBuilder->build(*mpFilamentEngine, mFilamentSunlight);
    mpFilamentScene->addEntity(mFilamentSunlight);
    mpGltfAssetLoader = gltfio::AssetLoader::create({ mpFilamentEngine, mpFilamentMaterialProvider, mpFilamentNameComponentManager });
    
    mpFilamentView = mpFilamentEngine->createView();
}

void ape::FilamentRenderCppPlugin::parseJson(){
    APE_LOG_FUNC_ENTER();
    std::stringstream fileFullPath;
    fileFullPath << mpCoreConfig->getConfigFolderPath() << "/apeFilamentRenderCppPlugin.json";
    FILE* apeFilamentRenderCppPluginConfigFile = std::fopen(fileFullPath.str().c_str(), "r");
    char readBuffer[65536];
    if (apeFilamentRenderCppPluginConfigFile)
    {
        rapidjson::FileReadStream jsonFileReaderStream(apeFilamentRenderCppPluginConfigFile, readBuffer, sizeof(readBuffer));
        rapidjson::Document jsonDocument;
        jsonDocument.ParseStream(jsonFileReaderStream);
        if (jsonDocument.IsObject())
        {
            rapidjson::Value& renderSystem = jsonDocument["renderSystem"];
            mFilamentRenderCppPluginConfig.renderSystem = renderSystem.GetString();
            rapidjson::Value& lodLevels = jsonDocument["lodLevels"];
            for (rapidjson::Value::MemberIterator lodLevelsMemberIterator =
                lodLevels.MemberBegin(); lodLevelsMemberIterator != lodLevels.MemberEnd(); ++lodLevelsMemberIterator)
            {
                if (lodLevelsMemberIterator->name == "autoGenerateAndSave")
                    mFilamentRenderCppPluginConfig.filamentLodLevelsConfig.autoGenerateAndSave = lodLevelsMemberIterator->value.GetBool();
                else if (lodLevelsMemberIterator->name == "bias")
                    mFilamentRenderCppPluginConfig.filamentLodLevelsConfig.bias = lodLevelsMemberIterator->value.GetFloat();
            }
            if (jsonDocument.HasMember("shading"))
            {
                rapidjson::Value& shading = jsonDocument["shading"];
                mFilamentRenderCppPluginConfig.shading = shading.GetString();
            }
            rapidjson::Value& renderWindows = jsonDocument["renderWindows"];
            for (auto& renderWindow : renderWindows.GetArray())
            {
                ape::FilamentRenderCppWindowConfig filamentRenderCppWindowConfig;
                for (rapidjson::Value::MemberIterator renderWindowMemberIterator =
                    renderWindow.MemberBegin(); renderWindowMemberIterator != renderWindow.MemberEnd(); ++renderWindowMemberIterator)
                {
                    if (renderWindowMemberIterator->name == "enable")
                        filamentRenderCppWindowConfig.enable = renderWindowMemberIterator->value.GetBool();
                    else if (renderWindowMemberIterator->name == "name")
                        filamentRenderCppWindowConfig.name = renderWindowMemberIterator->value.GetString();
                    else if (renderWindowMemberIterator->name == "monitorIndex")
                        filamentRenderCppWindowConfig.monitorIndex = renderWindowMemberIterator->value.GetInt();
                    else if (renderWindowMemberIterator->name == "hidden")
                        filamentRenderCppWindowConfig.hidden = renderWindowMemberIterator->value.GetBool();
                    else if (renderWindowMemberIterator->name == "resolution")
                    {
                        for (rapidjson::Value::MemberIterator resolutionMemberIterator =
                            renderWindow[renderWindowMemberIterator->name].MemberBegin();
                            resolutionMemberIterator != renderWindow[renderWindowMemberIterator->name].MemberEnd(); ++resolutionMemberIterator)
                        {
                            if (resolutionMemberIterator->name == "width")
                                filamentRenderCppWindowConfig.width = resolutionMemberIterator->value.GetInt();
                            else if (resolutionMemberIterator->name == "height")
                                filamentRenderCppWindowConfig.height = resolutionMemberIterator->value.GetInt();
                            else if (resolutionMemberIterator->name == "fullScreen")
                                filamentRenderCppWindowConfig.fullScreen = resolutionMemberIterator->value.GetBool();
                        }
                    }
                    else if (renderWindowMemberIterator->name == "miscParams")
                    {
                        for (rapidjson::Value::MemberIterator miscParamsMemberIterator =
                            renderWindow[renderWindowMemberIterator->name].MemberBegin();
                            miscParamsMemberIterator != renderWindow[renderWindowMemberIterator->name].MemberEnd(); ++miscParamsMemberIterator)
                        {
                            if (miscParamsMemberIterator->name == "vSync")
                                filamentRenderCppWindowConfig.vSync = miscParamsMemberIterator->value.GetBool();
                            else if (miscParamsMemberIterator->name == "vSyncInterval")
                                filamentRenderCppWindowConfig.vSyncInterval = miscParamsMemberIterator->value.GetInt();
                            else if (miscParamsMemberIterator->name == "colorDepth")
                                filamentRenderCppWindowConfig.colorDepth = miscParamsMemberIterator->value.GetInt();
                            else if (miscParamsMemberIterator->name == "FSAA")
                                filamentRenderCppWindowConfig.fsaa = miscParamsMemberIterator->value.GetInt();
                            else if (miscParamsMemberIterator->name == "FSAAHint")
                                filamentRenderCppWindowConfig.fsaaHint = miscParamsMemberIterator->value.GetString();
                        }
                    }
                    else if (renderWindowMemberIterator->name == "viewports")
                    {
                        rapidjson::Value& viewports = renderWindow[renderWindowMemberIterator->name];
                        for (auto& viewport : viewports.GetArray())
                        {
                            ape::FilamentRenderCppViewPortConfig filamentViewPortConfig;
                            for (rapidjson::Value::MemberIterator viewportMemberIterator =
                                viewport.MemberBegin();
                                viewportMemberIterator != viewport.MemberEnd(); ++viewportMemberIterator)
                            {
                                if (viewportMemberIterator->name == "zOrder")
                                    filamentViewPortConfig.zOrder = viewportMemberIterator->value.GetInt();
                                else if (viewportMemberIterator->name == "left")
                                    filamentViewPortConfig.left = viewportMemberIterator->value.GetInt();
                                else if (viewportMemberIterator->name == "top")
                                    filamentViewPortConfig.top = viewportMemberIterator->value.GetInt();
                                else if (viewportMemberIterator->name == "width")
                                    filamentViewPortConfig.width = viewportMemberIterator->value.GetInt();
                                else if (viewportMemberIterator->name == "height")
                                    filamentViewPortConfig.height = viewportMemberIterator->value.GetInt();
                                else if (viewportMemberIterator->name == "cameras")
                                {
                                    rapidjson::Value& cameras = viewport[viewportMemberIterator->name];
                                    for (auto& camera : cameras.GetArray())
                                    {
                                        ape::FilamentRenderCppCameraConfig filamentCameraConfig;
                                        for (rapidjson::Value::MemberIterator cameraMemberIterator =
                                            camera.MemberBegin();
                                            cameraMemberIterator != camera.MemberEnd(); ++cameraMemberIterator)
                                        {
                                            if (cameraMemberIterator->name == "name")
                                                filamentCameraConfig.name = cameraMemberIterator->value.GetString() + mUniqueID;
                                            else if (cameraMemberIterator->name == "nearClip")
                                                filamentCameraConfig.nearClip = cameraMemberIterator->value.GetFloat();
                                            else if (cameraMemberIterator->name == "farClip")
                                                filamentCameraConfig.farClip = cameraMemberIterator->value.GetFloat();
                                            else if (cameraMemberIterator->name == "fovY")
                                                filamentCameraConfig.fovY = cameraMemberIterator->value.GetFloat();
                                            else if (cameraMemberIterator->name == "positionOffset")
                                            {
                                                for (rapidjson::Value::MemberIterator elementMemberIterator =
                                                    viewport[viewportMemberIterator->name][cameraMemberIterator->name].MemberBegin();
                                                    elementMemberIterator != viewport[viewportMemberIterator->name][cameraMemberIterator->name].MemberEnd(); ++elementMemberIterator)
                                                {
                                                    if (elementMemberIterator->name == "x")
                                                        filamentCameraConfig.positionOffset.x = elementMemberIterator->value.GetFloat();
                                                    else if (elementMemberIterator->name == "y")
                                                        filamentCameraConfig.positionOffset.y = elementMemberIterator->value.GetFloat();
                                                    else if (elementMemberIterator->name == "z")
                                                        filamentCameraConfig.positionOffset.z = elementMemberIterator->value.GetFloat();
                                                }
                                            }
                                            else if (cameraMemberIterator->name == "orientationOffset")
                                            {
                                                ;
                                            }
                                            else if (cameraMemberIterator->name == "parentNodeName")
                                            {
                                                filamentCameraConfig.parentNodeName = cameraMemberIterator->value.GetString();
                                            }
                                        }
                                        filamentViewPortConfig.cameras.push_back(filamentCameraConfig);
                                    }
                                }
                            }
                            filamentRenderCppWindowConfig.viewportList.push_back(filamentViewPortConfig);
                        }
                    }
                }
                mFilamentRenderCppPluginConfig.filamentRenderWindowConfigList.push_back(filamentRenderCppWindowConfig);
            }
        }
        fclose(apeFilamentRenderCppPluginConfigFile);
    }
    APE_LOG_FUNC_LEAVE();
}
void ape::FilamentRenderCppPlugin::Init()
{
	APE_LOG_FUNC_ENTER();
	APE_LOG_FUNC_LEAVE();
}

void ape::FilamentRenderCppPlugin::Run()
{
	APE_LOG_FUNC_ENTER();
//	try
//	{
//
//		while (true)
//		{
//			processEventDoubleQueue();
//			if (mpFilamentRenderer->beginFrame(mpFilamentSwapChain))
//			{
//				mpFilamentRenderer->render(mpFilamentView);
//				//APE_LOG_DEBUG("render");
//				mpFilamentRenderer->endFrame();
//			}
//			std::this_thread::sleep_for(std::chrono::milliseconds(20));
//		}
//
//	}
//	catch (std::exception exp)
//	{
//		APE_LOG_DEBUG("");
//	}
//    while(true)
//        std::this_thread::sleep_for(std::chrono::milliseconds(20));
	APE_LOG_FUNC_LEAVE();
}

void ape::FilamentRenderCppPlugin::Step()
{
        try
        {
                processEventDoubleQueue();
                if (mpFilamentRenderer->beginFrame(mpFilamentSwapChain))
                {
                    mpFilamentRenderer->render(mpFilamentView);
                    //APE_LOG_DEBUG("render");
                    mpFilamentRenderer->endFrame();
                }
            }
        
        catch (std::exception exp)
        {
            APE_LOG_ERROR("");
        }
}

void ape::FilamentRenderCppPlugin::Stop()
{

}

void ape::FilamentRenderCppPlugin::Suspend()
{

}

void ape::FilamentRenderCppPlugin::Restart()
{

}
