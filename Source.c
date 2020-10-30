#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<time.h>
#include <ctype.h>
#include <stdbool.h>

void Separate_Command(char cmd[30], char* argv[5]) {
    int i = 0;
    int j = 0;
    int k = 0;
    while (cmd[i] != '\0') {
        if (cmd[i] != ' ') {
            argv[k][j] = cmd[i];
            i++;
            j++;
        }
        if (cmd[i] == ' ') {
            argv[k][j] = '\0';
            j = 0;
            k++;
        }
    }
}

void Delete_invalidCharacter(char* s) {
    int s_length = strlen(s) - 1;
    if (s[s_length] == '\n')
        s[s_length] = '\0';
}


int main() {
    int need_to_wait = 1;
    char cmd[30];
    char temp[30];
    char* argv[5] = { NULL };
    printf("\nWelcome to My Simple Shell ~ ~ ~\n\n");
    while (1) {

        printf("My Shell > ");
        fgets(cmd, sizeof(cmd), stdin);
        Delete_invalidCharacter(cmd);
        argv[0] = cmd;
        //Separate_Command(cmd, argv);
        pid_t pid = fork();

        if (strcmp(cmd, "exit") == 0) { break; }

        if (pid < 0)
        {
            printf("*** ERROR: forking child process failed\n");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            execvp(cmd, argv);
            printf("*** ERROR: INVALID COMMAND\n");
            exit(EXIT_FAILURE);
        }
        //parent process
        else {
            if (need_to_wait)
            {
                while (wait(NULL) != pid);
            }
            else {
                printf("[1]%d\n", pid);
            }
        }
    }
}