#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fts.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <math.h>

struct returnValues {
    // response_code = -1 if it failed, 0 if it succeeded
    int response_code;
    int numberOfFolders;
    long size;
};

struct output {
    char path[PATH_MAX];
    struct returnValues data;
};

struct output returnOutput[PATH_MAX];
int indexOutput = 0;

void * folderAnalysis(const char* path) {
    struct returnValues *ret = malloc(2 * sizeof(int) + sizeof(long long));
    ret->response_code = 0;
    ret->numberOfFolders = 0;
    ret->size = 0;

    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Failed to open directory\n");
        ret->response_code = -1;
        return ret;
    }

    int saveIndex = indexOutput;
    strcpy(returnOutput[saveIndex].path, path);

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
                    indexOutput++;

                    struct returnValues *ret_subdir = folderAnalysis(temp);

                    if (ret_subdir->response_code == -1) {
                        ret->response_code = -1;
                        closedir(dir);
                        free(ret_subdir);
                        returnOutput[saveIndex].data = *ret;
                        return ret;
                    }

                    ret->size += ret_subdir->size;

                    free(ret_subdir);

                } else {
                    ret->size += sb.st_size;
                }
            } else {
                perror("Failed to get file stats\n");
                ret->response_code = -1;
                closedir(dir);
                returnOutput[saveIndex].data = *ret;
                return ret;
            }
        } else {
            if (errno == 0) {
                closedir(dir);
                returnOutput[saveIndex].data = *ret;
                return ret;
            }

            closedir(dir);
            ret->response_code = -1;
            returnOutput[saveIndex].data = *ret;
            return ret;
        }
    }

    returnOutput[saveIndex].data = *ret;

    closedir(dir);
    return ret;
}

float calculatePercent(long size, long fullSize) {
    return (size * 100) / fullSize;
}

// TODO: make this recursive
void analyzeOutput(const struct output returnOutput[], int numberOfFolders, int pathSizeOfParent) {
    int j = 1;
    for (int i=0; i<numberOfFolders; i++) {
        int numberOfSubFolders = returnOutput[j].data.numberOfFolders;
        printf("|-%s/ %.2f%\t%.1fMB\n",returnOutput[j].path + pathSizeOfParent, calculatePercent(returnOutput[j].data.size, returnOutput[0].data.size), returnOutput[j].data.size / 1e6);
        int k = j+1;
        if (numberOfSubFolders < 1) {
            j = k;
            continue;
        }
        for (; k<j+1+numberOfSubFolders; k++) {
            printf("|-%s/ %.2f%\t%.1fMB\n",returnOutput[k].path + pathSizeOfParent, calculatePercent(returnOutput[k].data.size, returnOutput[0].data.size), returnOutput[k].data.size / 1e6);
        }
        j = k;
        printf("\n");
    }
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
        bool fullPath = true;
        if (fullPath == false) {
            printf("Number of folders: %i\n", ret->numberOfFolders);
            printf("%f MB\n", ret->size / 1e6);
        } else {
            printf("%s/ %.2f%\t%.1fMB\n|\n",returnOutput[0].path, 100.0f, returnOutput[0].data.size / 1e6);
            int pathSizeOfParent = strlen(returnOutput[0].path);
            analyzeOutput(returnOutput, returnOutput[0].data.numberOfFolders, pathSizeOfParent);
            /*
            for (int i=1; i<=indexOutput; i++) {
                printf("|-%s/ %.2f%\t%.1fMB\n",returnOutput[i].path + pathSizeOfParent, calculatePercent(returnOutput[i].data.size, ret->size), returnOutput[i].data.size / 1e6);
                int numberOfFolders = returnOutput[i].data.numberOfFolders;
                int j = i+1;
                for (; j<= i + numberOfFolders; j++) {
                    printf("|-%s/ %.2f%\t%.1fMB\n", returnOutput[j].path + pathSizeOfParent, calculatePercent(returnOutput[j].data.size, ret->size), returnOutput[j].data.size / 1e6);
                }
                printf("|\n");
                i = j+1;
            }*/
        }
    }

    if (ret != NULL)
        free(ret);

    return EXIT_SUCCESS;
}