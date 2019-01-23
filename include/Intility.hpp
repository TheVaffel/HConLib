//Intility.h - Haakon Flatval

#ifndef INCLUDED_INTILITY
#define INCLUDED_INTILITY

struct Bignum{
  char* data;
  
  Bignum(int i);

  Bignum(const char* i);
  void setBinDigTo(int i, int u);
  ~Bignum(){
    delete[] data;
  }
};

Bignum operator+(Bignum b1, Bignum b2);
Bignum createBignum(long long l);
    

long long gcd(long long a, long long b);
int gcd(int a, int b);
long long pollard(long long a);
int factorize(long long a, long long* res);
void setRandom(long long u);
long long random(long long a);
int random(int a);

long long rem(long long a, long long b);
int rem(int a, int b);

#endif // INCLUDED_INTILITY
