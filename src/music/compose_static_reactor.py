"""Original 20-second dark big-beat game loop; all instruments synthesized here.
144 BPM, twelve 4/4 bars, D minor. No samples or melodies from the reference.
"""
import numpy as np
from scipy import signal
from scipy.io import wavfile
from pathlib import Path
SR=48000; SECONDS=20; N=SR*SECONDS; BPM=144; BEAT=60/BPM
rng=np.random.default_rng(291044)
mix=np.zeros((N,2),np.float64)

def hz(midi):return 440*2**((midi-69)/12)
def env(n,attack=.004,release=.045):
 e=np.ones(n);a=min(int(attack*SR),n//2);r=min(int(release*SR),n//2)
 if a:e[:a]=np.linspace(0,1,a,endpoint=False)**.7
 if r:e[-r:]=np.linspace(1,0,r,endpoint=True)**1.4
 return e

def add(wave,beat,level=1,pan=0):
 wave=np.asarray(wave,dtype=float)*level
 gains=np.sqrt([(1-pan)/2,(1+pan)/2])
 idx=(int(round(beat*BEAT*SR))+np.arange(len(wave)))%N
 np.add.at(mix[:,0],idx,wave*gains[0]);np.add.at(mix[:,1],idx,wave*gains[1])

def filt(x,lo,hi):
 return signal.sosfilt(signal.butter(2,[lo,hi],btype='bandpass',fs=SR,output='sos'),x)
def kick():
 t=np.arange(int(.35*SR))/SR
 f=45+110*np.exp(-t*32)
 phase=2*np.pi*np.cumsum(f)/SR
 body=np.sin(phase)*np.exp(-t*15)
 click=filt(rng.normal(0,1,len(t)),1700,7000)*np.exp(-t*160)*.12
 return np.tanh(1.6*(body+click))*env(len(t),.0005,.035)
def snare():
 t=np.arange(int(.24*SR))/SR
 noise=filt(rng.normal(0,1,len(t)),1100,10500)*np.exp(-t*24)
 body=np.sin(2*np.pi*190*t)*np.exp(-t*30)*.48
 return np.tanh((noise*.7+body)*1.8)*env(len(t),.0005,.045)
def hat(opened=False):
 t=np.arange(int((.22 if opened else .065)*SR))/SR
 w=filt(rng.normal(0,1,len(t)),6500,17000)
 return w*np.exp(-t*(18 if opened else 65))*env(len(t),.0003,.015)
def tom(note):
 t=np.arange(int(.22*SR))/SR;f=hz(note)
 return np.sin(2*np.pi*(f*t+f*.007*(1-np.exp(-t*40))))*np.exp(-t*22)*env(len(t),.001,.02)
def crash():
 t=np.arange(int(1.25*SR))/SR
 return filt(rng.normal(0,1,len(t)),5000,17000)*np.exp(-t*4.8)*env(len(t),.001,.12)
def bass(note,duration):
 t=np.arange(int(duration*BEAT*SR))/SR;f=hz(note)
 w=np.sin(2*np.pi*f*t)+.25*np.sin(4*np.pi*f*t)+.12*signal.sawtooth(2*np.pi*f*t)
 return np.tanh(1.4*w)*env(len(t),.004,.018)
def guitar(note,duration,detune=0):
 t=np.arange(int(duration*BEAT*SR))/SR
 # Root/fifth/octave power chord; picked, saturated and darkened.
 w=np.zeros_like(t)
 for pitch,vol in [(note,1),(note+7,.65),(note+12,.45)]:
  f=hz(pitch)*(1+detune)
  for h in range(1,12):
   w+=vol*np.sin(2*np.pi*f*h*t+.13*h)/(h*2.1)
 w=np.tanh(2.8*w)*np.exp(-t*2.6)
 return filt(w,110,5700)*env(len(t),.003,.024)
def lead(note,duration):
 t=np.arange(int(duration*BEAT*SR))/SR;f=hz(note)
 phase=2*np.pi*f*t+.025*np.sin(2*np.pi*5.2*t)
 w=.66*np.sin(phase)+.2*np.sin(2*phase)+.12*np.sin(3*phase)
 return w*env(len(t),.009,.045)


def reese(note,duration):
 t=np.arange(int(duration*BEAT*SR))/SR;f=hz(note)
 # Detuned mid-bass over a clean sub; original rhythm and pitches.
 raw=.43*signal.sawtooth(2*np.pi*f*.996*t)+.43*signal.sawtooth(2*np.pi*f*1.004*t)
 raw+=.36*np.sin(2*np.pi*f*t)+.18*np.sin(np.pi*f*t)
 raw=filt(np.tanh(raw*3.2),40,2400)
 return raw*np.exp(-t*2.4)*env(len(t),.003,.025)
def stab(notes,duration):
 t=np.arange(int(duration*BEAT*SR))/SR;w=np.zeros_like(t)
 for note in notes:
  f=hz(note);phase=2*np.pi*f*t
  w+=(np.sin(phase+1.1*np.sin(phase*2))+.28*np.sin(phase*3))/len(notes)
 return filt(np.tanh(w*2.5),250,4700)*np.exp(-t*8)*env(len(t),.002,.025)
def grit_snare():
 w=snare()
 return filt(np.tanh(w*2.9),170,11000)*.82

drum_bus=np.zeros((N,2),dtype=float)
for bar in range(12):
 b=bar*4
 for off,vel in ([(0,1),(.75,.65),(1.75,.8),(2.5,1),(3.5,.6)] if bar%2==0 else [(0,1),(.5,.6),(1.5,.8),(2.25,.85),(2.75,.55)]):
  add(kick(),b+off,.72*vel,-.015)
 for off in [1,3]:
  add(grit_snare(),b+off,.41,.06)
  add(snare(),b+off+.028,.075,-.24)
 for off in ([1.75,2.75,3.75] if bar%2 else [.5,2.5]):
  add(grit_snare(),b+off,.07,-.22)
 for i in range(16):
  if i in [3,11] and bar%2==0:continue
  add(hat(i==14 and bar%2==1),b+i*.25,.10 if i%4==2 else .045 if i%2 else .065,(-.38 if i%2 else .38))
 if bar in [0,4,8]:add(crash(),b,.12,-.4)
 if bar%4==3:
  for off,pitch in [(3,48),(3.25,45),(3.5,41),(3.75,38)]:
   add(tom(pitch),b+off,.24,-.4+(off-3)*.9)

# Dark syncopated bass motif, with an answering riff every other bar.
patterns=[
 [(0,38,.42),(.5,38,.19),(.75,50,.4),(1.5,41,.3),(2,38,.4),(2.75,39,.18),(3,45,.28),(3.5,36,.3)],
 [(0,38,.65),(1,38,.2),(1.5,43,.35),(2.25,41,.2),(2.75,38,.4),(3.5,39,.3)]
]
for bar in range(12):
 for off,note,duration in patterns[bar%2]:
  if bar==7 and off>=3:continue
  add(reese(note,duration),bar*4+off,.34,0)
 for off,notes in ([(.5,[62,65,69]),(2.75,[60,63,67])] if bar%2==0 else [(1.5,[62,65,69]),(3.25,[65,68,72])]):
  if bar in [2,6,10]:continue
  w=stab(notes,.45)
  add(w,bar*4+off,.13,-.25)
  add(w,bar*4+off+.75,.028,.45)
 # Bright high percussion stabs, no sampled vocals.
 if bar in [3,7,11]:
  t=np.arange(int(.35*SR))/SR
  texture=filt(rng.normal(0,1,len(t)),1400,6500)
  texture*=np.exp(-t*12)*(np.sin(2*np.pi*38*t)>.1)
  add(texture,bar*4+2.5,.08,.35)

# Short room, cross-channel echoes, and circular tails for a clean loop.
dry=mix.copy()
for delay,amount in [(.025,.065),(.053,.04),(.091,.025)]:
 mix+=np.roll(dry,int(SR*delay),axis=0)[:,::-1]*amount
mix=np.tanh(mix*1.55)
mix*=10**(-1.2/20)/np.max(np.abs(mix))
pcm=np.rint(mix*32767).astype(np.int16)
out=Path(__file__).parent/'Static-Reactor-Big-Beat-20s.wav'
wavfile.write(out,SR,pcm)
assert len(pcm)==960000 and np.max(np.abs(pcm.astype(int)))<32767
print(out)
print('Duration:',len(pcm)/SR,'seconds; tempo:',BPM,'BPM; stereo 48 kHz')
print('Peak dBFS:',round(20*np.log10(np.max(np.abs(mix))),2))
print('RMS dBFS:',round(20*np.log10(np.sqrt(np.mean(mix**2))),2))
step=np.max(np.abs(mix[0]-mix[-1]));ordinary=np.percentile(np.abs(np.diff(mix,axis=0)),99)
assert step<ordinary
print('PASS: exact duration, no clipped samples, natural loop boundary')
