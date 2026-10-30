#pragma once
#include "xengine.h"
inline void NativeUiBolt(float x, float y) {
  XEngine->FillRect(x-3,y-3,6,6,.24f,.20f,.16f,1);
  XEngine->FillRect(x-2,y-2,4,4,.51f,.43f,.32f,1);
  XEngine->FillRect(x-1,y,3,1,.10f,.08f,.07f,1);
}
inline void NativeUiPanel(float x,float y,float w,float h) {
  XEngine->FillRect(x+4,y+5,w,h,0,0,0,.55f);
  XEngine->FillRect(x,y,w,h,.055f,.047f,.043f,.98f);
  XEngine->FillRect(x,y,w,2,.65f,.17f,.10f,1);
  XEngine->FillRect(x,y+h-2,w,2,.30f,.24f,.17f,1);
  XEngine->FillRect(x,y+2,1,h-4,.27f,.22f,.17f,1);
  XEngine->FillRect(x+w-1,y+2,1,h-4,.19f,.15f,.12f,1);
  NativeUiBolt(x+8,y+8);NativeUiBolt(x+w-8,y+8);
  NativeUiBolt(x+8,y+h-9);NativeUiBolt(x+w-8,y+h-9);
}
inline void NativeUiArrowDown(float x,float y) {
  XEngine->FillRect(x-4,y-17,10,18,.04f,.025f,.015f,.9f);
  XEngine->FillRect(x-4,y-20,8,20,1,.73f,.22f,1);
  for(int row=0;row<12;++row) {
    XEngine->FillRect(x-12+row+2,y+row+2,float(24-2*row),1,.04f,.025f,.015f,.9f);
    XEngine->FillRect(x-12+row,y+row,float(24-2*row),1,1,.73f,.22f,1);
  }
}
