#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <libgen.h>

int GLOBAL_X = 0;

int main(int argc, char *argv[]){

   if(argc != 2){
      perror("Wrong arg count\n");
      return -1;
   }

   printf("%s\n", basename(argv[0]));

   char *path = argv[1];
   int local_x = 0;

   int pid = fork();
   if(pid > 0){
      printf("parent process\n");
      printf("parent pid = %d, child pid = %d\n", getppid(), getpid());

      int status;
      wait(&status);

      if(WIFEXITED(status))
         printf("Child exit code: %d\n", WEXITSTATUS(status));
      else
         perror("Child process ends abnormally\n");
      printf("Parent's local = %d, parent's global = %d\n", local_x, GLOBAL_X);
      return 0;
   }
   else if(pid == 0){
      printf("child process\n");

      GLOBAL_X++;
      local_x++;

      printf("child pid = %d, parent pid = %d\n", getpid(), getppid());
      printf("child's local = %d, child's global = %d\n", local_x, GLOBAL_X);

      execl("/bin/ls", "ls", path, NULL);
   }
   else{
      perror("Fork failed\n");
      return -1;
   }

   return 0;
}
