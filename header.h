#ifndef HEADER_H
#define HEADER_H

#include <string>
#include <vector>
using namespace std;
using namespace std::filesystem;


bool init();
bool isValidRepo();
string hash_object(const string& filepath, bool write);
string process_hash_object(const string& filepath, bool write);
void stream_compress(istream& input, ostream& output);
string hashContent(const string& content,const string& type);
string stream_decompress(ifstream& input, stringstream& output);
void catFile(const string& hash, char flag);
string hash_tree(const string& content);
string writeTree(const path& dirPath, bool isRoot);
void listTree(const string& treeHash, bool nameOnly);
void add(vector<string>& paths);
void commit(const string& message);
void logCommits();
string getCurrentBranch();
string getParentCommit();
void checkout(const string& commitHash);
#endif