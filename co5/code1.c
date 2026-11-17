#include <stdio.h>
#include <limits.h>
static unsigned long long calls;
unsigned digital_root(unsigned n) { if(n<10)return n; unsigned s=0; while(n){s+=n%10;n/=10;} return digital_root(s); }
unsigned long long power_linear(unsigned long long b,unsigned n){calls++;if(!n)return 1;return b*power_linear(b,n-1);}
unsigned long long power_fast(unsigned long long b,unsigned n){calls++;if(!n)return 1;unsigned long long h=power_fast(b,n/2);return h*h*(n%2?b:1);}
unsigned long long power_bad(unsigned long long b,unsigned n){calls++;if(!n)return 1;return power_bad(b,n/2)*power_bad(b,n/2)*(n%2?b:1);}
unsigned long long power_iter(unsigned long long b,unsigned n){unsigned long long r=1;while(n){if(n&1)r*=b;b*=b;n>>=1;}return r;}
int main(void){unsigned ns[]={10,100,1000,10000};printf("digital_root(9875)=%u; digital_root(0)=%u\n",digital_root(9875),digital_root(0));calls=0;printf("linear: %llu in ",power_linear(2,30));printf("%llu calls\n",calls);calls=0;printf("fast : %llu in ",power_fast(2,30));printf("%llu calls\n",calls);calls=0;power_bad(2,30);printf("duplicated fast calls: %llu\n",calls);for(unsigned i=0;i<4;i++){calls=0;power_linear(1,ns[i]);unsigned long long a=calls;calls=0;power_fast(1,ns[i]);printf("n=%u linear=%llu fast=%llu iterative=%llu\n",ns[i],a,calls,power_iter(1,ns[i]));}return 0;}
