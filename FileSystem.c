#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fts.h>
#include <sys/stat.h>
#include <sys/types.h>

struct returnValues {
    // response_code = -1 if it failed, 0 if it succeeded
    int response_code;
    int numberOfFolders;
    long long size;
};

void * folderAnalysis(const char* path) {
    struct returnValues *ret = malloc(2 * sizeof(int) + sizeof(long long));
    ret->response_code = -1;
    ret->numberOfFolders = 0;
    ret->size = 0;

    int numberOfFolders = 0;
    long long size = 0;
    
    DIR *dir = opendir(path);
    
    if (dir == NULL) {
        perror("Failed to open directory\n");
        return ret;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Given number of arguments: %i, expected: 1\n", (argc-1));
        return EXIT_FAILURE; 
    }

    const char* path = argv[1];
    struct returnValues *ret = folderAnalysis("test");

    if (ret->response_code == -1) {
        perror("Failed to retrieve information about directory\n");

        if (ret != NULL) 
            free(ret);
            
        return EXIT_FAILURE;
    }


    if (ret != NULL) 
        free(ret);

    return EXIT_SUCCESS;
}