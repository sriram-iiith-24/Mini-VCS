#include <iostream>
#include <fstream>
#include <filesystem>
#include "header.h"

using namespace std;
using namespace std::filesystem;

bool createRepoStructure() {
    try {
        create_directories(".mygit/objects");
        create_directories(".mygit/refs/heads");
        return true;
    } catch(const filesystem_error& e) {
        cout << "Failed to create repository structure: " << e.what() << endl;
        return false;
    }
}

bool initializeRepoFiles() {
    try {
        ofstream head(".mygit/HEAD");
        if(!head.is_open()) {
            throw runtime_error("Failed to create HEAD file");
        }
        head << "ref: refs/heads/main";
        head.close();
        return true;
    } catch(const exception& e) {
        cout << "Failed to initialize repository files: " << e.what() << endl;
        return false;
    }
}

bool init() {
    if(exists(".mygit")) {
        cout << "Repository already exists in " << current_path() << endl;
        return false;
    }
    try {
        if(!createRepoStructure()) {
            throw runtime_error("Failed to create repository structure");
        }
        if(!initializeRepoFiles()) {
            throw runtime_error("Failed to initialize repository files");
        }
        return true;
    } catch(const exception& e) {
        if(exists(".mygit")) {
            remove_all(".mygit");
        }
        cout << "Error initializing repository: " << e.what() << endl;
        return false;
    }
}