#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "unistd.h"
#include "fcntl.h"
#include <sys/wait.h>
#include <ctype.h>

//while wait for child, parent do nothin

/**
 Liya Norng
 Operating System
 1/25/16
 So in this shell, I added the file rediction, and cd.
 */

void prompt (char*); // prompt the user
void exec ();     // execute the user request
void makestring(char*);  // parse the string
void f (int); // signal handler for Ctrl-z and will print out the history

// this is the global variables.
int runningTotal = 1;    // how many time user input

char * Bool = "FALSE";   // checking if the user declare for &

char argbuf[255]; // this have to be global since I need it for the

// this is for the count of how many are in the history
int beginList = 0;

// this is my string after done parsing
char** argslist;

// this is my array of string for history
char *history[];

int main ()
{
    // since, in the exec() function, I call back to main().
    // I don't want the shell to print out the welcome again.
    // so only print this when runningTotal is 1 which is when
    // the shell start up
    if ( runningTotal == 1)
    {
        printf("Welcome to LNShell. My pid is %d.\n", getpid());
    }
    
    // this is the signal handler for all he key board entries.
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    
    // I don't know why this signal won't work on school server, but on my mac it know this signal
    // signal(SIGINFO, SIG_IGN);
    signal( SIGTSTP, f );  // signal handler for history

    // while loop asking for user input, then execvp the user input
    while (1)
    {
        // prompting the user, and store it to the globar variable
        prompt(argbuf);
        
        // this if statement is for checking if the user in the history mode, and request a recent request
        if (!strcmp(argbuf, "r"))
        {
            // if this if statement true, then it will call to parse the string
            makestring(history[beginList - 1]);
        }
        else if ((argbuf[0] == 'r') && strlen(argbuf) > 2)
        {
            // this variable name location is for getting the location of the history so, I can get the last request to parse the string
            int location = atoi(&argbuf[2]);
            makestring(history[location - 1]);
        }
        else
        {
            // this is chekcing if the user just click on return or enter
            if (argbuf[strlen(argbuf) - 1] != '\0')
            {
                // creating the space for the new string
                history[beginList] = malloc(strlen(argbuf) + 1);
                
                // copying the string from argbuf to history, and store them in correct order
                strcpy(history[beginList++], argbuf);
            }
            makestring(argbuf);
        }
        
        // checking if there is any value, so if the user just click on enter, or return. it will not call exec().
        // This was just to fix printing out child complete even tho, there was nothing to execute
        if (*argslist)
        {
            // this is my extra build in for changing directory. I know that you mention, we don't have to do it. But
            // I already did it so, mine as well leave it there. So make the shell more commercial
            if (strcmp(argslist[0], "cd") == 0)
            {
                // this is my system call for changing the directory. Really simple, just one line of code
                chdir(argslist[1]);
            }
            
            // now I call exec to execute the user request
            exec();
        }
    }
}

// signal handler for Ctrl-z, to open the hisotry
void f(int signum)
{
    // varaible i is for my for loop, and spot is for my checking if user had type in more than 10 then set spot to some value.
    int i = 0, spot;
    printf("\n");
    
    // this is my if statement for checking for if the number of history is bigger than 10. If it is, then set the location of
    // the list to spot by getting the current beginList - 10.
    if (beginList > 10)
    {
        spot = beginList - 10;
    }
    else
    {
        // so if the number of history is less than 10, then I would want to start at 0. Don't want to leave anything out
        spot = 0;
    }
    
    // my for loop, and accessing the history to the user,and putthing them in correct order
    for (i = spot; i < beginList; i++)
    {
        printf("%d:  %s\n",i + 1, history[i]);
    }
}

/*
 So the method is to use fork and execvp and wait to do it.
 This is where all the action happen.
 */
void exec ()
{
    // @redirec is for checking for user type in < or >
    char redirec;
    
    // @exitstatus is the statu of child process.
    // @fd is my fp is my file discriptor
    // @i is my while loop
    int exitstatus, fp, i = 0;
    
    // @pid is to store the pid of child and parent
    int pid;
    
    // @childPid is for my to get the child pid
    int childPid = 0;
    
    //@whisper is for checking if the user type in whisper, then incrmenent by 1
    int whisper = 0;
    
    // this is for checking for exit.
    int exit1 = 0;
    
    // getting the current pid to use when the user exit
    childPid = getpid();

    // creating the call backing function to main, it use when the user request not to wait for the child process to be finish
    void (*callback)(void);
    callback = (void*)main;

    //check for all &, whisper, and history, since execvp do not
    // handle it this command
    while (argslist[i] != '\0')
    {
        // checking for &
        if (argslist[i][0] == '&')
        {
            Bool = "TRUE";  // For detecting if user request a background running
            
            // setting to '\0' is to erase '&' and replace it with '\0'
            argslist[i] = '\0';
            break; // it found the item so it break out of the loop
        }
        // checking for if the user reqest to exit
        else if (!strcmp(argslist[i],"exit"))
        {
            exit1 = 1;
            // setting the argslist by hand, it's not build in
            argslist[0] = "ps";
            argslist[1] = "-o";
            argslist[2] = "pid,ppid,pcpu,pmem,etime,user,command";
            argslist[3] = "-p";
            argslist[4] =(childPid + 1);
            argslist[3] = '\0';
            break;
        }

        // checking for whisper, if user input whisper, then replace it with echo
        // then turn the lowercase flag on
        else if (!strcmp(argslist[i],"whisper"))
        {
            whisper = 1;
            argslist[i] = "echo";
            int j = 1;
            while (argslist[j] != '\0')
            {
                // doing a for loop, and converting every char to lower case
                for (i = 0; i < strlen(argslist[j]); i++)
                {
                    argslist[j][i] = tolower(argslist[j][i]);
                }
                j++;
            }
            break;
        }
        i++;  // increment the loop by 1
    }
  
    // parent create another process, which then call that process a child process.
    // so from here on out, there should be two process at the same location, and
    // start at this location.
    pid = fork();
    
    // checking if fork is fail
    if (pid == -1)
    {
        perror( "fork failed\n" );
        exit(1);
    }
    
    // this is checking if this is child pid, then execute this, because then child
    // process can die. But not the parent, since child pid return 0.
    if (pid == 0)
    {
        // doing a while loop to check for <, and >.
        // For the I/O redirection.
        // For example user input:
        // who > t.txt
        // this will create a text file name t.txt on the current directory.
        while (argslist[++i] != '\0')
        {
            if ( argslist[i][0] == '<' || argslist[i][0] == '>')
            {
                redirec = argslist[i][0];
                break;
            }
        }
        
        // linux has this rule where when we open a file we get the lowest file discriptor on the array.
        // checking if it is <, close the file discriptor 0; and open again so the
        // file that we want to open is on file discriptor 0;
        // since linux uses standard file discriptor 0, 1, 2;
        // 0 for input, 1 for output, then 2 is for ouput error
        if (redirec == '<' )
        {
            // closing the standard file discriptor, and will try to reopen again with the file name so we can
            // get file discriptor 0
            close(0);
            
            // opening the file for write
            fp = open( argslist[2], O_RDONLY );
            if ( fp != 0 )
            {
                fprintf( stderr, "Could not open data as fd 0\n" );
            }
            argslist[i] = '\0';
            exit (3);
        }
        
        // this is the saming thing as <. But this time we want the ouput instead.
        // so we close file discriptor 1,and create a file. It will automatically
        // pick file discriptor 1 for the file that we just create.
        else if (redirec == '>' )
        {
            // same method, close, then open again.
            close(1);
            // create and open the file for write
            fp = open( argslist[i+1], O_WRONLY | O_CREAT, 0744 );
            if ( fp != 0 )
            {
                fprintf( stderr, "Could not open data as fd 1\n" );
            }
            argslist[i] = '\0';
            exit (4);
            
        }
        // finally, here is we declare for execvp to process the user input
        execvp( argslist[0], argslist );
        perror( "execvp failed\n" );
        exit (2);
    }
    
    // this is for checking for parent
    else
    {
        // the getpid() will get the current process ID
        printf("[Child pid = %d, background = %s]\n", pid, Bool);
        fflush(stdout);
        
        // this is checking if the user has enter &. then, we call back to main to not wait for child process
        if (!strcmp(Bool,"TRUE"))
        {
            Bool = "FALSE";
            // this will wait for ceratin pid, and get the return output from that pid
            int waitingPid = waitpid(1,&exitstatus, WCONTINUED);
            while (1)
            {
                // while waitinf for chilpid, we go back to main and prompt the user another request
                if (waitingPid != childPid)
                {
                    // this is the call back function to main();
                    callback();
                }
            }
        }
        else
        {
            // if the user did not input &, then the parent have to wait for the child process to be finish
            while (wait(&exitstatus) != pid);
            
            printf("Child process complete.\n");
            if (exit1 == 1)
            {
                free(argslist); // free the memory
                exit (0);
            }
        }
    }
}

// prompting the user, and setting the last char of the string to '\0'
void prompt(char* argbuf)
{
    printf( "LNShell [%d]:", runningTotal);
    runningTotal++;
    fgets( argbuf, 255, stdin );
    argbuf[strlen(argbuf) - 1] = '\0';
}

// parsing the string
void makestring(char* buf)
{
    int i = 0;
    argslist = malloc(256); // allocate the memory
    // separate the string by checking for space
    argslist[i] = strtok(buf, " ");
    while (argslist[i])
    {
        i++;
        argslist[i] = strtok(NULL, " ");
    }
}
