"""Validate exported canvases, alpha masks, and coverage against original assets."""
from PIL import Image
from pathlib import Path
import json
r=Path(__file__).resolve().parent.parent
m=json.loads((r/'game/supersuperhd/manifest.json').read_text())
originals={str(p.relative_to(r/'game/data')).lower():p for p in (r/'game/data').rglob('*') if p.is_file()}
for name in m['textures']:
 original=Image.open(originals[name]).convert('RGBA');data=original.load()
 for y in range(original.height):
  for x in range(original.width):
   if data[x,y][:3]==(0,0,0):data[x,y]=(0,0,0,0)
 out=Image.open(r/'game/supersuperhd/data'/name).convert('RGBA')
 assert out.size==(original.width*4,original.height*4),(name,out.size,original.size)
 assert out.getchannel('A').tobytes()==original.resize(out.size,Image.Resampling.LANCZOS).getchannel('A').tobytes(),name
for banned in ['babies/deadbaby.png','babies/baby_bomb.png']:
 assert banned not in m['textures']
assert not any(n.startswith('gibs/') for n in m['textures'])
print('PASS:',len(m['textures']),'textures, 4x dimensions, exact original alpha masks; injury/gore assets untouched')
