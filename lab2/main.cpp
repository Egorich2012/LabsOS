#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>

using namespace std;

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
    
    // Чтение директории
    DIR* dir = opendir(paths[0].c_str());
    if (!dir) {
        perror("myls");
        return 1;
    }
    
    vector<string> files;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        
        // Пропускаем скрытые файлы если нет -a
        if (!show_all && name[0] == '.') continue;
        
        files.push_back(name);
    }
    closedir(dir);
    
    // Вывод (пока просто список)
    for (const auto& file : files) {
        cout << file << " ";
    }
    cout << endl;
    
    return 0;
}