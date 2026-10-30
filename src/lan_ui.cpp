#include "lan.h"
#include <SDL.h>
#ifdef __ANDROID__
#include <jni.h>
void NativeShowLanSetup(bool mega){auto*env=(JNIEnv*)SDL_AndroidGetJNIEnv();jobject activity=(jobject)SDL_AndroidGetActivity();jclass cls=env->GetObjectClass(activity);jmethodID method=env->GetMethodID(cls,"showLanSetup","(Z)V");if(method)env->CallVoidMethod(activity,method,jboolean(mega));if(env->ExceptionCheck()){env->ExceptionDescribe();env->ExceptionClear();}env->DeleteLocalRef(cls);env->DeleteLocalRef(activity);}
extern "C" JNIEXPORT void JNICALL Java_com_joesnose_deadbabiesx_MainActivity_lanHost(JNIEnv*,jclass,jboolean mega){LanHost(mega);}
extern "C" JNIEXPORT void JNICALL Java_com_joesnose_deadbabiesx_MainActivity_lanStart(JNIEnv*,jclass){LanStart();}
extern "C" JNIEXPORT void JNICALL Java_com_joesnose_deadbabiesx_MainActivity_lanCancel(JNIEnv*,jclass){LanCancel();}
extern "C" JNIEXPORT void JNICALL Java_com_joesnose_deadbabiesx_MainActivity_lanJoin(JNIEnv*e,jclass,jstring ip){const char*s=e->GetStringUTFChars(ip,nullptr);LanJoin(s);e->ReleaseStringUTFChars(ip,s);}
extern "C" JNIEXPORT jstring JNICALL Java_com_joesnose_deadbabiesx_MainActivity_lanStatus(JNIEnv*e,jclass){auto text=LanStatus();return e->NewStringUTF(text.c_str());}
#else
void NativeShowLanSetup(bool){SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"LAN multiplayer","Use the Android Host / Join screen, or run the native LAN integration test.",nullptr);}
#endif
