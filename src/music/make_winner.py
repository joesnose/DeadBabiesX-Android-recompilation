"""Regenerate spoken Winner cue and new electric sting (eSpeak NG + ffmpeg)."""
import os,subprocess,tempfile
from pathlib import Path
import numpy as np
from scipy.io import wavfile
root=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory() as tmp:
 dry=Path(tmp)/'dry.wav';voice=Path(tmp)/'voice.wav'
 args=['espeak-ng']
 if os.getenv('ESPEAK_DATA_PARENT'):args+=['--path='+os.environ['ESPEAK_DATA_PARENT']]
 subprocess.run(args+['-v','en-us+m3','-s','125','-p','33','-a','180','-w',str(dry),'Winner!'],check=True)
 subprocess.run(['ffmpeg','-v','error','-y','-i',str(dry),'-af','aresample=48000,highpass=f=85,equalizer=f=1800:t=q:w=1:g=4,acompressor=threshold=0.1:ratio=3,alimiter=limit=0.95,apad=pad_dur=0.15','-ar','48000','-ac','2',str(voice)],check=True)
 sr,v=wavfile.read(voice);v=v.astype(float)/32768;v*=.95/max(1e-6,np.max(abs(v)))
 out=np.zeros((sr*3,2));out[:len(v)]+=v*.95
 for at,note,dur in [(.95,64,.16),(1.13,67,.16),(1.31,71,.16),(1.52,76,1.1)]:
  t=np.arange(int(dur*sr))/sr;f=440*2**((note-69)/12)
  w=np.tanh(2*(np.sin(2*np.pi*f*t)+.35*np.sin(4*np.pi*f*t)))*np.exp(-t*3)*.2
  w[:240]*=np.linspace(0,1,240);w[-480:]*=np.linspace(1,0,480)
  i=int(at*sr);out[i:i+len(w)]+=w[:,None]
 out*=.95/max(.95,np.max(abs(out)))
 wavfile.write(root/'game/audio/winner-fanfare.wav',sr,np.rint(out*32767).astype(np.int16))
