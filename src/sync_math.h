#pragma once
#include <cstdint>
#include <cstring>
// Fixed IEEE-754 operations, no device-specific libm approximations or FMA.
namespace DbxSyncMath {
inline void prepareEnvironment(){
#if defined(__aarch64__)
 uint64_t fpcr;asm volatile("mrs %0, fpcr":"=r"(fpcr));
 fpcr&=~((uint64_t(3)<<22)|(uint64_t(1)<<24)|(uint64_t(1)<<19));
 fpcr|=uint64_t(1)<<25;asm volatile("msr fpcr, %0\n\tisb"::"r"(fpcr):"memory");
#endif
}
inline float fromBits(uint32_t n){float f;std::memcpy(&f,&n,4);return f;}
inline uint32_t bits(float f){uint32_t n;std::memcpy(&n,&f,4);return n;}
inline float sqrt(float a){
 uint32_t b=bits(a),mag=b&0x7fffffffu;if(!mag)return a;
 if(b>>31)return fromBits(0x7fc00000u);
 if(mag>=0x7f800000u)return mag==0x7f800000u?a:fromBits(0x7fc00000u);
 int e=int((b>>23)&255)-127;uint32_t m=(b&0x7fffffu)|0x800000u;
 if(!(b&0x7f800000u)){m=b&0x7fffffu;e=-126;while(!(m&0x800000u)){m<<=1;e--;}}
 if(e%2){m<<=1;e--;}
 uint64_t n=uint64_t(m)<<23,remain=n,root=0,bit=uint64_t(1)<<46;
 while(bit>remain)bit>>=2;
 while(bit){if(remain>=root+bit){remain-=root+bit;root=(root>>1)+bit;}else root>>=1;bit>>=2;}
 if(n-root*root>root)root++;
 int exponent=e/2+127;if(root==0x1000000u){root>>=1;exponent++;}
 return fromBits((uint32_t(exponent)<<23)|(uint32_t(root)&0x7fffffu));
}
inline float sin(float degrees){
 if((bits(degrees)&0x7fffffffu)>=0x7f800000u)return fromBits(0x7fc00000u);
 if(degrees>100000000.f||degrees<-100000000.f)return 0;
 degrees-=float(int(degrees/360.f))*360.f;
 if(degrees>180.f)degrees-=360.f;if(degrees<-180.f)degrees+=360.f;
 if(degrees>90.f)degrees=180.f-degrees;if(degrees<-90.f)degrees=-180.f-degrees;
 float x=degrees*0.017453292519943295f,s=x*x;
 float p=1.f/6227020800.f;p=p*s-1.f/39916800.f;p=p*s+1.f/362880.f;p=p*s-1.f/5040.f;p=p*s+1.f/120.f;p=p*s-1.f/6.f;
 return x*(1.f+s*p);
}
inline float cos(float a){return sin(a+90.f);}
inline float atan2(float x,float y){
 float ax=x<0?-x:x,ay=y<0?-y:y;if(ax==0&&ay==0)return 0;
 bool swap=ax>ay;float r=swap?ay/ax:ax/ay,offset=0;
 if(r>0.414213562373095f){r=(r-1.f)/(r+1.f);offset=0.7853981633974483f;}
 float s=r*r,p=1.f/17.f;p=p*s-1.f/15.f;p=p*s+1.f/13.f;p=p*s-1.f/11.f;p=p*s+1.f/9.f;p=p*s-1.f/7.f;p=p*s+1.f/5.f;p=p*s-1.f/3.f;
 float angle=offset+r*(1.f+s*p);if(swap)angle=1.5707963267948966f-angle;if(y<0)angle=3.1415926535897932f-angle;if(x<0)angle=-angle;
 return angle*57.29577951308232f;
}
}
