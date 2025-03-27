#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sysexits.h>

int main(int argc, char *argv[]){

   if(argc != 2){
      perror("Wrong arg count\n");
      return -1;
   }

   int processCount = atoi(argv[1]);

   for(int i = 0; i < processCount; i++){
      int pid = fork();
      if(pid > 0)
         continue;
      else if(pid == 0){
         printf("Parent PID: %d \t current PID: %d\n", getppid(), getpid());
         exit(X_OK);
      }
      else{
         perror("Something went wrong\n");
         return -1;
      }
   }

   for(int i = 0; i < processCount; i++)
      wait(NULL);

   printf("%s\n", argv[1]);

   return 0;
}
