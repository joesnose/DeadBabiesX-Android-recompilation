"""Original 20-second acid-house loop and short winner fanfare; all synthesized."""
import numpy as np
from scipy import signal
from scipy.io import wavfile
from pathlib import Path
SR=48000;N=SR*20;BPM=120;BEAT=.5;rng=np.random.default_rng(301044)
out=Path(__file__).parent
mix=np.zeros((N,2),float)
def hz(n):return 440*2**((n-69)/12)
def env(n,a=.002,r=.02):
 e=np.ones(n);aa=min(int(a*SR),n//2);rr=min(int(r*SR),n//2)
 if aa:e[:aa]=np.linspace(0,1,aa,endpoint=False)
 if rr:e[-rr:]=np.linspace(1,0,rr)
 return e
def band(w,lo,hi):return signal.sosfilt(signal.butter(2,[lo,hi],btype='bandpass',fs=SR,output='sos'),w)
def add(w,beat,gain=1,pan=0):
 idx=(int(round(beat*BEAT*SR))+np.arange(len(w)))%N
 np.add.at(mix[:,0],idx,w*gain*np.sqrt((1-pan)/2));np.add.at(mix[:,1],idx,w*gain*np.sqrt((1+pan)/2))
def kick():
 t=np.arange(int(.32*SR))/SR;freq=45+100*np.exp(-t*35)
 return np.tanh(1.5*np.sin(2*np.pi*np.cumsum(freq)/SR))*np.exp(-t*14)*env(len(t),.0003,.02)
def clap():
 t=np.arange(int(.18*SR))/SR;n=band(rng.normal(size=len(t)),1100,10500)
 e=np.exp(-t*30)*.35
 for at in [0,.012,.025]:e+=np.where(t>=at,np.exp(-np.maximum(0,t-at)*190),0)
 return np.tanh(n*e*.8)*env(len(t),.0004,.035)
def hat(opened):
 t=np.arange(int((.21 if opened else .055)*SR))/SR
 return band(rng.normal(size=len(t)),7500,17500)*np.exp(-t*(18 if opened else 75))*env(len(t),.0002,.01)
for bar in range(10):
 for beat in range(4):add(kick(),bar*4+beat,.72)
 for beat in [1,3]:add(clap(),bar*4+beat,.27,.06)
 for i in range(8):add(hat(True),bar*4+i*.5+.5,.11,.25)
 for i in range(16):
  if i%2==0:add(hat(False),bar*4+i*.25,.038,-.3)
 if bar in [4,9]:
  for i in range(4):add(clap(),bar*4+3+i*.25,.055*(i+1),-.12)
# Original resonant saw sequence with note slides and accents.
notes=[38,38,50,41,None,45,38,48,38,None,43,50,41,38,45,36]
phase=0.;prev=hz(38);dry=np.zeros(N);cutoffs=np.zeros(N);amp=np.zeros(N)
stepN=int(SR*BEAT/4)
for step in range(160):
 start=step*stepN;note=notes[step%16]
 if note is None:continue
 if (step//16)%4==3 and step%16 in [6,14]:note+=12
 f=hz(note);t=np.arange(stepN)/SR
 freq=prev+(f-prev)*np.minimum(1,t/.045) if step%16 in [2,7,11,14] else np.full(stepN,f)
 ph=phase+2*np.pi*np.cumsum(freq)/SR;phase=ph[-1]%(2*np.pi);prev=f
 dry[start:start+stepN]=signal.sawtooth(ph)*.72+np.sin(ph)*.28
 accent=1.0 if step%16 in [0,5,10,14] else .66
 amp[start:start+stepN]=env(stepN,.0015,.009)*np.exp(-t*4)*accent
 sweep=.5+.5*np.sin(2*np.pi*(start/SR)/20-.8)
 cutoffs[start:start+stepN]=350+900*sweep+accent*2500*np.exp(-t*27)
# Resonant two-pole low-pass with coefficients updated every 64 samples.
filtered=np.zeros(N);zi=np.zeros(2)
for at in range(0,N,64):
 cutoff=max(180,float(np.mean(cutoffs[at:at+64])))
 w=2*np.pi*cutoff/SR;c=np.cos(w);alpha=np.sin(w)/(2*5.2)
 b=np.array([(1-c)/2,1-c,(1-c)/2])/(1+alpha)
 a=np.array([1,-2*c/(1+alpha),(1-alpha)/(1+alpha)])
 filtered[at:at+64],zi=signal.lfilter(b,a,dry[at:at+64],zi=zi)
acid=np.tanh(filtered*2.7)*amp
mix[:,0]+=acid*.34;mix[:,1]+=acid*.34
for delay,gain in [(.1875,.09),(.375,.035)]:
 mix+=np.roll(np.column_stack([acid,acid]),int(delay*SR),axis=0)*gain
mix=np.tanh(mix*1.35);mix*=10**(-1/20)/np.max(np.abs(mix))
wavfile.write(out/'Acid-Circuit-20s.wav',SR,np.rint(mix*32767).astype(np.int16))
assert len(mix)==N and np.max(np.abs(mix[0]-mix[-1]))<np.percentile(np.abs(np.diff(mix,axis=0)),99)
print('Acid Circuit: exact 20-second 120 BPM stereo loop, no clipping, verified seam')
# Two-second original brass/bell victory phrase in D minor.
fanfare=np.zeros((int(SR*2.5),2))
for start,notes,dur in [(0,[62],.19),(.19,[65],.19),(.38,[69],.19),(.64,[74,77,81],1.5)]:
 t=np.arange(int(dur*SR))/SR;wave=np.zeros_like(t)
 for note in notes:
  f=hz(note)
  wave+=(np.sin(2*np.pi*f*t)+.35*np.sin(4*np.pi*f*t)+.18*np.sin(6*np.pi*f*t))/len(notes)
 wave=np.tanh(wave*1.5)*env(len(t),.015,.25)*np.exp(-t*.6)
 at=int(start*SR);fanfare[at:at+len(wave),0]+=wave*.6;fanfare[at:at+len(wave),1]+=wave*.6
for delay,gain in [(.055,.14),(.11,.07)]:
 shift=int(SR*delay);fanfare[shift:]+=fanfare[:-shift]*gain
fanfare*=.88/np.max(np.abs(fanfare))
wavfile.write(out/'winner-fanfare.wav',SR,np.rint(fanfare*32767).astype(np.int16))
print('Winner fanfare: 2.5 seconds, stereo PCM')
