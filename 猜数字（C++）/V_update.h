#pragma once
#include<iostream>
#include<string>
#include<sstream>
#include<vector>
#include<curl/curl.h>

#define Local_Version "4.0.0"
#define Remote_Version_URL "https://raw.githubusercontent.com/ST-Kdream/Guessing_Number_Game/refs/heads/master/version.txt"
#define New_Download_URL "https://github.com/ST-Kdream/Guessing_Number_Game/releases"

size_t WriteCallback(void* contents, size_t size, size_t nmumb, std::string* s);
std::string get_remote_version();
bool is_update(const std::string& local_version,const std::string& remote_version);
void version_check(std::stringstream& output);