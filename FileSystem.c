#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>

struct returnValues {
    // response_code = -1 if it failed, 0 if it succeeded
    int response_code;
    int numberOfFolders;
    int numberOfFiles;
    long size;
};

struct output {
    char path[PATH_MAX];
    struct returnValues data;
};

// TODO: how to estimate current progress
struct task_info {
    int suspend_index;
    struct output returnOutput[PATH_MAX];
    struct returnValues running_info;
} taskInfo;

int indexOutput = 0;

void * folderAnalysis(const char* path) {
    /*while(isSuspended) {
        sleep(1);
    }*/

    struct returnValues *ret = malloc(2 * sizeof(int) + sizeof(long long));
    ret->response_code = 0;
    ret->numberOfFolders = 0;
    ret->size = 0;
    ret->numberOfFiles = 0;

    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Failed to open directory\n");
        ret->response_code = -1;
        return ret;
    }

    int saveIndex = indexOutput;
    strcpy(taskInfo.returnOutput[saveIndex].path, path);

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
                    taskInfo.running_info.numberOfFolders ++;
                    indexOutput++;
                    /*
                        // ! adaugam atribut de isAnalyzed pentru a putea da resume
                        if (isSuspended == false) {
                            // continuam parcurgerea recursiva
                            struct returnValues *ret_subdir = folderAnalysis(temp);
                        } else {
                            // salvam indexul la care suntem
                            lastIndex = savedIndex;
                            // lasam functia sa se termine natural (pentru a scrie tot ce a analizat in taskInfo.returnOutput
                        }
                    */
                    struct returnValues *ret_subdir = folderAnalysis(temp);

                    if (ret_subdir->response_code == -1) {
                        ret->response_code = -1;
                        closedir(dir);
                        free(ret_subdir);
                        taskInfo.returnOutput[saveIndex].data = *ret;
                        return ret;
                    }

                    ret->size += ret_subdir->size;
                    ret->numberOfFiles += ret_subdir->numberOfFiles;

                    free(ret_subdir);

                } else {
                    taskInfo.running_info.size += sb.st_size;
                    taskInfo.running_info.numberOfFiles++;

                    ret->size += sb.st_size;
                    ret->numberOfFiles++;
                }
            } else {
                perror("Failed to get file stats\n");
                ret->response_code = -1;
                closedir(dir);
                taskInfo.returnOutput[saveIndex].data = *ret;
                return ret;
            }
        } else {
            if (errno == 0) {
                closedir(dir);
                taskInfo.returnOutput[saveIndex].data = *ret;
                return ret;
            }

            closedir(dir);
            ret->response_code = -1;
            taskInfo.returnOutput[saveIndex].data = *ret;
            return ret;
        }
    }

    taskInfo.returnOutput[saveIndex].data = *ret;

    closedir(dir);
    return ret;
}

float calculatePercent(long size, long fullSize) {
    return (size * 100) / fullSize;
}

void analyzeOutput(struct output returnOutput[], int pathSizeOfParent) {
    int numberOfFolders = returnOutput[0].data.numberOfFolders;
    int i = 1;

    while (numberOfFolders) {
        int counter = 0;
        for (; i<=indexOutput; i++) {
            printf("|-%s/ %.2f%%\t%.1fMB\n",
                   returnOutput[i].path + pathSizeOfParent,
                   calculatePercent(returnOutput[i].data.size, returnOutput[0].data.size),
                   returnOutput[i].data.size / 1e6);

            counter += returnOutput[i].data.numberOfFolders;
            if (counter < 1) {
                i++;
                break;
            }

            counter--;
        }

        if (numberOfFolders > 1) printf("|\n");
        numberOfFolders--;
    }
}

int main(int argc, char* argv[]) {
    for(int i=0; i<PATH_MAX; i++) {
        taskInfo.returnOutput[i].data.response_code = 0;
        taskInfo.returnOutput[i].data.size = 0;
        taskInfo.returnOutput[i].data.numberOfFolders = 0;
        taskInfo.returnOutput[i].data.numberOfFiles = 0;
    }

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
            printf("%s/ %.2f%%\t%.1fMB\n|\n",taskInfo.returnOutput[0].path, 100.0f, taskInfo.returnOutput[0].data.size / 1e6);
            int pathSizeOfParent = strlen(taskInfo.returnOutput[0].path);
            analyzeOutput(taskInfo.returnOutput, pathSizeOfParent);
        }
    }

    if (ret != NULL)
        free(ret);

    return EXIT_SUCCESS;
}

// TODO: percent doesnt add up, because of rounding
// TODO: create another thread to analize folder BFS (count nr. of of folders it has)
        // and count the number of folder analized