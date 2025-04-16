#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <dlfcn.h>

int main (int l_param, char * wparam[]){
    int i;
    int tab[21]={1,2,3,4,5,6,7,8,9,0,0,1,2,3,4,5,6,7,8,9,0};

/*
1) otworz biblioteke
2) przypisz wskaznikom f1 i f2 adresy funkcji z biblioteki sumuj i mediana
3) stworz Makefile kompilujacy biblioteke 'bibl1' ladowana dynamicznie oraz kompilujacy ten program
4) Stosowne pliki powinny znajdowac sie w folderach '.', './bin', './'lib'. Nalezy uzyc: LD_LIBRARY_PATH
5) W Makefile nalezy dodac: test:  xxxxxx
*/

    void *handle = dlopen("./lib/bibl1.so", RTLD_LAZY);

    if(!handle){
        printf("Error while loading dynamic library\n");
        return 1;
    }

    double (*f1)(int*, int) = dlsym(handle, "mediana");
    int (*f2)(int*, int) = dlsym(handle, "sumuj");

    if(!f1 || !f2){
        printf("Error: Function not found\n");
        dlclose(handle);
        return 1;
    }

    for (i=0; i<5; i++) printf("Wynik: %lf, %d\n", f1(tab+2*i, 21-2*i), f2(tab+2*i, 21-2*i));
    return 0;
}
