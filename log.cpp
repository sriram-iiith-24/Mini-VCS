#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "header.h"

using namespace std;
using namespace std::filesystem;

struct CommitInfo {
    string hash;
    string tree;
    string parent;
    string author;
    string committer;
    string message;
};

CommitInfo parseCommit(const string& commitHash) {
    CommitInfo info;
    info.hash = commitHash;
    
    string objPath = ".mygit/objects/" + commitHash.substr(0,2) + "/" + commitHash.substr(2);
    ifstream commitFile(objPath, ios::binary);
    if(!commitFile) {
        throw runtime_error("Invalid commit: " + commitHash);
    }

    stringstream content;
    stream_decompress(commitFile, content);

    string line;
    bool messageStart = false;
    while(getline(content, line)) {
        if(line.empty()) {
            messageStart = true;
            continue;
        }
        
        if(messageStart) {
            info.message += line + "\n";
            continue;
        }

        string key = line.substr(0, line.find(' '));
        string value = line.substr(line.find(' ') + 1);
        
        if(key == "tree") info.tree = value;
        else if(key == "parent") info.parent = value;
        else if(key == "author") info.author = value;
        else if(key == "committer") info.committer = value;
    }
    
    return info;
}

void displayCommit(const CommitInfo& commit) {
    cout << "commit " << commit.hash << endl;
    if(!commit.parent.empty()) {
        cout << "parent " << commit.parent << endl;
    }
    cout << commit.author << endl;
    cout << "\n    " << commit.message;
    cout << "\n";
}

void logCommits() {
    if(!isValidRepo()) {
        throw runtime_error("Not a mygit repository");
    }

    string currentHash = getParentCommit();
    if(currentHash.empty()) {
        cout << "No commits yet" << endl;
        return;
    }

    while(!currentHash.empty()) {
        try {
            CommitInfo commit = parseCommit(currentHash);
            displayCommit(commit);
            currentHash = commit.parent;
        } catch(const exception& e) {
            throw runtime_error("Error reading commit history: " + string(e.what()));
        }
    }
}