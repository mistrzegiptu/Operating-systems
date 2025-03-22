#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ftw.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_BUFF_SIZE 1024

#pragma region FunctionDefinition
int flip_file(const char *sourceFilename, const char *targetFilename);
int move_files_from_tmp(const char *direction);
#pragma endregion FunctionDefinition

int nftw_func(const char *filepath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    char *filepathDup = malloc(sizeof(char)*(strlen(filepath) + 1));
    if(filepathDup == NULL){
        perror("Error allocating");
        return -1;
    }
    strncpy(filepathDup, filepath, strlen(filepath)+1);
    char *filename = basename(filepathDup);
    char *extension = strrchr(filename, '.');

    if(extension != NULL && strcmp(extension, ".txt") == 0){
        char *destination = malloc(sizeof(char) * 100);
        sprintf(destination, "./tmp/flipped_%s", filename);
        flip_file(filepath, destination);
        free(destination);
    }

    free(filepathDup);
    return 0;
}

int main(int argc, char *argv[]){

    if (argc != 3) {
        perror("Wrong args number\n");
        return -1;
    }

    const char *inputDir = argv[1];
    const char *outputDir = argv[2];

    struct stat st;
    if (stat(inputDir, &st) == -1){
        perror("Error there is no input directory");
        return -1;
    }
    if (stat(outputDir, &st) == -1){
        mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
        if (mkdir(outputDir, mode) == -1){
            perror("Error creating output directory");
            return -1;
        }
    }
    else if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error '%s' is file.\n", outputDir);
        return -1;
    }

    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    mkdir("./tmp", mode);

    int flags = FTW_F | FTW_D;
    int res = nftw(inputDir, nftw_func, 10, flags);
    if(res != 0){
        perror("Error with res");
        return -1;
    }
    move_files_from_tmp(outputDir);
    rmdir("./tmp");
    return 0;
}

int flip_file(const char *sourceFilename, const char *targetFilename){

    FILE *sourceFile = fopen(sourceFilename, "r");
    FILE *targetFile = fopen(targetFilename, "w");

    if(sourceFile == NULL || targetFile == NULL){
        perror("Error");
        return -1;
    }

    char *buffer = malloc(sizeof(char) * MAX_BUFF_SIZE);
    if(buffer == NULL){
        perror("Error allocating");
        fclose(sourceFile);
        fclose(targetFile);

        return -1;
    }

    while(fgets(buffer, MAX_BUFF_SIZE, sourceFile) != NULL){

        int buffSizeMultiplier = 2;
        while(buffer[strlen(buffer) - 1] != '\n'){

            void *newBuffer = realloc(buffer, sizeof(char) * MAX_BUFF_SIZE * buffSizeMultiplier);
            if(newBuffer == NULL){
                perror("Error reallocating");
                free(buffer);
                fclose(sourceFile);
                fclose(targetFile);

                return -1;
            }
            buffer = newBuffer;

            buffSizeMultiplier += 1;

            if(fgets(buffer, MAX_BUFF_SIZE * buffSizeMultiplier, sourceFile) == NULL)
                break;
        }
        size_t bufferLen = strlen(buffer);
        size_t flippedLen = bufferLen;
        char *flippedBuffer = malloc(sizeof(char) * (flippedLen + 1));
        if(flippedBuffer == NULL){
            perror("Error allocating");
            free(buffer);
            fclose(sourceFile);
            fclose(targetFile);

            return -1;
        }

        if(buffer[bufferLen - 1] == '\n'){
            flippedLen -= 1;
        }

        for(size_t i = 0; i < flippedLen; i++){
            flippedBuffer[i] = buffer[flippedLen - i - 1];
        }

        if(buffer[bufferLen - 1] == '\n'){
            flippedBuffer[flippedLen] = '\n';
            flippedLen += 1;
        }

        flippedBuffer[flippedLen] = '\n';

        if(fwrite(flippedBuffer, sizeof(char), flippedLen, targetFile) != flippedLen){
            perror("Error writing to file");
            fclose(sourceFile);
            fclose(targetFile);
            free(flippedBuffer);

            return -1;
        }

        free(flippedBuffer);
    }
    free(buffer);
    fclose(sourceFile);
    fclose(targetFile);

    return 0;
}

int move_files_from_tmp(const char *direction){
    DIR *dir = opendir("./tmp");
    struct dirent *entry;
    struct stat buffer;

    if (dir == NULL) {
        perror("Error opening directory");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", "./tmp", entry->d_name);

        if (stat(filepath, &buffer) == -1) {
            perror("Error getting file info");
            closedir(dir);
            return -1;
        }

        if (!S_ISDIR(buffer.st_mode)) {
            char newPath[1024];
            snprintf(newPath, sizeof(filepath), "%s/%s", direction, entry->d_name);
            rename(filepath, newPath);
        }
    }

    closedir(dir);
    return 0;
}