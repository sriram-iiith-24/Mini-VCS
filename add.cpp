#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>
#include <map>
#include "header.h"
#include <sys/stat.h>

using namespace std;
using namespace std::filesystem;

struct IndexEntry {
    string mode;
    string type;
    string hash;
    string path;
};

bool readIndexFile(map<string, IndexEntry>& indexMap) {
    ifstream file(".mygit/index");
    string line;
    
    while(getline(file, line)) {
        stringstream ss(line);
        IndexEntry entry;
        ss >> entry.mode >> entry.type >> entry.hash;
        getline(ss >> ws, entry.path);
        indexMap[entry.path] = entry;
    }
    return true;
}

bool isValidPath(const path& filePath) {
    if(filePath.string().find(".mygit") != string::npos) return false;
    if(!exists(filePath)) {
        cout << "Path does not exist: " << filePath << endl;
        return false;
    }
    return true;
}

string constructRelativePath(const path& filePath) {
    return proximate(filePath, current_path()).string();
}

string getEntryMode(const path& filePath) {
    struct stat fileStats;
    if(stat(filePath.c_str(), &fileStats) != 0) {
        throw runtime_error("Failed to get file stats");
    }
    
    if(fileStats.st_mode & S_IXUSR) {
        return "100755";
    }
    return "100644";  
}

void processFile(const path& filePath, map<string, IndexEntry>& indexMap) {
    string relPath = constructRelativePath(filePath);
    string fileHash = hash_object(filePath.string(), true);
    
    IndexEntry entry;
    entry.mode = getEntryMode(filePath);
    entry.type = "blob";
    entry.hash = fileHash;
    entry.path = relPath;
    
    indexMap[relPath] = entry;
}

void processDirectory(const path& dirPath, map<string, IndexEntry>& indexMap) {
    for(const auto& entry : recursive_directory_iterator(dirPath)) {
        if(!is_regular_file(entry) || !isValidPath(entry.path())) continue;
        processFile(entry.path(), indexMap);
    }
}

void updateIndex(const map<string, IndexEntry>& indexMap) {
    ofstream indexFile(".mygit/index", ios::trunc);
    
    for(const auto& [path, entry] : indexMap) {
        if(exists(path)) {
            indexFile << entry.mode << " " 
                     << entry.type << " " 
                     << entry.hash << " " 
                     << entry.path << endl;
        }
    }
}

void processPath(const string& pathStr, map<string, IndexEntry>& indexMap) {
    path currentPath(pathStr);
    
    if(!isValidPath(currentPath)) return;
    
    if(is_directory(currentPath)) {
        processDirectory(currentPath, indexMap);
    } else {
        processFile(currentPath, indexMap);
    }
}

void add(vector<string>& paths) {
    if(!isValidRepo()) {
        throw runtime_error("Repository not initialized");
    }

    create_directories(".mygit/objects");
    
    if(!exists(".mygit/index")) {
        ofstream createIndex(".mygit/index");
        createIndex.close();
    }

    map<string, IndexEntry> indexMap;
    readIndexFile(indexMap);

    for(const auto& path : paths) {
        processPath(path, indexMap);
    }

    updateIndex(indexMap);
}