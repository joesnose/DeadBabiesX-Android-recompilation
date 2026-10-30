#include "cartoon_shading.h"
#include <cassert>
#include <set>
#include <iostream>
int main() {
 unsigned w=16,h=8;std::vector<unsigned char> pixels(w*h*4,0);
 for(unsigned y=2;y<6;++y)for(unsigned x=2;x<14;++x) {
  size_t at=(y*w+x)*4;pixels[at]=x<8?240:20;pixels[at+1]=x<8?20:240;pixels[at+2]=20;pixels[at+3]=255;
 }
 NativeApplyCartoon(pixels,w,h);assert(w==64&&h==32);
 assert(pixels[3]==0&&pixels[((h-1)*w+w-1)*4+3]==0);
 size_t red=(16*w+20)*4,green=(16*w+44)*4;
 assert(pixels[red]>pixels[red+1]&&pixels[green+1]>pixels[green]);
 assert(pixels[red+3]==255&&pixels[green+3]==255);
 w=256;h=16;pixels.assign(w*h*4,255);
 for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)for(int c=0;c<3;++c)pixels[(y*w+x)*4+c]=x;
 NativeApplyCartoon(pixels,w,h);assert(w==512&&h==32);
 std::set<int> shades;for(unsigned x=5;x<w-5;++x)shades.insert(pixels[(16*w+x)*4]);
 assert(shades.size()<=5);
 auto invalid=pixels;unsigned badW=1,badH=1;NativeApplyCartoon(invalid,badW,badH);assert(invalid==pixels);
 std::cout<<"PASS: cartoon upscaling, original aspect/colors, transparent margins, flat shading, invalid input\n";
}
