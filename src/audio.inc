using Voice = NativeAudioVoice;
static std::mutex audioMutex;
static std::vector<Voice> voices;
static uint64_t nextVoice = 1;
static uint64_t musicVoiceId = 0;
static int musicVolume = 1000;
static int musicChoice = 0;
int NativeMusicChoice() { return musicChoice; }
const char *NativeMusicLabel(int choice) { return choice == 3 ? "ROCK" : choice == 2 ? "ACID HOUSE" : choice == 1 ? "STATIC REACTOR" : "ORIGINAL"; }
static bool musicPaused = false;
static std::vector<std::unique_ptr<IDirectMusicSegmentState>> tokens;
void CXEngine::StopAllSounds(bool releaseTokens) {
  std::lock_guard<std::mutex> lock(audioMutex);
  voices.clear();
  musicVoiceId = 0;
  musicPaused = false;
  if (releaseTokens)
    tokens.clear();
}
static bool audioPaused = false;
static uint32_t u32(const unsigned char *p) {
  uint32_t v;
  memcpy(&v, p, 4);
  return v;
}
void CSound::Create(const char *name) {
  static std::map<std::string, std::shared_ptr<SoundData>> cache;
  std::string filename = path(name);
  auto found = cache.find(filename);
  if (found != cache.end()) {
    sound = found->second;
    return;
  }
  if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".ogg") {
    int channels = 0, rate = 0;
    short *pcm = nullptr;
    int count =
        stb_vorbis_decode_filename(filename.c_str(), &channels, &rate, &pcm);
    sound = std::make_shared<SoundData>();
    if (count > 0 && channels > 0 && rate > 0) {
      size_t n = (size_t)count * 48000 / rate;
      sound->samples.resize(n * 2);
      for (size_t i = 0; i < n; i++)
        for (int ch = 0; ch < 2; ch++)
          sound->samples[i * 2 + ch] =
              pcm[std::min((size_t)count - 1, i * rate / 48000) * channels +
                  std::min(ch, channels - 1)] /
              32768.f;
      cache[filename] = sound;
    }
    free(pcm);
    return;
  }
  std::ifstream f(filename, std::ios::binary);
  std::vector<unsigned char> b((std::istreambuf_iterator<char>(f)), {});
  sound = std::make_shared<SoundData>();
  if (b.size() < 12 || memcmp(b.data(), "RIFF", 4)) {
    fprintf(stderr, "Sound missing/invalid: %s\n", filename.c_str());
    return;
  }
  unsigned channels = 1, rate = 44100, bits = 16, format = 1;
  size_t at = 12, data = 0, len = 0;
  while (at + 8 <= b.size()) {
    size_t n = u32(b.data() + at + 4);
    if (n > b.size() - at - 8)
      break;
    if (!memcmp(b.data() + at, "fmt ", 4) && n >= 16) {
      format = b[at + 8] | b[at + 9] << 8;
      channels = b[at + 10] | b[at + 11] << 8;
      rate = u32(b.data() + at + 12);
      bits = b[at + 22] | b[at + 23] << 8;
    }
    if (!memcmp(b.data() + at, "data", 4)) {
      data = at + 8;
      len = n;
    }
    at += 8 + n + (n & 1);
  }
  if (format != 1 || channels < 1 || channels > 2 ||
      (bits != 8 && bits != 16) || !rate || !data) {
    fprintf(stderr, "Unsupported WAV: %s\n", filename.c_str());
    return;
  }
  size_t count = len / (channels * (bits / 8)),
         framesOut = count * 48000 / rate;
  sound->samples.resize(framesOut * 2);
  for (size_t i = 0; i < framesOut; i++)
    for (unsigned ch = 0; ch < 2; ch++) {
      size_t sample = std::min(count - 1, i * rate / 48000) * channels +
                      std::min(ch, channels - 1);
      float v = bits == 16 ? (float)(int16_t)(b[data + sample * 2] |
                                              b[data + sample * 2 + 1] << 8) /
                                 32768.0f
                           : ((int)b[data + sample] - 128) / 128.0f;
      sound->samples[i * 2 + ch] = v;
    }
  cache[filename] = sound;
}
void CSound::PlaySound() {
  if (!sound || sound->samples.empty() || XEngine->m_SoundsPerFrame <= 0)
    return;
  XEngine->m_SoundsPerFrame--;
  std::lock_guard<std::mutex> lock(audioMutex);
  if (voices.size() < 96)
    voices.push_back({sound, 0, loop, nextVoice++});
}
void CSound::PlaySound(IDirectMusicSegmentState **channel) {
  if (!channel)
    return;
  std::lock_guard<std::mutex> lock(audioMutex);
  auto token = std::make_unique<IDirectMusicSegmentState>();
  token->id = nextVoice++;
  *channel = token.get();
  if (sound && !sound->samples.empty() && voices.size() < 96)
    voices.push_back({sound, 0, loop, token->id});
  tokens.push_back(std::move(token));
}
void CXEngine::StopChannel(IDirectMusicSegmentState *channel) {
  if (!channel)
    return;
  std::lock_guard<std::mutex> lock(audioMutex);
  voices.erase(std::remove_if(voices.begin(), voices.end(),
                              [&](auto &v) { return v.id == channel->id; }),
               voices.end());
}
#ifndef DBX_STREAM_MUSIC
bool CXEngine::SelectMusic(int choice) {
  if (choice < 0 || choice > 3) return false;
  const char *filename = choice == 3 ? "rock.ogg" : choice == 2 ? "acid-house.ogg" : choice == 1 ? "static-reactor.ogg" : "music3.ogg";
  if (GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES) {
    fprintf(stderr, "Music file missing: %s\n", filename);
    return false;
  }
  CSound candidate;
  candidate.Create(filename);
  if (!candidate.sound || candidate.sound->samples.size() < 2) {
    fprintf(stderr, "Music decode failed: %s\n", filename);
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(audioMutex);
    musicVoiceId = NativeReplaceMusic(voices, musicVoiceId, nextVoice++,
                                     candidate.sound, .44f * musicVolume / 1000.f);
    musicChoice = choice;
    musicPaused = false;
  }
  fprintf(stderr, "MUSIC selected %s\n", NativeMusicLabel(choice));
  return true;
}
#endif
void CXEngine::PlayMusic(const char*) {if(!SelectMusic(musicChoice))SelectMusic(0);}
void CXEngine::PlayResultCue(bool winner) {
  const char *name=winner?"game/audio/winner-fanfare.wav":"game/audio/game-over.wav";
  CSound cue;cue.Create(name);
  if(!cue.sound||cue.sound->samples.empty()) {
    fprintf(stderr,"AUDIO cue unavailable: %s\n",name);return;
  }
  {
    std::lock_guard<std::mutex> lock(audioMutex);
    voices.push_back({cue.sound,0,false,nextVoice++,1.f,false,true,true});
  }
  fprintf(stderr,"AUDIO cue %s\n",winner?"WINNER FANFARE":"GAME OVER");
}
void CXEngine::BeginResultsAudio() {
  size_t stopped;
  {
    std::lock_guard<std::mutex> lock(audioMutex);
    stopped=NativeStopEffects(voices,musicVoiceId);
    audioPaused=false;
  }
  fprintf(stderr,"AUDIO results boundary: removed %zu effect voices\n",stopped);
  PlayResultCue(false);
}
void CXEngine::PauseMusic(){std::lock_guard<std::mutex>l(audioMutex);musicPaused=!musicPaused;for(auto&v:voices)if(v.id==musicVoiceId)v.paused=musicPaused;}
void CXEngine::SetMusicVolume(int v){std::lock_guard<std::mutex>l(audioMutex);musicVolume=std::clamp(v,0,1000);for(auto&v:voices)if(v.id==musicVoiceId)v.gain=.44f*musicVolume/1000.f;}
float Sin(float a) { return LanActive()?DbxSyncMath::sin(a):std::sin(a * 0.017453292519943295f); }
float Cos(float a) { return LanActive()?DbxSyncMath::cos(a):std::cos(a * 0.017453292519943295f); }
float ATan2(float x, float y) { return LanActive()?DbxSyncMath::atan2(x,y):std::atan2(x, y) * 57.29577951308232f; }
float fAbs(float a) { return std::abs(a); }
float Rand() { return float(NativeGameRandom()) / 2147483648.f; }
float Pow(float a, float b) { return std::pow(a, b); }
float Sqrt(float a) { return LanActive()?DbxSyncMath::sqrt(a):std::sqrt(a); }

COLLISIONINFO CXEngine::Collide(float X1, float Y1, float R1, float DX,
                                float DY, float X2, float Y2, float R2) {
  COLLISIONINFO MyCollisionInfo;

  D3DXVECTOR2 D1;

  D1.x = DX;
  D1.y = DY;

  float D1Length = Sqrt(D1.x * D1.x + D1.y * D1.y);

  D1.x /= D1Length;
  D1.y /= D1Length;

  D3DXVECTOR2 D2;

  D2.x = X2 - X1;
  D2.y = Y2 - Y1;

  float D2Length = Sqrt(D2.x * D2.x + D2.y * D2.y);

  D2.x /= D2Length;
  D2.y /= D2Length;

  float A = (D1.x * D2.x + D1.y * D2.y) * D2Length;
  float O = Sqrt(D2Length * D2Length - A * A);

  float DestX = X1 + DX, DestY = Y1 + DY;
  float LowX, HighX, LowY, HighY;

  if (DestX < X1) {
    LowX = DestX;
    HighX = X1;
  } else {
    LowX = X1;
    HighX = DestX;
  }

  if (DestY < Y1) {
    LowY = DestY;
    HighY = Y1;
  } else {
    LowY = Y1;
    HighY = DestY;
  }

  LowX = LowX - R1 - R2;
  HighX = HighX + R1 + R2;
  LowY = LowY - R1 - R2;
  HighY = HighY + R1 + R2;

  if (O < R1 + R2 && X2 > LowX && X2 < HighX && Y2 > LowY && Y2 < HighY) {
    MyCollisionInfo.Collided = true;

    MyCollisionInfo.Pos.x = D1.x * (A - ((R1 + R2) - O)); // WRONG
    MyCollisionInfo.Pos.y = D1.y * (A - ((R1 + R2) - O)); // WRONG
  } else {
    MyCollisionInfo.Collided = false;

    MyCollisionInfo.Pos.x = D1.x * D1Length;
    MyCollisionInfo.Pos.y = D1.y * D1Length;
  }

  return MyCollisionInfo;
}
