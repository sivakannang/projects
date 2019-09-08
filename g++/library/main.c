
#include <stdio.h>
#include "libcalc.h"
#include "libmaths.h"
#include <dlfcn.h>

int dl_library_test();

int main() {

	printf("\naddition(4,5) = %lf", addition(4,5));
	printf("\navg(4, 5, 6) = %d", average(4, 5, 6));
	dl_library_test();
	void *p = maths_create(2, 3, 4);
	printf("\nmaths_avg(p, 2,3,4) = %d", maths_avg(p));
	int *tmp = (int *)p;
	printf("\nprint private members from c         : %ld %ld %ld", *tmp, *(tmp+1), *(tmp+2) );
	*tmp = *tmp * 2, *(tmp+1) = *(tmp+1) * 2, *(tmp+2) = *(tmp+2) * 2;
	printf("\naltered private members by *2 from c : %ld %ld %ld", *tmp, *(tmp+1), *(tmp+2) );
	printf("\nmaths_avg(p, 2,3,4) = %d", maths_avg(p));
	maths_destroy(p);
	printf("\n");
	return 0;
}

int dl_library_test() {

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

	fn = (double (*)(double, double))dlsym(lib_handle, "multiplication");
	if ((error = dlerror()) != NULL)  
	{
		fprintf(stderr, "%s\n", error);
		return -1;
	}

	x = (*fn)(5, 10);
	printf("\nmultiplication(5, 10) = %lf\n",x);

	dlclose(lib_handle);
	return 0;
}
