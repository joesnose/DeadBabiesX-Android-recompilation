#include "cel_shading.h"
#include <cassert>
#include <iostream>
#include <set>
int main() {
  const unsigned w = 12, h = 12;
  std::vector<unsigned char> rgba(w * h * 4, 0);
  for (unsigned y=2;y<10;++y) for (unsigned x=2;x<10;++x) {
    size_t at=(y*w+x)*4; rgba[at]=200; rgba[at+1]=100; rgba[at+2]=40; rgba[at+3]=255;
  }
  rgba[(5*w+5)*4+3]=128;
  auto original=rgba;
  NativeApplyCelShading(rgba,w,h);
  for (size_t i=0;i<rgba.size();i+=4) {
    assert(rgba[i+3]==original[i+3]);
    if (!original[i+3]) for (int c=0;c<4;++c) assert(rgba[i+c]==original[i+c]);
  }
  assert(rgba[(2*w+2)*4] < original[(2*w+2)*4]/2); // inward silhouette ink
  assert(rgba[(7*w+7)*4] > rgba[(2*w+2)*4]);
  assert(rgba[(7*w+7)*4] > rgba[(7*w+7)*4+1]); // team/equipment hue preserved
  assert(original[(7*w+7)*4]==200); // source/cache variant remains intact
  std::vector<unsigned char> gradient(256*8*4,255);
  for(int y=0;y<8;++y) for(int x=0;x<256;++x) for(int c=0;c<3;++c)
    gradient[(y*256+x)*4+c]=x;
  NativeApplyCelShading(gradient,256,8);
  std::set<int> values;
  for(int x=1;x<255;++x) values.insert(gradient[(4*256+x)*4]);
  assert(values.size()<=6); // continuous brightness becomes discrete color bands
  auto invalid=original; NativeApplyCelShading(invalid,1,1); assert(invalid==original);
  std::cout << "PASS: cel bands, inward outlines, hue, partial/zero alpha, source isolation, invalid input\n";
}
