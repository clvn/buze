#if !defined(LUNAR_PLUGIN_LOCALS_H) && !defined(LUNAR_STD_BUILD)
#error You must first include the header generated for the Lunar plugin before including fx.h.
#endif

#if !defined(LUNAR_FX_H)
#define LUNAR_FX_H

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(LUNAR_NO_FX_STDLIB)

// if your platform expects a different type
// for size_t, you need to pass a define
// at build time stating the platform,
// catch it here with an ifdef and have different 
// typedefs per platform.
#if defined(__X86_64__) || defined(CONFIG_X86_64) || defined(__APPLE__)
typedef long unsigned int lunar_size_t;
#else
typedef unsigned int lunar_size_t;
#endif

// stdlib
int lunar_printf(const char *format, ...);
int lunar_snprintf(char *buffer, int size, const char *format, ...);
void *lunar_memset(void *b, int c, lunar_size_t len);
void *lunar_memcpy(void *b, const void *a, lunar_size_t num);
void *lunar_calloc(lunar_size_t nelem, lunar_size_t elsize);
void *lunar_malloc(lunar_size_t len);
void lunar_free (void *);
void *lunar_realloc(void *ptr, lunar_size_t len);
// math
float lunar_pow(float, float);
float lunar_log(float);
float lunar_log10(float);
float lunar_exp(float);
float lunar_abs(float);
inline float lunar_min(float a, float b) { return (a < b)?a:b; }
inline float lunar_max(float a, float b) { return (a > b)?a:b; }
float lunar_sin(float);
float lunar_cos(float);
float lunar_tan(float);
float lunar_sinh(float);
float lunar_cosh(float);
float lunar_atan(float);
float lunar_sqrt(float);
float lunar_sqrt(float);
float lunar_fabs(float);
// new:
float lunar_floor(float);
float lunar_ceil(float);
long int lunar_lrint(float);

#if !defined(LUNAR_NO_FX_STDLIB_DEFINES)

#define M_PI 3.14159265358979323846


// stdlib
#define size_t lunar_size_t
#define printf lunar_printf
#define snprintf lunar_snprintf
#define memset lunar_memset
#define memcpy lunar_memcpy
#define calloc lunar_calloc
#define malloc lunar_malloc
#define free lunar_free
#define realloc lunar_realloc
// math
#define pow lunar_pow
#define log lunar_log
#define log10 lunar_log10
#define exp lunar_exp
#define abs lunar_abs
#define min lunar_min
#define max lunar_max
#define sin lunar_sin
#define cos lunar_cos
#define tan lunar_tan

#define sinh lunar_sinh
#define cosh lunar_cosh
#define atan lunar_atan
#define sqrt lunar_sqrt
#define fabs lunar_fabs
// new:
#define floor lunar_floor
#define ceil lunar_ceil
#define lrint lunar_lrint
#endif // LUNAR_NO_FX_STDLIB_DEFINES

#endif // LUNAR_NO_FX_STDLIB

typedef struct _lunar_transport {
	int beats_per_minute;
	int ticks_per_beat;
	int samples_per_second;
	float samples_per_tick;
	int tick_position;
	float ticks_per_second;
	int song_position;
} lunar_transport_t;

typedef struct _lunar_midi_message {
	int device;
	unsigned long timestamp;
	unsigned long message;
} lunar_midi_message_t;

typedef struct _lunar_note_message {
	int track;
	int note;
	int wave;
	int amp;
} lunar_note_message_t;

typedef struct _lunar_host lunar_host_t;
typedef struct _lunar_voice lunar_voice_t;

int lunar_is_envelope(lunar_host_t *, int instr, int envelope);
float lunar_get_envelope_sustain_position(lunar_host_t *, int instr, int envelope);
void lunar_get_envelope(lunar_host_t *, int instr, int envelope, float start, float size, float *buffer, int n);
lunar_voice_t *lunar_new_voice(lunar_host_t *);
void lunar_voice_set_instrument(lunar_voice_t *, int instr);
void lunar_voice_set_note(lunar_voice_t *, float note);
void lunar_voice_process_stereo(lunar_voice_t *, float *outL, float *outR, int n);
void lunar_voice_reset(lunar_voice_t *);
void lunar_request_gui_redraw(lunar_host_t *);
void lunar_midi_out(lunar_host_t *, lunar_midi_message_t*, int);
void lunar_note_out(lunar_host_t *, lunar_note_message_t*, int);
bool lunar_is_channel_connected(lunar_host_t *, int);

#if !defined(LUNAR_USE_LOCALS)
typedef void lunar_globals_t;
typedef void lunar_track_t;
typedef void lunar_controllers_t;
typedef void lunar_attributes_t;
#endif // !LUNAR_USE_LOCALS

struct lunar_fx {
	unsigned int size;
	lunar_transport_t *transport;
	lunar_host_t *host;
	lunar_globals_t *globals;
	lunar_track_t *tracks;
	int track_count;
	lunar_attributes_t *attributes;
 	lunar_controllers_t *controllers;
	void (*init)(lunar_fx *);
	void (*exit)(lunar_fx *);
	void (*process_events)(lunar_fx *);
	void (*process_controller_events)(lunar_fx *);
	void (*process_stereo)(lunar_fx *, float *, float *,float *, float *, int n);
	void (*process_timer)(lunar_fx *, int*, int*);
	void (*instrument_changed)(lunar_fx *, int instr);
	void (*transport_changed)(lunar_fx *);
	void (*tracks_changed)(lunar_fx *);
	void (*attributes_changed)(lunar_fx *);
	int (*get_latency)(lunar_fx *);
	void (*redraw_gui)(lunar_fx *);
	void (*resize_gui)(lunar_fx *, int width, int height);
	bool (*process_audio)(lunar_fx *, float **, float **, int n);
	void (*process_midi)(lunar_fx *, lunar_midi_message_t*, int n);
	const char* (*describe_value)(lunar_fx *, lunar_parameter_names_t, float);
};

inline void lunar_init_fx(lunar_fx *fx) {
	memset(fx, 0, sizeof(lunar_fx));
	fx->size = sizeof(lunar_fx);
}

typedef lunar_fx *(*lunar_new_fx_t)();

lunar_fx *new_fx();

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // LUNAR_FX_H
