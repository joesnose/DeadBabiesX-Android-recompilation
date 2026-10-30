"""Export enhanced original-art sheets into fixed original sprite canvases.
Original transparency constrains each sprite; this is not a gameplay conversion.
Requires Pillow. Run from the package root with --game-data game/data.
"""
from pathlib import Path
from PIL import Image,ImageChops,ImageDraw
import colorsys,json,argparse
parser=argparse.ArgumentParser();parser.add_argument('--game-data',type=Path,default=Path('game/data'));parser.add_argument('--output',type=Path,default=Path('game/supersuperhd/data'));parser.add_argument('--art',type=Path,default=Path(__file__).parent/'supersuperhd_art');args=parser.parse_args()
root=args.game_data;out=args.output;art=args.art;layout=json.loads((Path(__file__).parent/'hd_art/layout.json').read_text());layout['babies']['files']=layout['babies']['files'][:13];manifest=[]
def original(p):
 im=Image.open(root/p).convert('RGBA');a=im.load()
 for y in range(im.height):
  for x in range(im.width):
   if a[x,y][:3]==(0,0,0):a[x,y]=(0,0,0,0)
 return im

def save(im,p):
 dest=out/p.lower();dest.parent.mkdir(parents=True,exist_ok=True);im.save(dest);manifest.append(p.lower())

def fit(cell,p):
 src=original(p);b=src.getbbox();size=(src.width*4,src.height*4)
 base=src.resize(size,Image.Resampling.LANCZOS)
 cb=cell.getchannel('A').point(lambda v:255 if v>24 else 0).getbbox()
 if not cb:raise ValueError('Empty enhancement cell '+p)
 cell=cell.crop(cb).resize(((b[2]-b[0])*4,(b[3]-b[1])*4),Image.Resampling.LANCZOS)
 detail=Image.new('RGBA',size);detail.alpha_composite(cell,(b[0]*4,b[1]*4))
 mixed=Image.alpha_composite(base,detail)
 # Use the original silhouette, including holes in aiming reticles/crate frames.
 mixed.putalpha(base.getchannel('A'))
 save(mixed,p);return mixed

for name in ['hero','babies','props']:
 sheet=Image.open(art/(name+'.png')).convert('RGBA');item=layout[name];cols=item['cols'];rows=item['rows']
 for i,p in enumerate(item['files']):
  cell=sheet.crop((i%cols*sheet.width//cols,i//cols*sheet.height//rows,(i%cols+1)*sheet.width//cols,(i//cols+1)*sheet.height//rows))
  result=fit(cell,p)
  if name=='hero':
   red=original(p)
   for colour,hue in [('Yellow',1/6),('Blue',2/3),('Green',1/3)]:
    target=p.replace('/Red/','/'+colour+'/').replace('/Red_','/'+colour+'_')
    targetsrc=original(target)
    difference=ImageChops.difference(red.convert('RGB'),targetsrc.convert('RGB'))
    mask=difference.convert('L').point(lambda v:255 if v>6 else 0).resize(result.size,Image.Resampling.NEAREST)
    variant=result.copy();px=variant.load();mp=mask.load()
    tp=targetsrc.resize(result.size,Image.Resampling.NEAREST).load();rp=red.resize(result.size,Image.Resampling.NEAREST).load()
    for y in range(variant.height):
     for x in range(variant.width):
      if mp[x,y]<128:continue
      r,g,b,a=px[x,y];h,s,v=colorsys.rgb_to_hsv(r/255,g/255,b/255)
      if s>.35 and (h<.055 or h>.97):
       th,ts,tv=colorsys.rgb_to_hsv(*(c/255 for c in tp[x,y][:3]))
       rv=max(rp[x,y][:3])/255
       rgb=colorsys.hsv_to_rgb(th,ts,min(1,v*tv/max(rv,.1)));px[x,y]=tuple(round(c*255) for c in rgb)+(a,)
    variant.putalpha(targetsrc.resize(variant.size,Image.Resampling.LANCZOS).getchannel('A'));save(variant,target)

# Export full-body legacy/life sprites from the correctly aligned modular poses.
for colour in ['Red','Yellow','Blue','Green']:
 base=Image.open(out/f'player/{colour.lower()}/{colour.lower()}_legs_stand.png').convert('RGBA');upper=Image.open(out/f'player/{colour.lower()}/{colour.lower()}_0.png').convert('RGBA');base.alpha_composite(upper)
 base.putalpha(original('Player/'+colour+'/'+colour+'_Stand.png').resize(base.size,Image.Resampling.LANCZOS).getchannel('A'))
 save(base,'Player/'+colour+'/'+colour+'_Stand.png');fit(base,'Player/'+colour+'/'+colour+'_Lives.png')
# Scenery falls back to Super HD; injury/gore textures are not replaced.
save(Image.open(out/'babies/babyballoon3.png'),'Babies/BabyBalloon.png')
(out.parent/'manifest.json').write_text(json.dumps({'style':'Super Super HD - realistic adult/equipment materials, stylized living cartoon enemies','textures':manifest,'sprite_export_scale':4,'fallback':'superhd then hd then original','original_sprite_canvases':True,'original_alpha_silhouettes':True,'no_gameplay_changes':True,'injury_and_gore_unchanged':True},indent=2))
print('Exported',len(manifest),'Super Super HD textures')
