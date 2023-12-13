#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
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
    ret->response_code = 0;
    ret->numberOfFolders = 0;
    ret->size = 0;

    int numberOfFolders = 0;
    long long size = 0;
    
    DIR *dir = opendir(path);
    
    if (dir == NULL) {
        perror("Failed to open directory\n");
        ret->response_code = -1;
        return ret;
    }

    struct dirent* dp;
    struct stat sb;

    while ((dp = readdir(dir)) != NULL) {
        errno = 0;

        if (dp) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
                continue;

            char temp[strlen(path) + strlen(dp->d_name) + 1];
            strcpy(temp, path);
            strcat(temp, "/");
            strcat(temp, dp->d_name);

            if (stat(temp, &sb) == 0) {
                if (S_ISDIR(sb.st_mode)) {
                    ret->numberOfFolders++;

                    struct returnValues *ret_subdir = folderAnalysis(temp);

                    if (ret_subdir->response_code == -1) {
                        ret->response_code = -1;
                        closedir(dir);
                        free(ret_subdir);
                        return ret;
                    }

                    ret->numberOfFolders += ret_subdir->numberOfFolders;
                    ret->size += ret_subdir->size;

                    free(ret_subdir);

                } else {
                    ret->size += sb.st_size;
                }
            } else {
                perror("Failed to get file stats\n");
                ret->response_code = -1;
                closedir(dir);
                return ret;
            }
        } else {
            if (errno == 0) {
                closedir(dir);
                return ret;
            }

            closedir(dir);
            ret->response_code = -1;
            return ret;
        }
    }

    closedir(dir);
    return ret;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Given number of arguments: %i, expected: 1\n", (argc-1));
        return EXIT_FAILURE; 
    }

    const char* path = argv[1];
    struct returnValues *ret = folderAnalysis(path);

    if (ret->response_code == -1) {
        perror("Failed to retrieve information about directory\n");

        if (ret != NULL)
            free(ret);

        return EXIT_FAILURE;
    } else {
        printf("Number of folders: %i\n", ret->numberOfFolders);
        printf("%f MB\n", ret->size / 1e6);
    }

    if (ret != NULL) 
        free(ret);

    return EXIT_SUCCESS;
}