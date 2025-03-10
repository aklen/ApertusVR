#ifndef CONFIG_NODE_H
#define CONFIG_NODE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <memory>
#include <iostream>

namespace ape
{
    class ConfigNode
    {
    public:
        using ValueType = std::variant<
            std::string, int, double, bool,
            std::vector<std::string>,
            std::vector<std::shared_ptr<ConfigNode>>,
            std::shared_ptr<ConfigNode>
        >;

    private:
        std::unordered_map<std::string, ValueType> data;

    public:
        ConfigNode() = default;

        std::string getString(const std::string& key) const {
            if (data.count(key) && std::holds_alternative<std::string>(data.at(key)))
                return std::get<std::string>(data.at(key));
            return "";
        }

        int getInt(const std::string& key) const {
            if (data.count(key) && std::holds_alternative<int>(data.at(key)))
                return std::get<int>(data.at(key));
            return 0;
        }

        double getNumber(const std::string& key) const {
            if (data.count(key) && std::holds_alternative<double>(data.at(key)))
                return std::get<double>(data.at(key));
            return 0.0;
        }

        bool getBool(const std::string& key) const {
            if (data.count(key) && std::holds_alternative<bool>(data.at(key)))
                return std::get<bool>(data.at(key));
            return false;
        }

        ConfigNode& getObject(const std::string& key)
        {
            if (!data.count(key) || !std::holds_alternative<std::shared_ptr<ConfigNode>>(data[key]))
                data[key] = std::make_shared<ConfigNode>();

            return *std::get<std::shared_ptr<ConfigNode>>(data[key]);
        }

        std::vector<std::shared_ptr<ConfigNode>>& getArray(const std::string& key)
        {
            if (!data.count(key) || !std::holds_alternative<std::vector<std::shared_ptr<ConfigNode>>>(data[key]))
                data[key] = std::vector<std::shared_ptr<ConfigNode>>();

            return std::get<std::vector<std::shared_ptr<ConfigNode>>>(data[key]);
        }

        ConfigNode& operator[](const std::string& key)
        {
            return getObject(key);
        }

        ConfigNode& operator[](size_t index)
        {
            auto& array = getArray("array");
            if (index >= array.size())
                array.resize(index + 1, std::make_shared<ConfigNode>());
            return *array[index];
        }

        void pushArrayElement(const std::string& key, std::shared_ptr<ConfigNode> element)
        {
            getArray(key).push_back(element);
        }

        void setString(const std::string& key, const std::string& value)
        {
            data[key] = value;
        }

        void setInt(const std::string& key, int value)
        {
            data[key] = value;
        }

        void setDouble(const std::string& key, double value)
        {
            data[key] = value;
        }

        void setBool(const std::string& key, bool value)
        {
            data[key] = value;
        }

        void setArray(const std::string& key, const std::vector<std::string>& values)
        {
            data[key] = values;
        }

        void print(int indent = 0) const
        {
            std::string prefix(indent, ' ');
            std::cout << prefix << "{\n";

            size_t count = 0;
            size_t total = data.size();

            for (auto it = data.begin(); it != data.end(); ++it, ++count)
            {
                const auto& key = it->first;
                const auto& value = it->second;

                std::cout << prefix << "  \"" << key << "\": ";

                if (std::holds_alternative<std::string>(value))
                {
                    std::cout << "\"" << std::get<std::string>(value) << "\"";
                }
                else if (std::holds_alternative<int>(value))
                {
                    std::cout << std::get<int>(value);
                }
                else if (std::holds_alternative<double>(value))
                {
                    std::cout << std::get<double>(value);
                }
                else if (std::holds_alternative<bool>(value))
                {
                    std::cout << (std::get<bool>(value) ? "true" : "false");
                }
                else if (std::holds_alternative<std::vector<std::string>>(value))
                {
                    const auto& arr = std::get<std::vector<std::string>>(value);
                    std::cout << "[\n";
                    for (size_t i = 0; i < arr.size(); ++i)
                    {
                        std::cout << prefix << "    \"" << arr[i] << "\"";
                        if (i < arr.size() - 1) std::cout << ",";
                        std::cout << "\n";
                    }
                    std::cout << prefix << "  ]";
                }
                else if (std::holds_alternative<std::vector<std::shared_ptr<ConfigNode>>>(value))
                {
                    const auto& arr = std::get<std::vector<std::shared_ptr<ConfigNode>>>(value);
                    std::cout << "[\n";
                    for (size_t i = 0; i < arr.size(); ++i)
                    {
                        arr[i]->print(indent + 4);
                        if (i < arr.size() - 1) std::cout << ",";
                        std::cout << "\n";
                    }
                    std::cout << prefix << "  ]";
                }
                else if (std::holds_alternative<std::shared_ptr<ConfigNode>>(value))
                {
                    std::cout << "\n";
                    std::get<std::shared_ptr<ConfigNode>>(value)->print(indent + 4);
                }

                if (count < total - 1) std::cout << ",";
                std::cout << "\n";
            }

            std::cout << prefix << "}\n";  // 🔹 Csak a legutolsó '}' után teszünk sortörést
        }
    };
}

#endif
