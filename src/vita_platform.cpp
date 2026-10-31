#include "vita_platform.h"
#ifdef __vita__
#include <vitasdk.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
extern "C" { unsigned int _newlib_heap_size_user=128*1024*1024; }
static void*netHeap=nullptr;static bool netOwned=false,ctlOwned=false;static int oldCpu=0;
void VitaPlatformInit(){sceIoMkdir("ux0:data/DeadBabiesX",0777);chdir("ux0:data/DeadBabiesX");freopen("native.log","w",stderr);setvbuf(stderr,nullptr,_IONBF,0);
 oldCpu=scePowerGetArmClockFrequency();scePowerSetArmClockFrequency(444);sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
 netHeap=malloc(1024*1024);SceNetInitParam params{};params.memory=netHeap;params.size=1024*1024;params.flags=0;int rc=sceNetInit(&params);netOwned=rc==0;if(!netOwned){free(netHeap);netHeap=nullptr;fprintf(stderr,"sceNetInit: 0x%08x\n",rc);}ctlOwned=sceNetCtlInit()==0;fprintf(stderr,"DeadBabiesX Vita preview started; saves: ux0:data/DeadBabiesX\n");}
void VitaPlatformShutdown(){if(ctlOwned)sceNetCtlTerm();ctlOwned=false;if(netOwned)sceNetTerm();netOwned=false;free(netHeap);netHeap=nullptr;if(oldCpu>0)scePowerSetArmClockFrequency(oldCpu);}
std::string VitaWifiAddress(){SceNetCtlInfo info{};return sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS,&info)==0?info.ip_address:"Enable Wi-Fi in Vita Settings";}
#else
void VitaPlatformInit(){}void VitaPlatformShutdown(){}std::string VitaWifiAddress(){return "127.0.0.1";}
#endif
