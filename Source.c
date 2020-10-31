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

char *History = NULL;

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
                perror("allocation error");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, DELIM);

    }
    tokenArr[pos] = NULL;

    return tokenArr;
}

void Delete_invalidCharacter(char *s){
      int s_length = strlen(s) - 1;
      if(s[s_length] == '\n')
           s[s_length] = '\0';
}
 

int main() {
int need_to_wait = 1;
char cmd[30];
char **argv = NULL;
printf("\nWelcome to My Simple Shell ~ ~ ~\n\n");
while(1){

printf("My Shell > ");
fgets(cmd, sizeof(cmd), stdin);

Delete_invalidCharacter(cmd);
argv = parseSpace(cmd);
pid_t pid = fork();

if(cmd[0] == '\0') continue;
if(strcmp(cmd, "exit") == 0) break; 

if(strcmp(cmd, "!!") != 0 && cmd[0] != (char)'\0'){
	if (History != NULL) free(History);
        History = (char*)malloc(strlen(cmd) + 1);
        strcpy(History, cmd);
}

if(strcmp(cmd, "!!") == 0){
	if(History == NULL) {
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
            while(wait(NULL) != pid);
	}
        else {
            printf("[1]%d\n",pid);
        }
    } 
}
}
