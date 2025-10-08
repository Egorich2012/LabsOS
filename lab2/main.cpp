#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>

using namespace std;

// Цветовые коды
#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[34m"
#define COLOR_GREEN "\033[32m"
#define COLOR_CYAN "\033[36m"

string get_color(const string& filename, const string& path = ".") {
    string full_path = path + "/" + filename;
    
    // Упрощенная версия для Windows
    DIR* test_dir = opendir(full_path.c_str());
    if (test_dir) {
        closedir(test_dir);
        return COLOR_BLUE;  // Директория
    }
    
    // Проверка на исполняемый файл (по расширению для простоты)
    if (filename.find(".exe") != string::npos || 
        filename.find(".bat") != string::npos) {
        return COLOR_GREEN;
    }
    
    return COLOR_RESET;
}

int main(int argc, char* argv[]) {
    vector<string> paths;
    bool show_all = false;
    bool long_format = false;
    
    int opt;
    while ((opt = getopt(argc, argv, "la")) != -1) {
        switch (opt) {
            case 'l': long_format = true; break;
            case 'a': show_all = true; break;
        }
    }
    
    for (int i = optind; i < argc; i++) {
        paths.push_back(argv[i]);
    }
    if (paths.empty()) paths.push_back(".");
    
    DIR* dir = opendir(paths[0].c_str());
    if (!dir) {
        perror("myls");
        return 1;
    }
    
    vector<string> files;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (!show_all && name[0] == '.') continue;
        files.push_back(name);
    }
    closedir(dir);
    
    sort(files.begin(), files.end());
    
    for (const auto& file : files) {
        string color = get_color(file, paths[0]);
        cout << color << file << COLOR_RESET << " ";
    }
    cout << endl;
    
    return 0;
}