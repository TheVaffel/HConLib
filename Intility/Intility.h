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

#ifdef INTILITY_IMPLEMENTATION

Bignum::Bignum(int i){
  data = new char[i];
}

Bignum::Bignum(const char* num){
  int l = 0;
  while(num[l++]);
  
}

void Bignum::setBinDigTo(int i, int u){
  int a = i>>3;
  int b = i&7;
  data[a] = (data[a] & ~(1<<b)) | ((u > 0)<<b);
}

Bignum createBignum(long long l){
  Bignum b(8);
  ((long long* )b).data[0]) = l;
}

long long rem(long long a, long long b){
  long long c = a%b;
  return c>0?c:c+b;
}

int rem(int a, int b){
  int c = a%b;
  return c>=0?c:c+b;
}

long long _intility_temp;
long long _random_seed = 123456789;
void setRandom(long long u){
  _random_seed = u;
}

long long random(long long a){
  return ((_random_seed = ((_random_seed^10203043LL) >>6LL)*213LL)&(~(1LL<<63LL)))%a;
}

int random(int a){
  return (_random_seed = ((_random_seed^10203043LL) >>6LL)*213LL)%a;
}


long long gcd(long long a, long long b){
  if(a < b){
    _intility_temp = a;
    a = b;
    b = _intility_temp;
  }
  
  return !b?a:(
	       ((b&1LL)&&(a&1LL)?gcd((a - b)>>1LL, b)
	       :((a&1LL?
		  gcd(a, b>>1LL):(b&1LL?
				gcd(a>>1LL, b):gcd(a>>1LL, b>>1LL)<<1LL)))));
}

int gcd(int a, int b){
  if(a < b){
    _intility_temp = a;
    a = b;
    b = _intility_temp;
  }
  return !b?a:(
	       ((b&1)&&(a&1)?gcd((a - b)>>1, b)
	       :((a&1?
		  gcd(a, b>>1):(b&1?
				gcd(a>>1, b):gcd(a>>1, b>>1)<<1)))));
}

long long pollard(long long a){
  int i = 1; long long x = random(a);
  long long y = x;
  long long k = 2;

  while(1){
    i++;
    x = rem((x*x)&(~(1LL<<63)) - 1, a);
    long long d = gcd(rem(y - x, a), a);
    if(d != 1 && d != a){
      return d;
    }
    if(i == k){
      y = x;
      k = k<<1;
    }
    if(i >  1000000){
      return 1;

    }
  }
  
}

int _num_first_primes = 15;
long long _first_primes[] = {2, 3, 5, 7, 11,
			   13, 17, 19, 23, 29,
			   31, 37, 41, 43, 53};

int factorize_small_number(long long a, long long* res){
  int num = 0;
  for(int i = 0; i < _num_first_primes; i++){
    while(!(a%_first_primes[i]))
      res[num++] = _first_primes[i],a/=_first_primes[i];
  }
  if(a != 1){
    res[num++] = a;
  }
  return num;
}

int factorize(long long a, long long* res){
  int num = 0;
  while(a >1){
    if(a < 2500){
      num+=factorize_small_number(a, res + num);
      return num; 
    }
    long long u;
    u = pollard(a);
    if(u == 1){
      res[num++] = a;
      return num;
      
    }
    num += factorize(u, res + num);

    a/= u;
  }
  return num;
}

#endif // INTILITY_IMPLEMENTATION
