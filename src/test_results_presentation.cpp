#include "results_presentation.h"
#include <cassert>
#include <climits>
#include <iostream>
int main() {
  int scores[]={10,200,50,999};
  assert((NativeResultOrder(15,scores)==std::vector<int>{3,1,2,0}));
  assert((NativeResultOrder(5,scores)==std::vector<int>{2,0}));
  int tied[]={800,50,800,50};auto order=NativeResultOrder(15,tied);
  assert((order==std::vector<int>{0,2,1,3}));
  assert(NativeResultRank(order,tied,0)==1&&NativeResultRank(order,tied,1)==1);
  assert(NativeResultRank(order,tied,2)==3&&NativeResultRank(order,tied,3)==3);
  assert(NativeResultOrder(0,scores).empty());
  for(size_t count=1;count<=4;++count) {
    float duration=NativeResultRollDuration(count);
    float earlyTravel=0,lateTravel=0,last=NativeResultCursor(count,0);
    for(int step=1;step<=2000;++step) {
      float time=duration*step/2000.f,current=NativeResultCursor(count,time);
      assert(current>=0 && current<=float(count-1));
      if(time<.5f)earlyTravel+=std::abs(current-last);
      if(time>duration-.5f)lateTravel+=std::abs(current-last);
      last=current;
    }
    assert(NativeResultCursor(count,duration)==0);
    assert(NativeResultCursor(count,duration+10)==0);
    if(count>1)assert(earlyTravel>lateTravel*4);
    assert(NativeResultDisplayedScore(12345,count,duration-.01f)==0);
    assert(NativeResultDisplayedScore(12345,count,duration+1.3f)==12345);
    assert(NativeResultDisplayedScore(INT_MAX,count,duration+1.3f)==INT_MAX);
    assert(!NativeResultsCanContinue(count,duration));
    assert(NativeResultsCanContinue(count,duration+1.7f));
  }
  std::cout<<"PASS: rankings, eliminated-player masks, ties, bounds, deceleration, winner landing, score reveal, continue gating\n";
}
