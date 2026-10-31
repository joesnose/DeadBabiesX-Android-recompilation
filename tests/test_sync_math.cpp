#include "../src/sync_math.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
int main(){uint64_t hash=1469598103934665603ull;float maxSin=0,maxAtan=0;uint32_t state=1;int bad=0;
 auto add=[&](float f){hash^=DbxSyncMath::bits(f);hash*=1099511628211ull;};
 for(int i=-200000;i<200000;i++){float a=i*.12345f;float s=DbxSyncMath::sin(a);maxSin=std::max(maxSin,std::abs(s-float(std::sin(double(a)*3.14159265358979323846/180))));add(s);float x=(i%701)*.37f,y=(i%977)*.29f;float at=DbxSyncMath::atan2(x,y);maxAtan=std::max(maxAtan,std::abs(at-float(std::atan2(double(x),double(y))*180/3.14159265358979323846)));add(at);}
 for(int i=0;i<1000000;i++){state^=state<<13;state^=state>>17;state^=state<<5;uint32_t bits=state&0x7fffffffu;if(bits>=0x7f800000)continue;float f=DbxSyncMath::fromBits(bits),s=DbxSyncMath::sqrt(f);bad+=DbxSyncMath::bits(s)!=DbxSyncMath::bits(std::sqrt(f));add(s);}
 printf("max_sin_error=%g max_atan_degrees_error=%g sqrt_mismatches=%d checksum=%016llx\n",maxSin,maxAtan,bad,(unsigned long long)hash);return maxSin<.000002f&&maxAtan<.00004f&&bad==0?0:1;}
