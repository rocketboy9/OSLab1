#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

//Operating Systems Lab 1
//Drew Diettrich
//This program forks a child process and that child process forks a grandchild process to execute a command on the command line.
//The child process then records the time it took for the grandchilds command to execute. A loop is used in order to go through
//the commands based on how many commands are entered. At the end when all the grandchildren and children are finished, the parent
//process is used to get the cumulative times for how long all of the processes took. 

void PrintUserKernelTimeDifference(struct tms userKernelTime);
void PrintRealTimeDifference(int startTimeSeconds, int startTimeMicroseconds, int endTimeSeconds, int endTimeMicroseconds);

int main(int argc, char* argv[], char* env[]){

    pid_t child;                                    //variable for the child process
    pid_t grandchild;                               //variable for the grandchildren processes
    
    struct tms childUserKernelTime;                       //buffer variable for the child's user and kernel times

    struct timeval childStartTimeReal, childEndTimeReal;    //These variables are for the summary statistics at the end
    
    gettimeofday(&childStartTimeReal, NULL);

    for (int i = 1; i < argc; ++i){         //for loop that goes through all of the commands passed in
                        
        child = fork();                     //creates a child for each command in order to time each of the grandchildren                 
        if(child==0){ 
        
        char* currentCommand = argv[i];             //argv[i] holds the commands for each iteration of the loop
        
        char* path[] = {"/bin/bash"};   //path for starting bash
        char* argGrandchild[] = {"/bin/bash", "-c", currentCommand, NULL};//argument pointer array
        char* argChild[] = {"/bin/bash", "-c", "time", NULL};
        
        struct timeval grandchildStartTimeReal, grandchildEndTimeReal; //initializes the start and end times for the real times 
        struct tms grandchildUserKernelTime;//buffer variable for the grandchilds user and kernel times

        gettimeofday(&grandchildStartTimeReal, NULL);     //grandchildStartTimeReal

        grandchild = fork();            //forks a grandchild for executing a command
        
        if(grandchild == 0){
            printf("Executing: %s",argv[i]);
            printf("\n");

            execve(path[0], argGrandchild, NULL); //This is when the command takes place
            exit(1);
        }
        waitpid(grandchild, NULL, 0);                     //I wait until the grandchild process is done with the command
        gettimeofday(&grandchildEndTimeReal, NULL);                   //gives timestamp for the end of the real time
        times(&grandchildUserKernelTime);        //gives number of ticks for the previous process
        
        //These two lines print the time it took for the grandchild process to run
        PrintRealTimeDifference(grandchildStartTimeReal.tv_sec, grandchildStartTimeReal.tv_usec, grandchildEndTimeReal.tv_sec, grandchildEndTimeReal.tv_usec);//gets real time difference
        PrintUserKernelTimeDifference(grandchildUserKernelTime);
         
        printf("\n");
        exit(1);        //this kills the child process in order for another one to run at the top of the for loop
        }

        while(wait(NULL) > 0);//the parent waits to go into another loop
    }

    while(wait(NULL) > 0);                              //waits for all the children to finish before the parent
    if(child==0){
        exit(1);
    }

    printf("Summary Statistics:\n");
    gettimeofday(&childEndTimeReal, NULL);          //Gets the childs end time after they are executed
    PrintRealTimeDifference(childStartTimeReal.tv_sec, childStartTimeReal.tv_usec, childEndTimeReal.tv_sec, childEndTimeReal.tv_usec);//prints realTimediff for children

    times(&childUserKernelTime);                //Gets the time it took for all the children to finish executing
    PrintUserKernelTimeDifference(childUserKernelTime);//prints the user and kernel times for the whole process
}

void PrintUserKernelTimeDifference(struct tms userKernelTime){
    int numberOfTicks = sysconf(_SC_CLK_TCK);               //number of ticks per second

    double sysTime = ((double)userKernelTime.tms_cstime/numberOfTicks);//calculating the sys time for the command
    double userTime = ((double)userKernelTime.tms_cutime/numberOfTicks);//calculating the user time for the command

    printf("Usr:    %.3fs\n", userTime);//printing the user time
    printf("Sys:    %.3fs\n", sysTime);//printing the system time
}

void PrintRealTimeDifference(int startTimeSeconds, int startTimeMicroseconds, int endTimeSeconds, int endTimeMicroseconds){
    int secondsDiff = endTimeSeconds - startTimeSeconds;
    int microsecondsDiff = endTimeMicroseconds - startTimeMicroseconds;

    int milliseconds = ((1000*secondsDiff) + (microsecondsDiff/1000)); //in milliseconds
    double result = (double)milliseconds/1000;
    
    printf("Real:   %.3fs\n", result);
}
