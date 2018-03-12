
LibGmeBackendAdapter = (function(){ var $this = function () { 
		$this.base.call(this, Module, 2);	
		
		this.ref = Module.allocate(1, "i32", Module.ALLOC_STATIC);
		this.emu= -1;
		this.numSamples = 1024 * 8;
		this.numChannels = 2;
		
		this.hesHack = 32;	// bloody hack: there seems to be some bug in the HES emulation that causes irritating noise at buffer boundary.. 
		
		this.buffer = Module.allocate(this.numSamples * this.numChannels+this.hesHack, "i32", Module.ALLOC_STATIC);
		
		this.currentTrack =0;

	}; 
	// LibGme's sample buffer contains 2-byte integer sample data (i.e. must be rescaled) 
	// of 2 interleaved channels
	extend(EmsHEAP16BackendAdapter, $this, {  
		getAudioBuffer: function() {
			var ptr=  this.buffer;			
			// make it a this.Module.HEAP16 pointer
			return (ptr >> 1) +this.hesHack;	// 2 x 16 bit samples			
		},
		getAudioBufferLength: function() {
		//	var len= this.numSamples >>2;
			var len= this.numSamples;
			return len -(this.hesHack>>1);
		},
		computeAudioSamples: function() {
			// gme_play: Generate 'count' 16-bit signed samples info 'out'. Output is in stereo.

		
		
			var err = Module.ccall("gme_play", "number", ["number", "number", "number"], [this.emu, this.numSamples * this.numChannels, this.buffer]);

		
			if (Module.ccall("gme_track_ended", "number", ["number"], [this.emu]) == 1) {
				console.log("end of stream");
				return 1;
			}
			return 0;
		},
		getMaxPlaybackPosition: function() { 
			return 1; //this.Module.ccall('getXmpMaxPosition', 'number');
		},
		getPlaybackPosition: function() {
			return 0; //this.Module.ccall('getXmpCurrentPosition', 'number');
		},
		seekPlaybackPosition: function(pos) { 
			return 0; //this.Module.ccall('seekXmpPosition', 'number', ['number'], [pos]);
		},

		getPathAndFilename: function(filename) {
			return ['/', filename];
		},
		registerFileData: function(pathFilenameArray, data) {
			return 0;	// not used 
		},
		loadMusicData: function(sampleRate, path, filename, data) {
//			var buf = this.Module._malloc(data.length);
//			this.Module.HEAPU8.set(data, buf);
		
			// the libgme impl seems uses the int8array directly.. unlike my libs...
			if (Module.ccall("gme_open_data", "number", ["array", "number", "number", "number"], [data, data.length, this.ref, sampleRate]) != 0){
				console.error("gme_open_data failed.");
				return 1;
			}
			this.emu = Module.getValue(this.ref, "i32");
			
	//		this.Module._free(buf);

			return 0;			
		},
		evalTrackOptions: function(options) {
			if (typeof options.timeout != 'undefined') {
				ScriptNodePlayer.getInstance().setPlaybackTimeout(options.timeout*1000);
			}
			
			var track = (typeof options.track != 'undefined') ? options.track : 0
			
			if (Module.ccall("gme_start_track", "number", ["number", "number"], [this.emu, track]) != 0) {
				console.log("could not load track");
				return 1;
			} else {
				this.currentTrack= track;
				return 0;
			}
		},				
		teardown: function() {
			if ((this.emu != -1) && Module.ccall("gme_delete", "number", ["number"], [this.emu]) != 0)
				console.log("could not stop track");
		},
		getSongInfoMeta: function() {
			return {channels: Number,
					subtuneCount: Number,
					currentTrack: Number,
					author: String,
					game: String,
					copyright: String,
					comment: String
					};
		},
		parse_metadata: function(ref) {
			var offset = 0;

			var read_int32 = function() {
				var value = Module.getValue(ref + offset, "i32");
				offset += 4;
				return value;
			}

			var read_string = function() {
				var value = Module.Pointer_stringify(Module.getValue(ref + offset, "i8*"));
				offset += 4;
				return value;
			}

			var res = {};

			res.length = read_int32();
			res.intro_length = read_int32();
			res.loop_length = read_int32();
			res.play_length = read_int32();

			offset += 4*12; // skip unused bytes

			res.system = read_string();
			res.game = read_string();
			res.song = read_string();
			res.author = read_string();
			res.copyright = read_string();
			res.comment = read_string();

			return res;
		},

		updateSongInfo: function(filename, result) {
		
			var subtune_count = Module.ccall("gme_track_count", "number", ["number"], [this.emu]);

			Module.ccall("gme_ignore_silence", "number", ["number"], [this.emu, 1]);
			var voice_count = Module.ccall("gme_voice_count", "number", ["number"], [this.emu]);

			
			if (Module.ccall("gme_track_info", "number", ["number", "number", "number"], [this.emu, this.ref, this.currentTrack]) != 0)
			console.error("could not load metadata");

			var metadata = this.parse_metadata(Module.getValue(this.ref, "*"));

			result.channels= voice_count;
			result.subtuneCount= subtune_count;	
			result.currentTrack= this.currentTrack;
			result.author= metadata.author;
			result.game= metadata.game;
			result.copyright= metadata.copyright;
			result.comment= metadata.comment;
		}
	});	return $this; })();
	