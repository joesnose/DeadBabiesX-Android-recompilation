#pragma once
// Minimal fixed-pipeline compatibility for this game's 2D draw calls.
// SDL queues native GXM geometry on Vita; no OpenGL plugin/shader compiler is needed.
#include <SDL.h>
#include <array>
#include <algorithm>
#include <vector>
#include <map>
#include <cmath>
#include <cstring>
#include <stdexcept>
using GLenum=unsigned;using GLuint=unsigned;using GLfloat=float;
constexpr unsigned GL_TEXTURE_2D=1,GL_BLEND=2,GL_SRC_ALPHA=3,GL_ONE_MINUS_SRC_ALPHA=4,GL_ONE=5,
 GL_PROJECTION=6,GL_MODELVIEW=7,GL_COLOR_BUFFER_BIT=8,GL_RGBA=9,GL_UNSIGNED_BYTE=10,GL_TEXTURE_MIN_FILTER=11,
 GL_TEXTURE_MAG_FILTER=12,GL_LINEAR=13,GL_TEXTURE_WRAP_S=14,GL_TEXTURE_WRAP_T=15,GL_CLAMP=16,
 GL_TRIANGLE_STRIP=17,GL_QUADS=18,GL_CURRENT_COLOR=19,GL_RENDERER=20,GL_NO_ERROR=0;
namespace Fixed2D {
inline SDL_Renderer* renderer=nullptr;inline SDL_Window* window=nullptr;
inline GLuint nextTexture=1,bound=0;inline std::map<GLuint,SDL_Texture*>textures;
inline bool textured=true,additive=false;inline float color[4]={1,1,1,1},u=0,v=0;
inline float left=-20,right=660,top=-20,bottom=500;inline SDL_Rect viewport{0,0,960,544};
inline unsigned matrixMode=GL_MODELVIEW,primitive=GL_TRIANGLE_STRIP;
struct Matrix{float a=1,b=0,c=0,d=1,x=0,y=0;};inline Matrix matrix;inline std::vector<Matrix>stack;
inline std::vector<SDL_Vertex>vertices;
inline void init(SDL_Window*w){window=w;renderer=SDL_CreateRenderer(w,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
#ifndef __vita__
 if(!renderer)renderer=SDL_CreateRenderer(w,-1,SDL_RENDERER_SOFTWARE);
#endif
 if(!renderer)throw std::runtime_error(SDL_GetError());SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"1");}
inline void shutdown(){for(auto&t:textures)SDL_DestroyTexture(t.second);textures.clear();if(renderer)SDL_DestroyRenderer(renderer);renderer=nullptr;window=nullptr;bound=0;nextTexture=1;matrix={};stack.clear();vertices.clear();}
inline void size(int*w,int*h){SDL_GetRendererOutputSize(renderer,w,h);}
inline void present(){SDL_RenderPresent(renderer);}
}
inline void glViewport(int x,int y,int w,int h){Fixed2D::viewport={x,y,w,h};SDL_RenderSetViewport(Fixed2D::renderer,&Fixed2D::viewport);}
inline void glClearColor(float,float,float,float){}
inline void glClear(unsigned){SDL_SetRenderDrawColor(Fixed2D::renderer,0,0,0,255);SDL_RenderClear(Fixed2D::renderer);}
inline void glMatrixMode(unsigned m){Fixed2D::matrixMode=m;}
inline void glLoadIdentity(){if(Fixed2D::matrixMode==GL_MODELVIEW)Fixed2D::matrix={};}
inline void glOrtho(float l,float r,float b,float t,float,float){Fixed2D::left=l;Fixed2D::right=r;Fixed2D::top=t;Fixed2D::bottom=b;}
inline void glPushMatrix(){Fixed2D::stack.push_back(Fixed2D::matrix);}
inline void glPopMatrix(){if(!Fixed2D::stack.empty()){Fixed2D::matrix=Fixed2D::stack.back();Fixed2D::stack.pop_back();}}
inline void glTranslatef(float x,float y,float){auto&m=Fixed2D::matrix;m.x+=m.a*x+m.c*y;m.y+=m.b*x+m.d*y;}
inline void glScalef(float x,float y,float){auto&m=Fixed2D::matrix;m.a*=x;m.b*=x;m.c*=y;m.d*=y;}
inline void glRotatef(float degrees,float,float,float){float angle=degrees*.017453292519943295f,s=std::sin(angle),c=std::cos(angle);auto&m=Fixed2D::matrix;auto old=m;m.a=old.a*c+old.c*s;m.b=old.b*c+old.d*s;m.c=old.c*c-old.a*s;m.d=old.d*c-old.b*s;}
inline void glColor4f(float r,float g,float b,float a){Fixed2D::color[0]=r;Fixed2D::color[1]=g;Fixed2D::color[2]=b;Fixed2D::color[3]=a;}
inline void glGetFloatv(unsigned,float*out){std::memcpy(out,Fixed2D::color,sizeof(Fixed2D::color));}
inline void glEnable(unsigned m){if(m==GL_TEXTURE_2D)Fixed2D::textured=true;}
inline void glDisable(unsigned m){if(m==GL_TEXTURE_2D)Fixed2D::textured=false;}
inline void glBlendFunc(unsigned,unsigned dst){Fixed2D::additive=dst==GL_ONE;}
inline void glTexParameteri(unsigned,unsigned,int){}
inline void glGenTextures(int n,GLuint*out){while(n--)*out++=Fixed2D::nextTexture++;}
inline void glBindTexture(unsigned,GLuint t){Fixed2D::bound=t;}
inline void glTexImage2D(unsigned,int,int,int w,int h,int,unsigned,unsigned,const void*p){auto&texture=Fixed2D::textures[Fixed2D::bound];if(texture)SDL_DestroyTexture(texture);texture=SDL_CreateTexture(Fixed2D::renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STATIC,w,h);if(!texture||SDL_UpdateTexture(texture,nullptr,p,w*4))throw std::runtime_error(SDL_GetError());SDL_SetTextureScaleMode(texture,SDL_ScaleModeLinear);}
inline void glBegin(unsigned kind){Fixed2D::primitive=kind;Fixed2D::vertices.clear();}
inline void glTexCoord2f(float u,float v){Fixed2D::u=u;Fixed2D::v=v;}
inline void glVertex2f(float x,float y){auto&m=Fixed2D::matrix;float tx=m.a*x+m.c*y+m.x,ty=m.b*x+m.d*y+m.y;SDL_Vertex v{};v.position={(tx-Fixed2D::left)/(Fixed2D::right-Fixed2D::left)*Fixed2D::viewport.w,(ty-Fixed2D::top)/(Fixed2D::bottom-Fixed2D::top)*Fixed2D::viewport.h};v.tex_coord={Fixed2D::u,Fixed2D::v};Uint8*rgba=&v.color.r;for(int i=0;i<4;i++)rgba[i]=Uint8(std::clamp(Fixed2D::color[i],0.f,1.f)*255);Fixed2D::vertices.push_back(v);}
inline void glEnd(){using namespace Fixed2D;if(vertices.size()<3)return;std::vector<int>indices;if(primitive==GL_QUADS){for(size_t i=0;i+3<vertices.size();i+=4)for(int k:{0,1,2,0,2,3})indices.push_back(int(i)+k);}else for(size_t i=2;i<vertices.size();i++){indices.push_back(int(i-2));indices.push_back(int(i-1));indices.push_back(int(i));}
 SDL_Texture*t=nullptr;if(textured){auto it=textures.find(bound);if(it!=textures.end())t=it->second;}
 auto mode=additive?SDL_BLENDMODE_ADD:SDL_BLENDMODE_BLEND;if(t)SDL_SetTextureBlendMode(t,mode);else SDL_SetRenderDrawBlendMode(renderer,mode);
 if(SDL_RenderGeometry(renderer,t,vertices.data(),vertices.size(),indices.data(),indices.size()))throw std::runtime_error(SDL_GetError());}
inline unsigned glGetError(){return GL_NO_ERROR;}
inline const unsigned char*glGetString(unsigned){static SDL_RendererInfo info{};SDL_GetRendererInfo(Fixed2D::renderer,&info);return (const unsigned char*)(info.name?info.name:"SDL renderer");}
