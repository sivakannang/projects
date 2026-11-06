
#include <stdio.h>
#include <dlfcn.h>
#include "libmaths.h"

int dl_library_test(void);

// C program, calling a C++ library (libmaths) which is linked, and a C library (libcalc) which is loaded at run time
int main(void) {

	printf("avg(4, 5, 6) = %d\n", average(4, 5, 6));

	if (dl_library_test() != 0)
		return 1;

	MathsHandle *p = maths_create(2, 3, 4);
	if (!p)
	{
		fprintf(stderr, "maths_create failed\n");
		return 1;
	}
	printf("maths_avg(p) = %d\n", maths_avg(p));

	// private is checked only by the compiler, memory has no protection. Never do this in real code, as it depends on the object layout.
	int *tmp = (int *)p;
	printf("print private members from c         : %d %d %d\n", *tmp, *(tmp+1), *(tmp+2));
	*tmp = *tmp * 2, *(tmp+1) = *(tmp+1) * 2, *(tmp+2) = *(tmp+2) * 2;
	printf("altered private members by *2 from c : %d %d %d\n", *tmp, *(tmp+1), *(tmp+2));
	printf("maths_avg(p) after altering = %d\n", maths_avg(p));

	maths_destroy(p);
	return 0;
}

int dl_library_test(void) {

	void *lib_handle;
	double (*fn)(double, double);
	double x;
	char *error;

	lib_handle = dlopen("libcalc.so", RTLD_LAZY);
	if (!lib_handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		return -1;
	}

	dlerror();    // Clear any old error, as NULL from dlsym alone doesn't always mean a failure
	fn = (double (*)(double, double))dlsym(lib_handle, "multiplication");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		dlclose(lib_handle);
		return -1;
	}

	x = (*fn)(5, 10);
	printf("multiplication(5, 10) = %lf\n", x);

	dlclose(lib_handle);
	return 0;
}
