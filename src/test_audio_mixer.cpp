#include "audio_mixer.h"
#include <array>
#include <cassert>
#include <iostream>
int main() {
  auto song = std::make_shared<SoundData>();
  song->samples = {.1f,-.1f,.2f,-.2f,.3f,-.3f};
  std::vector<NativeAudioVoice> voices;
  uint64_t music = 0;
  for (int i=1;i<=10000;++i) music=NativeReplaceMusic(voices,music,i,song,1);
  assert(voices.size()==1 && music==10000);
  std::array<short,2048> output{};
  NativeMixAudio(voices,output.data(),1024,false);
  for(size_t i=0;i<output.size();++i)
    assert(output[i]==static_cast<short>(song->samples[i%6]*32767.f));
  auto position=voices[0].pos;
  voices.push_back({song,0,false,99999,1});
  NativeMixAudio(voices,output.data(),1024,true);
  assert(voices[0].pos!=position && voices[1].pos==0); // music previews, SFX stay paused
  voices.pop_back();
  position=voices[0].pos;
  voices[0].paused=true;
  NativeMixAudio(voices,output.data(),1024,true);
  assert(voices[0].pos==position);
  for(short value:output)assert(value==0);
  voices[0].paused=false;
  voices.push_back({song,0,false,10001,1});
  auto replacement=std::make_shared<SoundData>();replacement->samples={-.1f,.1f};
  music=NativeReplaceMusic(voices,music,10002,replacement,.5f);
  assert(voices.size()==2 && voices[0].id==10001 && voices[1].id==music);
  NativeMixAudio(voices,output.data(),1024,false);
  assert(voices.size()==1 && voices[0].id==music);
  voices.push_back({std::make_shared<SoundData>(),0,true,10003});
  auto odd=std::make_shared<SoundData>();odd->samples={.2f,-.2f,.9f};
  voices.push_back({odd,0,true,10004});
  NativeMixAudio(voices,output.data(),1024,false);
  assert(voices.size()==2); // empty loop removed, incomplete stereo tail ignored
  uint64_t kept=voices[0].id;
  assert(NativeStopEffects(voices,kept)==1);
  assert(voices.size()==1&&voices[0].id==kept);
  assert(NativeStopEffects(voices,0)==1&&voices.empty());
  // Announcements duck the music, then restore it after completion.
  auto tone=std::make_shared<SoundData>();tone->samples={.5f,.5f};
  music=NativeReplaceMusic(voices,0,20000,tone,.44f);
  auto speech=std::make_shared<SoundData>();speech->samples={.5f,.5f};
  voices.push_back({speech,0,false,20001,1.f,false,true,true});
  NativeMixAudio(voices,output.data(),1,false);
  assert(output[0]==static_cast<short>((.5f*.44f*.18f+.5f)*32767.f));
  assert(voices.size()==1);
  NativeMixAudio(voices,output.data(),1,false);
  assert(output[0]==static_cast<short>(.5f*.44f*32767.f));
  std::cout<<"PASS: singleton music, loop boundaries, stereo, music preview, explicit pause, SFX isolation, empty/odd input\n";
}
