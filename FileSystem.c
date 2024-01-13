#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <unistd.h>
#define MAX_NUMBER_OF_FOLDERS 800

struct returnValues {
    // response_code = -1 if it failed, 0 if it succeeded, 1 if limit has reached
    int response_code;
    int numberOfFolders;
    int numberOfFiles;
    long size;
};

struct output {
    char path[PATH_MAX];
    struct returnValues data;
    float percentage;
};

typedef struct task_info {
    struct output returnOutput[MAX_NUMBER_OF_FOLDERS];
    struct returnValues running_info;
    int suspended;
    
} task_info;


typedef struct task_info_and_path{
    task_info * task;
    char path[PATH_MAX]; 
} task_info_and_path;


int indexOutput = 0;
void * folderAnalysis(void* arguments) {
    task_info_and_path* get_union_taskInfo_path = arguments;
    struct task_info * taskInfo = get_union_taskInfo_path->task; 
    const char* path = get_union_taskInfo_path->path;

    static bool limitReached = false;
    static float totalPercentage = 0;
    struct returnValues *ret = malloc(2 * sizeof(int) + sizeof(long long));

    ret->response_code = 0;
    ret->numberOfFolders = 0;
    ret->size = 0;
    ret->numberOfFiles = 0;

    if (limitReached == true) {
        ret->response_code = 1;
        return ret;
    }

    if (indexOutput >= MAX_NUMBER_OF_FOLDERS-2) {
        taskInfo->returnOutput[indexOutput].data.size = 0;
        taskInfo->returnOutput[indexOutput].data.response_code = 1;
        taskInfo->returnOutput[indexOutput].data.numberOfFolders = 0;
        taskInfo->returnOutput[indexOutput].data.numberOfFiles = 0;

        char temp[strlen(path) + strlen(taskInfo->returnOutput[0].path) + 1];
        strcpy(temp, taskInfo->returnOutput[0].path);
        strcat(temp, "...");
        strcpy(taskInfo->returnOutput[indexOutput].path,temp);

        limitReached = true;
        return ret;
    }

    while (taskInfo->suspended == true) {
        sleep(0.2);
    }

    DIR *bfs_traversal = opendir(path);

    if (bfs_traversal == NULL) {
        perror("Failed to open directory\n");
        ret->response_code = -1;
        return ret;
    }

    int numberFolders = 0;
    struct dirent* bfs;
    struct stat st;
    while ((bfs = readdir(bfs_traversal)) != NULL) {
        if (bfs) {
            if (strcmp(bfs->d_name, ".") == 0 || strcmp(bfs->d_name, "..") == 0 || bfs->d_name[0] == '.')
                continue;

            char temp[strlen(path) + strlen(bfs->d_name) + 1];
            strcpy(temp, path);
            strcat(temp, "/");
            strcat(temp, bfs->d_name);
            if (stat(temp, &st) == 0 && S_ISDIR(st.st_mode)) {
                numberFolders++;
            }
        }
    }

    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Failed to open directory\n");
        ret->response_code = -1;
        return ret;
    }

    int saveIndex = indexOutput;
    strcpy(taskInfo->returnOutput[saveIndex].path, path);

    struct dirent* dp;
    struct stat sb;

    while ((dp = readdir(dir)) != NULL) {
        errno = 0;

        if (dp) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 || dp->d_name[0] == '.')
                continue;

            char temp[strlen(path) + strlen(dp->d_name) + 1];
            strcpy(temp, path);
            strcat(temp, "/");
            strcat(temp, dp->d_name);

            if (stat(temp, &sb) == 0) {
                if (S_ISDIR(sb.st_mode)) {
                    ret->numberOfFolders++;
                    taskInfo->running_info.numberOfFolders ++;
                    indexOutput++;
                    
                    task_info_and_path* union_taskInfo_path=malloc(sizeof(task_info_and_path));
                    union_taskInfo_path->task=get_union_taskInfo_path->task;
                    if (indexOutput < MAX_NUMBER_OF_FOLDERS-2)
                        union_taskInfo_path->task->returnOutput[indexOutput].percentage = taskInfo->returnOutput[saveIndex].percentage / numberFolders;
                    
                    strcpy(union_taskInfo_path->path, temp);

                    struct returnValues *ret_subdir = folderAnalysis(union_taskInfo_path);

                    free(union_taskInfo_path);

                    if (ret_subdir->response_code == -1) {
                        ret->response_code = -1;
                        closedir(dir);
                        free(ret_subdir);
                        taskInfo->returnOutput[saveIndex].data = *ret;
                        return ret;
                    }

                    if (ret_subdir->response_code == 1) {
                        ret->numberOfFolders--;
                    }

                    ret->size += ret_subdir->size;
                    ret->numberOfFiles += ret_subdir->numberOfFiles;

                    free(ret_subdir);

                } else {
                    taskInfo->running_info.size += sb.st_size;
                    taskInfo->running_info.numberOfFiles++;

                    ret->size += sb.st_size;
                    ret->numberOfFiles++;
                }
            } else {
                perror("Failed to get file stats\n");
                ret->response_code = -1;
                closedir(dir);
                taskInfo->returnOutput[saveIndex].data = *ret;
                return ret;
            }
        } else {
            if (errno == 0) {
                closedir(dir);
                taskInfo->returnOutput[saveIndex].data = *ret;
                return ret;
            }

            closedir(dir);
            ret->response_code = -1;
            taskInfo->returnOutput[saveIndex].data = *ret;
            return ret;
        }
    }

    taskInfo->returnOutput[saveIndex].data = *ret;
    if (taskInfo->returnOutput[saveIndex].data.numberOfFolders == 0)
        totalPercentage += taskInfo->returnOutput[saveIndex].percentage;
    printf("%.2f%%\n",totalPercentage);

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
        for (; i<MAX_NUMBER_OF_FOLDERS; i++) {
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

int main() {
    task_info ts;
    task_info_and_path spatiu;
    spatiu.task=&ts;
    
    task_info_and_path* task = &spatiu;

    for(int i=0; i<MAX_NUMBER_OF_FOLDERS; i++) {
        task->task->returnOutput[i].data.response_code = 0;
        task->task->returnOutput[i].data.size = 0;
        task->task->returnOutput[i].data.numberOfFolders = 0;
        task->task->returnOutput[i].data.numberOfFiles = 0;
        task->task->returnOutput[i].percentage = 0;
    }
    task->task->returnOutput[0].percentage = 100;

    strcpy(task->path,"/home/skpha/Desktop/University-Work");

    struct returnValues *ret = folderAnalysis(task);

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
            printf("%s/ %.2f%%\t%.1fMB\n|\n",task->task->returnOutput[0].path, 100.0f, task->task->returnOutput[0].data.size / 1e6);
            int pathSizeOfParent = strlen(task->task->returnOutput[0].path);
            analyzeOutput(task->task->returnOutput, pathSizeOfParent);
        }
    }

    if (ret != NULL)
        free(ret);

    return EXIT_SUCCESS;
}

// TODO: percent doesnt add up, because of rounding