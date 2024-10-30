#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>
#include <openssl/sha.h>
#include <zstd.h>
#include "header.h"

using namespace std;
using namespace std::filesystem;

bool isValidRepo() {
    return exists(".mygit") && exists(".mygit/objects") && exists(".mygit/refs/heads") && exists(".mygit/HEAD");
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            throw runtime_error("Usage: ./mygit <command> [<args>]");
        }
        string command = argv[1];
        if(command == "init") {
            if(init()) {
                cout << "Initialized empty MyGit repository in " << current_path() / ".mygit" << endl;
                return 0;
            } else {
                cout << "Failed to initialize repository" << endl;
                return 1;
            }
        }
        else if (command != "init" && !isValidRepo()) {
            throw runtime_error("Not a mygit repository (or any parent directory)");
        }

        else if(command == "hash-object") {
            if(argc < 3) {
                throw runtime_error("Usage: ./mygit hash-object [-w] <file>");
            }
            bool write = false;
            string filepath;
            if(string(argv[2]) == "-w") {
                if(argc < 4) {
                    throw runtime_error("No file specified");
                }
                write = true;
                filepath = argv[3];
            } else {
                filepath = argv[2];
            }
            try {
                string result = hash_object(filepath, write);
                cout << result << endl;  
                cout.flush();  
            } catch(const exception& e) {
                cout << "Error: " << e.what() << endl;
                return 1;
            }
        }
        else if(command == "cat-file") {
            if(argc != 4) {
                throw runtime_error("Usage: ./mygit cat-file [-p|-t|-s] <object>");
            }
            string flag = argv[2];
            if(flag.length() != 2 || flag[0] != '-' || 
            (flag[1] != 'p' && flag[1] != 't' && flag[1] != 's')) {
                throw runtime_error("Invalid flag. Use -p, -t, or -s");
            }
            try {
                catFile(argv[3], flag[1]);
            } catch(const exception& e) {
                cout << "Error: " << e.what() << endl;
                return 1;
            }
        }
        else if(command == "write-tree") {
            try {
                string treeHash = writeTree(".",true);
                cout << treeHash << endl;
            } catch(const exception& e) {
                cout << "Error: " << e.what() << endl;
                return 1;
            }
        }
        else if(command == "ls-tree") {
            bool nameOnly = false;
            string treeHash;
            
            if(argc == 4 && string(argv[2]) == "--name-only") {
                nameOnly = true;
                treeHash = argv[3];
            } else if(argc == 3) {
                treeHash = argv[2];
            } else {
                throw runtime_error("Usage: ./mygit ls-tree [--name-only] <tree-sha>");
            }

            try {
                listTree(treeHash, nameOnly);
            } catch(const exception& e) {
                cout << "Error: " << e.what() << endl;
                return 1;
            }
        }
        else if(command == "add") {
            if(argc < 3) {
                throw runtime_error("Please specify path to add");
            }
            
            vector<string> targetPaths;
            for(int i = 2; i < argc; i++) {
                if(string(argv[i]) == "-h" || string(argv[i]) == "--help") {
                    cout << "Usage: ./mygit add <path1> [path2 ...]" << endl;
                    return 0;
                }
                targetPaths.push_back(argv[i]);
            }
            
            try {
                add(targetPaths);
                cout << "Changes staged for commit" << endl;
            } catch(const exception& e) {
                cout << "Failed to add files: " << e.what() << endl;
                return 1;
            }
        }
        else if(command == "commit") {
            if(argc != 4 || string(argv[2]) != "-m") {
                throw runtime_error("Usage: ./mygit commit -m \"message\"");
            }
            try {
                commit(argv[3]);
            } catch(const exception& e) {
                cout << e.what() << endl;
                return 1;
            }
        }
        else if(command == "log") {
            try {
                logCommits();
            } catch(const exception& e) {
                cout << "Error: " << e.what() << endl;
                return 1;
            }
        }
        else if(command == "checkout") {
            if(argc != 3) {
                throw runtime_error("Usage: ./mygit checkout <commit-hash>");
            }
            try {
                checkout(argv[2]);
            } catch(const exception& e) {
                cout << "Error: " << e.what() << endl;
                return 1;
            }
        }

    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}