#!/usr/bin/env python3
"""Run real TCP sessions and game updates in independent native processes."""
import os,sys,subprocess,time,re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
exe=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'build-host-lan/dbx-host'
env=os.environ.copy();logs=root/'tests/lan-logs';logs.mkdir(exist_ok=True);env['DBX_LAN_DIAGNOSTICS']=str(logs/'diagnostics.log')
def match(players,frames,mode):
 procs=[];files=[]
 try:
  for i in range(players):
   f=open(logs/f'{players}p-{mode}-{i}.log','w');files.append(f)
   role='--lan-host' if i==0 else '--lan-client'
   procs.append(subprocess.Popen([str(exe),role,str(players),str(frames),mode],cwd=root/'android/assets',env=env,stdout=f,stderr=f))
   if i==0:time.sleep(.2)
  deadline=time.monotonic()+50+frames/80
  while any(p.poll() is None for p in procs):
   if time.monotonic()>deadline:raise AssertionError(f'{mode}: timed out')
   time.sleep(.05)
  for f in files:f.flush()
  texts=[Path(f.name).read_text() for f in files]
  codes=[p.returncode for p in procs]
  assert all(c==0 for c in codes),(mode,codes,[t[-1500:]for t in texts])
  if mode=='normal':
   hashes=[re.findall(r'FRAME (\d+) HASH (\w+)',t) for t in texts]
   assert all(h==hashes[0] for h in hashes),'Different game states on independent peers'
   assert all('paused=1 resumed=1 projectiles=1' in t for t in texts)
  if mode=='results':assert all('RESULTS RETURN SYNCHRONIZED' in t for t in texts)
  print(f'PASS: {players} players, {mode}, {frames} frames',flush=True)
 finally:
  for p in procs:
   if p.poll() is None:p.kill()
  for p in procs:p.wait()
  for f in files:f.close()
match(2,14400,'normal')
match(4,7200,'normal')
match(2,600,'results')
match(2,60,'desync')
match(2,60,'disconnect')
# An intentionally malformed peer must fail before allocating unbounded data or starting play.
import socket,struct,threading
for expected,payload,length,version in [
 ('packet size',b'',129,0x00010401),
 ('version',b'',None,0x00010402),
 ('assignment',b'bad-slot',None,0x00010401),
 ('disconnected',b'truncate',None,0x00010401),
]:
 listener=socket.socket();listener.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1);listener.bind(('127.0.0.1',39828));listener.listen(1);listener.settimeout(8)
 f=open(logs/('reject-'+expected.replace(' ','-')+'.log'),'w')
 proc=subprocess.Popen([str(exe),'--lan-client','2','0','reject:'+expected],cwd=root/'android/assets',env=env,stdout=f,stderr=f)
 try:
  conn,_=listener.accept();conn.settimeout(5)
  data=b''
  while len(data)<12:data+=conn.recv(12-len(data))
  start=struct.pack('!IIBBB',version,42,4 if payload==b'bad-slot' else 1,2,0)
  declared=129 if length==129 else len(start)
  packet=struct.pack('!IHH',0x4442584c,2,declared)+(b'' if length==129 else start[:2] if payload==b'truncate' else start)
  conn.sendall(packet);conn.close()
  assert proc.wait(timeout=15)==0,expected
  print('PASS: malformed peer rejected ('+expected+')',flush=True)
 finally:
  if proc.poll() is None:proc.kill();proc.wait()
  f.close();listener.close()
print('LAN integration checks passed.')
