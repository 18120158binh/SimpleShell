
/**
* After reading user input, the steps are:
* (1) fork a child process using fork()
* (2) the child process will invoke execvp()
* (3) parent will invoke wait() unless command included &
*/
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

#define nullptr ((void*)0)
#define isPipe 1
#define isOutRedirect 2
#define isInRedirect 3
#define isSimpleCommand 4

// bien toan cuc luu cau lenh truoc nhat
char* history = nullptr;


//dem so phan tu trong mang con tro parse
int num_of_parse(char** parse)
{
    int i = 1;
    while (parse[i] != nullptr)
    {
        i++;
    }
    return i;
}

//ham xoa  
//mac dinh la xoa whitespace o dau va cuoi chuoi nhu " \n\t\r\a"
//neu co gan c vao thi xoa ki tu do o CUOI CHUOI (viet chu yeu dung de xoa ki tu & cuoi cau trong chuoi)
void delete_white_space_or_character(char* str, char c)
{
    c = '\0';
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

// ham kiem tra co dau & o cuoi cau khong
bool is_ampersand(char** parse)
{
    bool res = false;
    int n = num_of_parse(parse);
    if (strcmp(parse[n - 1], "&") == 0)
    {
        res = true;
        parse[n - 1] = nullptr; //xoa dau & vi shell khong hieu
    }
    else // lo nguoi dung nhap dinh lien, vd ls&
    {
        int len = strlen(parse[n - 1]);
        char tmp = parse[n - 1][len - 1];
        if (tmp == '&')
        {
            res = true;
            delete_white_space_or_character(parse[n - 1], tmp);
        }
    }

    return res;
}

// Ham nhan cau lenh tu nguoi dung, 
//neu cau lenh la !! thi khong luu vao history, nguoc lai thi luu
//ham tra ve cau lenh cua nguoi dung
char* take_input()
{
    char* line = nullptr;
    size_t bufSize = 0;

    int numChar = getline(&line, &bufSize, stdin);

    if (line[numChar - 1] == '\n')
    {
        line[numChar - 1] = '\0';
    }


    //history feature
    if (strcmp(line, "!!") != 0 && line[0] != '\0')
    {
        if (history != nullptr) free(history);
        history = (char*)malloc(strlen(line) + 1);
        strcpy(history, line);
    }
    return line;
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
                perror("allocation error");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, DELIM);

    }
    tokenArr[pos] = nullptr;

    return tokenArr;
}

// ham thuc thi lenh binh thuong
void execArgv(char** parse)
{
    bool found_amp = is_ampersand(parse);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        execvp(parse[0], parse);
        perror("exevcp failed");
        exit(EXIT_FAILURE);
    }
    //parent process
    else if (!found_amp)
    {
        waitpid(pid, NULL, 0);
    }
    return;
}

/* Ham kiem tra cau lenh nhap vao inputLine
-neu cau lenh nhap vao la PIPE, argv1 se luu nhung token o ben trai cua |, argv2 luu token ben phai
-neu la REDIRECT, argv1 luu token o ben trai cua (<,>), argv2 luu ben phai (ten file)
-con neu cau lenh binh thuong, luu tat ca vao argv1, argv2 = null
*/
int checkInput(char* inputLine, char** argv1, char** argv2)
{
    int type = isSimpleCommand;
    char** tmp = nullptr;
    int i;
    for (i = 0; i < strlen(inputLine); i++)
    {
        if (inputLine[i] == '|')
        {
            type = isPipe;
            break;
        }

        else if (inputLine[i] == '>')
        {
            type = isOutRedirect;
            break;
        }

        else if (inputLine[i] == '<')
        {
            type = isInRedirect;
            break;
        }
    }

    if (type != isSimpleCommand)
    {
        /*tmp = parsePipe(inputLine);
        argv1 = parseSpace(tmp[0]);
        argv2 = parseSpace(tmp[1]);*/
    }
    else
    {
        argv1 = parseSpace(inputLine);
        argv2 = nullptr;
    }

    return type;

}

void shell_loop()
{
    char* inputLine;
    char** tokenArr1 = nullptr;
    char** tokenArr2 = nullptr;
    int status;
    char* intro = (char*)"TuongSHELL > ";
    char* noHistory = (char*)"No commands in history!!!\n";

    while (1)
    {
        write(STDOUT_FILENO, intro, strlen(intro));

        //take input
        inputLine = take_input();

        //check input (check nhung truong hop NULL, exit, !!)
        delete_white_space_or_character(inputLine, '\0');

        // neu nhap cau rong
        if (inputLine[0] == (char)'\0')
        {
            continue;
        }

        if (strcmp(inputLine, "exit") == 0)  // neu nhap exit
        {
            waitpid(-1, &status, 0);
            break;
        }

        if (strcmp(inputLine, "!!") == 0)    // neu nhap !! de truy cap lich su
        {
            if (history == nullptr) //neu khong co lenh gan day trong lich su
            {
                write(STDOUT_FILENO, noHistory, strlen(noHistory));
                continue;
            }
            else
            {
                strcpy(inputLine, history);
                write(STDOUT_FILENO, intro, strlen(intro));
                write(STDOUT_FILENO, inputLine, strlen(inputLine));
                write(STDOUT_FILENO, "\n", strlen("\n"));
            }
        }

        //kiem tra cau lenh nhap vao la loai nao trong (pipe, redirect hay cau lenh binh thuong)
        int type = checkInput(inputLine, tokenArr1, tokenArr2);


        if (type == isOutRedirect)
        {
            /* output_redirection(tokenArr1, tokenArr2);*/
        }
        else if (type == isInRedirect)
        {
            /*input_redirect(tokenArr1, tokenArr2);*/
        }
        else if (type == isPipe)
        {
            /*execPipe(tokenArr1, tokenArr2);
            usleep(20);*/
        }
        else
        {
            execArgv(tokenArr1);
        }

    }
}

int main(void)
{
    shell_loop();
    return EXIT_SUCCESS;
}


//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
//#include <unistd.h>
//
//#include <sys/types.h>
//#include <sys/wait.h>
//#define MAX LINE 80 /* The maximum length command */
//#define BUFFER_LEN 1024
//int main(void)
//{
//    char line[1024];  //get command line
//    char* argv[100];        //user command
//    char* path = "/bin/";    //set path at bin
//    char progpath[20];      //full file path
//    int argc;               //arg count
//
//    printf("SIMPLE SHELL: Type 'exit' or send EOF to exit.\n");
//    while (1) {
//        printf("My Shell >");
//        fflush(stdout);
//
//        if (!fgets(line, 1024, stdin)) {  //get command and put it in line
//            break;                                //if user hits CTRL+D break
//        }
//
//        int pid = fork();              //fork child
//
//        if (pid == 0) {               //Child
//            if (execvp(progpath, argv) < 0) {
//                printf("\nCould not execute command..");
//            }
//            exit(0);
//        }
//        else {                    //Parent
//            wait(NULL);
//            printf("Child exited\n");
//        }
//    }
//    return 0;
//}