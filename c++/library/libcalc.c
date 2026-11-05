
/************************************************** Libraries ******************************************************************
 * Libraries :
 *
 * 	- static library  ( .a )  : object files copied into the executable at link time
 * 	- shared library  ( .so ) : loaded by the dynamic loader when the program starts, or by dlopen() while it runs
 *
 * Why Libraries ?
 *
 * 	- Reuse the code in many programs, and rebuild only what changed
 * 	- Ship the code with only a header, without the source
 *
 * 	                     static                            shared
 * 	Executable size      bigger                            smaller
 * 	Needed at run time   nothing                           the .so file must be found
 * 	Memory               a copy in each program            code is shared between processes
 * 	Update the library   relink every program              replace the .so, if the ABI is unchanged
 * 	Call cost            direct call                       goes through the PLT, unless inlined or bound early
 * 	Load time            none                              symbols resolved by the loader
 *
 *
 * Library build steps, run in this order. All of them are from this folder.
 *
 * static library creation:
 *
 *  $ gcc -c libcalc.c -o libcalc.o
 *  $ ar rcs libcalc.a libcalc.o
 *  $ ar t libcalc.a                  -> list the objects archived in libcalc.a library
 *  $ g++ -c libmaths.cpp -o libmaths.o
 *  $ ar rcs libmaths.a libmaths.o
 *  $ g++ main.cpp -o main_cpp -L. -lmaths -lcalc      or     g++ main.cpp -o main_cpp libmaths.a libcalc.a
 *  $ ./main_cpp
 *
 *  A C program using a static C++ library must add the C++ runtime, as gcc doesn't link it. g++ does.
 *  $ gcc main.c -o main_c libmaths.a -lstdc++
 *
 * shared library creation:
 *
 *  $ g++ -c -fPIC libmaths.cpp -o libmaths.o
 *  $ g++ -shared libmaths.o -o libmaths.so
 *  $ gcc -c -fPIC libcalc.c -o libcalc.o
 *  $ gcc -shared libcalc.o -o libcalc.so
 *
 *  $ g++ main.cpp -o main_cpp -L. -lmaths -lcalc -Wl,-rpath,'$ORIGIN'
 *  $ ./main_cpp
 *
 *  main.c loads libcalc with dlopen, so it isn't linked here. 'ldd main_c' doesn't list it.
 *  $ gcc main.c -o main_c -L. -lmaths -Wl,-rpath,'$ORIGIN'
 *  $ ./main_c
 *
 *  With -fsanitize=address, dlopen doesn't use that rpath, so run it as 'LD_LIBRARY_PATH=. ./main_c'
 *
 *  If both libcalc.a and libcalc.so are in the same path, -lcalc picks libcalc.so. Name the .a file to get the static one.
 *
 * -I[path]            - look for include files in the specified path
 * -L[path]            - look for libraries for linking in the specified path. It isn't used at run time.
 * -l[library]         - link the specified static or shared library. Put it after the files which use it.
 * ar rcs              - separate program, not a gcc flag. Archive the specified objects into a static library.
 * -Wl,-rpath,[path]   - store a path in the executable, where the loader looks for its libraries at run time
 *                       $ORIGIN means the folder of the executable. Single quotes stop the shell expanding it.
 *                       '.' means the current folder when it runs, so it fails when started from any other folder.
 * -Wl,-z,nodefaultlib - the loader skips the default paths and the cache, so even libc must be found through the other ways
 * -ldl                - needed for dlopen only with glibc older than 2.34
 *
 *
 * Shared library search order at run time:
 *
 *  1. DT_RPATH of the executable, older form, ignored if DT_RUNPATH is set
 *  2. LD_LIBRARY_PATH, ignored for setuid programs
 *  3. DT_RUNPATH of the executable, which -rpath sets by default on most current linux distros. See it with 'readelf -d'.
 *  4. /etc/ld.so.cache, which ldconfig builds from /etc/ld.so.conf and /etc/ld.so.conf.d/
 *  5. /lib and /usr/lib
 *
 *  So there are 4 ways that you can manage how and where the libraries need to be searched
 *
 *  1. Add your library directory to a file in /etc/ld.so.conf.d/, then run 'sudo ldconfig'. It stays after a reboot.
 *  2. Copy the library to /usr/local/lib, then run 'sudo ldconfig'. /usr/local/lib is listed by default on ubuntu.
 *  3. export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/siva/Desktop/library  , only for this shell and the programs started from it
 *  4. Use -rpath to set the runpath for your library during linkage
 *
 *  'ldconfig -n [path]' only creates the soname symlinks in that path. It doesn't add anything to the cache.
 *
 *
 * Versioning with soname:
 *
 *  $ gcc -shared -Wl,-soname,libcalc.so.1 libcalc.o -o libcalc.so.1.0
 *  $ ldconfig -n .                      -> creates libcalc.so.1 -> libcalc.so.1.0, used at run time
 *  $ ln -sf libcalc.so.1 libcalc.so     -> used by -lcalc at link time
 *
 *  The executable records the soname libcalc.so.1, so a compatible libcalc.so.1.1 can replace 1.0 without a relink.
 *  A change which breaks the ABI gets a new soname, libcalc.so.2.
 *
 *
 * Useful commands :
 *
 *  $ ldd ./main_c                       -> shared libraries needed, and where they are found
 *  $ readelf -d libcalc.so              -> NEEDED, SONAME, RUNPATH
 *  $ nm -D libcalc.so                   -> symbols exported by a shared library
 *  $ ldconfig -p                        -> list the cache
 *  $ LD_DEBUG=libs ./main_c             -> show how the loader searches, as it runs
 *  $ LD_PRELOAD=./libfake.so ./main_cpp -> load this library first, so its symbols replace the others. Used for mocks and for malloc replacements.
 *
 *  Hide the internal symbols of a shared library, which gives a smaller symbol table and a cleaner ABI :
 *  $ g++ -shared -fPIC -fvisibility=hidden libmaths.cpp -o libmaths.so
 *    Then mark the public ones with __attribute__((visibility("default")))
 *
 *
 * How to access C functions from C++ and C++ functions from C:
 *
 * 	- C uses the plain function name in the symbol table. ( $ nm libcalc.o )
 * 	- C++ adds the argument types to the name, called name mangling, to support function overloading. ( $ nm libmaths.o , $ nm -C libmaths.o )
 * 	- If a C++ file includes a plain C header, it looks for the mangled name, and the link fails with undefined reference
 * 	- extern "C" turns off name mangling for those functions, so they can't be overloaded
 * 	- So a C header which works in both C and C++ looks like libcalc.h
 * 	- To access C++ functions from C, declare them as extern "C" in the C++ header. See average in libmaths.h.
 * 	- To access C++ objects from C, create wrapper functions exposed using extern "C", which pass an opaque handle. See maths_create in libmaths.h.
 * 	- An exception must not cross into C, so the wrappers catch it. See maths_create in libmaths.cpp.
 *
 ********************************************************************************************************************************************/



#include "libcalc.h"


double addition(double a, double b) {
	return a+b;
}

double subtraction(double a, double b) {
	return a-b;
}

double multiplication(double a, double b) {
	return a*b;
}

double division(double a, double b) {
	return a/b;
}
