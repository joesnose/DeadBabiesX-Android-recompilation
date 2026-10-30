#include "xengine.h"
#include "game.h"
#include "lan.h"
#include "sync_rng.h"
#include "cartoon_shading.h"
#include "cel_shading.h"
#undef XInputSetState
#include <SDL.h>
#if defined(__ANDROID__) || defined(DBX_GLES)
#include <GLES/gl.h>
#include <unistd.h>
#else
#include <SDL_opengl.h>
#endif
#include <array>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"
extern "C" int stb_vorbis_decode_filename(const char*,int*,int*,short**);
static SDL_Window* window;
#ifndef __ANDROID__
static bool headlessInputTest=false;
bool NativeIsHeadlessInputTest(){return headlessInputTest;}
void NativeSetHeadlessInputTest(bool value){headlessInputTest=value;}
#endif
static SDL_GLContext context;
static bool running=true,menuBack=false,background=false;
static uint64_t lastFrame;
static GLuint fontTexture;
static int viewportW=960,viewportH=720;
static SDL_Rect view;
static std::map<std::string,std::unique_ptr<IDirect3DTexture8>> textures;
CXEngine* XEngine=new CXEngine;
static std::string path(const char*name){
 std::string p=name;if(p.size()>2&&p[1]==':')p=p.substr(3);
 for(char&c:p){if(c=='\\')c='/';if(c>='A'&&c<='Z')c+=32;}
 if(GetFileAttributesA(p.c_str())!=INVALID_FILE_ATTRIBUTES)return p;return "game/"+p;
}
#include "audio.inc"
static SDL_AudioDeviceID audioDevice;
static void audioCallback(void*,Uint8*stream,int bytes){std::lock_guard<std::mutex> l(audioMutex);NativeMixAudio(voices,(short*)stream,bytes/4,audioPaused);}
void NativeStopAudio(){if(audioDevice){SDL_CloseAudioDevice(audioDevice);audioDevice=0;}}
static void initAudio(){SDL_AudioSpec s{};s.freq=48000;s.format=AUDIO_S16SYS;s.channels=2;s.samples=1024;s.callback=audioCallback;audioDevice=SDL_OpenAudioDevice(nullptr,0,&s,nullptr,0);if(audioDevice)SDL_PauseAudioDevice(audioDevice,0);}
// GLES 1.1 has the fixed pipeline but no immediate-mode vertex entry points.
#if defined(__ANDROID__) || defined(DBX_GLES)
static std::vector<GLfloat> verts,uvs;
static GLfloat tu=0,tv=0;static GLenum primitive;
static void glBegin(GLenum p){primitive=p;verts.clear();uvs.clear();}
static void glTexCoord2f(GLfloat u,GLfloat v){tu=u;tv=v;}
static void glVertex2f(GLfloat x,GLfloat y){verts.push_back(x);verts.push_back(y);uvs.push_back(tu);uvs.push_back(tv);}
static void glEnd(){glEnableClientState(GL_VERTEX_ARRAY);glEnableClientState(GL_TEXTURE_COORD_ARRAY);glVertexPointer(2,GL_FLOAT,0,verts.data());glTexCoordPointer(2,GL_FLOAT,0,uvs.data());glDrawArrays(primitive,0,verts.size()/2);glDisableClientState(GL_VERTEX_ARRAY);glDisableClientState(GL_TEXTURE_COORD_ARRAY);}
#define glOrtho glOrthof
#define GL_QUADS GL_TRIANGLE_FAN
#define GL_CLAMP GL_CLAMP_TO_EDGE
#endif
static void applyViewport(){SDL_GL_GetDrawableSize(window,&viewportW,&viewportH);int w=viewportW,h=viewportH;if(w*3>h*4)w=h*4/3;else h=w*3/4;view={(viewportW-w)/2,(viewportH-h)/2,w,h};glViewport(view.x,view.y,w,h);}
static void prepareFrame(){applyViewport();glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(-20,660,500,-20,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();}
static void initControllers();
static void makeFont(){int w,h,n;auto*b=stbi_load("font.png",&w,&h,&n,4);if(!b)throw std::runtime_error("Font asset missing");glGenTextures(1,&fontTexture);glBindTexture(GL_TEXTURE_2D,fontTexture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,b);stbi_image_free(b);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);}
static void resetRuntimeState();
extern void ReleaseNativeRound();
void CXEngine::XInit(){
 resetRuntimeState();
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMECONTROLLER))throw std::runtime_error(SDL_GetError());
#ifdef __ANDROID__
 chdir(SDL_AndroidGetInternalStoragePath());
#endif
#if defined(__ANDROID__) || defined(DBX_GLES)
 SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,1);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
#endif
 SDL_SetHint(SDL_HINT_ORIENTATIONS,"LandscapeLeft LandscapeRight");
 SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"0");SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS,"0");
 window=SDL_CreateWindow("Dead Babies X",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,960,720,SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
 if(!window)throw std::runtime_error(SDL_GetError());context=SDL_GL_CreateContext(window);if(!context)throw std::runtime_error(SDL_GetError());SDL_GL_SetSwapInterval(1);
 initControllers();glEnable(GL_TEXTURE_2D);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);makeFont();initAudio();lastFrame=SDL_GetPerformanceCounter();prepareFrame();fprintf(stderr,"Android renderer: %s\n",glGetString(GL_RENDERER));
}
void CXEngine::RequestExit(){running=false;}
bool CXEngine::Running()const{return running;}
bool CXEngine::TakeMenuBack(){bool v=menuBack;menuBack=false;return v;}
void CXEngine::ClearInputEdges(){menuBack=false;for(auto&p:GamePad){p.wPressedButtons=0;std::fill(std::begin(p.bPressedAnalogButtons),std::end(p.bPressedAnalogButtons),0);}}
void CXEngine::SetAudioPaused(bool p){std::lock_guard<std::mutex>l(audioMutex);audioPaused=p;}
#include "touch.inc"
void NativeLanPump(){pollEvents();updatePads();if(XEngine->TakeMenuBack()||!XEngine->Running())LanCancel();}
void CXEngine::XUpdate(){
#ifndef __ANDROID__
 if(headlessInputTest){if(!LanActive())m_Ticks+=m_GameSpeed;m_SoundsPerFrame=16;pollEvents();updatePads();return;}
#endif
 if(LanActive()){char label[64];snprintf(label,sizeof(label),"LAN - YOU: PLAYER %d",LanPlayerSlot()+1);DrawUiText(label,320,472,9,235,215,165,true);}
 drawTouch();SDL_GL_SwapWindow(window);
 uint64_t now=SDL_GetPerformanceCounter(),freq=SDL_GetPerformanceFrequency();double dt=double(now-lastFrame)/freq;if(dt<1./60.)SDL_Delay((Uint32)((1./60.-dt)*1000));lastFrame=SDL_GetPerformanceCounter();m_Millisecs.QuadPart=lastFrame;if(!LanActive())m_Ticks+=m_GameSpeed;m_SoundsPerFrame=16;
 pollEvents();updatePads();prepareFrame();
 if(const char*s=getenv("DBX_TEST_SECONDS"))if(SDL_GetTicks64()>atof(s)*1000)running=false;
}
#include "art.inc"
IDirect3DTexture8 *CXEngine::LoadTexture(const char *name) {
  std::string filename = path(name);
  bool replacementArt = false;
  const char *skin = NativeArtSkin();
  if (skin && std::string(skin) != "cartoon") {
    std::string relative = name;
    if (relative.size() > 2 && relative[1] == ':') relative = relative.substr(3);
    for (char &c : relative) {
      if (c == '\\') c = '/';
      if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    }
    std::vector<const char *> packs{skin};
    if (std::string(skin) == "celshaded") packs.push_back("supersuperhd");
    if (std::string(skin) == "supersuperhd" || std::string(skin) == "celshaded") packs.push_back("superhd");
    if (std::string(skin) != "hd") packs.push_back("hd");
    for (size_t i = 0; i < packs.size(); ++i) {
      std::string replacement = std::string("game/") + packs[i] + "/" + relative;
      if (GetFileAttributesA(replacement.c_str()) != INVALID_FILE_ATTRIBUTES) {
        filename = replacement;
        replacementArt = true;
        break;
      }
    }
  }

  const bool celShaded = skin && std::string(skin) == "celshaded";
  const bool cartoon = skin && std::string(skin) == "cartoon";
  const std::string cacheKey = cartoon ? "cartoon:" + filename : celShaded ? "celshaded:" + filename : filename;
  auto it = textures.find(cacheKey);
  if (it != textures.end())
    return it->second.get();
  auto texture=std::make_unique<IDirect3DTexture8>();
  int w,h,n;auto*raw=stbi_load(filename.c_str(),&w,&h,&n,4);
  if(!raw)throw std::runtime_error("Texture missing: "+filename);
  texture->width=w;texture->height=h;
  std::vector<unsigned char>pixels(raw,raw+(size_t)w*h*4);stbi_image_free(raw);
  if (!replacementArt)
    for (size_t i = 0; i < pixels.size(); i += 4)
      if (pixels[i] == 0 && pixels[i + 1] == 0 && pixels[i + 2] == 0)
        pixels[i + 3] = 0;
  if (cartoon) NativeApplyCartoon(pixels, texture->width, texture->height);
  if (celShaded) NativeApplyCelShading(pixels, texture->width, texture->height);
  if (replacementArt || cartoon)
    fprintf(stderr, "ART %s: %s (%ux%u)\n", skin, filename.c_str(),
            texture->width, texture->height);
  // Skip empty canvas margins at draw time, retaining UVs and sprite pivots.
  // Two transparent texels of padding preserve bilinear filtering at edges.
  unsigned left = texture->width, top = texture->height, right = 0, bottom = 0;
  for (unsigned y = 0; y < texture->height; ++y)
    for (unsigned x = 0; x < texture->width; ++x)
      if (pixels[(size_t(y) * texture->width + x) * 4 + 3]) {
        left = std::min(left, x); top = std::min(top, y);
        right = std::max(right, x + 1); bottom = std::max(bottom, y + 1);
      }
  texture->empty = right == 0;
  if (!texture->empty) {
    texture->u0 = float(left > 2 ? left - 2 : 0) / texture->width;
    texture->v0 = float(top > 2 ? top - 2 : 0) / texture->height;
    texture->u1 = float(std::min(right + 2, texture->width)) / texture->width;
    texture->v1 = float(std::min(bottom + 2, texture->height)) / texture->height;
  }
#ifndef __ANDROID__
  if(headlessInputTest){auto*result=texture.get();textures[cacheKey]=std::move(texture);return result;}
#endif
  glGenTextures(1, &texture->id);
  glBindTexture(GL_TEXTURE_2D, texture->id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  auto *result = texture.get();
  textures[cacheKey] = std::move(texture);
  return result;
}
void CXEngine::SetMaterial(float r, float g, float b, float a) {
  glColor4f(1, 1, 1, std::clamp(a, 0.f, 1.f));
}
void CXEngine::SetSpriteMode(int mode) {
  glBlendFunc(GL_SRC_ALPHA,
              mode == SM_ALPHA_ADD ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
}
void CXEngine::SetCameraPos(float x, float y) {
  cameraX = x;
  cameraY = -y;
}
static void spriteQuad(const IDirect3DTexture8 *t, float x0, float y0,
                       float x1, float y1) {
  float left = x0 + (x1 - x0) * t->u0, right = x0 + (x1 - x0) * t->u1;
  float top = y0 + (y1 - y0) * t->v0, bottom = y0 + (y1 - y0) * t->v1;
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(t->u0, t->v0); glVertex2f(left, top);
  glTexCoord2f(t->u1, t->v0); glVertex2f(right, top);
  glTexCoord2f(t->u0, t->v1); glVertex2f(left, bottom);
  glTexCoord2f(t->u1, t->v1); glVertex2f(right, bottom);
  glEnd();
}
void CXEngine::DrawSprite(IDirect3DTexture8 *t, float x, float y, float w,
                          float h, float rot) {
  if (!t || t->empty)
    return;
  glBindTexture(GL_TEXTURE_2D, t->id);
  glPushMatrix();
  glTranslatef(x + cameraX, y + cameraY, 0);
  glRotatef(-rot, 0, 0, 1);
  spriteQuad(t, -w / 2, -h / 2, w / 2, h / 2);
  glPopMatrix();
}
void CXEngine::DrawSpriteOC(IDirect3DTexture8 *t, float x, float y, float w,
                            float h, float rot) {
  if (!t || t->empty)
    return;
  glBindTexture(GL_TEXTURE_2D, t->id);
  glPushMatrix();
  glTranslatef(x + cameraX, y + cameraY, 0);
  // Original offset-centre path: offset, rotate, scale, translate.
  // Scaling before rotation in GL also preserves mirrored enemy animation.
  glScalef(w, h, 1);
  glRotatef(-rot, 0, 0, 1);
  spriteQuad(t, -.85f, -.15f, .15f, .85f);
  glPopMatrix();
}
void CXEngine::DrawTracer(IDirect3DTexture8 *t, float x1, float y1, float x2,
                          float y2, float w) {
  if (!t)
    return;
  float theta = ATan2(x2 - x1, y2 - y1), dx = Sin(theta - 90) * w,
        dy = Cos(theta - 90) * w;
  glBindTexture(GL_TEXTURE_2D, t->id);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0, 1);
  glVertex2f(x1 - dx + cameraX, y1 - dy + cameraY);
  glTexCoord2f(1, 1);
  glVertex2f(x1 + dx + cameraX, y1 + dy + cameraY);
  glTexCoord2f(0, 0);
  glVertex2f(x2 - dx + cameraX, y2 - dy + cameraY);
  glTexCoord2f(1, 0);
  glVertex2f(x2 + dx + cameraX, y2 + dy + cameraY);
  glEnd();
}
static void text(const char *s, float x, float y, float size, int r, int g,
                 int b) {
  GLfloat old[4];
  glGetFloatv(GL_CURRENT_COLOR, old);
  glColor4f(r / 255.f, g / 255.f, b / 255.f, 1);
  glBindTexture(GL_TEXTURE_2D, fontTexture);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (; *s; s++, x += size * .6f) {
    unsigned ch = (unsigned char)*s;
    if (ch < 32 || ch > 126)
      continue;
    float u = (ch % 16) / 16.f, v = (ch / 16 - 2) / 8.f;
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(u, v);
    glVertex2f(x, y);
    glTexCoord2f(u + 1 / 16.f, v);
    glVertex2f(x + size * 1.45f, y);
    glTexCoord2f(u, v + 1 / 8.f);
    glVertex2f(x, y + size * 1.45f);
    glTexCoord2f(u + 1 / 16.f, v + 1 / 8.f);
    glVertex2f(x + size * 1.45f, y + size * 1.45f);
    glEnd();
  }
  glColor4f(old[0],old[1],old[2],old[3]);
}
static float textWidth(const char *s, float size) {
  size_t n = strlen(s);
  return n ? ((n - 1) * .6f + 1.45f) * size : 0;
}
void CXEngine::DrawUiText(const char *s, float x, float y, float size, int r,
                          int g, int b, bool centered) {
  text(s, centered ? x - textWidth(s, size) / 2 : x, y, size, r, g, b);
}
void CXEngine::DrawHudScore(const char *s, float center, int r, int g, int b) {
  // The visible border inside the 256px HUD texture is only 153px wide.
  // Keep every glyph's full quad within a padded 140px interior, including
  // the largest signed 32-bit score, and position it inside the 37px height.
  float units = textWidth(s, 1);
  float size = units > 0 ? std::min(18.f, 140.f / units) : 18.f;
  text(s, center - textWidth(s, size) / 2, 9, size, r, g, b);
}
void CXEngine::FillRect(float x, float y, float w, float h, float r, float g,
                        float b, float a) {
  GLfloat old[4];
  glGetFloatv(GL_CURRENT_COLOR, old);
  glDisable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(r, g, b, a);
  glBegin(GL_QUADS);
  glVertex2f(x, y);
  glVertex2f(x + w, y);
  glVertex2f(x + w, y + h);
  glVertex2f(x, y + h);
  glEnd();
  glEnable(GL_TEXTURE_2D);
  glColor4f(old[0],old[1],old[2],old[3]);
}
void CXEngine::DrawText(const char *s, int x, int y, int r, int g, int b) {
  text(s, x, y, 20, r, g, b);
}
void CXEngine::DrawScoreText(const char *s, int x, int y, int r, int g, int b) {
  text(s, x, y, 14, r, g, b);
}
void CXEngine::DrawMenuText(const char *s, int x, int y, int r, int g, int b) {
  text(s, x, y, 28, r, g, b);
}
void CXEngine::EnableMotionBlur() {}
void CXEngine::DisableMotionBlur() {}
void CXEngine::RenderMotionBlur(float) {}
void CXEngine::SetGameSpeed(float speed) { m_GameSpeed = std::max(0.f, speed); }
void NativeRumble(HANDLE,PadFeedback*){}
void CXEngine::Shutdown(){running=false;LanStop();NativeStopAudio();ReleaseNativeRound();for(auto&c:controllers)if(c){SDL_GameControllerClose(c);c=nullptr;}textures.clear();if(context)SDL_GL_DeleteContext(context);context=nullptr;if(window)SDL_DestroyWindow(window);window=nullptr;fontTexture=0;SDL_Quit();}
#ifndef __ANDROID__
#include "frontend.h"
static void capture(const char*name){std::vector<unsigned char>p((size_t)viewportW*viewportH*4);glReadPixels(0,0,viewportW,viewportH,GL_RGBA,GL_UNSIGNED_BYTE,p.data());FILE*f=fopen(name,"wb");fprintf(f,"P6\n%d %d\n255\n",viewportW,viewportH);for(int y=viewportH-1;y>=0;y--)for(int x=0;x<viewportW;x++)fwrite(&p[((size_t)y*viewportW+x)*4],1,3,f);fclose(f);}
int NativeAndroidSelfTest(){
 int failures=0;auto check=[&](bool v,const char*s){fprintf(stderr,"%s: %s\n",v?"PASS":"FAIL",s);failures+=!v;};
 XEngine->XInit();SDL_SetWindowSize(window,1280,720);prepareFrame();
 Frontend f;f.Update();f.Draw();capture("menu.ppm");
 // Real SDL finger events pass through the production pump and touch mapping.
 auto touch=[&](SDL_FingerID id,float x,float y,int type){SDL_Event e{};e.type=type;e.tfinger.fingerId=id;e.tfinger.x=x;e.tfinger.y=y;SDL_PushEvent(&e);pollEvents();updatePads();};
 auto tap=[&](float gx,float gy){float x=(view.x+(gx+20)*view.w/680.f)/viewportW,y=(view.y+(gy+20)*view.h/520.f)/viewportH;touch(50,x,y,SDL_FINGERDOWN);touch(50,x,y,SDL_FINGERUP);};
 for(int row=1;row<6;row++){tap(300,170+row*31+10);check(NativeTakeMenuTap(false)==row,"touch menu row at viewport offset");}
 tap(300,180);check(f.Update()==MenuAction::Start,"tap Start uses existing menu action");
 SetStartingPlayers(1);Game.Init();f.Play();NativeSetTouchScene(true,false);
 touch(1,.18f,.76f,SDL_FINGERDOWN);touch(2,.94f,.65f,SDL_FINGERDOWN);
 check(XEngine->GamePad[0].fX1>0&&XEngine->GamePad[0].fX2>0&&XEngine->GamePad[0].bAnalogButtons[7]==255,"independent multitouch movement and aim/fire");
 for(int i=0;i<90;i++){for(auto*p:Game.m_PlayerList){p->m_PlayerCondition=PC_INVINCIBLE;p->m_ConditionTime=300;}Game.Update();}
 touch(2,.94f,.65f,SDL_FINGERUP);check(XEngine->GamePad[0].bAnalogButtons[7]==0&&XEngine->GamePad[0].fX1>0,"releasing aim stops fire while movement remains");touch(1,.18f,.76f,SDL_FINGERUP);check(XEngine->GamePad[0].fX1==0,"releasing movement recentres");
 for(int role=2;role<=5;role++){float w=viewportW*520.f/viewportH;auto b=touchButtons()[role==2?2:role==3?0:role==4?1:3];float x=(b.x+b.w/2)/w,y=(b.y+b.h/2)/520;touch(3,x,y,SDL_FINGERDOWN);int analog=role==2?6:role==3?0:role;check(XEngine->GamePad[0].bAnalogButtons[analog]==255,"alternate/pickup/special touch button");touch(3,x,y,SDL_FINGERUP);check(XEngine->GamePad[0].bAnalogButtons[analog]==0,"action releases without sticking");}
 Game.Refresh();drawTouch();capture("gameplay.ppm");
 touch(4,.95f,.08f,SDL_FINGERDOWN);check(f.WantsPause(),"touch Pause opens frontend pause");f.OpenPause();NativeSetTouchScene(false,false);f.Draw();tap(300,200);f.Update();check(f.Update()==MenuAction::None,"pause action does not repeat");
 // Focus loss clears all held touches.
 NativeSetTouchScene(true,false);touch(1,.19f,.76f,SDL_FINGERDOWN);SDL_Event lost{};lost.type=SDL_WINDOWEVENT;lost.window.event=SDL_WINDOWEVENT_FOCUS_LOST;SDL_PushEvent(&lost);pollEvents();updatePads();check(fingers.empty()&&XEngine->GamePad[0].fX1==0&&XEngine->TakeMenuBack(),"focus loss clears touches and requests pause");
 for(int mode=0;mode<6;mode++){check(NativeSetArtMode(mode),"graphics pack available");Game.LoadArt();prepareFrame();Game.Refresh();check(glGetError()==GL_NO_ERROR,"graphics theme renders on GLES");}
 for(int choice=0;choice<4;choice++){check(XEngine->SelectMusic(choice),"music decodes");std::lock_guard<std::mutex>l(audioMutex);check(std::count_if(voices.begin(),voices.end(),[](auto&v){return v.music;})==1,"music remains singleton");}
 XEngine->BeginResultsAudio();{std::lock_guard<std::mutex>l(audioMutex);check(std::none_of(voices.begin(),voices.end(),[](auto&v){return !v.music&&!v.announcement;}),"results stop all gameplay effect loops");}
 XEngine->PlayResultCue(true);check(glGetError()==GL_NO_ERROR,"no GLES errors at test end");XEngine->Shutdown();fprintf(stderr,"ANDROID PORT TESTS: %d failures\n",failures);return failures?1:0;
}
#endif
#ifndef __ANDROID__
int NativeInputSelfTest(){
 int failures=0;auto check=[&](bool v,const char*s){fprintf(stderr,"%s: %s\n",v?"PASS":"FAIL",s);failures+=!v;};
 SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
 if(SDL_Init(SDL_INIT_GAMECONTROLLER|SDL_INIT_EVENTS)){fprintf(stderr,"SDL input init failed: %s\n",SDL_GetError());return 1;}
 for(auto size:std::array<std::pair<int,int>,3>{{{960,720},{1280,720},{2400,1080}}}){
  viewportW=size.first;viewportH=size.second;float w=viewportW*520.f/viewportH;
  NativeSetTouchScene(true,false);
  for(const auto&b:touchButtons()){
   float x=(b.x+b.w/2)/w,y=(b.y+b.h/2)/520;
   check(roleAt(x,y)==b.role,"visible side button matches hit target");
   if(b.role==6)continue;
   fingerEvent(10,x,y,SDL_FINGERDOWN);updatePads();int a=b.role==2?6:b.role==3?0:b.role;
   check(XEngine->GamePad[0].bAnalogButtons[a]==255,"side button activates correct game action");
   fingerEvent(10,x,y,SDL_FINGERUP);updatePads();check(XEngine->GamePad[0].bAnalogButtons[a]==0,"side button releases");
  }
  check(roleAt(.5f,.90f)==-1,"bottom centre stays clear");
  fingerEvent(1,.14f,.76f,SDL_FINGERDOWN);fingerEvent(2,.91f,.70f,SDL_FINGERDOWN);updatePads();check(XEngine->GamePad[0].fX1>0&&XEngine->GamePad[0].fX2>0&&XEngine->GamePad[0].bAnalogButtons[7]==255,"both touch sticks remain independent");resetTouches();
 }
 SDL_VirtualJoystickDesc desc{};desc.version=SDL_VIRTUAL_JOYSTICK_DESC_VERSION;desc.type=SDL_JOYSTICK_TYPE_GAMECONTROLLER;desc.naxes=SDL_CONTROLLER_AXIS_MAX;desc.nbuttons=SDL_CONTROLLER_BUTTON_MAX;desc.axis_mask=(1u<<SDL_CONTROLLER_AXIS_MAX)-1;desc.button_mask=(1u<<SDL_CONTROLLER_BUTTON_MAX)-1;desc.name="DBX virtual controller";
 int index=SDL_JoystickAttachVirtualEx(&desc);check(index>=0,"virtual game controller attaches");initControllers();pollEvents();check(controllers[0]!=nullptr&&controllers[1]==nullptr,"startup and queued add event open controller once");
 if(controllers[0]){
 auto*c=controllers[0];auto*j=SDL_GameControllerGetJoystick(c);
 auto axis=[&](SDL_GameControllerAxis a,Sint16 v){auto b=SDL_GameControllerGetBindForAxis(c,a);SDL_JoystickSetVirtualAxis(j,b.value.axis,v);SDL_GameControllerUpdate();pollEvents();updatePads();};
 auto button=[&](SDL_GameControllerButton b,Uint8 v){auto bind=SDL_GameControllerGetBindForButton(c,b);SDL_JoystickSetVirtualButton(j,bind.value.button,v);SDL_GameControllerUpdate();pollEvents();updatePads();};
 axis(SDL_CONTROLLER_AXIS_LEFTX,24000);axis(SDL_CONTROLLER_AXIS_RIGHTY,-24000);check(XEngine->GamePad[0].fX1>.6f&&XEngine->GamePad[0].fY2>.6f,"controller movement and aim sticks mapped");axis(SDL_CONTROLLER_AXIS_LEFTX,0);axis(SDL_CONTROLLER_AXIS_RIGHTY,0);
 axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT,32767);check(XEngine->GamePad[0].bAnalogButtons[7]>200,"right trigger fires");axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT,-32768);check(XEngine->GamePad[0].bAnalogButtons[7]==0,"right trigger release stops firing");axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT,32767);check(XEngine->GamePad[0].bAnalogButtons[6]>200,"left trigger alternate fire");axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT,-32768);
 button(SDL_CONTROLLER_BUTTON_A,1);check(XEngine->GamePad[0].bPressedAnalogButtons[0],"controller A pickup/confirm edge");button(SDL_CONTROLLER_BUTTON_A,0);
 button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,1);check(XEngine->GamePad[0].bAnalogButtons[4]==255,"right shoulder special one");button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,0);button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER,1);check(XEngine->GamePad[0].bAnalogButtons[5]==255,"left shoulder special two");button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER,0);
 button(SDL_CONTROLLER_BUTTON_DPAD_LEFT,1);check(XEngine->GamePad[0].fX1==-1,"D-pad moves player");button(SDL_CONTROLLER_BUTTON_DPAD_LEFT,0);
 button(SDL_CONTROLLER_BUTTON_START,1);check(XEngine->GamePad[0].wPressedButtons&16,"Start pause edge");button(SDL_CONTROLLER_BUTTON_START,0);
 SDL_JoystickDetachVirtual(index);pollEvents();updatePads();check(!controllers[0]&&XEngine->GamePad[0].fX1==0&&XEngine->TakeMenuBack(),"disconnect clears controller and requests pause");
 }
 index=SDL_JoystickAttachVirtualEx(&desc);pollEvents();check(controllers[0]!=nullptr,"hot-plug reconnect opens controller");if(index>=0)SDL_JoystickDetachVirtual(index);pollEvents();
 SDL_Quit();fprintf(stderr,"INPUT TESTS: %d failures\n",failures);return failures?1:0;
}
#endif
#ifndef __ANDROID__
int NativeGameplayControllerSelfTest(){
 int failures=0;auto check=[&](bool v,const char*s){fprintf(stderr,"%s: %s\n",v?"PASS":"FAIL",s);failures+=!v;};
 headlessInputTest=true;SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");SDL_Init(SDL_INIT_GAMECONTROLLER|SDL_INIT_EVENTS);
 SDL_VirtualJoystickDesc d{};d.version=SDL_VIRTUAL_JOYSTICK_DESC_VERSION;d.type=SDL_JOYSTICK_TYPE_GAMECONTROLLER;d.naxes=SDL_CONTROLLER_AXIS_MAX;d.nbuttons=SDL_CONTROLLER_BUTTON_MAX;d.axis_mask=(1u<<SDL_CONTROLLER_AXIS_MAX)-1;d.button_mask=(1u<<SDL_CONTROLLER_BUTTON_MAX)-1;d.name="Passive first device";
 int passive=SDL_JoystickAttachVirtualEx(&d);d.name="Player active controller";int active=SDL_JoystickAttachVirtualEx(&d);initControllers();pollEvents();updatePads();check(controllers[0]&&controllers[1],"two Android-style device slots");
 if(!controllers[1])return 1;
 auto*c=controllers[1];auto*j=SDL_GameControllerGetJoystick(c);
 auto button=[&](SDL_GameControllerButton b,Uint8 v){auto bind=SDL_GameControllerGetBindForButton(c,b);SDL_JoystickSetVirtualButton(j,bind.value.button,v);SDL_GameControllerUpdate();pollEvents();updatePads();};
 auto axis=[&](SDL_GameControllerAxis a,Sint16 v){auto b=SDL_GameControllerGetBindForAxis(c,a);SDL_JoystickSetVirtualAxis(j,b.value.axis,v);SDL_GameControllerUpdate();pollEvents();updatePads();};
 Frontend f;f.Update();button(SDL_CONTROLLER_BUTTON_A,1);
 check(f.Update()==MenuAction::Start,"second enumerated controller starts round through frontend");check(controllers[0]==c,"starting controller assigned to player one");
 button(SDL_CONTROLLER_BUTTON_A,0);setenv("DBX_TEST_WEAPON","2",1);SetStartingPlayers(1);Game.Init();f.Play();NativeSetTouchScene(true,false);auto*p=Game.m_PlayerList.front();p->m_PlayerPos.x=200;p->m_AimerPos={300,200};
 axis(SDL_CONTROLLER_AXIS_LEFTX,24000);axis(SDL_CONTROLLER_AXIS_RIGHTX,24000);axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT,32767);float start=p->m_PlayerPos.x,aim=p->m_AimerPos.x;
 for(int i=0;i<8;i++)Game.Update();
 check(p->m_PlayerPos.x>start,"controller moves actual player after menu start");check(p->m_AimerPos.x>aim,"controller aims actual player");check(!Game.m_BulletList.empty(),"controller trigger fires actual gameplay projectiles");
 axis(SDL_CONTROLLER_AXIS_LEFTX,0);axis(SDL_CONTROLLER_AXIS_RIGHTX,0);axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT,-32768);start=p->m_PlayerPos.x;Game.Update();check(p->m_PlayerPos.x==start&&XEngine->GamePad[0].bAnalogButtons[7]==0,"release stops movement and fire");
 button(SDL_CONTROLLER_BUTTON_START,1);check(f.WantsPause(),"same controller pauses actual active player");f.OpenPause();NativeSetTouchScene(false,false);button(SDL_CONTROLLER_BUTTON_START,0);f.Update();button(SDL_CONTROLLER_BUTTON_A,1);check(f.Update()==MenuAction::Resume,"same controller resumes pause menu");f.Play();NativeSetTouchScene(true,false);button(SDL_CONTROLLER_BUTTON_A,0);axis(SDL_CONTROLLER_AXIS_LEFTX,-24000);start=p->m_PlayerPos.x;Game.Update();check(p->m_PlayerPos.x<start,"controller still moves player after resume");
 // Simulate another round started using the screen, with the passive device first.
 axis(SDL_CONTROLLER_AXIS_LEFTX,0);std::swap(controllers[0],controllers[1]);lastMenuController=-1;NativePrepareGameplayController(-1);start=p->m_PlayerPos.x;
 axis(SDL_CONTROLLER_AXIS_LEFTX,24000);Game.Update();check(controllers[0]==c&&p->m_PlayerPos.x>start,"touch-started solo round adopts active controller as player one");
 XEngine->StopAllSounds();SDL_JoystickDetachVirtual(active);SDL_JoystickDetachVirtual(passive);pollEvents();SDL_Quit();fprintf(stderr,"GAMEPLAY CONTROLLER TESTS: %d failures\n",failures);return failures?1:0;
}
#endif

// Android can call SDL_main again in the same process after Activity recreation.
// Static initialization does not run again, so each launch must reset native state.
static void resetRuntimeState(){
 LanStop();NativeStopAudio();ReleaseNativeRound();
 running=true;menuBack=false;background=false;lastFrame=0;
 XEngine->m_Ticks=0;XEngine->m_GameSpeed=1;XEngine->m_Millisecs.QuadPart=0;XEngine->m_SoundsPerFrame=16;
 XEngine->SetCameraPos(0,0);resetTouches();touchPlaying=false;touchResults=false;
 lastMenuController=-1;awaitingSoloController=false;lanLocalInput=XBGAMEPAD{};lanButtonLatch=0;lanAnalogLatch=0;
 textures.clear();fontTexture=0;artOverride=-1;
 {std::lock_guard<std::mutex>lock(audioMutex);voices.clear();tokens.clear();nextVoice=1;musicVoiceId=0;musicChoice=0;musicPaused=false;audioPaused=false;musicVolume=1000;}
}
#ifndef __ANDROID__
int NativeRelaunchSelfTest(){
 headlessInputTest=true;int failures=0;
 auto check=[&](bool ok,const char*name){fprintf(stderr,"%s: %s\n",ok?"PASS":"FAIL",name);failures+=!ok;};
 for(int launch=0;launch<3;launch++){
  resetRuntimeState();check(XEngine->Running(),"new launch clears exit flag");check(!menuBack&&!background&&fingers.empty(),"new launch clears background and input flags");
  check(Game.m_PlayerList.empty()&&Game.m_NumPlayers==0&&textures.empty()&&voices.empty()&&tokens.empty(),"new launch has no old round/resources");
  SDL_Init(SDL_INIT_GAMECONTROLLER|SDL_INIT_EVENTS);SetStartingPlayers(1);Game.Init();auto*p=Game.m_PlayerList.front();p->m_PlayerPos.x=200;
  XEngine->GamePad[0].fX1=1;Game.Update();check(p->m_PlayerPos.x>200,"actual gameplay runs after launch");
  NativeSetTouchScene(true,false);fingerEvent(1,.14f,.76f,SDL_FINGERDOWN);menuBack=true;background=true;XEngine->RequestExit();check(!XEngine->Running(),"Exit Game stops current launch");
  XEngine->Shutdown();check(Game.m_PlayerList.empty()&&textures.empty()&&tokens.empty(),"shutdown releases round, textures and channels");
 }
 resetRuntimeState();fprintf(stderr,"RELAUNCH TESTS: %d failures\n",failures);return failures?1:0;
}
#endif
