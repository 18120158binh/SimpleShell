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

char* History = NULL;
int need_to_wait = 1;

void Exec_NormalCommand(char* Command, char** Argv) {
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("*** ERROR: forking child process failed\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        execvp(Command, Argv);
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

void Redirect_Input(char** Command, char** FileName) {
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("*** ERROR: forking child process failed\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        int Input_fds = open(FileName[0], O_RDONLY);
        if (Input_fds < 0) {
            printf("*** ERROR: Cannot open files.\n");
            exit(EXIT_FAILURE);
        }
        if (dup2(Input_fds, STDIN_FILENO) < 0) {
            printf("*** ERROR: Unable to duplicate file descriptor.\n");
            exit(EXIT_FAILURE);
        }
        execvp(Command[0], Command);
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

void Redirect_Output(char** Command, char** FileName) {
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("*** ERROR: forking child process failed\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        int Output_fds = open(FileName[0], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        if (Output_fds < 0) {
            printf("*** ERROR: Cannot open files.\n");
            exit(EXIT_FAILURE);
        }
        if (dup2(Output_fds, STDOUT_FILENO) < 0) {
            printf("*** ERROR: Unable to duplicate file descriptor.\n");
            exit(EXIT_FAILURE);
        }
        execvp(Command[0], Command);
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

int CheckCommand(char* s) {
    int s_length = strlen(s) - 1;
    int i;
    for (i = 0; i < s_length; i++) {
        if (s[i] == '>') return 2;
        if (s[i] == '<') return 1;
        if (s[i] == '|') return 3;
    }
    return 0;
}

//ham tach cau lenh thanh token theo white space DELIM
#define TOKEN_BUFSIZE 64
char** parse(char* Comamnd)
{
    int bufSize = TOKEN_BUFSIZE;
    int pos = 0;
    char* token;
    char** tokenArr = (char**)malloc(sizeof(char*) * bufSize);

    if (!tokenArr)
    {
        printf("Alloction Error!\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(Comamnd, "<>");
    tokenArr[0] = token;

    token = strtok(NULL, "\0");
    tokenArr[1] = token;
    tokenArr[2] = NULL;

    return tokenArr;
}

//ham tach cau lenh thanh token theo white space DELIM
#define TOKEN_BUFSIZE 64
#define DELIM " \t\r\n\a"
char** parseSpace(char* line)
{
    int bufSize = TOKEN_BUFSIZE;
    int pos = 0;
    char* token;
    char** tokenArr = (char**)malloc(sizeof(char*) * bufSize);

    if (!tokenArr)
    {
        perror("alloction error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIM);

    while (token != NULL)
    {
        tokenArr[pos] = token;
        pos++;

        if (pos >= bufSize)
        {
            bufSize += TOKEN_BUFSIZE;
            tokenArr = (char**)realloc(tokenArr, bufSize * sizeof(char*));
            if (!tokenArr)
            {
                printf("Allocation Error");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, DELIM);

    }
    tokenArr[pos] = NULL;

    return tokenArr;
}

void Delete_invalidCharacter(char* s) {
    int s_length = strlen(s) - 1;
    if (s[s_length] == '\n' || s[s_length] == ' ')
        s[s_length] = '\0';
    if(s[0] == ' ') s = strtok(s, " ");
}
 

int main() {
    char cmd[30];
    char** argv = NULL;
    char** argv1 = NULL;
    char** argv2 = NULL;
    printf("\nWelcome to My Simple Shell ~ ~ ~\n\n");
    while (1) {

        printf("My Shell > ");
        fgets(cmd, sizeof(cmd), stdin);

        Delete_invalidCharacter(cmd);

        if (cmd[0] == '\0') continue;
        if (strcmp(cmd, "exit") == 0) break;

        if (strcmp(cmd, "!!") != 0 && cmd[0] != (char)'\0') {
            if (History != NULL) free(History);
            History = (char*)malloc(strlen(cmd) + 1);
            strcpy(History, cmd);
        }

        if (strcmp(cmd, "!!") == 0) {
            if (History == NULL) {
                printf("No command in history!\n");
                continue;
            }
            else
            {
                strcpy(cmd, History);
                printf("My Shell > ");
                printf(cmd);
                printf("\n");
            }
        }

        int TypeCmd = CheckCommand(cmd);
        if (TypeCmd == 1) {
            argv = parse(cmd);
            Delete_invalidCharacter(argv[0]);
            argv1 = parseSpace(argv[0]);
            Delete_invalidCharacter(argv[1]);
            argv2 = parseSpace(argv[1]);
            Redirect_Input(argv1, argv2);
        }
        if (TypeCmd == 2) {
            argv = parse(cmd);
            Delete_invalidCharacter(argv[0]);
            argv1 = parseSpace(argv[0]);
            Delete_invalidCharacter(argv[1]);
            argv2 = parseSpace(argv[1]);
            Redirect_Output(argv1, argv2);
        }
        if (TypeCmd == 0) {
            argv1 = parseSpace(cmd);
            Exec_NormalCommand(cmd, argv1);
        }
    }
}
