#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <malloc.h>

#define LINE_SIZE 1024

int file_is_directory(char* path) {
    struct stat st;
    stat(path, &st);
    return S_ISDIR(st.st_mode);
}

char* join_path(const char* path1, const char* path2) {
    char* result = malloc(strlen(path1) + strlen(path2) + 2);
    strcpy(result, path1);
    strcat(result, "/");
    strcat(result, path2);
    return result;
}

void dirs(char*pre_path, char* text, char* type) {
    struct dirent* de;
    DIR *dir = opendir(pre_path);

    if(dir == NULL){
        return;
    }

    while((de = readdir(dir)) != NULL){
        char* path = de->d_name;

        if(strcmp(path,".") == 0 || strcmp(path, "..") == 0 ){
            continue;
        }

        if(path == NULL) {
            continue;
        }
        char* abs_path = join_path(pre_path, path);

        if(file_is_directory(abs_path)) {
            dirs(abs_path, text, type); 
        } else {
            if(strcmp(type, "--t") == 0) {
                char* line = malloc(sizeof(char)*LINE_SIZE);
                
                FILE* file = fopen(abs_path, "r");

                if(file == NULL) {
                    printf("file not found %s \n", abs_path);
                    continue;
                }

                int line_num = 1;
                while(fgets(line, LINE_SIZE, file)){
                    char* word = strtok(line, " ");

                    while(word != NULL) {
                        if(strcmp(word, text) == 0) {     
                            printf("Found '%s' at file '%s' at line %d \n", text, abs_path, line_num); 
                            break;
                        } 

                        word = strtok(NULL, " ");
                    }
                    ++line_num;

                }

                free(line);
                fclose(file);
                
                line = NULL;
                file = NULL;

            } else if(strcmp(type, "--p") == 0){

                regex_t reg;
                int value = regcomp(&reg, text, 0);
                char* line = malloc(sizeof(char)*LINE_SIZE);

                FILE* file = fopen(abs_path, "r");

                if(file == NULL) {
                    printf("file not found %s \n", abs_path);
                    continue;
                }

                int line_num = 1;
                while(fgets(line, LINE_SIZE, file)) {

                    if(regexec(&reg, line, 0, NULL, 0) == 0) {
                        printf("Found '%s' at file '%s' at line %d \n", text, abs_path, line_num); 
                    }

                    ++line_num;
                }

                free(line);
                regfree(&reg);
                fclose(file);

                line = NULL;
                file = NULL;
            }
        }

        free(abs_path);
        abs_path = NULL;
    }
    
    closedir(dir);
    dir = NULL;
}

int main(int argc, char** argv) {
    int index = 1;

    if(argc == 1) {
        printf("No arguments inputed. Try --help");
        return 1;
    }

    if(strcmp(argv[index], "--help") == 0) {
        printf("Usage : fetch --t [TEXT] --f [FILE]... \n");
        printf("Example : fetch --l --t 'hello world' --f 'home/user/desktop/main.h' main.c \n");
        printf("\n");
        printf("Options : \n");
        printf("--f     File names or file paths to search. If left out it looks at all files in current directory and sub directories! \n");
        printf("--t     The text to search for in file(s), must be a exact word seperated by spaces. Example, test(). \n");
        printf("--p     A pattern/regex to find pattern in a file. Value can be text or a regex. Example, main\( or \{. \n");
        return 0;
    } else if(strcmp(argv[index],"--t") == 0) {
        char* text = argv[++index];

        if(index+1 < argc && strcmp(argv[++index], "--f") == 0){
            while(index < argc) {
                char* path = argv[++index];
                char* line = malloc(sizeof(char)*LINE_SIZE);

                if(path == NULL) {
                    break;
                }

                FILE* file = fopen(path, "r");

                if(file == NULL){
                    printf("file not found \n");
                    continue;
                }

                int line_num = 1;
                while(fgets(line, sizeof(line), file)) {
                    char* word = strtok(line, " ");

                    while(word != NULL) {
                        if(strcmp(word, text) == 0) {     
                            printf("Found %s at file %s at line %d \n", text, path, line_num); 
                            break;
                        } 

                        word = strtok(NULL, " ");
                    }
                    ++line_num;
                }

                free(line);
                fclose(file);
                line = NULL;
                file = NULL;
            }
        } else {
            dirs(".", text, "--t");
        }
    } else if(strcmp(argv[index],"--p") == 0) {

        char* pattern = argv[++index];

        if(index+1 < argc && strcmp(argv[++index], "--f") == 0){
            regex_t reg;
            int value = regcomp(&reg, pattern, 0);
            if(value == 0) {

                while(index < argc) {
                    char* path = argv[++index];
                    char* line = malloc(sizeof(char)*LINE_SIZE);

                    if(path == NULL) {
                        break;
                    }

                    FILE* file = fopen(path, "r");

                    if(file == NULL){
                        continue;
                    }

                    int line_num = 1;
                    while(fgets(line, sizeof(line), file)) {
                        if(regexec(&reg, line, 0, NULL, 0) == 0) {
                            printf("Found %s at file %s at line %d \n", pattern, path, line_num); 
                        }

                        ++line_num;
                    }

                    fclose(file);
                    free(line);

                    file = NULL;
                    line = NULL;
                }
                regfree(&reg);
            }
        } else {
            dirs(".", pattern, "--p");
        }

    } else {
        printf("Invalid option or options. Try --help to see options.");
    }


    return 0;
}

