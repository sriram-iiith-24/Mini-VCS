#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>
#include <sys/stat.h>
#include <algorithm>
#include "header.h"

using namespace std;
using namespace std::filesystem;

string getFileMode(const path& filePath) {
    struct stat fileStats;
    if(stat(filePath.c_str(), &fileStats) != 0) {
        throw runtime_error("Failed to get file stats");
    }

    if(S_ISDIR(fileStats.st_mode)) {
        return "40000";
    }
    if(S_ISREG(fileStats.st_mode)) {
        if(fileStats.st_mode & S_IXUSR) {
            return "100755"; 
        }
        return "100644"; 
    }
    throw runtime_error("Unsupported file type");
}

string writeTree(const path& dirPath, bool isRoot = true) {
    vector<string> entries;
    
    for(const auto& entry : directory_iterator(dirPath)) {
        string entryPath = entry.path().string();
        string name = entry.path().filename().string();
        
        if(name[0] == '.' || name == "mygit") {
            continue;
        }

        string mode = getFileMode(entry.path());
        string hash;

        if(is_directory(entry.path())) {
            hash = writeTree(entry.path(), false);
        } else {
            hash = hash_object(entryPath, true);
        }

        entries.push_back(mode + " " + name + " " + hash);
    }
    
    sort(entries.begin(), entries.end());
    
    string content;
    for(const auto& entry : entries) {
        content += entry + "\n";
    }
    
    if(content.empty()) {
        throw runtime_error("Empty directory");
    }

    return hash_tree(content);
}