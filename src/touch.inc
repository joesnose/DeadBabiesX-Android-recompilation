static bool touchPlaying=false,touchResults=false;static int tapRow=-1;static float tapX=-999,tapY=-999;static bool tapPending=false;
struct Finger{float x,y;int role;};static std::map<SDL_FingerID,Finger> fingers;
static SDL_JoystickID lastMenuController=-1;
static bool awaitingSoloController=false;
static XBGAMEPAD lanLocalInput;static WORD lanButtonLatch=0;static unsigned lanAnalogLatch=0;
static SDL_GameController* controllers[4]{};static bool pausePulse=false,confirmPulse=false;
void NativeSetTouchScene(bool playing,bool results){if(playing!=touchPlaying||results!=touchResults){fingers.clear();tapPending=false;}touchPlaying=playing;touchResults=results;}
int NativeTakeMenuTap(bool paused){if(!tapPending)return -1;tapPending=false;float top=paused?190:170;float rowHeight=paused?35:31;int row=(int)std::floor((tapY-(top-4))/rowHeight);if(tapX>=132&&tapX<=508&&row>=0&&row<(paused?5:7)&&tapY<top+row*rowHeight+26)return row;return -1;}
struct TouchButton {int role;float x,y,w,h;const char*label;};
static std::array<TouchButton,5> touchButtons(){
 float w=viewportW*520.f/viewportH;
 return {{{3,16,145,100,48,"PICKUP"},{4,16,205,100,48,"SP1"},
          {2,w-116,145,100,48,"ALT"},{5,w-116,205,100,48,"SP2"},
          {6,w-84,12,68,42,"PAUSE"}}};
}
static int roleAt(float x,float y){
 float w=viewportW*520.f/viewportH,px=x*w,py=y*520;
 for(const auto&b:touchButtons())if(px>=b.x&&px<=b.x+b.w&&py>=b.y&&py<=b.y+b.h)return b.role;
 if(std::abs(px-.12f*w)<=58&&std::abs(py-.76f*520)<=58)return 0;
 if(std::abs(px-.89f*w)<=58&&std::abs(py-.70f*520)<=58)return 1;
 return -1;
}
static void openController(int index){
 if(!SDL_IsGameController(index))return;
 auto id=SDL_JoystickGetDeviceInstanceID(index);
 for(auto*c:controllers)if(c&&SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(c))==id)return;
 for(auto&c:controllers)if(!c){c=SDL_GameControllerOpen(index);if(c)fprintf(stderr,"Controller connected: %s\n",SDL_GameControllerName(c));break;}
}
void NativePrepareGameplayController(int slot){
 if(slot<0||slot>=4||!controllers[slot]){
  slot=-1;
  for(int i=0;i<4;i++)if(controllers[i]&&SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllers[i]))==lastMenuController){slot=i;break;}
 }
 awaitingSoloController=slot<0;
 if(slot>0){std::swap(controllers[0],controllers[slot]);
  for(auto&p:XEngine->GamePad)p=XBGAMEPAD{};
  fprintf(stderr,"Gameplay player one bound to menu controller: %s\n",SDL_GameControllerName(controllers[0]));
 }
}
static void initControllers(){
 int loaded=SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
 fprintf(stderr,"Controller mappings loaded: %d\n",loaded);
 for(int i=0;i<SDL_NumJoysticks();i++)openController(i);
}
static void fingerEvent(SDL_FingerID id,float x,float y,int type){
 if(type==SDL_FINGERUP){auto it=fingers.find(id);if(it!=fingers.end()){if(!touchPlaying){float dx=x-it->second.x,dy=y-it->second.y;if(dx*dx+dy*dy<.0016f){tapX=(x*viewportW-view.x)*680/view.w-20;tapY=(y*viewportH-view.y)*520/view.h-20;tapPending=true;}}fingers.erase(it);}return;}
 if(type==SDL_FINGERDOWN){if(touchResults){confirmPulse=true;return;}int role=touchPlaying?roleAt(x,y):-1;if(role==6){pausePulse=true;return;}fingers[id]={x,y,role};}
 else if(touchPlaying){auto it=fingers.find(id);if(it!=fingers.end()){it->second.x=x;it->second.y=y;}}
}
static void resetTouches(){fingers.clear();tapPending=false;pausePulse=false;confirmPulse=false;for(auto&p:XEngine->GamePad)p=XBGAMEPAD{};}
static void pollEvents(){SDL_Event e;do{
 while(SDL_PollEvent(&e)){
 switch(e.type){case SDL_QUIT:running=false;break;
 case SDL_APP_WILLENTERBACKGROUND:case SDL_APP_DIDENTERBACKGROUND:background=true;resetTouches();menuBack=touchPlaying;if(audioDevice)SDL_PauseAudioDevice(audioDevice,1);break;
 case SDL_APP_DIDENTERFOREGROUND:background=false;lastFrame=SDL_GetPerformanceCounter();if(audioDevice)SDL_PauseAudioDevice(audioDevice,0);break;
 case SDL_WINDOWEVENT:if(e.window.event==SDL_WINDOWEVENT_FOCUS_LOST){resetTouches();menuBack=touchPlaying;}break;
 case SDL_KEYDOWN:if(e.key.keysym.sym==SDLK_AC_BACK||e.key.keysym.sym==SDLK_ESCAPE)menuBack=true;break;
 case SDL_FINGERDOWN:case SDL_FINGERMOTION:case SDL_FINGERUP:if(!background)fingerEvent(e.tfinger.fingerId,e.tfinger.x,e.tfinger.y,e.type);break;
 case SDL_MOUSEBUTTONDOWN:case SDL_MOUSEBUTTONUP:if(e.button.which!=SDL_TOUCH_MOUSEID){int w,h;SDL_GetWindowSize(window,&w,&h);fingerEvent(-100,e.button.x/(float)w,e.button.y/(float)h,e.type==SDL_MOUSEBUTTONDOWN?SDL_FINGERDOWN:SDL_FINGERUP);}break;
 case SDL_MOUSEMOTION:if(e.motion.which!=SDL_TOUCH_MOUSEID&&e.motion.state){int w,h;SDL_GetWindowSize(window,&w,&h);fingerEvent(-100,e.motion.x/(float)w,e.motion.y/(float)h,SDL_FINGERMOTION);}break;
 case SDL_CONTROLLERDEVICEADDED:openController(e.cdevice.which);break;
 case SDL_CONTROLLERDEVICEREMOVED:for(auto&c:controllers)if(c&&SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(c))==e.cdevice.which){SDL_GameControllerClose(c);c=nullptr;if(touchPlaying)menuBack=true;}break;
 }
 }
 if(background&&running)SDL_Delay(30);
 }while(background&&running);}
static void updatePads(){
 // A round started by touch can adopt the first controller actually used.
 if(awaitingSoloController&&touchPlaying&&(Game.m_NumPlayers==1||LanActive())){
  for(int i=0;i<4;i++)if(controllers[i]){
   bool active=false;
   for(int b=0;b<SDL_CONTROLLER_BUTTON_MAX;b++)active|=SDL_GameControllerGetButton(controllers[i],(SDL_GameControllerButton)b)!=0;
   for(int a=0;a<4;a++)active|=std::abs(SDL_GameControllerGetAxis(controllers[i],(SDL_GameControllerAxis)a))>11000;
   active|=SDL_GameControllerGetAxis(controllers[i],SDL_CONTROLLER_AXIS_TRIGGERLEFT)>11000||SDL_GameControllerGetAxis(controllers[i],SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>11000;
   if(active){NativePrepareGameplayController(i);break;}
  }
 }
 for(int i=0;i<4;i++){
 auto&p=XEngine->GamePad[i];WORD buttons=0;BYTE analog[8]{};float lx=0,ly=0,rx=0,ry=0;
 auto*c=controllers[i];if(c){auto axis=[&](SDL_GameControllerAxis a){float v=SDL_GameControllerGetAxis(c,a)/32767.f;return std::abs(v)<.25f?0.f:std::clamp(v,-1.f,1.f);};lx=axis(SDL_CONTROLLER_AXIS_LEFTX);ly=-axis(SDL_CONTROLLER_AXIS_LEFTY);rx=axis(SDL_CONTROLLER_AXIS_RIGHTX);ry=-axis(SDL_CONTROLLER_AXIS_RIGHTY);
 SDL_GameControllerButton b[]={SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_B,SDL_CONTROLLER_BUTTON_X,SDL_CONTROLLER_BUTTON_Y,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,SDL_CONTROLLER_BUTTON_LEFTSHOULDER};for(int j=0;j<6;j++)analog[j]=SDL_GameControllerGetButton(c,b[j])?255:0;analog[6]=std::max(0.f,axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT))*255;analog[7]=std::max(0.f,axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT))*255;
 if(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_START))buttons|=16;if(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_BACK))buttons|=32;SDL_GameControllerButton d[]={SDL_CONTROLLER_BUTTON_DPAD_UP,SDL_CONTROLLER_BUTTON_DPAD_DOWN,SDL_CONTROLLER_BUTTON_DPAD_LEFT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT};for(int j=0;j<4;j++)if(SDL_GameControllerGetButton(c,d[j]))buttons|=1<<j;
 }
 if(!touchPlaying&&c&&(buttons||analog[0]||analog[1]||analog[4]||analog[5]||std::abs(lx)>.35f||std::abs(ly)>.35f||std::abs(rx)>.35f||std::abs(ry)>.35f))
  lastMenuController=SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(c));
 if(touchPlaying&&c){
  if(buttons&4)lx=-1;if(buttons&8)lx=1;if(buttons&1)ly=1;if(buttons&2)ly=-1;
 }
 if(i==0){for(auto&entry:fingers){auto&f=entry.second;if(f.role==0){float radius=viewportH*.13f;lx=std::clamp((f.x-.12f)*viewportW/radius,-1.f,1.f);ly=std::clamp((.76f-f.y)*viewportH/radius,-1.f,1.f);}if(f.role==1){float radius=viewportH*.13f;rx=std::clamp((f.x-.89f)*viewportW/radius,-1.f,1.f);ry=std::clamp((.70f-f.y)*viewportH/radius,-1.f,1.f);analog[7]=255;}if(f.role==2)analog[6]=255;if(f.role==3)analog[0]=255;if(f.role==4)analog[4]=255;if(f.role==5)analog[5]=255;}
 if(confirmPulse){analog[0]=255;confirmPulse=false;}if(pausePulse){buttons|=16;pausePulse=false;}
 const Uint8*k=SDL_GetKeyboardState(nullptr);if(k[SDL_SCANCODE_RETURN])buttons|=16;if(k[SDL_SCANCODE_A])lx=-1;if(k[SDL_SCANCODE_D])lx=1;if(k[SDL_SCANCODE_W])ly=1;if(k[SDL_SCANCODE_S])ly=-1;if(k[SDL_SCANCODE_LEFT])rx=-1;if(k[SDL_SCANCODE_RIGHT])rx=1;if(k[SDL_SCANCODE_UP])ry=1;if(k[SDL_SCANCODE_DOWN])ry=-1;if(k[SDL_SCANCODE_SPACE])analog[7]=255;
 }
 p.hDevice=(c||i==0)?(HANDLE)(uintptr_t)(i+1):nullptr;p.wPressedButtons=buttons&~p.wLastButtons;p.wButtons=p.wLastButtons=buttons;
 for(int j=0;j<8;j++){p.bPressedAnalogButtons[j]=analog[j]>75&&!p.bLastAnalogButtons[j];p.bLastAnalogButtons[j]=analog[j]>75;p.bAnalogButtons[j]=analog[j];}p.fX1=lx;p.fY1=ly;p.fX2=rx;p.fY2=ry;
 if(i==0){if(LanActive()){lanLocalInput=p;lanButtonLatch|=buttons;for(int j=0;j<8;j++)if(analog[j]>75)lanAnalogLatch|=1u<<j;}else{lanLocalInput=XBGAMEPAD{};lanButtonLatch=0;lanAnalogLatch=0;}}

 }
}
static void controlBox(float x,float y,float w,float h,const char*label){XEngine->FillRect(x,y,w,h,.05f,.04f,.03f,.62f);XEngine->FillRect(x,y,w,1,.8f,.56f,.3f,.8f);XEngine->DrawUiText(label,x+w/2,y+h/2-6,9,245,214,170,true);}
static void drawTouch(){if(!touchPlaying||touchResults)return;glViewport(0,0,viewportW,viewportH);glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,viewportW*520.f/viewportH,520,0,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();XEngine->SetCameraPos(0,0);float w=viewportW*520.f/viewportH;
 controlBox(.12f*w-58,.76f*520-58,116,116,"MOVE");
 controlBox(.89f*w-58,.70f*520-58,116,116,"AIM / FIRE");
 for(const auto&b:touchButtons())controlBox(b.x,b.y,b.w,b.h,b.label);
}

XBGAMEPAD NativeLanReadLocalInput(){auto p=lanLocalInput;p.wButtons|=lanButtonLatch;for(int j=0;j<8;j++)if(lanAnalogLatch&(1u<<j))p.bAnalogButtons[j]=255;lanButtonLatch=0;lanAnalogLatch=0;return p;}
#ifndef __ANDROID__
bool NativeLanLatchSelfTest(){pausePulse=true;confirmPulse=true;updatePads();updatePads();auto first=NativeLanReadLocalInput();updatePads();auto second=NativeLanReadLocalInput();return (first.wButtons&16)&&first.bAnalogButtons[0]==255&&!(second.wButtons&16)&&second.bAnalogButtons[0]==0;}
#endif
