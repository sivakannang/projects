
#ifndef _LIB_MATHS
#define _LIB_MATHS

#ifdef __cplusplus

class Maths {
	private:
		int _a, _b, _c;
	public:
		Maths(int a = 1, int b = 1, int c =1);
		int avg();
};

#endif


#ifdef __cplusplus
extern "C" {
#endif

// non member function exposed directly
int average(int a, int b, int c);

// member function's exposed through wrapper functions
void* maths_create(int a, int b, int c);
int maths_avg(void *p);
void maths_destroy(void *p);

#ifdef __cplusplus
}
#endif


#endif
