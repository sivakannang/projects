
/*
 *
 * http://en.wikipedia.org/wiki/Cryptography
http://en.wikipedia.org/wiki/RSA_(cryptosystem)
https://www.cs.utexas.edu/~mitra/honors/soln.html
http://sourcecode4all.wordpress.com/2012/03/28/rsa-algorithm-in-c/
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "math.h"



bool isPrime(long n);
long getPrime(long ignore_prime);
bool isRelativePrime(float X,float Y);
long selectE();
long calculateD();

int p, q, t, n, e, d;


int main()
{
	bool ret = false;

	
	do
	{
		printf("\nEnter prime number p : ");
		scanf("%d", &p);
	}while(!isPrime(p));
	
	do
	{
		printf("\nEnter prime number q : ");
		scanf("%d", &q);
	}while( (!isPrime(p)) || (p == q) );


	//p = getPrime(0);
	printf("\n P = %d", p);

	//q = getPrime(p);
	printf("\n Q = %d", q);

	n = p*q;
	t = (p-1)*(q-1);

	printf("\n N = %d", n);
	printf("\n T = %d", t);

	e = selectE();
	printf("\n E = %d", e);

	d = calculateD();
	printf("\n D = %d", d);


	printf("\n");

	return 0;


}


bool isPrime(long n) 
{
    if (n <= 3) {
        return n > 1;
    } else if (n % 2 == 0 || n % 3 == 0) {
        return false;
    } else {
        for (int i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0) {
                return false;
            }
        }
        return true;
    }
}

long getPrime(long ignore_prime)
{
	long odd,tmp_odd;
	int i;

	srand( (unsigned)time( NULL ) );
	
	while(1)
	{
		odd = rand();
		if(odd%2==0)
			odd++;

		if(ignore_prime == odd) continue;

		//test whether 'odd' is prime number or not 
		for(i=2;i<=odd/2;i++)
		{
			tmp_odd=odd%i;
			if(tmp_odd ==0) 
			{
				i=-1;
				break;
			}
		}//x of 'for' loop 
		if(i != -1) 
			break;
	}//x of 'while' loop 
	return odd;
}

bool isRelativePrime(float X,float Y)
{
	float R;
	///algo: EUCLID
	//Input: X,Y	

	if(X<Y)//if X is smaller then swap the values
	{
		X=X+Y;
		Y=X-Y;
		X=X-Y;
	}
	
	//X is always great than Y at this point

	for(;;)
	{
		if(Y==0) 
			break;
		R=fmod(X,Y);
		X=Y;
		Y=R;
	}

	if(X != 1) 
		return false;
	else 
		return true;//relatively prime

}

long selectE()
{
	bool isSecond = false;
	int count = 0;
	float i;

	for ( i=2; i<100000; i++){
		
		if ( isRelativePrime((float)t, i) ){
			count++;
			if ( count == 3)
				return (long)i;
		}


	}


}

long calculateD()
{
	//(d * e) MOD t

	long i = 0;

	for ( i = 1; i < 100000; i++)
	{

		if ((i * e) % t == 1)
			return d = i;
	}

	return 0;
}
