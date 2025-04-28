//-----------------------------------------------------------------------------------------
/**
 * \file       configLoader.h
 * \author     Šárka Prokopová
 * \date       2025/4/27
 * \brief      Init function for loading config.txt
 *
*/
//-----------------------------------------------------------------------------------------
#pragma once
#ifndef __CONFIGLOADER_H
#define __CONFIGLOADER_H
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include "data.h"

/// <summary>
/// loading file with props and parsing them into structure
/// </summary>
/// <param name="filename"></param>
/// <returns></returns>
std::map<std::string, ObjectProp> loadConfig(const std::string& filename);
#endif