#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


bool areEqual(char a[], char b[]) {

    if(strcmp(a,b) == 0)
        return true;
    else
        return false;
}


char* state_to_num(char* state){

    if(areEqual(state, "on"))
        return "1";
    else
        return "0";
}


char* read_file(char* file_name){

    char* path = "/sys/kernel/kobject_example/";

    char *full_path = malloc(strlen(path)+strlen(file_name)+1);//+1 for the zero-terminator
    strcpy(full_path, path);
    strcat(full_path, file_name);

    FILE* fh;
    fh = fopen(full_path, "r");

    //check if file exists
    if (fh == NULL){
        printf("file does not exists %s", file_name);
        return 0;
    }

    //read line by line
    const size_t line_size = 300;
    char* line = malloc(line_size);
    while (fgets(line, line_size, fh) != NULL) {}
    //free(line);    // dont forget to free heap memory
    if (line[0] == '1')
        line = "on";
    else
        line = "off";

    return line;
}

void write_file(char* file_name, char* state){

    char* path = "/sys/kernel/kobject_example/";

    char *full_path = malloc(strlen(path)+strlen(file_name)+1);//+1 for the zero-terminator
    strcpy(full_path, path);
    strcat(full_path, file_name);

    //full_path = "/home/ahmedelgamal/Desktop/test";
    FILE *f = fopen(full_path, "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    /* print some text */
    fprintf(f, "%s\n", state);
    fclose(f);
}

int main(int argc, char* argv[]){
    if(argc == 3){
        printf("%s\n",read_file(argv[2]));
    }else if(argc == 4){
        write_file(argv[2], state_to_num(argv[3]));
    }

    return 0;
}
