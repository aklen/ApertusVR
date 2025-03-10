#include "apeConfigManagerImpl.h"
#include <iostream>
#include <fstream>
#include <vector>

ape::ConfigManagerImpl::ConfigManagerImpl()
{
    APE_LOG_FUNC_ENTER();
    msSingleton = this;
    mpLogManager = ape::ILogManager::getSingletonPtr();
    APE_LOG_FUNC_LEAVE();
}

ape::ConfigManagerImpl::~ConfigManagerImpl()
{
    APE_LOG_FUNC_ENTER();
    APE_LOG_FUNC_LEAVE();
}

bool ape::ConfigManagerImpl::loadJson(const std::string& filePath, ConfigNode& config)
{
    APE_LOG_DEBUG("[ConfigManager] Loading config file: " << filePath);

    FILE* file = fopen(filePath.c_str(), "r");
    if (!file)
    {
        APE_LOG_ERROR("[ConfigManager] Error: Cannot open config file: " << filePath);
        return false;
    }

    char readBuffer[65536];
    rapidjson::FileReadStream jsonFileReaderStream(file, readBuffer, sizeof(readBuffer));
    rapidjson::Document jsonDocument;
    jsonDocument.ParseStream(jsonFileReaderStream);
    fclose(file);

    if (jsonDocument.HasParseError())
    {
        APE_LOG_ERROR("[ConfigManager] Error: Failed to parse JSON file: " << filePath);
        return false;
    }

    parseJsonToConfigNode(jsonDocument, config);
    return true;
}

void ape::ConfigManagerImpl::parseJsonToConfigNode(const rapidjson::Value& jsonValue, ConfigNode& node, const std::string& key)
{
    if (jsonValue.IsObject())
    {
        for (auto it = jsonValue.MemberBegin(); it != jsonValue.MemberEnd(); ++it)
        {
            const std::string key = it->name.GetString();
            const rapidjson::Value& value = it->value;

            if (value.IsObject())
            {
                node.getObject(key);  // Biztosítja, hogy létrejöjjön az objektum
                parseJsonToConfigNode(value, node[key]);
            }
            else if (value.IsArray())
            {
                if (value.Size() > 0 && value[0].IsString()) 
                {
                    // 🔹 STRING TÖMB (pl. "uris": ["http://example.com/1", "http://example.com/2"])
                    std::vector<std::string> strArray;
                    for (auto& v : value.GetArray())
                    {
                        strArray.push_back(v.GetString());
                    }
                    node.setArray(key, strArray);
                }
                else 
                {
                    // 🔹 OBJEKTUM TÖMB
                    auto& array = node.getArray(key);
                    for (auto& v : value.GetArray())
                    {
                        auto objNode = std::make_shared<ConfigNode>();
                        parseJsonToConfigNode(v, *objNode);
                        array.push_back(objNode);
                    }
                }
            }
            else if (value.IsString())
            {
                node.setString(key, value.GetString());
            }
            else if (value.IsInt())
            {
                node.setInt(key, value.GetInt());
            }
            else if (value.IsDouble())
            {
                node.setDouble(key, value.GetDouble());
            }
            else if (value.IsBool())
            {
                node.setBool(key, value.GetBool());
            }
        }
    }
}
