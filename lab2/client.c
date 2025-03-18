#include <stdio.h>
#include "collatz.h"

#ifdef DYNAMIC_LOAD
#include <dlfcn.h>
#endif


int main(){
   int inputs[] = {5, 10, 100, 1000};
   int max_iter = 100;
   int steps[100];

#ifdef DYNAMIC_LOAD
   void *handle = dlopen("./libcollatz.so", RTLD_LAZY);

   if(!handle){
      printf("Error while loading dynamic library");
      return 1;
   }

   int (*collatz_conjecture)(int) = dlsym(handle, "collatz_conjecture");
   int (*test_collatz_convergence)(int, int, int*) = dlsym(handle, "test_collatz_convergence");

   if(!collatz_conjecture || !test_collatz_convergence){
      printf("Error: Function not found");
      dlclose(handle);
      return 1;
   }
#endif

   for(size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++){

      int input = inputs[i];
      int result = test_collatz_convergence(input, max_iter, steps);

      if(result > 0){
         printf("Number %d converge to 1 in %d steps: ", input, result);
         for(int j = 0; j < result; j++){
            printf("%d ", steps[j]);
         }
         printf("\n");
      }
      else{
         printf("Number %d does not converge to 1 in %d \n", input, max_iter);
      }
   }


#ifdef DYNAMIC_LOAD
   dlclose(handle);
#endif


   return 0;
}
