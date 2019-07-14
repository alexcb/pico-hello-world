#include <mpg123.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include <dirent.h>

#include "math.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <pulse/simple.h>
#include <pulse/error.h>
#include <stdio.h>

#include <picoapi.h>


//#define RATE 44100
#define RATE 16000

size_t synth_text( pico_Engine *engine, const char *s, char *buf, int max_samples )
{
	pico_Status status;
	pico_Int16 textLeft = strlen(s) + 1;
	pico_Int16 textRead = 0;
	pico_Int16 sum = 0;

	while(textLeft != sum){
		pico_putTextUtf8( *engine, (const pico_Char*)s, textLeft, &textRead );
		sum += textRead;
	}

	size_t total_bytes = 0;
	pico_Int16 n;
	pico_Int16 type;
	while( 1 ) {
		status = pico_getData( *engine, (void*)buf, max_samples/2, &n, &type);
		buf += n;
		total_bytes += n;
		printf("got %d %d %d %d\n", max_samples, status, n, PICO_STEP_BUSY);

		if( status == PICO_STEP_IDLE ) {
			break;
		}
	}
	return total_bytes;
}

#define MEM_SIZE 2500000
#define MAX_BUFF_SIZE 10240

int main(int argc, char *argv[])
{
	int char_start;
	int char_end;

	if( argc == 1 ) {
		char_start = 0;
		char_end = 37;
	} else if( argc == 3  ) {
		char_start = atoi(argv[1]);
		char_end = atoi(argv[2]);
	} else {
		fprintf(stderr, "usage: %s <char start> <char end>\n", argv[0]);
		return 1;
	}

	if( char_end <= char_start || char_end < 0 || char_end > 37) {
		fprintf(stderr, "<char start> must be less than <char end> (from 0 to 37)\n");
		return 1;
	}

	pico_System sys;
	pico_Engine engine;
	char ta_resource_name[1024];
	char sg_resource_name[1024];
	
	pico_Resource ta_resource;
	pico_Resource sg_resource;

	const char *voice_name = "English";
	const char *ta_path = "/usr/share/pico/lang/en-US_ta.bin";
	const char *sg_path = "/usr/share/pico/lang/en-US_lh0_sg.bin";

	void *mem_space = (void*)malloc(MEM_SIZE);
	assert( pico_initialize( mem_space, MEM_SIZE, &sys ) == PICO_OK );
	assert( pico_loadResource( sys, (const pico_Char *) ta_path, &ta_resource) == PICO_OK );
	assert( pico_loadResource( sys, (const pico_Char *) sg_path, &sg_resource) == PICO_OK );
	assert( pico_createVoiceDefinition( sys, (const pico_Char*) voice_name ) == PICO_OK );
	assert( pico_getResourceName( sys, ta_resource, (char *)ta_resource_name ) == PICO_OK );
	assert( pico_getResourceName( sys, sg_resource, (char *)sg_resource_name ) == PICO_OK );
	assert( pico_addResourceToVoiceDefinition( sys, (const pico_Char*) voice_name, (const pico_Char *) &ta_resource_name) == PICO_OK );
	assert( pico_addResourceToVoiceDefinition( sys, (const pico_Char*) voice_name, (const pico_Char *) &sg_resource_name) == PICO_OK );
	assert( pico_newEngine( sys, (const pico_Char*) voice_name, &engine) == PICO_OK );

	int res;
	int error;

	static const pa_sample_spec ss = {
		.format = PA_SAMPLE_S16LE,
		.rate = RATE,
		.channels = 1
	};

	pa_simple *pa_handle;
	pa_handle = pa_simple_new(NULL, "alexplayer", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error);
	if( pa_handle == NULL ) {
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		assert( 0 );
	}

	int max_seconds = 10;
	size_t max_samples = RATE * max_seconds;
	size_t buf_size = max_samples * 2;

	size_t buf_len;
	char *buf = malloc( buf_size );

	while(1) {
		char s[1024];
		sprintf(s, "this is a test of the new voice system for the music player");
		buf_len = synth_text( &engine, s, buf, max_samples );
		assert( buf_len > 0 );

		if( buf_len > 0 ) {
			res = pa_simple_write( pa_handle, buf, buf_len, &error );
			assert( res == 0 );

			// drain
			res = pa_simple_drain( pa_handle, &error );
			assert( res == 0 );
		}
	}
	return 0;
}
