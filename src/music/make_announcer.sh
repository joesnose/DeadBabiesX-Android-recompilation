#!/usr/bin/env bash
# Optional asset regeneration; the game uses the bundled WAV and needs no TTS tool.
set -euo pipefail
project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
temp_voice="$(mktemp /tmp/dbx-announcer-XXXXXX.wav)"
trap 'rm -f "$temp_voice"' EXIT
data_args=()
if [ -n "${ESPEAK_DATA_PARENT:-}" ]; then data_args=("--path=$ESPEAK_DATA_PARENT"); fi
espeak-ng "${data_args[@]}" -v en-us+m3 -s 115 -p 25 -a 180 -w "$temp_voice" 'Game over!'
mkdir -p "$project_dir/game/audio"
ffmpeg -v error -i "$temp_voice" -af 'asetrate=19845,aresample=48000,highpass=f=60,equalizer=f=120:t=q:w=0.8:g=5,acompressor=threshold=0.12:ratio=3:attack=4:release=120,aecho=0.8:0.8:65|115|180:0.22|0.14|0.08,alimiter=limit=0.88,apad=pad_dur=0.2' -ar 48000 -ac 2 -c:a pcm_s16le -y "$project_dir/game/audio/game-over.wav"

# Normalize the bundled voice so its source recording uses the available range.
python3 - "$project_dir/game/audio/game-over.wav" <<'PYVOICE'
import sys,numpy as np
from scipy.io import wavfile
p=sys.argv[1];sr,x=wavfile.read(p);x=x.astype(float)
x*=.92*32767/max(1,float(np.max(abs(x))))
wavfile.write(p,sr,np.rint(x).astype(np.int16))
PYVOICE
