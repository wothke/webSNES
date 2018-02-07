# websnes (Web plugin of libGME - see live demo: http://www.wothke.ch/websnes/)

In this case the WebAudio version of libGme had already been done by somebody else: http://onakasuita.org/jsgme/
	
This project just provides the adapter/glue code to use it with my generic WebAudio player (see separate project).

## License

Copyright (C) 2016 Juergen Wothke

This program (i.e web extensions of ASAP) is free software: you can 
redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


## Credits

* Original C code of "LibGME": see https://bitbucket.org/mpyne/game-music-emu/wiki/Home
* WebAudio version: http://onakasuita.org/jsgme/

## How to use

Copy the folder to the document root of some web server so you can browse the page using a WebAudio
enabled browser. 

The project does not contain any SNES music files - you'll have to get those elsewhere. You'll find 
a "add-your-music-here.spc" example entry within the playlist defined in index.html (you can likewise configure all
your music files that you copied to the 'music' sub-folder). 
