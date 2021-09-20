#include <helper.hpp>

std::string genid(int id) {
    std::vector<char> basev;
    for (char i=33; i<127; ++i)
        basev.push_back(i);

    std::string encoded;
    int base = basev.size();

    while (id/base>0) {
        encoded = basev[id%base] + encoded;
        id = id/base;
    }
    encoded = basev[id%base] + encoded;

    return encoded;
}

//source: https://stackoverflow.com/a/64166
int memParseLine(char* line){
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int vMemGetValue(){
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = memParseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

int pMemGetValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = memParseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}
