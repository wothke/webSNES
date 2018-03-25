/*
* This file adapts Game_Music_Emu to the interface expected by my generic JavaScript player.
*
* Copyright (C) 2018 Juergen Wothke
*
*
* Credits:   
* 
* The project is based on: https://bitbucket.org/mpyne/game-music-emu (by Shay Green, et al.)
* zlib (C) 1995-2005 Jean-loup Gailly and Mark Adler
*
* Notes:
* 
* Only a small subset of the emulators available in GME are actually activated/used here, i.e. AY, GYM & SPC support.
* The reason is that for the other systems I already have other better emulators and there is no point in
* bloating this lib with unused stuff (this is also true for AY but dependency wise it seems to be required
* anyway). I added support for packed GYM (see hack below).
*
* License:
*
* The GME plugin is licensed using GNU LESSER GENERAL PUBLIC LICENSE Version 2.1, February 1999 (see license.txt in gme folder)
*/

#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>     
#include <string.h> // memcpy

#include <iostream>
#include <fstream>

#include "gme/gme.h"
#include "Gym_Emu.h"

#include "zlib.h"

typedef  short Int16;

#ifdef EMSCRIPTEN
#define EMSCRIPTEN_KEEPALIVE __attribute__((used))
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

#define CHANNELS 2				
#define BYTES_PER_SAMPLE 2
#define SAMPLE_BUF_SIZE	1024
#define SAMPLE_FREQ	44100


Int16 sample_buffer[SAMPLE_BUF_SIZE * CHANNELS];
int samples_available= 0;

char* info_texts[7];


#define TEXT_MAX	255
char title_str[TEXT_MAX];
char artist_str[TEXT_MAX];
char game_str[TEXT_MAX];
char comment_str[TEXT_MAX];
char copyright_str[TEXT_MAX];
char dumper_str[TEXT_MAX];
char system_str[TEXT_MAX];

#define RAW_INFO_MAX	1024
char raw_info_buffer[RAW_INFO_MAX];

struct StaticBlock {
    StaticBlock(){
		info_texts[0]= title_str;
		info_texts[1]= artist_str;
		info_texts[2]= game_str;
		info_texts[3]= comment_str;
		info_texts[4]= copyright_str;
		info_texts[5]= dumper_str;
		info_texts[6]= system_str;
    }
};

static StaticBlock g_emscripen_info;


// now to the GME stuff:
Music_Emu* emu_;
gme_info_t* track_info_;

const char* handle_error( const char* str )
{
	if ( str ){
		fprintf(stderr,  "Error: %s\n", str );
	}
	return str;
}


extern "C" void emu_teardown (void)  __attribute__((noinline));
extern "C" void EMSCRIPTEN_KEEPALIVE emu_teardown (void) {
	
	title_str[0]= artist_str[0]= game_str[0]= 
		comment_str[0]= copyright_str[0]= 
		dumper_str[0]= system_str[0] = 0;
		
	gme_free_info( track_info_ ); track_info_= 0;

	gme_delete( emu_ ); emu_= 0;
}

int track_count()
{
	return emu_ ? gme_track_count( emu_ ) : 1;
}
bool track_ended()
{
	return emu_ ? gme_track_ended( emu_ ) : false;
}

int computeSamples() {
	/* Fill sample buffer */
	if (handle_error( gme_play( emu_, SAMPLE_BUF_SIZE, (Int16 *)sample_buffer ) ))
		return 1;
	
	if (track_ended()) {
		samples_available= 0;
		return 1;	
	}
	samples_available= SAMPLE_BUF_SIZE>>1;
	return 0;
}

unsigned int getLE(unsigned char *buf) {
	return buf[0]+(buf[1]<<8)+(buf[2]<<16)+(buf[3]<<24);
}

unsigned char * unpacked= 0;

extern "C"  int emu_load_file(char *filename, void * inBuffer, uint32_t inBufSize)  __attribute__((noinline));
extern "C"  int EMSCRIPTEN_KEEPALIVE emu_load_file(char *filename, void * inBuffer, uint32_t inBufSize) {
	emu_teardown();

	// LOL GME's GYM impl doesn't support packed files.. not much use without packed file support!
	if ( memcmp( inBuffer, "GYMX", 4 ) == 0 ) {	// seems to be easier to unpack here than to fix the emu
		// A 32-bit unsigned integer in little-endian format denoting how large the 
		//             remainder of the data is if it's GZipped;
		unsigned long unpackedSize= getLE((unsigned char*)&((Gym_Emu::header_t const*) inBuffer)->packed[0]);
		if (unpackedSize != 0 ) {	// Packed GYM file
			if (unpacked) free(unpacked);	// limit the garbage
			
			unpacked= (unsigned char *)malloc(Gym_Emu::header_size+unpackedSize);
			memcpy(unpacked, inBuffer, Gym_Emu::header_size);	// header
			*((unsigned int*)(((Gym_Emu::header_t const*) unpacked)->packed))= 0;	// mark as unpacked
			
			if (Z_OK != uncompress(unpacked+Gym_Emu::header_size, &unpackedSize, (const unsigned char*)inBuffer+Gym_Emu::header_size, inBufSize-Gym_Emu::header_size))
				fprintf(stderr, "ERROR uncompressing\n");
			
			inBuffer = (void*)unpacked;
			inBufSize= Gym_Emu::header_size+unpackedSize;
		}
	}
	
	if (handle_error(gme_open_data( inBuffer, inBufSize,&emu_, SAMPLE_FREQ  ) ))	
		return 1;
	return 0;					
}

extern "C" int emu_get_sample_rate() __attribute__((noinline));
extern "C" EMSCRIPTEN_KEEPALIVE int emu_get_sample_rate()
{
	return SAMPLE_FREQ;
}

int update_info(int track) {
	if (track_info_) gme_free_info( track_info_ );

	if (handle_error( gme_track_info( emu_, &track_info_, track ) ))
		return 1;
		
	if (handle_error( gme_start_track( emu_, track ) ))
		return 1;
	
	// Calculate track length
	if ( track_info_->length <= 0 )
		track_info_->length = track_info_->intro_length +
					track_info_->loop_length * 2;
	
	if ( track_info_->length <= 0 )
		track_info_->length = (long) (2.5 * 60 * 1000);
	gme_set_fade( emu_, track_info_->length );
	
	snprintf(title_str, TEXT_MAX, "%s", track_info_->song);	
	snprintf(artist_str, TEXT_MAX, "%s", track_info_->author);	
	snprintf(game_str, TEXT_MAX, "%s", track_info_->game);	
	snprintf(comment_str, TEXT_MAX, "%s", track_info_->comment);	
	snprintf(copyright_str, TEXT_MAX, "%s", track_info_->copyright);	
	snprintf(dumper_str, TEXT_MAX, "%s", track_info_->dumper);	
	snprintf(system_str, TEXT_MAX, "%s", track_info_->system);	
	
	return 0;
}

extern "C" int emu_set_subsong(int subsong, unsigned char boost) __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE emu_set_subsong(int track, unsigned char boost) {
	
	if (track < 0) track= 0;
	if (track >= track_count()) track=  track_count()-1;
	
	return update_info(track);
}

extern "C" const char** emu_get_track_info() __attribute__((noinline));
extern "C" const char** EMSCRIPTEN_KEEPALIVE emu_get_track_info() {
	return (const char**)info_texts;
}

extern "C" char* EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer(void) __attribute__((noinline));
extern "C" char* EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer(void) {
	return (char*)sample_buffer;
}

extern "C" long EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer_length(void) __attribute__((noinline));
extern "C" long EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer_length(void) {
	return samples_available;
}

extern "C" int emu_compute_audio_samples() __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE emu_compute_audio_samples() {
	return computeSamples();
}

extern "C" int emu_get_current_position() __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE emu_get_current_position() {
	return gme_tell( emu_);
}

extern "C" void emu_seek_position(int pos) __attribute__((noinline));
extern "C" void EMSCRIPTEN_KEEPALIVE emu_seek_position(int msec) {
	gme_seek( emu_, msec );
}

extern "C" int emu_get_max_position() __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE emu_get_max_position() {
	return track_info_->length;
}

