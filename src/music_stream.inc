struct stb_vorbis;
extern "C" {
stb_vorbis* stb_vorbis_open_filename(const char*,int*,const void*);
void stb_vorbis_close(stb_vorbis*);
int stb_vorbis_get_samples_float_interleaved(stb_vorbis*,int,float*,int);
int stb_vorbis_seek_start(stb_vorbis*);
}
static stb_vorbis* streamingMusic=nullptr;
static void closeMusicStream(){std::lock_guard<std::mutex>l(audioMutex);if(streamingMusic)stb_vorbis_close(streamingMusic);streamingMusic=nullptr;musicVoiceId=0;}
static void mixStream(short*output,size_t frames){if(!streamingMusic||musicPaused||audioPaused)return;static std::vector<float>samples;samples.resize(frames*2);size_t got=0;int retries=0;while(got<frames){int n=stb_vorbis_get_samples_float_interleaved(streamingMusic,2,samples.data()+got*2,(frames-got)*2);if(n<=0){if(++retries>2||!stb_vorbis_seek_start(streamingMusic))break;}else got+=size_t(n);}
 bool announcing=std::any_of(voices.begin(),voices.end(),[](const auto&v){return v.announcement&&v.sound&&v.pos<v.sound->samples.size();});float gain=.44f*musicVolume/1000.f*(announcing?.18f:1.f);
 for(size_t i=0;i<got*2;i++){float sample=output[i]/32768.f+samples[i]*gain;output[i]=short(std::clamp(sample,-1.f,1.f)*32767);}}
bool CXEngine::SelectMusic(int choice){if(choice<0||choice>3)return false;const char*file=choice==3?"rock.ogg":choice==2?"acid-house.ogg":choice==1?"static-reactor.ogg":"music3.ogg";std::lock_guard<std::mutex>l(audioMutex);int error=0;auto*next=stb_vorbis_open_filename(path(file).c_str(),&error,nullptr);if(!next){fprintf(stderr,"Music stream failed %s: %d\n",file,error);return false;}if(streamingMusic)stb_vorbis_close(streamingMusic);streamingMusic=next;musicChoice=choice;musicVoiceId=UINT64_MAX;musicPaused=false;return true;}
