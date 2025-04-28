//-----------------------------------------------------------------------------------------
/**
 * \file       confidLoader.cpp
 * \author     Šárka Prokopová
 * \date       2025/4/27
 * \brief      Implemented loading file and parsing props, handling errors when
 *              file unreadable or something occurs. Done for specific format
 *
*/
//-----------------------------------------------------------------------------------------
#include "configLoader.h"

std::map<std::string, ObjectProp> loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if(!file.is_open()) {
        std::cerr << "Error: file '" << filename << "' is unable to load" << std::endl;
        return {};
    }

    std::string line;
    std::map<std::string, ObjectProp> objects;
    std::string currentObject;

    int lineNumber = 0;
    while (std::getline(file, line)) {
        lineNumber++;
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line.back() == ']') {
            currentObject = line.substr(1, line.size() - 2);
            if (currentObject.empty()) {
                std::cerr << "File on line " << lineNumber << ": empty name!" << std::endl;
                continue;
            }
            objects[currentObject] = ObjectProp();
        }
        else {
            auto delimiterPos = line.find('=');
            if (delimiterPos == std::string::npos) {
                std::cerr << "File on line " << lineNumber << ": expected '='." << std::endl;
                continue;
            }
            if (currentObject.empty()) {
                std::cerr << "File on line " << lineNumber << ": klic mimo sekci!" << std::endl;
                continue;
            }

            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            ObjectProp& obj = objects[currentObject];

            try {
                if (key == "front") {
                    std::stringstream ss(value);
                    float x, y, z;
                    char comma1, comma2;
                    if (!(ss >> x >> comma1 >> y >> comma2 >> z) || comma1 != ',' || comma2 != ',') {
                        throw std::runtime_error("Wrong format 'front'");
                    }
                    obj.front = glm::vec3(x, y, z);
                }
                else if (key == "up") {
                    std::stringstream ss(value);
                    float x, y, z;
                    char comma1, comma2;
                    if (!(ss >> x >> comma1 >> y >> comma2 >> z) || comma1 != ',' || comma2 != ',') {
                        throw std::runtime_error("Wrong format 'up'");
                    }
                    obj.up = glm::vec3(x, y, z);
                }
                else if (key == "position") {
                    std::stringstream ss(value);
                    float x, y, z;
                    char comma1, comma2;
                    if (!(ss >> x >> comma1 >> y >> comma2 >> z) || comma1 != ',' || comma2 != ',') {
                        throw std::runtime_error("Wrong format 'position'");
                    }
                    obj.position = glm::vec3(x, y, z);
                }
                else if (key == "size") {
                    obj.size = std::stof(value);
                }
                else if (key == "angle") {
                    obj.angle = std::stof(value);
                }
                else if (key == "align") {
                    obj.align = (value == "true" || value == "1");
                }
                else {
                    std::cerr << "Warning: " << lineNumber << ": unknown key '" << key << "'." << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error: " << lineNumber << " parsing '" << key << "': " << e.what() << std::endl;
            }
        }
    }

    return objects;
}