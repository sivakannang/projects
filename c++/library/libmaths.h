
#ifndef LIBMATHS_H
#define LIBMATHS_H

#ifdef __cplusplus

class Maths {
	private:
		int _a, _b, _c;
	public:
		Maths(int a = 1, int b = 1, int c = 1);
		int avg() const;
};

#endif


#ifdef __cplusplus
extern "C" {
#endif

// non member function exposed directly
int average(int a, int b, int c);

// member function's exposed through wrapper functions
// C sees only an opaque handle. It is safer than void*, as the compiler rejects any other pointer type.
typedef struct MathsHandle MathsHandle;

MathsHandle* maths_create(int a, int b, int c);    // Returns NULL on failure
int maths_avg(const MathsHandle *p);               // p must not be NULL
void maths_destroy(MathsHandle *p);

#ifdef __cplusplus
}
#endif


#endif
