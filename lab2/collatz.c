#include "collatz.h"

int collatz_conjecture(int input){

   if(input % 2 == 0)
      return input / 2;
   else
      return 3 * input + 1;
}

int test_collatz_convergence(int input, int max_iter, int *steps){

   int iter = 0;
   int current_number = input;

   while(iter < max_iter && current_number != 1){
      current_number = collatz_conjecture(current_number);
      steps[iter] = current_number;
      iter++;
   }

   if(iter == max_iter){
      return 0;
   }

   return iter;
}
