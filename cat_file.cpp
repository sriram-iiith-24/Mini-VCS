#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include "header.h"

using namespace std;
using namespace std::filesystem;

string extractObjectType(const string& header) {
    return header.substr(0, header.find(' '));
}

size_t extractObjectSize(const string& header) {
    return stoull(header.substr(header.find(' ') + 1));
}

void processContent(const string& content, const string& header, char flag) {
    string type = extractObjectType(header);
    size_t size = extractObjectSize(header);

    if(type != "blob" && type != "tree" && type != "commit") {
        throw runtime_error("Invalid object type: " + type);
    }

    switch(flag) {
        case 'p':
            if(type == "tree") {
                cout << "tree " << content;
            } else if(type == "commit") {
                cout << content;
            } else {
                cout << content;
            }
            cout << "\n";
            break;
        case 't':
            cout << type << endl;
            break;
        case 's':
            cout << size << endl;
            break;
        default:
            throw runtime_error("Invalid flag: " + string(1, flag));
    }
}

void catFile(const string& hash, char flag) {
    if(!isValidRepo()) {
        throw runtime_error("Not a mygit repository");
    }

    string dirName = ".mygit/objects/" + hash.substr(0, 2);
    string fileName = hash.substr(2);
    string filePath = dirName + "/" + fileName;
    
    ifstream file(filePath, ios::binary);
    if(!file) {
        throw runtime_error("Object not found: " + hash);
    }

    stringstream decompressedContent;
    string header = stream_decompress(file, decompressedContent);
    processContent(decompressedContent.str(), header, flag);
}