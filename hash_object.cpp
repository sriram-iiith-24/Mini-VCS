#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include "header.h"
#include <string>

using namespace std;
using namespace std::filesystem;

bool validateFile(const string& filepath) {
    if(!exists(filepath)) {
        cout << "File does not exist: " << filepath << endl;
        return false;
    }
    if(is_directory(filepath)) {
        cout << "Path is a directory: " << filepath << endl;
        return false;
    }
    return true;
}

string process_hash_object(const string& filepath, bool write) {
    if(!validateFile(filepath)) {
        throw runtime_error("Invalid file");
    }
    try {
        string hash = hash_object(filepath, write);
        cout<<hash;
        if(write) {
            string obj_path = ".mygit/objects/" + hash.substr(0, 2) + "/" + hash.substr(2);
            if(!exists(path(obj_path))) {
                throw runtime_error("Failed to write object");
            }
        }
        return hash;
    } catch(const exception& e) {
        throw runtime_error("Hash-object failed: " + string(e.what()));
    }
}
