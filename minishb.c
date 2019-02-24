#include<stdio.h> 
#include<sys/types.h> 
#include<unistd.h> 
#include<stdlib.h> 
#include<string.h> 
#include<signal.h> 
#include<fcntl.h> 
#include<errno.h>
#define DELIM " \t\r\n\a"
#define BACKGROUND 10 	//BACKGROUND swicth case type
#define INPUT_REDIRECTION 20 //INPUT_REDIRECTION swicth case type
#define OUTPUT_REDIRECTION 30 //OUTPUT_REDIRECTION swicth case type
#define NORMAL 40
#define PIPELINE 50			//PIPELINE swicth case type

pid_t minish_id;

typedef struct process
{
pid_t pid;                  /* process ID */
pid_t pid_child;
int status ;                 /* reported status value  set default to 0 ie foreground. 1 for background*/
}
process;
process proc[100] ={0};
static int counter=0;

//Enumeration defined for buffer size
enum{ MAXLINE = 120 };

char buf1[MAXLINE]; // for input the fgets buf1
char * arg[MAXLINE]; // for tokeninsing the arg
char * arg1[MAXLINE];
pid_t pid1, status; 
int argsize,argsize1;
int pointer; //pointer for redirecting when file handling
int pointerp; 	//pointerp for redirecting when file handling
int backgroundflag; //backgroundflag for 
int argsizep = 0; 
char * temp[MAXLINE]; // temp for pipeline 
int tsize = 0; //tsize for temp

void handle_SIGINT()
{}

//Signal handler for SIGCHLD signal
void Signalhandler() 
{
waitpid(pid1, & status, WNOHANG); 
}

//Function for returning the type of command in the minish
int Type() {
int i;
for (i = 0; i < argsize; ++i) {
if (strcmp(arg[i], "&") == 0)
{
backgroundflag = 1;
arg[i] = NULL;
return BACKGROUND;
}
if (strcmp(arg[i], "<") == 0)
{
backgroundflag = 0;
arg[i] = NULL;
pointer = i;
return INPUT_REDIRECTION;
}
if (strcmp(arg[i], ">") == 0) 
{
backgroundflag = 0;
arg[i] = NULL;
pointer = i;
return OUTPUT_REDIRECTION;
}}
if (i == argsize)
{
backgroundflag = 0;
return NORMAL;
}}

//Tokenizer which seperates the Deliminators from the input
void tokenizer() {
int i;
char * token;
token = strtok(buf1, DELIM);
i = 0;
while (token) {
arg[i++] = token;
token = strtok(NULL, DELIM);
}
arg[i] = NULL;
argsize = i;
}

//Execution function

void execute(int type) {

pid_t pid1, status;
int ec; 
int fd; // for file descriptors

proc[counter].pid=getpid();

pid1 = fork();

if (pid1 < 0) 
{
printf("Fork Failed\n");
}


if (pid1 == 0) 
{
switch (type)
{
	
case BACKGROUND: 	//Foreground process
{
if(backgroundflag == 1)
{
signal(SIGINT,SIG_IGN);	
ec = execvp(arg[0], arg);
if (ec == -1)
{
printf("\n Exec() Error" );
}}
break;
}

case INPUT_REDIRECTION: // < input redirection
{
fd = open(arg[pointer + 1], O_RDONLY);          // opening the file
if(fd == -1)
{
printf("\n No Such file present\n");	
exit(0);
}
else
{
//dup2 for file descriptor
if (dup2(fd, 0) < 0) 
{
perror("Dup Error:");
exit(0);
}
else
{
close(fd);
ec = execvp(arg[0], arg);
if (ec == -1)
{
printf("\n Exec() Error" );
}}}
signal(SIGINT,handle_SIGINT);
break;
}

case OUTPUT_REDIRECTION:
{  
//For creating the file returns the file descriptor
fd = creat(arg[pointer + 1], 0644);
if(fd == -1)
{
printf("\n File creation failed\n");	
}
else
{
//dup2 for file descriptor
if (dup2(fd, 1) < 0) 
{
perror("Dup Error:");
exit(0);
}
else
{
close(fd); //closing the file descriptor
ec = execvp(arg[0], arg);
if (ec == -1)
{
printf("\n Exec() Error" );
}}}
signal(SIGINT,handle_SIGINT);
break;
}

case NORMAL:		
{
signal(SIGINT,handle_SIGINT);
ec = execvp(arg[0], arg);
if (ec == -1)
{
printf("\n Exec() Error" );
}
break;
}}}

if (pid1 > 0) {
proc[counter].pid_child=pid1;
if (backgroundflag == 1) {
proc[counter].status=1;		
printf("Process:%d is in background\n", pid1);
} 
else
{
proc[counter].status=0;	
waitpid(pid1, & status, 0);
}
counter++;
}
}


//Tokenizer for Pipe 
void tokenizerp() {

int i;
char * token;
token = strtok(buf1, "|");
i = 0;
while (token) {
arg[i++] = token;
token = strtok(NULL, "|");
}
arg[i] = NULL;
argsizep = i;
}

//Checking the pipe is their or not
int checkpipe()
{
int i;
i = 0;
while(buf1[i]!= '\0')
{
if(buf1[i] == '|')
{
return PIPELINE;
}
i++;
}		
}


//Returns mode of PIPELINE
int Typep()
{
int i;
for (i = 0; i < tsize; ++i) {
if (strcmp(temp[i], "<") == 0)
{
backgroundflag = 0;
temp[i] = NULL;
pointerp = i;
return INPUT_REDIRECTION;
}
if (strcmp(temp[i], ">") == 0) 
{
backgroundflag = 0;
temp[i] = NULL;
pointerp = i;
return OUTPUT_REDIRECTION;
}}

if (i == tsize)
{
backgroundflag = 0;
return NORMAL;
}
}






//Execution of loop pipe

void looppipe()
{
int fd[2];
pid_t pid;
int fdd = 0;
int in,out;
int k = 0;
char * token1;
int n = 0;
int j  = 0 ;
int mode;
int i = 0;

//Tokenization of PIPELINE ARGUMENTS
while (arg[i] != NULL) {
	
token1 = strtok(arg[i], " ");
k = 0;
while (token1) 
{
temp[k++] = token1;
token1 = strtok(NULL, " ");
}
temp[k] = NULL;
tsize = k;


//Checking the type of the PIPELINE
mode = Typep();

int p  = pipe(fd);		

if(p == -1)
{	
printf("\n Pipe Error");	
}


if ((pid = fork()) == -1) 
{
perror("fork");
exit(1);
}

else if (pid == 0) 
{
if(dup2(fdd, 0) == -1) //opening the read end of pipe
{	
perror(" ");
}
if ((arg[ i + 1 ])!= NULL) 
{
if(dup2(fd[1], 1) == -1) //opening the write end of the pipe
{
perror(" ");
}
}


switch(mode)
{
	
case NORMAL:
break;
		
case INPUT_REDIRECTION:
//for file descriiptor of open
in = open(temp[pointerp + 1], O_RDONLY);
if(in == -1)
{
perror("\n Opening error");	
}
// file discriptor for the input file
if(dup2(in,0)==-1)
{
perror(" ");
}
int c = close(in); //closing the input file
if(c == -1)
{
perror("\n Closing error close(in)");		
}
break;	
		
case OUTPUT_REDIRECTION:
//for file descriiptor of creat
out = creat(temp[pointerp + 1] , 0644) ;
if(out == -1)
{
perror("\n Opening error");	
}
// file discriptor for the output file
if(dup2(out,1)==-1)
{
perror(" ");
}
close(out); //closing the out file
break;
}
int c = close(fd[0]);  //closing the pipe read end
if(c == -1)
{
perror("\n Closing error fd[0]");		
}
int ec = execvp(temp[0],temp);
if (ec == -1)
{
printf("\n Exec() Error" );
}


}
else 
{
wait(NULL); 	
int c = close(fd[1]); //closing the write end of the pipe
if(c == -1)
{
perror("\n Closing error fd[1]");		
}
fdd = fd[0]; // saving the state of pipe for futher use 
}
i++;
}}


int main() 
{
signal(SIGINT,SIG_IGN);	
signal(SIGCHLD,Signalhandler);  //SIGNAL handler for the child
pid_t shell_id=getpid();
minish_id= shell_id;

while (printf("minish:>") && (fgets(buf1, MAXLINE, stdin) != NULL)) 
{

buf1[strlen(buf1) - 1] ='\0';	
if (strcmp(buf1, "\n") == 0) 
{
continue;
}
else
{		
if(strcmp(buf1, "exit") == 0)
{
int i=0;
while( i<100){
i++;
{
kill(proc[i].pid_child, SIGTERM);
}
}		
}		
else
{		
int mode;
mode = checkpipe();
if(mode == PIPELINE)
{
tokenizerp();
looppipe();
}
else
{
tokenizer();
int type = Type();
execute(type);


}}}}
return 0;
}