"""Compile the actual art-selection functions with file/renderer test doubles.
Run: python source/test_art_modes.py (requires g++).
"""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parent
engine=(root/'xengine.cpp').read_text()
engine=engine[engine.index('static int artOverride'):engine.index('IDirect3DTexture8 *CXEngine::LoadTexture')]
front=(root/'frontend.cpp').read_text()
front=front[front.index('MenuAction Frontend::ToggleArt'):front.index('void Frontend::Draw')]
code=r'''
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cassert>
static bool installed[6]={true,true,true,true,true,true};
#define INVALID_FILE_ATTRIBUTES (-1)
int GetFileAttributesA(const char *p) {
 const char *names[]={"", "game/hd/manifest.json", "game/superhd/manifest.json", "game/supersuperhd/manifest.json"};
 for(int i=1;i<4;++i) if(std::string(p)==names[i]) return installed[i] ? 0 : -1;
 return -1;
}
''' +engine+r'''
enum class MenuAction { None, ArtChanged };
struct Engine { void *LoadTexture(const char*) { return nullptr; } } renderer;
Engine *XEngine=&renderer;
struct Frontend { int artMode=0; void *backdrop=nullptr; MenuAction ToggleArt(int); };
''' +front+r'''
int main() {
 Frontend f;
 int checks=0;
 for(int mask=0;mask<8;++mask) {
  for(int mode=1;mode<4;++mode) installed[mode]=(mask & (1<<(mode-1)))!=0;
  for(int start=0;start<6;++start) {
   if(!installed[start]) continue;
   for(int direction : {1,-1}) {
    f.artMode=start; assert(NativeSetArtMode(start));
    int expected=start;
    do { expected=(expected+direction+6)%6; } while(!installed[expected]);
    MenuAction result=f.ToggleArt(direction);
    assert(f.artMode==expected);
    assert(result==(expected==start ? MenuAction::None : MenuAction::ArtChanged));
    ++checks;
   }
  }
 }
 for(int i=0;i<4;++i) installed[i]=true;
 assert(NativeSetArtMode(3)); assert(std::string(NativeArtSkin())=="supersuperhd");
 assert(!NativeSetArtMode(6)); assert(!NativeSetArtMode(-1));
 assert(std::string(NativeArtSkin())=="supersuperhd");
 assert(std::string(NativeArtLabel(3))=="SUPER SUPER HD");
 artOverride=-1; unsetenv("DBX_CARTOON"); unsetenv("DBX_CEL_SHADED"); unsetenv("DBX_ORIGINAL_ART"); unsetenv("DBX_SUPER_HD"); unsetenv("DBX_SUPER_SUPER_HD");
 assert(std::string(NativeArtSkin())=="hd");
 setenv("DBX_SUPER_SUPER_HD","1",1); assert(std::string(NativeArtSkin())=="supersuperhd");
 setenv("DBX_CEL_SHADED","1",1); assert(std::string(NativeArtSkin())=="celshaded");
 assert(std::string(NativeArtLabel(4))=="CEL SHADED");
 assert(NativeSetArtMode(4)); assert(std::string(NativeArtSkin())=="celshaded");
 artOverride=-1;
 setenv("DBX_CARTOON","1",1); assert(std::string(NativeArtSkin())=="cartoon");
 assert(NativeSetArtMode(5)); assert(std::string(NativeArtLabel(5))=="CARTOON");
 artOverride=-1;
 setenv("DBX_ORIGINAL_ART","1",1); assert(NativeArtSkin()==nullptr);
 printf("PASS: %d six-mode transitions across all pack availability combinations; defaults, overrides, invalid modes\n",checks);
}

'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(code)
 subprocess.run(['g++','-std=c++17','-Wall','-Wextra',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
