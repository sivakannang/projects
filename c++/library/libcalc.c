
/************************************************** Libraries ******************************************************************
 * Libraries :
 *
 * 	- static library  ( .a,  .lib )
 * 	- dynamic library ( .so, .dll )
 *
 * Why Libraries ?
 *
 *
 * Library build steps:
 *
 * static library creation:
 *
 *  $ gcc -c libcalc.c -o libcalc.o
 *  $ ar -rcs libcalc.a libcalc.o
 *  $ gcc main.c  -o main -L. -lcalc  or gcc main.c  -o main  libcalc.a
 *  $ ar -t libcalc.a                 -> list the objects archived in libcalc.a librarary
 *  $ ./main
 *
 * dynamic library creation:
 *
 *  $ g++ -c -fPIC libmaths.cpp -o libmaths
 *  $ g++ -shared libmaths.o -o libmaths.so
 *  $ g++ main.cpp -o main -L. -lmaths
 *  $ ./main
 *
 *  $ gcc -c -fPIC libcalc.c -o libcalc.o
 *  $ gcc -shared libcalc.o -o libcalc.so
 *  $
 *  $ gcc main.c -o main -L. -lcalc -lmaths -ldl
 *  $ or
 *  $ gcc main.c -o main -L. -lcalc -lmaths -ldl -Wl,-rpath,.
 *  $
 *  $ ./main
 *  $
 *
 * -I[path]          - look for include files in the specified path
 * -L[path]          - look for libraries for linking in the specified path
 * -l[library]       - link this specified shared library
 * -ar []            - archive the specified objects into a single object as static library
 * -Wl,rpath,[path]  - library dependencies will be searched in this path, this path need to be set during linking stage of the application build
 * -z nodefaultlib   - linker option, libraries in the default library paths are skipped.In the trusted default path /lib, and then /usr/lib
 *
 *  Default shared library search path: /etc/ld.so.conf
 *  	- It may have entries as below
 *  	  /usr/X11R6/lib
 *  	  /usr/lib
 *  	  /usr/lib/sane
 *  	  /usr/lib/mysql
 *  	  /opt/lib
 *  	  ...
 *
 *  So there are 4 ways that you can manage how and where the libraries need to be searched
 *
 *  1. Add your library directry entry in this file
 *  2. Use 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/siva/Desktop/library' , but this is temporary, till system is up. (for rollback use 'unset LD_LIBRARY_PATH')
 *  3. ldconfig -n /home/siva/Desktop/library   - add in ld cache, but this is temporary, till system is up
 *  4. Use -rpath to set the runpath for your library during linkage
 *
 * How to access C functions from C++ and C++ functions from C:
 *
 * 	- C uses address and function name in symbol table. ( $ nm c.out )
 * 	- C++ uses address and name mangling name in the symbol table, for support function overload. ( $ nm cpp.out )
 * 	- If we simply include a C file header, g++ compiler will throw linker error because it use name mangling
 * 	- C++ provides a special keyword 'extern "C"' for avoid this , so name mangling will be avoided for those and it will be treated in 'C' style
 * 	- so we can define 'C' headers as below for work in both C and C++
 * 		
 * 		#ifndef LIB_HEADER
 * 		#define LIB_HEADER
 *
 * 		#ifdef __cplusplus
 * 			extern "C" {
 * 		#endif
 * 		
 *		int addition(int a, int b);
 *		int sub(int a, int b);
 *
 * 		#ifdef __cplusplus
 * 			}
 * 		#endif
 *
 * 		#endif
 * 	- For access C++ functions in C, we need to add 'extern "C"' in the c++ header
 * 	- For access C++ objects/functions we need to create a wrapper method in C++ and the those methods need to be exposed using `extern "C"`
 *
 ********************************************************************************************************************************************/



#include "libcalc.h"


double addition(double a, double b) {
	return a+b;
}

double substraction(double a, double b) {
	return a-b;
}

double multiplication(double a, double b) {
	return a*b;
}

double division(double a, double b) {
	return a/b;
}
