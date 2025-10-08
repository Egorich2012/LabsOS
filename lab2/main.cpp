#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {
    vector<string> paths;
    bool show_all = false;
    bool long_format = false;
    
    int opt;
    while ((opt = getopt(argc, argv, "la")) != -1) {
        switch (opt) {
            case 'l':
                long_format = true;
                break;
            case 'a':
                show_all = true;
                break;
        }
    }
    
   
    for (int i = optind; i < argc; i++) {
        paths.push_back(argv[i]);
    }
    

    if (paths.empty()) {
        paths.push_back(".");
    }
    

    cout << "Debug: long=" << long_format << " all=" << show_all << " paths=" << paths.size() << endl;
    
    return 0;
}