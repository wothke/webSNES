::  POOR MAN'S DOS PROMPT BUILD SCRIPT.. make sure to delete the respective built/*.bc files before building!
::  existing *.bc files will not be recompiled. 

setlocal enabledelayedexpansion

SET ERRORLEVEL
VERIFY > NUL


:: **** use the "-s WASM" switch to compile WebAssembly output. warning: the SINGLE_FILE approach does NOT currently work in Chrome 63.. ****
set "OPT=  -s WASM=1 -DHAVE_ZLIB_H -s ASSERTIONS=1 -s VERBOSE=0 -s FORCE_FILESYSTEM=1 -DEMSCRIPTEN -DNO_DEBUG_LOGS -DHAVE_LIMITS_H -DHAVE_STDINT_H -Wcast-align -fno-strict-aliasing -s SAFE_HEAP=1 -s DISABLE_EXCEPTION_CATCHING=0 -Wno-pointer-sign -I. -I.. -I../gme -I../zlib  -Os -O3 "
if not exist "built/zlib.bc" (
	call emcc.bat %OPT% ../zlib/adler32.c ../zlib/compress.c ../zlib/crc32.c ../zlib/gzio.c ../zlib/uncompr.c ../zlib/deflate.c ../zlib/trees.c ../zlib/zutil.c ../zlib/inflate.c ../zlib/infback.c ../zlib/inftrees.c ../zlib/inffast.c  -o built/zlib.bc
	IF !ERRORLEVEL! NEQ 0 goto :END
)
if not exist "built/base.bc" (
	call emcc.bat %OPT% -std=c++11 ../gme/Blip_Buffer.cpp ../gme/Classic_Emu.cpp ../gme/Data_Reader.cpp ../gme/Dual_Resampler.cpp ../gme/Effects_Buffer.cpp ../gme/Fir_Resampler.cpp ../gme/gme.cpp ../gme/Gme_File.cpp ../gme/M3u_Playlist.cpp ../gme/Multi_Buffer.cpp ../gme/Music_Emu.cpp -o built/base.bc
	IF !ERRORLEVEL! NEQ 0 goto :END
)
if not exist "built/emus.bc" (
	call emcc.bat  %OPT% -std=c++11 ../gme/Sms_Apu.cpp ../gme/Ay_Apu.cpp ../gme/Ym2612_Emu.cpp ../gme/Ay_Cpu.cpp ../gme/Ay_Emu.cpp ../gme/Gym_Emu.cpp ../gme/Snes_Spc.cpp ../gme/Spc_Cpu.cpp ../gme/Spc_Dsp.cpp ../gme/Spc_Emu.cpp ../gme/Spc_Filter.cpp -o built/emus.bc
	IF !ERRORLEVEL! NEQ 0 goto :END
)
call emcc.bat %OPT% -std=c++11 -s TOTAL_MEMORY=67108864 --closure 1 --llvm-lto 1  --memory-init-file 0 built/zlib.bc  built/base.bc built/emus.bc adapter.cpp   -s EXPORTED_FUNCTIONS="[ '_emu_load_file','_emu_teardown','_emu_get_current_position','_emu_seek_position','_emu_get_max_position','_emu_set_subsong','_emu_get_track_info','_emu_get_sample_rate','_emu_get_audio_buffer','_emu_get_audio_buffer_length','_emu_compute_audio_samples', '_malloc', '_free']"  -o htdocs/snes.js  -s SINGLE_FILE=0 -s EXTRA_EXPORTED_RUNTIME_METHODS="['ccall', 'Pointer_stringify']"  -s BINARYEN_ASYNC_COMPILATION=1 -s BINARYEN_TRAP_MODE='clamp' && copy /b shell-pre.js + htdocs\snes.js + shell-post.js htdocs\web_snes3.js && del htdocs\snes.js && copy /b htdocs\web_snes3.js + snes_adapter.js htdocs\backend_snes.js && del htdocs\web_snes3.js
:END

