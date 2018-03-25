# webSNES

Copyright (C) 2018 Juergen Wothke

This is a JavaScript/WebAudio plugin of stripped down Game_Music_Emu 0.6.2. Various formats have been disabled - since
I already have better emulators for them: .hes/.gbs/.nsf/.kss (webNEZ), .sap (webASAP), .vgm (webVGM). Only 
supported formats are: .ay (spectreZX is probably the better choice here but dependency wise it seems to be 
required anyway..), .gym (I added support for packed format) and .spc (the one format that this player might be 
most useful for..)


A live demo of this program can be found here: http://www.wothke.ch/webSNES/


## Credits
Game_Music_Emu library copyright (C) 2003-2009 Shay Green.
Sega Genesis YM2612 emulator copyright (C) 2002 Stephane Dallongeville.
zlib (C) 1995-2005 Jean-loup Gailly and Mark Adler

## Project

All the "Web" specific additions (i.e. the whole point of this project) are contained in the 
"emscripten" subfolder. 


## Howto build

You'll need Emscripten (http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). The make script 
is designed for use of emscripten version 1.37.29 (unless you want to create WebAssembly output, older versions might 
also still work).

The below instructions assume that the webSNES project folder has been moved into the main emscripten 
installation folder (maybe not necessary) and that a command prompt has been opened within the 
project's "emscripten" sub-folder, and that the Emscripten environment vars have been previously 
set (run emsdk_env.bat).

The Web version is then built using the makeEmscripten.bat that can be found in this folder. The 
script will compile directly into the "emscripten/htdocs" example web folder, were it will create 
the backend_snes.js library. (To create a clean-build you have to delete any previously built libs in the 
'built' sub-folder!) The content of the "htdocs" can be tested by first copying it into some 
document folder of a web server. 


## Depencencies

The current version requires version 1.03 (older versions will not
support WebAssembly or may have problems skipping playlist entries) 
of my https://github.com/wothke/webaudio-player.

This project comes without any music files, so you'll also have to get your own and place them
in the htdocs/music folder (you can configure them in the 'songs' list in index.html).


## License

GNU LESSER GENERAL PUBLIC LICENSE Version 2.1, February 1999 (see separate license.txt in gme folder).


