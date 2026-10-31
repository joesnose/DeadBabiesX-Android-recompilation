"""Original Rock: 20 seconds, 144 BPM, twelve bars; synthesized guitars/drums."""
import numpy as np
from scipy import signal
from scipy.io import wavfile
from pathlib import Path
sr=48000;n=sr*20;beat=60/144;rng=np.random.default_rng(92017);mix=np.zeros((n,2))
def add(w,b,g=1,pan=0):
 i=(round(b*beat*sr)+np.arange(len(w)))%n
 for c,a in enumerate([(1-pan)**.5,(1+pan)**.5]):np.add.at(mix[:,c],i,w*g*a*.707)
def guitar(note,dur,palm=False,detune=0):
 t=np.arange(round(dur*sr))/sr;f=440*2**((note-69+detune)/12);w=np.zeros(len(t))
 for semitone in [0,7,12]:
  freq=f*2**(semitone/12)
  for h in range(1,20):
   if freq*h>14000:break
   w+=np.sin(2*np.pi*freq*h*t+rng.uniform(0,6.28))*np.exp(-t*(2+h*.2))/h
 w=np.tanh(w*4)*np.exp(-t*(12 if palm else 1.5))
 w=signal.sosfilt(signal.butter(2,[95,6500],btype='bandpass',fs=sr,output='sos'),w)
 a=min(96,len(w));w[:a]*=np.linspace(0,1,a);w[-min(720,len(w)):]*=np.linspace(1,0,min(720,len(w)))
 return w
for bar in range(12):
 root=[40,40,43,45,40,40,48,47,40,43,45,47][bar]
 for step in range(8):
  for pan,dt in [(-.75,-.05),(.75,.05)]:add(guitar(root,beat*.49,step%4!=0,dt),bar*4+step*.5,.19,pan)
  t=np.arange(round(beat*.48*sr))/sr;f=440*2**((root-12-69)/12)
  add(np.tanh(np.sin(2*np.pi*f*t)*2)*np.exp(-t*8),bar*4+step*.5,.19)
 for b in [0,1.5,2,2.75]:
  t=np.arange(int(.25*sr))/sr;add(np.sin(2*np.pi*np.cumsum(48+110*np.exp(-t*45))/sr)*np.exp(-t*20),bar*4+b,.55)
 for b in [1,3]:
  t=np.arange(int(.18*sr))/sr;noise=signal.sosfilt(signal.butter(2,1300,btype='highpass',fs=sr,output='sos'),rng.normal(size=len(t)))
  add((noise*.5+np.sin(2*np.pi*185*t)*.5)*np.exp(-t*25),bar*4+b,.4)
 for step in range(8):
  t=np.arange(int(.065*sr))/sr;w=signal.sosfilt(signal.butter(2,7500,btype='highpass',fs=sr,output='sos'),rng.normal(size=len(t)))*np.exp(-t*65)
  add(w,bar*4+step*.5,.12,.3)
# A short original lead motif, doubled an octave above the rhythm.
for bar in range(12):
 for j,note in enumerate([64,67,69,67,71,69,67,62]):add(guitar(note,beat*.46),bar*4+j*.5,.075,.1)
mix=np.tanh(mix*1.2);mix*=.89/np.max(np.abs(mix))
# Tiny boundary ramps remove a click while retaining exact beat timing.
mix[:96]*=np.linspace(0,1,96)[:,None];mix[-96:]*=np.linspace(1,0,96)[:,None]
wavfile.write(Path(__file__).with_name('Rock-20s.wav'),sr,np.rint(mix*32767).astype(np.int16))
print('Rock: original 20s 144 BPM stereo guitar loop, peak',np.max(np.abs(mix)))
