#include "utils.h"

bool inputCorrect(int argc, char*argv[]){
    if(argc != 3){
        printf("usage: <port> <directory>\n");
        return false;
    }

    try{
        int port = stoi(argv[1]);
        if(port < 0 || port > 65535){
            printf("program arguments error: wrong port value\n");
            return false;
        }
    }
    catch(invalid_argument const &ex){
        printf("program arguments error: wrong port format\n");
        return false;
    }

    const string pathString = "./" + (string)(argv[2]);
    const filesystem::path path(pathString); 

    error_code ec;
    if (!filesystem::is_directory(path, ec))
    { 
        printf("program arguments error: not a directory\n");
        return false;
    }

    return true;
}

bool readFileIntoString(const string& path, vector<char>& message,
                        string type) {
    ifstream input_file;

    if (type.find("image/") != string::npos) {
        input_file.open(path, ios::binary);
    } else {
        input_file.open(path);
    }

    if (!input_file.is_open()) {
        return false;
    }

    vector<char> vec;

    if (!input_file.eof() && !input_file.fail()) {
        input_file.seekg(0, std::ios_base::end);
        std::streampos fileSize = input_file.tellg();
        vec.resize(fileSize);

        input_file.seekg(0, std::ios_base::beg);
        input_file.read(&vec[0], fileSize);
    }

    message = vec;

    return true;
}

bool fileExists(string path) {
    ifstream input_file(path);

    if (!input_file.is_open()) {
        return false;
    }

    input_file.close();

    return true;
}

int getFileSize(const string& path) {
    ifstream input_file(path, ios::binary);
    input_file.seekg(0, ios::end);
    return input_file.tellg();
}
