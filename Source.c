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

//ham xoa 
//mac dinh la xoa whitespace o dau va cuoi chuoi nhu " \n\t\r\a"
//neu co gan c vao thi xoa ki tu do o CUOI CHUOI (viet chu yeu dung de xoa ki tu & cuoi cau trong chuoi)
void delete_white_space_or_character(char* str, char c)
{
    int i;
    int begin = 0, end = strlen(str) - 1;
    if (c == '\0')
    {
        while ((str[begin] == ' ' || str[begin] == '\n' || str[begin] == '\t' || str[begin] == '\a' || str[begin] == '\r') && (begin < end))
        {
            begin++;
        }
        while ((end >= begin) && (str[end] == ' ' || str[end] == '\n' || str[end] == '\t' || str[end] == '\a' || str[end] == '\r'))
        {
            end--;
        }
    }
    else end--;//neu muon xoa ki tu c o cuoi chuoi


    for (i = begin; i <= end; i++)
    {
        str[i - begin] = str[i];
    }
    str[i - begin] = '\0';
}

//Ham thuc thi lenh binh thuong
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

//Ham Redirecting input
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

//Ham Redirecting output
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

//Ham thuc thi Communication via a pipe
void execPipe(char** Argv1, char** Argv2) {
    int pipefd[2];

    if (pipe(pipefd) < 0) {
        printf("*** ERROR: Pipe failed\n");
        exit(EXIT_FAILURE);
    }
    pid_t pid1, pid2;
      
    pid1 = fork(); 
    if (pid1 < 0) { 
        printf("*** ERROR: forking child process failed\n");
        exit(EXIT_FAILURE);
    } 
    if (pid1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
        if (execvp(Argv1[0], Argv1) < 0) { 
            printf("*** ERROR: INVALID COMMAND\n");
            exit(EXIT_FAILURE);
        } 
    } 
      
    pid2 = fork();
    if (pid2 < 0) { 
        printf("*** ERROR: forking child process failed\n");
        exit(EXIT_FAILURE);
    } 
  
    // Child 2 executing.. 
    // It only needs to read at the read end 
    if (pid2 == 0) { 
        close(pipefd[1]); 
        dup2(pipefd[0], STDIN_FILENO); 
        close(pipefd[0]); 
        if (execvp(Argv2[0], Argv2) < 0) { 
            printf("*** ERROR: INVALID COMMAND\n"); 
            exit(EXIT_FAILURE); 
        } 
    } 
    
    close(pipefd[0]); 
    close(pipefd[1]); 
    // parent executing, waiting for two children
    waitpid(pid1, NULL, 0); 
    waitpid(pid2, NULL, 0); 
}

//Ham kiem tra loai cua lenh command
int CheckCommand(char* s) {
    int s_length = strlen(s) - 1;
    int i;
    for (i = 0; i < s_length; i++) {
	if (s[i] == '<') return 1;	
	if (s[i] == '>') return 2;
        if (s[i] == '|') return 3;
    }
    return 0;
}

//ham phan tach chuoi input dau vao theo ki tu < > |
char** parse(char* Comamnd)
{
    int bufSize = 100;
    int pos = 0;
    char* token;
    char** tokenArr = (char**)malloc(sizeof(char*) * 100);

    if (!tokenArr)
    {
        printf("Alloction Error!\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(Comamnd, "<>|");
    delete_white_space_or_character(token, '\0');
    tokenArr[0] = token;

    token = strtok(NULL, "\0");
    delete_white_space_or_character(token, '\0');
    tokenArr[1] = token;
    tokenArr[2] = NULL;

    return tokenArr;
}

//Ham tra ve toan bo chuoi sau lenh grep/echo
char* Normalization(char* s) {
	int n = strlen(s);
	int signal = 0;
	int i = 0;
	int j = 0;
	char* temp = (char*)malloc(sizeof(char) * 30);
	for (i = 4; i < n; i++) {
		if (s[i] != ' ') { signal = 1; }
		if (signal == 1) {
			temp[j] = s[i];
			j++;
		}
	}
	temp[j] = '\0';
	return temp;
}

//Ham phan tach chuoi input dau vao theo khoang trang
#define DELIM " \t\r\n\a"
char** parseSpace(char* line)
{
    int i = 0;
    char* token;
    char* temp = (char*)malloc(sizeof(char) * 30);
    strcpy(temp, line);
    char** tokenArr = (char**)malloc(sizeof(char*) * 100);

    if (!tokenArr)
    {
        printf("*** ERROR: Allocation Error!\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIM);

    while (token != NULL)
    {
        tokenArr[i] = token;
        i++;
        if (strcmp(token, "grep") == 0 || strcmp(token, "echo") == 0)
        {
            tokenArr[i] = Normalization(temp);
	    i++;
            break;
        }
        if (i >= 100)
        {
            tokenArr = (char**)realloc(tokenArr, 150 * sizeof(char*));
            if (!tokenArr)
            {
                printf("*** ERROR: Allocation Error!\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, DELIM);
    }
    tokenArr[i] = NULL;

    return tokenArr;
}

//Ham chuan hoa bo dau "" va \n
void Delete_invalidCharacter(char* s) {
	int s_length = strlen(s) - 1;
	int i = 0;
	int j = 0;
	int count = 0;
	if (s[s_length] == '\n')
		s[s_length] = '\0';
	for (i = 0; i < s_length; i++) {
		if (s[i] == '"') {
			i++;
			count++;
		}
		s[j] = s[i];
		j++;
	}
	s[s_length - count] = '\0';
}
 

int main() {
    char cmd[100];
    char** argv = NULL;
    char** argv1 = NULL;
    char** argv2 = NULL;
    printf("\nWelcome to My Simple Shell ~ ~ ~\n\n");
    while (1) {

        printf("My Shell > ");
	fflush(stdin);
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
	if (TypeCmd == 0) {
            argv1 = parseSpace(cmd);
            Exec_NormalCommand(cmd, argv1);
        }
        else {
            argv = parse(cmd);
            argv1 = parseSpace(argv[0]);
            argv2 = parseSpace(argv[1]);
            if (TypeCmd == 1) {
                Redirect_Input(argv1, argv2);
            }
            if (TypeCmd == 2) {
                Redirect_Output(argv1, argv2);
            }
            if (TypeCmd == 3) {
                execPipe(argv1, argv2);
            }
        }
    }
}
