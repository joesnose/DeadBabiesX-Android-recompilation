#include "controller_map.h"
#include "xengine.h"
#include <string>
#include <cstdio>
#include <array>
#include <cstring>
#include <unistd.h>
namespace {
struct Binding {bool axis;int source;};
#if defined(__vita__) || defined(DBX_VITA_CONTROLS)
const std::array<Binding,7> defaults{{{false,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},{false,SDL_CONTROLLER_BUTTON_LEFTSHOULDER},{false,SDL_CONTROLLER_BUTTON_A},{false,SDL_CONTROLLER_BUTTON_X},{false,SDL_CONTROLLER_BUTTON_Y},{false,SDL_CONTROLLER_BUTTON_START},{false,SDL_CONTROLLER_BUTTON_BACK}}};
#else
const std::array<Binding,7> defaults{{{true,SDL_CONTROLLER_AXIS_TRIGGERRIGHT},{true,SDL_CONTROLLER_AXIS_TRIGGERLEFT},{false,SDL_CONTROLLER_BUTTON_A},{false,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},{false,SDL_CONTROLLER_BUTTON_LEFTSHOULDER},{false,SDL_CONTROLLER_BUTTON_START},{false,SDL_CONTROLLER_BUTTON_BACK}}};
#endif
std::array<Binding,7> bindings=defaults;
const char* names[]={"FIRE","ALTERNATE FIRE","PICKUP / CONFIRM","SPECIAL 1","SPECIAL 2","PAUSE / START","BACK / SELECT"};
bool opened=false,swapSticks=false,armed=false;int selected=0,waiting=-1,stickHeld=0;std::string path="controller-bindings-v1.txt",message;
void save(){auto tmp=path+".tmp";FILE*f=fopen(tmp.c_str(),"w");if(!f){message="Could not save mapping";return;}bool ok=fprintf(f,"DBXMAP1 %d\n",swapSticks)>0;for(auto b:bindings)ok&=fprintf(f,"%d %d\n",b.axis,b.source)>0;ok=fclose(f)==0&&ok;if(!ok||rename(tmp.c_str(),path.c_str()))message="Could not save mapping";else message="Mapping saved";}
std::string label(Binding b){if(b.axis)return b.source==SDL_CONTROLLER_AXIS_TRIGGERLEFT?"LEFT TRIGGER":"RIGHT TRIGGER";
#if defined(__vita__) || defined(DBX_VITA_CONTROLS)
 switch(b.source){case SDL_CONTROLLER_BUTTON_A:return "CROSS";case SDL_CONTROLLER_BUTTON_B:return "CIRCLE";case SDL_CONTROLLER_BUTTON_X:return "SQUARE";case SDL_CONTROLLER_BUTTON_Y:return "TRIANGLE";case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:return "L";case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:return "R";case SDL_CONTROLLER_BUTTON_START:return "START";case SDL_CONTROLLER_BUTTON_BACK:return "SELECT";default:break;}
#endif
 const char*s=SDL_GameControllerGetStringForButton((SDL_GameControllerButton)b.source);return s?s:"UNKNOWN";}
void choose(int row){selected=row;if(row<7){waiting=row;armed=false;message="Release buttons, then press the new button or trigger";}else if(row==7){swapSticks=!swapSticks;save();}else if(row==8){bindings=defaults;swapSticks=false;save();}else opened=false;}
bool released(){for(int i=0;i<SDL_NumJoysticks();i++){auto*c=SDL_GameControllerFromInstanceID(SDL_JoystickGetDeviceInstanceID(i));if(!c)continue;for(int b=0;b<SDL_CONTROLLER_BUTTON_MAX;b++)if(SDL_GameControllerGetButton(c,(SDL_GameControllerButton)b))return false;if(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERLEFT)>8000||SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>8000)return false;}return true;}
void assign(Binding binding){bool swapped=false;for(int i=0;i<7;i++)if(i!=waiting&&bindings[i].axis==binding.axis&&bindings[i].source==binding.source){bindings[i]=bindings[waiting];swapped=true;break;}bindings[waiting]=binding;waiting=-1;save();if(swapped)message="Saved: the existing button assignment was swapped";}
}
void ControllerMapLoad(const char*p){path=p;bindings=defaults;swapSticks=false;opened=false;waiting=-1;FILE*f=fopen(path.c_str(),"r");if(!f)return;char magic[16]{};int swap=0;auto candidate=defaults;bool ok=fscanf(f,"%15s %d",magic,&swap)==2&&!strcmp(magic,"DBXMAP1")&&(swap==0||swap==1);for(auto&b:candidate){int axis=-1,source=-1;ok&=fscanf(f,"%d %d",&axis,&source)==2;if(axis==1){ok&=source==SDL_CONTROLLER_AXIS_TRIGGERLEFT||source==SDL_CONTROLLER_AXIS_TRIGGERRIGHT;}else ok&=axis==0&&source>=0&&source<SDL_CONTROLLER_BUTTON_MAX;b={axis==1,source};}fclose(f);if(ok){bindings=candidate;swapSticks=swap;}}
void ControllerMapOpen(){opened=true;waiting=-1;selected=0;stickHeld=0;message="Use D-pad / stick and A, or tap a row";}
bool ControllerMapActive(){return opened;}
void ControllerMapTick(){if(opened&&waiting>=0&&!armed&&released()){armed=true;message="Press a physical button or pull a trigger";}}
bool ControllerMapEvent(const SDL_Event&e){if(!opened)return false;
 if(e.type==SDL_KEYDOWN&&(e.key.keysym.sym==SDLK_ESCAPE||e.key.keysym.sym==SDLK_AC_BACK)){if(waiting>=0)waiting=-1;else opened=false;return true;}
 if(e.type==SDL_CONTROLLERBUTTONDOWN){
  int b=e.cbutton.button;
  if(waiting>=0){if(armed)assign({false,b});return true;}
  if(b==SDL_CONTROLLER_BUTTON_DPAD_UP)selected=(selected+9)%10;
  else if(b==SDL_CONTROLLER_BUTTON_DPAD_DOWN)selected=(selected+1)%10;
  else if(b==SDL_CONTROLLER_BUTTON_A)choose(selected);
  else if(b==SDL_CONTROLLER_BUTTON_B||b==SDL_CONTROLLER_BUTTON_START)opened=false;
  return true;
 }
 if(e.type==SDL_CONTROLLERAXISMOTION){if(waiting>=0){if(armed&&e.caxis.value>16000&&(e.caxis.axis==SDL_CONTROLLER_AXIS_TRIGGERLEFT||e.caxis.axis==SDL_CONTROLLER_AXIS_TRIGGERRIGHT))assign({true,e.caxis.axis});return true;}
  if(e.caxis.axis==SDL_CONTROLLER_AXIS_LEFTY){int dir=e.caxis.value>16000?1:e.caxis.value<-16000?-1:0;if(dir&&dir!=stickHeld)selected=(selected+dir+10)%10;stickHeld=dir;}return true;
 }
 bool touch=e.type==SDL_FINGERDOWN;bool mouse=e.type==SDL_MOUSEBUTTONDOWN&&e.button.which!=SDL_TOUCH_MOUSEID;
 if(touch||mouse){int w=0,h=0;SDL_Window*window=SDL_GetWindowFromID(mouse?e.button.windowID:e.tfinger.windowID);if(!window)window=SDL_GetKeyboardFocus();if(window)SDL_GetWindowSize(window,&w,&h);if(w<=0||h<=0)return true;float x=touch?e.tfinger.x*w:e.button.x,y=touch?e.tfinger.y*h:e.button.y;int vw=w,vh=h;if(vw*3>vh*4)vw=vh*4/3;else vh=vw*3/4;float gx=(x-(w-vw)/2)*680/vw-20,gy=(y-(h-vh)/2)*520/vh-20;
  if(waiting>=0){if(gy>415)waiting=-1;return true;}int row=int((gy-123)/27);if(gx>=80&&gx<=560&&gy>=123&&row>=0&&row<10)choose(row);return true;
 }
 if(e.type==SDL_KEYDOWN&&(e.key.keysym.sym==SDLK_RETURN||e.key.keysym.sym==SDLK_UP||e.key.keysym.sym==SDLK_DOWN)){if(waiting<0){if(e.key.keysym.sym==SDLK_RETURN)choose(selected);else selected=(selected+(e.key.keysym.sym==SDLK_UP?9:1))%10;}return true;}
 return e.type==SDL_FINGERUP||e.type==SDL_FINGERMOTION||e.type==SDL_MOUSEBUTTONUP||e.type==SDL_CONTROLLERBUTTONUP;
}
void ControllerMapApply(SDL_GameController*c,XBGAMEPAD&p){
 if(!c)return;if(swapSticks){std::swap(p.fX1,p.fX2);std::swap(p.fY1,p.fY2);}
 p.wButtons&=~uint16_t(48);int analogIndices[]={7,6,0,4,5};
 for(int i=0;i<7;i++){auto b=bindings[i];bool on=b.axis?SDL_GameControllerGetAxis(c,(SDL_GameControllerAxis)b.source)>9500:SDL_GameControllerGetButton(c,(SDL_GameControllerButton)b.source)!=0;if(i<5)p.bAnalogButtons[analogIndices[i]]=on?255:0;else if(on)p.wButtons|=i==5?16:32;}
}
void ControllerMapDraw(){XEngine->SetCameraPos(0,0);XEngine->FillRect(-20,-20,680,520,.025f,.02f,.015f,.98f);XEngine->DrawUiText("PHYSICAL CONTROLLER",320,35,24,250,225,185,true);XEngine->DrawUiText("Applies to connected controllers; touch controls stay the same",320,78,10,185,165,140,true);
 for(int i=0;i<10;i++){float y=126+i*27;bool active=i==selected;XEngine->FillRect(80,y-3,480,25,active?.24f:.07f,active?.10f:.06f,.04f,1);std::string text;if(i<7)text=std::string(names[i])+": "+label(bindings[i]);else if(i==7)text=swapSticks?"STICKS: SWAPPED":"STICKS: DEFAULT";else text=i==8?"RESET DEFAULTS":"DONE";XEngine->DrawUiText(text.c_str(),320,y,13,active?255:185,active?229:165,active?185:140,true);}
 XEngine->DrawUiText(message.c_str(),320,420,10,225,195,145,true);XEngine->DrawUiText("CIRCLE / SELECT: CANCEL OR DONE",320,447,10,175,160,140,true);
 if(waiting>=0){XEngine->FillRect(60,170,520,150,.06f,.04f,.025f,.98f);XEngine->DrawUiText(names[waiting],320,190,20,255,225,180,true);XEngine->DrawUiText(armed?"PRESS A BUTTON OR TRIGGER":"RELEASE HELD BUTTONS FIRST",320,240,13,230,200,160,true);XEngine->DrawUiText("TAP BELOW TO CANCEL",320,440,10,200,180,150,true);}
}
int ControllerMapSelfTest(){
 int failures=0;auto check=[&](bool ok,const char*name){fprintf(stderr,"%s: %s\n",ok?"PASS":"FAIL",name);failures+=!ok;};
 std::string temp="/tmp/dbx-controller-mapping-"+std::to_string(getpid())+".txt";ControllerMapLoad(temp.c_str());bindings[0]={false,SDL_CONTROLLER_BUTTON_X};swapSticks=true;save();bindings=defaults;swapSticks=false;ControllerMapLoad(temp.c_str());check(!bindings[0].axis&&bindings[0].source==SDL_CONTROLLER_BUTTON_X&&swapSticks,"custom button and stick preference survive reload");
 SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");SDL_Init(SDL_INIT_GAMECONTROLLER|SDL_INIT_EVENTS);
 SDL_VirtualJoystickDesc d{};d.version=SDL_VIRTUAL_JOYSTICK_DESC_VERSION;d.type=SDL_JOYSTICK_TYPE_GAMECONTROLLER;d.naxes=SDL_CONTROLLER_AXIS_MAX;d.nbuttons=SDL_CONTROLLER_BUTTON_MAX;d.axis_mask=(1u<<SDL_CONTROLLER_AXIS_MAX)-1;d.button_mask=(1u<<SDL_CONTROLLER_BUTTON_MAX)-1;d.name="Mapping test pad";
 int index=SDL_JoystickAttachVirtualEx(&d);auto*c=SDL_GameControllerOpen(index);check(c!=nullptr,"virtual physical controller opens");
 if(c){auto*j=SDL_GameControllerGetJoystick(c);auto button=[&](SDL_GameControllerButton b,bool on){auto bind=SDL_GameControllerGetBindForButton(c,b);SDL_JoystickSetVirtualButton(j,bind.value.button,on);SDL_GameControllerUpdate();};
  XBGAMEPAD p;p.fX1=.7f;p.fX2=-.4f;button(SDL_CONTROLLER_BUTTON_X,true);ControllerMapApply(c,p);check(p.bAnalogButtons[7]==255,"new physical fire binding reaches game input");check(p.fX1==-.4f&&p.fX2==.7f,"stick swap reaches game input");button(SDL_CONTROLLER_BUTTON_X,false);
  ControllerMapOpen();button(SDL_CONTROLLER_BUTTON_A,true);SDL_Event e{};e.type=SDL_CONTROLLERBUTTONDOWN;e.cbutton.button=SDL_CONTROLLER_BUTTON_A;ControllerMapEvent(e);ControllerMapTick();check(waiting==0&&!armed,"capture waits for the menu button to be released");button(SDL_CONTROLLER_BUTTON_A,false);ControllerMapTick();e.cbutton.button=SDL_CONTROLLER_BUTTON_Y;ControllerMapEvent(e);check(waiting==-1&&bindings[0].source==SDL_CONTROLLER_BUTTON_Y,"capture assigns the next physical button");
  button(SDL_CONTROLLER_BUTTON_Y,true);p=XBGAMEPAD{};ControllerMapApply(c,p);check(p.bAnalogButtons[7]==255,"captured button activates fire");button(SDL_CONTROLLER_BUTTON_Y,false);
  bindings=defaults;swapSticks=false;save();p=XBGAMEPAD{};auto axis=SDL_GameControllerGetBindForAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT);SDL_JoystickSetVirtualAxis(j,axis.value.axis,32767);SDL_GameControllerUpdate();ControllerMapApply(c,p);check(p.bAnalogButtons[7]==255,"reset restores right-trigger fire");
  SDL_GameControllerClose(c);SDL_JoystickDetachVirtual(index);
 }
 if(FILE*f=fopen(temp.c_str(),"w")){fputs("DBXMAP1 0\n1 999\n",f);fclose(f);}ControllerMapLoad(temp.c_str());check(bindings[0].axis&&bindings[0].source==SDL_CONTROLLER_AXIS_TRIGGERRIGHT,"invalid saved mapping falls back to defaults");
 remove(temp.c_str());bindings=defaults;swapSticks=false;opened=false;path="controller-bindings-v1.txt";SDL_Quit();fprintf(stderr,"CONTROLLER MAPPING TESTS: %d failures\n",failures);return failures?1:0;
}
