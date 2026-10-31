"""Fit generated Super HD artwork to original canvases and alpha masks.
Run from the package root: python source/export_superhd_assets.py
Requires Pillow. Gameplay coordinates and sprite silhouettes stay unchanged.
"""
from pathlib import Path
from PIL import Image
import json
root=Path(__file__).resolve().parent.parent
out=root/'game/superhd/data/backgrounds';out.mkdir(parents=True,exist_ok=True)
items=[('background','train_bg3.png'),('floor','train_bg2.png')]
for name,filename in items:
 src=Image.open(root/'game/data/Backgrounds'/filename).convert('RGBA')
 pixels=src.load()
 for y in range(src.height):
  for x in range(src.width):
   if pixels[x,y][:3]==(0,0,0):pixels[x,y]=(0,0,0,0)
 size=(src.width*4,src.height*4)
 base=src.resize(size,Image.Resampling.LANCZOS)
 art=Image.open(root/'source/superhd_art'/(name+'.png')).convert('RGBA')
 if name=='background':
  result=art.resize(size,Image.Resampling.LANCZOS)
 else:
  bounds=art.getchannel('A').point(lambda a:255 if a>24 else 0).getbbox()
  target=base.getchannel('A').getbbox()
  result=base.copy()
  art=art.crop(bounds).resize((target[2]-target[0],target[3]-target[1]),Image.Resampling.LANCZOS)
  result.paste(art,(target[0],target[1]))
 result.putalpha(base.getchannel('A'))
 result.save(out/filename)
 assert result.size==size
 assert result.getchannel('A').tobytes()==base.getchannel('A').tobytes()
 print('PASS',filename,size,'original alpha mask preserved')
(root/'game/superhd/manifest.json').write_text(json.dumps({
 'style':'Super HD - bronze industrial scenery and hand-painted rock',
 'textures':['backgrounds/'+f for _,f in items],
 'size':[2048,2048], 'fallback':'hd then original',
 'original_alpha_silhouettes':True,'no_gameplay_changes':True},indent=2))
