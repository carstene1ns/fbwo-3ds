#ifndef AUDIO_H
#define AUDIO_H

extern Music music;

#include <stdio.h>
#include <tremor/ivorbisfile.h>

u8 audio_init(const char*);
void audio_music_load();
void audio_music_play();
void audio_music_pause();
void audio_music_check();
void audio_fini();
void looped_fread(u32 bytes_to_read, u32 start_point, u32 data_size, FILE* fp, u8* buffer); 
void looped_vorbis_read(OggVorbis_File *vf, u8* buffer, u32 length);

#endif //AUDIO_H
