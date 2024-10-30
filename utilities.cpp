#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>
#include <openssl/sha.h>
#include <zstd.h>
#include "header.h"
#include <algorithm>

using namespace std;
using namespace std::filesystem;

const size_t BUFFER_SIZE = 8* 1024*1024;

string calculate_sha1(ifstream& file) {
    SHA_CTX sha1;
    SHA1_Init(&sha1);

    vector<char> buffer(BUFFER_SIZE);
    while (file.read(buffer.data(), BUFFER_SIZE)) {
        SHA1_Update(&sha1, buffer.data(), BUFFER_SIZE);
    }
    if (file.gcount() > 0) {
        SHA1_Update(&sha1, buffer.data(), file.gcount());
    }

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash, &sha1);
    
    stringstream ss;
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

void stream_compress(istream& input, ostream& output) {
    size_t const cSize = ZSTD_CStreamOutSize();
    vector<char> in_buffer(BUFFER_SIZE);
    vector<char> out_buffer(cSize);

    ZSTD_CStream* zcs = ZSTD_createCStream();
    if (zcs == nullptr) throw runtime_error("ZSTD_createCStream() failed");

    ZSTD_initCStream(zcs, 22);

    size_t read;
    while ((read = input.read(in_buffer.data(), BUFFER_SIZE).gcount()) > 0) {
        ZSTD_inBuffer input_buf = { in_buffer.data(), read, 0 };
        while (input_buf.pos < input_buf.size) {
            ZSTD_outBuffer output_buf = { out_buffer.data(), out_buffer.size(), 0 };
            size_t const remaining = ZSTD_compressStream(zcs, &output_buf, &input_buf);
            if (ZSTD_isError(remaining)) {
                ZSTD_freeCStream(zcs);
                throw runtime_error(ZSTD_getErrorName(remaining));
            }
            output.write(out_buffer.data(), output_buf.pos);
        }
    }

    ZSTD_outBuffer output_buf = { out_buffer.data(), out_buffer.size(), 0 };
    size_t const remaining = ZSTD_endStream(zcs, &output_buf);
    if (remaining > 0) {
        ZSTD_freeCStream(zcs);
        throw runtime_error("Compression ended early");
    }
    output.write(out_buffer.data(), output_buf.pos);
    output.flush();
    ZSTD_freeCStream(zcs);
}


string stream_decompress(ifstream& input, stringstream& output) {
    size_t const dSize = ZSTD_DStreamOutSize(); 
    vector<char> in_buffer(BUFFER_SIZE);
    vector<char> out_buffer(dSize);

    ZSTD_DStream* zds = ZSTD_createDStream();
    if (zds == nullptr) {
        throw runtime_error("ZSTD_createDStream() failed");
    }

    ZSTD_initDStream(zds);
    string header;
    getline(input, header, '\0');
    while (!input.eof()) {
        input.read(in_buffer.data(), in_buffer.size());
        size_t const inSize = input.gcount();
        if (inSize == 0) break;

        ZSTD_inBuffer input_buf = { in_buffer.data(), inSize, 0 };
        while (input_buf.pos < input_buf.size) {
            ZSTD_outBuffer output_buf = { out_buffer.data(), out_buffer.size(), 0 };
            size_t const ret = ZSTD_decompressStream(zds, &output_buf, &input_buf);
            if (ZSTD_isError(ret)) {
                ZSTD_freeDStream(zds);
                throw runtime_error(string("ZSTD error: ") + ZSTD_getErrorName(ret));
            }
            output.write(out_buffer.data(), output_buf.pos);
        }
    }

    ZSTD_freeDStream(zds);
    return header;
}
uintmax_t get_file_size(const string& filepath) {
    return file_size(filepath);
}

void printFileHex(const string& filepath) {
    ifstream file(filepath, ios::binary);
    if (!file) return;
    
    vector<unsigned char> buffer(32);
    file.read(reinterpret_cast<char*>(buffer.data()), 32);
    size_t bytesRead = file.gcount();
    
    cout << "First " << bytesRead << " bytes:" << endl;
    for(size_t i = 0; i < bytesRead; i++) {
        cout << hex << setw(2) << setfill('0') << (int)buffer[i] << " ";
    }
    cout << endl;
}

string hash_object(const string& filepath, bool write) {
    ifstream file(filepath, ios::binary);
    if (!file) {
        throw runtime_error("Cannot open file: " + filepath);
    }
    
    stringstream contentBuffer;
    contentBuffer << file.rdbuf();
    string fileContent = contentBuffer.str();
    
    uintmax_t file_size = fileContent.length();
    string header = "blob " + to_string(file_size) + '\0';
    
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, header.c_str(), header.length());
    SHA1_Update(&sha1, fileContent.c_str(), fileContent.length());
    
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash, &sha1);
    
    stringstream ss;
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    string hash_str = ss.str();

    if (write) {
        string dir_name = ".mygit/objects/" + hash_str.substr(0, 2);
        string file_name = hash_str.substr(2);
        create_directories(dir_name);
        
        ofstream out_file(dir_name + "/" + file_name, ios::binary);
        if (!out_file) {
            throw runtime_error("Cannot create object file");
        }

        out_file.write(header.data(), header.size());
        stringstream content_stream(fileContent);
        stream_compress(content_stream, out_file);
        out_file.close();
    }
    
    return hash_str;
}
string hashContent(const string& content,const string& type="tree") {
    string header = type + " " + to_string(content.length()) + '\0';
    
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, header.c_str(), header.length());
    SHA1_Update(&sha1, content.c_str(), content.length());
    
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash, &sha1);
    
    stringstream ss;
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string hash_tree(const string& content) {
    vector<string> contentLines;
    stringstream ss(content);
    string line;
    while (getline(ss, line)) {
        contentLines.push_back(line);
    }
    sort(contentLines.begin(), contentLines.end());

    string sortedContent;
    for (const auto& line : contentLines) {
        sortedContent += line + "\n";
    }
    
    string header = "tree " + to_string(sortedContent.length()) + '\0';
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, header.c_str(), header.length());
    SHA1_Update(&sha1, sortedContent.c_str(), sortedContent.length());
    
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash, &sha1);
    
    stringstream hashStream;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        hashStream << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    string hash_str = hashStream.str();

    string dir_name = ".mygit/objects/" + hash_str.substr(0, 2);
    string file_name = hash_str.substr(2);
    create_directories(dir_name);

    ofstream out_file(dir_name + "/" + file_name, ios::binary);
    if (!out_file) {
        throw runtime_error("Cannot create tree object file");
    }

    out_file.write(header.data(), header.size());
    stringstream input(sortedContent);
    stream_compress(input, out_file);
    out_file.close();

    return hash_str;
}