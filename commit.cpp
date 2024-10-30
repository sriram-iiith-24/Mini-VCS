#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <filesystem>
#include <vector>
#include <map>
#include <algorithm>
#include "header.h"

using namespace std;
using namespace std::filesystem;

struct TreeEntry {
    string mode;
    string type;
    string hash;
    string name;
};

string formatTimestamp() {
    time_t now = time(nullptr);
    struct tm* lt = localtime(&now);
    stringstream ss;
    ss << lt->tm_year + 1900 << "-"
       << setfill('0') << setw(2) << lt->tm_mon + 1 << "-"
       << setfill('0') << setw(2) << lt->tm_mday << " "
       << setfill('0') << setw(2) << lt->tm_hour << ":"
       << setfill('0') << setw(2) << lt->tm_min << ":"
       << setfill('0') << setw(2) << lt->tm_sec;
    return ss.str();
}

string buildTreeFromIndex() {
    stringstream treeContent;
    ifstream indexFile(".mygit/index");
    string line;
    vector<string> entries;

    while(getline(indexFile, line)) {
        entries.push_back(line);
    }
    sort(entries.begin(), entries.end());

    for(const string& entry : entries) {
        treeContent << entry << "\n";
    }

    return hash_tree(treeContent.str());
}

string getCurrentBranch() {
    ifstream headFile(".mygit/HEAD");
    string ref;
    getline(headFile, ref);
    if(ref.substr(0, 5) == "ref: ") {
        return ref.substr(16);
    }
    return "detached";
}

string getParentCommit() {
    string branch = getCurrentBranch();
    if(branch == "detached") return "";
    
    ifstream branchFile(".mygit/refs/heads/" + branch);
    if(!branchFile) return "";
    
    string commitHash;
    getline(branchFile, commitHash);
    return commitHash;
}

string createCommit(const string& treeHash, const string& parentHash, const string& message) {
    stringstream commitContent;
    string timestamp = formatTimestamp();
    
    commitContent << "tree " << treeHash << "\n";
    if(!parentHash.empty()) {
        commitContent << "parent " << parentHash << "\n";
    }
    
    commitContent << "author MyGit <mygit@example.com> " << timestamp << "\n"
                 << "committer MyGit <mygit@example.com> " << timestamp << "\n\n"
                 << message << "\n";

    string commitData = commitContent.str();
    string commitHash = hashContent(commitData,"commit");

    string objDir = ".mygit/objects/" + commitHash.substr(0, 2);
    create_directories(objDir);
    
    ofstream commitFile(objDir + "/" + commitHash.substr(2), ios::binary);
    string header = "commit " + to_string(commitData.length()) + '\0';
    commitFile.write(header.data(), header.size());
    stringstream input(commitData);
    stream_compress(input, commitFile);

    return commitHash;
}
void updateRefs(const string& commitHash) {
    string branch = getCurrentBranch();
    if(branch == "detached") {
        ofstream headFile(".mygit/HEAD", ios::trunc);
        headFile << commitHash;
    } else {
        create_directories(".mygit/refs/heads");
        ofstream branchFile(".mygit/refs/heads/" + branch, ios::trunc);
        branchFile << commitHash;
    }
}


void verifyAndUpdateIndex() {
    vector<string> existingEntries;
    ifstream indexFile(".mygit/index");
    string line;
    while(getline(indexFile, line)) {
        existingEntries.push_back(line);
    }

    for(const auto& entry : existingEntries) {
        istringstream iss(entry);
        string mode, type, hash, path;
        iss >> mode >> type >> hash;
        getline(iss >> ws, path);

        if(exists(path)) {
            string newHash = hash_object(path, false);
            if(newHash != hash) {
                throw runtime_error("File " + path + " has modifications that are not staged for commit\nUse 'mygit add' to update");
            }
        }
    }
}

void commit(const string& message) {
    if(!isValidRepo()) {
        throw runtime_error("Not a mygit repository");
    }

    if(message.empty()) {
        throw runtime_error("Empty commit message");
    }

    if(!exists(".mygit/index")) {
        throw runtime_error("Nothing to commit (create/copy files and use 'mygit add')");
    }
    verifyAndUpdateIndex();  

    string treeHash = buildTreeFromIndex();
    string parentHash = getParentCommit();
    string commitHash = createCommit(treeHash, parentHash, message);
    updateRefs(commitHash);
    
    cout << "[" << (parentHash.empty() ? "root" : parentHash.substr(0,7))
         << " -> " << commitHash.substr(0,7) << "] " << message << endl;
}