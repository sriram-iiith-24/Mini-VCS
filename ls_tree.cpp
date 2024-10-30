#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include "header.h"

using namespace std;
using namespace std::filesystem;

void printTreeContents(const string& treeHash, bool nameOnly, const string& prefix = "") {
    string folder = treeHash.substr(0, 2);
    string file = treeHash.substr(2);
    string path = ".mygit/objects/" + folder + "/" + file;

    if(!exists(path)) {
        throw runtime_error("Tree object not found: " + treeHash);
    }

    ifstream treeFile(path, ios::binary);
    stringstream output;
    stream_decompress(treeFile, output);

    string line;
    while(getline(output, line)) {
        if(line.empty()) continue;

        istringstream lineStream(line);
        string mode, name, hash;
        lineStream >> mode >> name >> hash;

        if(nameOnly) {
            cout << prefix << name << (mode == "40000" ? "/" : "") << endl;
        } else {
            cout << prefix << mode << " ";
            if(mode == "40000") {
                cout << "tree ";
            } else {
                cout << "blob ";
            }
            cout << hash << "\t" << name << endl;
        }

        if(mode == "40000") {
            printTreeContents(hash, nameOnly, prefix + (nameOnly ? "" : "    "));
        }
    }
}

void listTree(const string& treeHash, bool nameOnly) {
    if(!isValidRepo()) {
        throw runtime_error("Not a mygit repository");
    }

    if(treeHash.length() != 40) {
        throw runtime_error("Invalid tree hash");
    }

    try {
        printTreeContents(treeHash, nameOnly);
    } catch(const exception& e) {
        throw runtime_error("Failed to list tree: " + string(e.what()));
    }
}