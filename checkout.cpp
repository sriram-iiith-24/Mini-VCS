#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <map>
#include <set>
#include "header.h"

using namespace std;
using namespace std::filesystem;

string extractTreeFromCommit(const string& commitHash) {
    string objPath = ".mygit/objects/" + commitHash.substr(0,2) + "/" + commitHash.substr(2);
    ifstream commitFile(objPath, ios::binary);
    if(!commitFile) {
        throw runtime_error("Commit object not found: " + commitHash);
    }

    stringstream decompressedContent;
    string header = stream_decompress(commitFile, decompressedContent);
    
    string line;
    getline(decompressedContent, line);
    if(line.substr(0, 5) != "tree ") {
        throw runtime_error("Invalid commit object format");
    }
    return line.substr(5);
}

void restoreFromTree(const string& treeHash) {
    string objPath = ".mygit/objects/" + treeHash.substr(0,2) + "/" + treeHash.substr(2);
    ifstream treeFile(objPath, ios::binary);
    if(!treeFile) {
        throw runtime_error("Tree object not found: " + treeHash);
    }

    stringstream decompressedContent;
    string header = stream_decompress(treeFile, decompressedContent);
    string content = decompressedContent.str();

    istringstream contentStream(content);
    string line;
    while(getline(contentStream, line)) {
        istringstream iss(line);
        string mode, type, hash, filePath;
        iss >> mode >> type >> hash;
        getline(iss >> ws, filePath);

        if(type == "blob") {
            string blobPath = ".mygit/objects/" + hash.substr(0,2) + "/" + hash.substr(2);
            ifstream blobFile(blobPath, ios::binary);
            if(!blobFile) {
                throw runtime_error("Failed to read blob: " + hash);
            }

            path dirPath = path(filePath).parent_path();
            if(!dirPath.empty()) {
                create_directories(dirPath);
            }

            stringstream blobContent;
            string blobHeader = stream_decompress(blobFile, blobContent);
            string fileContent = blobContent.str();
            ofstream outFile(filePath, ios::binary);
            if(!outFile) {
                throw runtime_error("Failed to create file: " + filePath);
            }
            outFile.write(fileContent.data(), fileContent.size());
            outFile.close();
            if(mode == "100755") {
                permissions(filePath, 
                    perms::owner_exec | perms::group_exec,
                    perm_options::add);
            }
        }
    }
}
void updateIndex(const string& treeHash) {
    string objPath = ".mygit/objects/" + treeHash.substr(0,2) + "/" + treeHash.substr(2);
    ifstream treeFile(objPath, ios::binary);
    if(!treeFile) {
        throw runtime_error("Failed to read tree for index update");
    }

    stringstream content;
    stream_decompress(treeFile, content);
    
    ofstream indexFile(".mygit/index", ios::trunc);
    string line;
    while(getline(content, line)) {
        if(!line.empty()) {
            indexFile << line << endl;
        }
    }
    indexFile.close();
}

void updateHEAD(const string& commitHash) {
    ofstream headFile(".mygit/HEAD", ios::trunc);
    if(!headFile) {
        throw runtime_error("Failed to update HEAD");
    }
    headFile << commitHash;
    headFile.close();
}

void checkout(const string& commitHash) {
    if(!isValidRepo()) {
        throw runtime_error("Not a mygit repository");
    }

    try {
        cout << "Extracting tree from commit..." << endl;
        string treeHash = extractTreeFromCommit(commitHash);
        cout << "Found tree hash: " << treeHash << endl;
        
        cout << "Restoring files..." << endl;
        restoreFromTree(treeHash);

        cout << "Updating index..." << endl;
        updateIndex(treeHash);
        
        cout << "Updating HEAD..." << endl;
        updateHEAD(commitHash);
        
        cout << "Successfully checked out " << commitHash.substr(0,7) << endl;
    } catch(const exception& e) {
        throw runtime_error("Checkout failed: " + string(e.what()));
    }
}