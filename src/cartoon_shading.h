#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
// Resample original artwork with premultiplied alpha, then flatten its palette.
// Geometry, sprite pivots and aspect ratios are preserved; no replacement art.
inline void NativeApplyCartoon(std::vector<unsigned char> &pixels,unsigned &width,unsigned &height) {
  if(!width||!height||pixels.size()!=size_t(width)*height*4)return;
  const unsigned scale=width<=128&&height<=128?4:2;
  const unsigned oldW=width,oldH=height;auto original=pixels;
  width*=scale;height*=scale;pixels.assign(size_t(width)*height*4,0);
  for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x) {
    float sx=std::clamp((x+.5f)/scale-.5f,0.f,float(oldW-1));
    float sy=std::clamp((y+.5f)/scale-.5f,0.f,float(oldH-1));
    unsigned ix=unsigned(sx),iy=unsigned(sy);float dx=sx-ix,dy=sy-iy;
    float alpha=0,color[3]{};
    for(unsigned oy=0;oy<2;++oy)for(unsigned ox=0;ox<2;++ox) {
      unsigned nx=std::min(ix+ox,oldW-1),ny=std::min(iy+oy,oldH-1);
      size_t at=(size_t(ny)*oldW+nx)*4;
      float weight=(ox?dx:1-dx)*(oy?dy:1-dy)*original[at+3]/255.f;
      alpha+=weight;for(int c=0;c<3;++c)color[c]+=original[at+c]*weight;
    }
    size_t at=(size_t(y)*width+x)*4;pixels[at+3]=static_cast<unsigned char>(std::lround(alpha*255));
    if(alpha>0)for(int c=0;c<3;++c)pixels[at+c]=static_cast<unsigned char>(std::lround(color[c]/alpha));
  }
  const auto smooth=pixels;const int inkRadius=std::clamp(int(std::min(width,height)/64),1,4);
  for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x) {
    size_t at=(size_t(y)*width+x)*4;if(!smooth[at+3])continue;
    bool edge=false;
    const int offsets[4][2]={{-inkRadius,0},{inkRadius,0},{0,-inkRadius},{0,inkRadius}};
    for(const auto &o:offsets) {
      int nx=int(x)+o[0],ny=int(y)+o[1];
      if(nx<0||ny<0||nx>=int(width)||ny>=int(height)||smooth[(size_t(ny)*width+nx)*4+3]<40)edge=true;
    }
    float r=smooth[at]/255.f,g=smooth[at+1]/255.f,b=smooth[at+2]/255.f;
    float v=std::max({r,g,b}),low=std::min({r,g,b}),d=v-low;
    float hue=0;
    if(d>.0001f) {
      if(v==r)hue=std::fmod((g-b)/d+6.f,6.f);
      else if(v==g)hue=(b-r)/d+2;
      else hue=(r-g)/d+4;
    }
    hue=std::round(hue*4)/4; // preserve original color families in 24 hue sectors
    float sat=v>.0001f?std::round(d/v*3)/3:0;
    v=edge?.12f:std::max(.13f,std::round(v*3)/3);
    float chroma=v*sat,second=chroma*(1-std::abs(std::fmod(hue,2.f)-1)),m=v-chroma;
    float out[3]{};
    int sector=int(hue)%6;
    if(sector==0){out[0]=chroma;out[1]=second;}else if(sector==1){out[0]=second;out[1]=chroma;}
    else if(sector==2){out[1]=chroma;out[2]=second;}else if(sector==3){out[1]=second;out[2]=chroma;}
    else if(sector==4){out[0]=second;out[2]=chroma;}else{out[0]=chroma;out[2]=second;}
    for(int c=0;c<3;++c)pixels[at+c]=static_cast<unsigned char>(std::lround((out[c]+m)*255));
  }
}
