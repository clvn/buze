/***

mixer stuff

the mixer implements methods for non-blocking and non-waiting communication
with the audio thread.

the mixer uses three ringbuffers for message passing between the threads:

	- user_event_queue - for audio->user thread events
	- audio_event_queue -> for immediate user->audio thread events
	- commit_event_queue -> for delayed user->audio thread events

call mixer::process_user_event_queue() to poll the user_event_queue.
this will result in calls to user_event_listener::user_event() on objects
registered in mixer::user_listeners. (i.e armstrong::frontend::player)

the audio_event_queue is used to pass state changes that should be executed
immediately. such as play/stop/resume, seeking, note playing and parameter 
changes that bypasses the undo buffer. examples of methods in the mixer class
that use the audio_event_queue are set_play_position(), alter_parameter(), 
set_state() and barrier().

call mixer::barrier() to write the changes to the running graph and invoke 
events in the commit-queue. the commit-queue is used to perform mixer state 
changes as part of a flushing operation. compare e.g mixer::set_parameter()
with mixer::alter_parameter(): set_parameter() does not change the parameter 
until commit() is called, but alter_parameter() changes the parameter as soon
as possible.

***/

#pragma once

#pragma warning(push)
#pragma warning(disable:4311) // warning C4311: 'type cast' : pointer truncation from 'void *' to 'long'
#pragma warning(disable:4312) // warning C4312: 'type cast' : conversion from 'long' to 'void *' of greater size
#pragma warning(disable:4244) // warning C4244: 'argument' : conversion from 'intptr_t' to 'long', possible loss of data
#pragma warning(disable:4267) // warning C4267: 'argument' : conversion from 'size_t' to 'unsigned int', possible loss of data
#include <boost/shared_ptr.hpp>
#include <boost/lockfree/queue.hpp>
#pragma warning (pop)
#include <deque>
#include <map>
#include "thread_id.h"
#include "song.h"
#include "timer.h"
#include "synchronization.h"
#include "patternplayer.h"
#include "events.h"
#include "ringbuffer.h"
#include "allocpool.h"
#include "encoder.h"
#include "threadqueue.h"

int convert_sample_count(int samples, int from_rate, int to_rate);

namespace zzub {

struct patternplayer;

struct newplugindata {
	int id;
	int timesourceid;
	std::vector<unsigned char> data;
	bool initialized;
};

struct find_new_plugin : public std::unary_function<const newplugindata&, bool> {
	int id;
	find_new_plugin(int _id) {
		id = _id;
	}
	bool operator()(const newplugindata& param) {
		return param.id == id;
	}
};

struct find_new_timeplugin : public std::unary_function<const newplugindata&, bool> {
	int id;
	find_new_timeplugin(int _id) {
		id = _id;
	}
	bool operator()(const newplugindata& param) {
		return param.timesourceid == id;
	}
};

struct multithreadworker {
	struct mixer* mixer;
	//HANDLE hThread;
	thread_handle_t thread;
	synchronization::event* start_event;
	int sample_count;
	volatile bool quitting;
	boost::atomic<bool> sleeping;
};

struct mixer {
	enum {
		max_channels = 64,
		max_threadqueue = 128
	};

	std::vector<metaplugin*> workorder;
	int work_position;								// total accumulation of samples processed, used by feedback buffer
	int song_position;								// total accumulation of samples processed while state == player_state_play
	zzub::timer timer;								// hires timer, for cpu-meter

	threadqueuearray<song, max_threadqueue> songinfo;
	threadqueuearray<metaplugin, max_threadqueue> plugins;
	threadqueuearray<metaconnection, max_threadqueue> connections;
	threadqueuearray<midimapping, max_threadqueue> midimappings;
	threadqueuearray<pattern, max_threadqueue> patterns;
	threadqueuearray<pattern_events, max_threadqueue> patternevents;
	threadqueuearray<patternformattrack, max_threadqueue> patternformattracks;
	threadqueuearray<patternformatcolumn, max_threadqueue> patterncolumns;
	threadqueuearray<patternformat, max_threadqueue> patternformats;
	threadqueuearray<wave_info, max_threadqueue> waves;
	threadqueuearray<wave_level, max_threadqueue> wavelevels;
	threadqueuearray<patternplayer, max_threadqueue> patternplayers;
	threadqueuearray<multithreadworker, max_threadqueue> threadworkers;
	patternplayer* rootplayer;

	bool is_recording_parameters;
	bool is_syncing_midi_transport;
	int midi_plugin;
	keyjazz playnotes;
	player_state state;

	// event queues
	boost::lockfree::spsc_queue<user_event_data, boost::lockfree::capacity<4096> > user_event_queue;
	boost::lockfree::spsc_queue<audio_event_data, boost::lockfree::capacity<1024> > audio_event_queue;
	boost::lockfree::spsc_queue<user_event_data, boost::lockfree::capacity<1024> > encoder_user_event_queue;
	std::vector<commit_event_data>* commit_events;

	// recipients of mixer events
	std::vector<user_event_listener*> user_listeners;

	// internals, stats, buffers
	float* input_buffer[max_channels];
	int input_channel_count;
	float* output_buffer[max_channels];
	int output_buffer_count[max_channels];
	int output_master_channel;
	int output_channel_count;

	midi_message* input_midi_messages;
	int input_midi_message_count;
	std::vector<std::pair<int, std::string> > output_midi_device_names;
	midi_message* output_midi_messages;
	int output_midi_message_count;
	int output_midi_message_maxcount;

	thread_id_t user_thread_id;						// thread id as returned by GetCurrentThreadId or pthread_self
	bool dirty_graph;
	bool dirty_mutestate;
	bool dirty_latencies; // handled in the audio-thread

	// refs to swapped-out objects for gc collecting
	std::deque<std::vector<boost::shared_array<short> > > buffer_garbage;
	std::deque<std::vector<zzub::plugin*> > userplugin_garbage;

	std::vector<newplugindata> new_plugins;
	bool lock_mode; // false: next commit is wait+lock free, true: next commit blocks the audio thread
	synchronization::critical_section audiomutex;
	host_info hostinfo;

	int buffer_size;
	int buffer_position;
	float falloff; // set when samplerate changes
	int latency_buffer_position;

	std::map<mutetrackkeytype, int> mutestate; // int = column_id. existence of a key means muting
	allocpool encoderbuffers;
	encoder encodermgr;

	int orderlist_position;
	int queue_index;

	int thread_count;
	boost::lockfree::queue<metaplugin*, boost::lockfree::capacity<2048> > thread_workqueue;
	boost::atomic<int> thread_sleep_count; 
	synchronization::event thread_done_event;

	mixer();
	~mixer();

	// audio thread methods
	int generate_audio(float** pin, float** pout, int sample_count, zzub::midi_message* pmidiin, int inmidi_count, zzub::midi_message* pmidiout, int* outmidi_count, int outmidi_maxcount);
	void process_audio_event_queue();	// called from audio thread
	bool process_sequencer(int& min_sequence_chunk, bool offline);
	void process_sequencer_events();
	void process_seek(int row);
	void process_midi_bindings(zzub::midi_message* pmidi, int midi_count);
	void on_play_note(metaplugin* plugin, int note, int prevnote, int velocity);
	void on_set_state(player_state newstate, int stoprow = -1);
	void on_set_position(int orderindex, int position);
	void on_set_midi_plugin(int pluginid);
	void on_set_parameter(int pluginid, int group, int track, int column, int value, bool record);
	void on_set_state_format(metaplugin* oldplugin, metaplugin* newplugin);
	void on_insert_connection(metaconnection* conn, metaplugin* to_plugin, metaplugin* from_plugin);
	void on_delete_connection(metaconnection* conn, metaplugin* to_plugin, metaplugin* from_plugin);
	void on_process_events(metaplugin* plugin);
	bool on_process_internal_events(zzub::metaplugin* k);
	bool on_process_interpolators(int group, zzub::metaplugin* k);
	void on_advance_interpolators(int group, zzub::metaplugin* k, int sample_count);
	void on_barrier(std::vector<commit_event_data>* events);
	void on_parameter_change(int pluginid, int group, int track, int column, int value, bool automate);
	void on_set_master(int pluginid);
	void on_set_samplerate(int samplerate);
	void on_update_audioconnection(int id);
	void on_set_attribute(metaplugin* plugin, int index, int value);
	void on_play_pattern(int pattern_id, int row, int stoprow);
	void on_insert_plugin(metaplugin* plugin);
	void on_delete_plugin(metaplugin* plugin);
	void process_graph(int num_samples);
	void process_stereo(metaplugin* mixplugin, int sample_count);
	void process_stereo_chunk(metaplugin* k, int offset, float** plin, float** plout, int sample_count, int flags);
	void process_midi(metaplugin* mixplugin, int sample_count);
	void process_encoder(metaplugin* mixplugin, int sample_count);
	void process_encoder_buffer(metaplugin* mixplugin);
	void process_parameter_changes(metaplugin* k, int g);
	bool format_has_column(int formatid, int pluginid, int group, int track, int column);
	bool get_currently_playing_pattern_row_for_column(int pluginid, int group, int track, int column, int* pattern_id, int* pattern_row);
	void process_graph_events();
	void process_controllers();
	void process_sequence_events(bool& reprocess);
	void process_interval_plugins();
	void update_interval_plugins();
	void process_graph_multithreaded_worker(multithreadworker* worker);
	void process_graph_multithreaded(int sample_count);
	void process_graph_singlethreaded(int sample_count);
	bool write_output(int channel, int write_offset, float* samples, int numsamples, float amp);
	int write_midi(midi_message* messages, int midi_count);

	void reset_interval_plugins();
	void apply_interval_size(int& min_sequence_chunk);
	void apply_interval_chunk_size(int chunksize);
	void on_tempo_changed(patternplayer* player, int timepluginid, int timegroup, int timetrack);
	void on_update_plugin_timesource(metaplugin* plugin, bool notifyplugin);
	void end_of_pattern(patternplayer* player, bool offline);
	void end_of_loop(patternplayer* player, bool offline, int beginloop);
	void set_orderlist_index(int index);
	void on_insert_patternevent(std::vector<commit_event_data>::iterator i);
	void on_remove_patternevent(std::vector<commit_event_data>::iterator i);
	void on_update_patternevent(std::vector<commit_event_data>::iterator i);
	void update_latency_compensation();
	int get_max_latency(metaplugin* k);
	int set_latencies(metaplugin* k, int max_latency);
	void on_sync_orderlist_timeshift(int index, int timeshift);
	void on_set_thread_count(int threads);
	void set_plugin_sequence_chain(zzub::metaplugin* k, bool in_sequence_chain);

	// user thread methods
	void insert_song();
	void update_song(int loop_begin, int loop_end, bool loop_enabled, const std::vector<int>& order_patterns);
	void insert_plugin(int id, zzub::plugin* userplugin, zzub::info* loader, std::vector<unsigned char>& data, std::string name, int num_tracks, std::string stream_source, bool is_muted, bool is_bypassed, int timesourceid, int timesourcegroup, int timesourcetrack);
	void update_plugin(int id, std::string name, int num_tracks, std::string stream_source, bool is_muted, bool is_bypassed, int timesourceid, int timesourcegroup, int timesourcetrack);
	void delete_plugin(int id);
	void insert_connection(int id, int plugin_id, int from_plugin_id, int to_plugin_id, zzub_connection_type type, int first_input, int first_output, int inputs, int outputs, std::string mididevice);
	void delete_connection(int id);
	void update_audioconnection(int id, int first_input, int first_output, int inputs, int outputs, int flags);
	void update_midiconnection(int id, std::string device);
	void add_event_connection_binding(int connid, int sourceparam, int group, int track, int column);
	void delete_event_connection_binding(int connid, int sourceparam, int group, int track, int column);
	void insert_pattern(int id, int formatid, std::string name, int length, int resolution, int beginloop, int endloop, bool loopenabled);
	void update_pattern(int id, int formatid, std::string name, int length, int resolution, int beginloop, int endloop, bool loopenabled);
	void delete_pattern(int id);
	void insert_patternformat(int id);
	void delete_patternformat(int id);
	void insert_patternformatcolumn(int id, int formatid, int pluginid, int group, int track, int column);
	void delete_patternformatcolumn(int id);
	void insert_patternformattrack(int id, int formatid, int pluginid, int group, int track, int is_muted);
	void update_patternformattrack(int id, int formatid, int pluginid, int group, int track, int is_muted);
	void delete_patternformattrack(int id);
	void insert_wave(int id, int flags, float volume);
	void update_wave(int id, int flags, float volume);
	void delete_wave(int id);
	void insert_wavelevel(int id, int waveid, int beginloop, int endloop, int numsamples, int samplerate, zzub_wave_buffer_type format, int basenote, const std::vector<int>& slices);
	void update_wavelevel(int id, int waveid, int beginloop, int endloop, int numsamples, int samplerate, zzub_wave_buffer_type format, int basenote, const std::vector<int>& slices);
	void delete_wavelevel(int id);
	void insert_midimapping(int id, int pluginid, int group, int track, int column, int midichannel, int midicc);
	void delete_midimapping(int id);
	void set_wavelevel_samples(int id, unsigned char* ptr, int samplecount, bool legacyheader);
	void insert_pattern_value(int id, int patternid, int pluginid, int group, int track, int column, int time, int value);
	void remove_pattern_value(int id, int patternid);
	void update_pattern_value(int id, int patternid, int pluginid, int group, int track, int column, int time, int value);
	void play_plugin_note(int plugin_id, int note, int prevNote, int velocity);
	void reset_keyjazz();
	void set_state(player_state newstate, int stoprow = -1);
	void set_play_position(int orderindex, int position);
	void set_midi_plugin(int pluginid);
	void process_user_event_queue();	// call from user thread
	void set_parameter(int id, int group, int track, int column, int value, bool record);
	int get_parameter(int id, int group, int track, int column);
	void alter_parameter(int id, int group, int track, int column, int value, bool record);
	void set_parameter_check(int id, int group, int track, int column, int value, bool record);
	void set_parameter_mode(int id, int group, int track, int column, int mode);
	void process_events(int plugin_id, bool immediate_mode);
	int get_currently_playing_row(int patternid);
	void set_master(int pluginid);
	void set_samplerate(int samplerate);
	void set_attribute(int plugin_id, int index, int value);
	void play_pattern(int pattern_id, int row, int stoprow);
	void commit();
	void add_user_listener(user_event_listener* listener);
	void remove_user_listener(user_event_listener* listener);
	void set_tpb(int tpb);
	void set_bpm(int bpm);
	void set_swing(float swing);
	void set_swing_ticks(int swingticks);
	void set_queue_index(int pos);
	void set_order_position(int pos);
	void sync_orderlist_timeshift(int index, int timeshift);
	void set_thread_count(int threads);
	void finalize_thread_count(user_event_data::user_event_data_threads_change& ev);
	void set_plugin_latency(int plugin_id, int samplecount);
	void on_set_plugin_latency(int plugin_id, int samplecount);

	void initialize_new_plugins();
	void initialize_plugin(int id, std::vector<unsigned char>& data);
	bool in_user_thread();
	bool in_encoder_thread();

	void note_out(int from_plugin_id, const zzub::note_message& message);
	void on_note_out(int from_plugin_id, const zzub::note_message& message);

	void midi_out(int from_plugin_id, const zzub::midi_message& message);
	void on_midi_out(int from_plugin_id, const zzub::midi_message& message);

	bool is_input_channel_connected(int to_plugin_id, int index);

	// user thread internals
	bool invoke_audio_event(audio_event_data& data); // call from user thread
	bool invoke_user_event(user_event_data& data); // called from user thread = immediate, from audio thread = queued, from encoder thread = queued
	bool notify_user_event_listeners(user_event_data& data);
	bool invoke_commit_event(commit_event_data& data); // call from user thread
	void invalidate_state_format(int pluginid);
	void invalidate_graph();
	void invalidate_mutestates();
	void update_legacy_wavelevel(wave_level* wavelevel);
	void finalize_commit_event(commit_event_data& data);
	void commit_garbage();
	void finalize_garbage();
	void set_plugin_timesource(patternplayer* pp, int pluginid, int group, int track);
	patternplayer* create_patternplayer(int samplerate, int bpm, int tpb, float swing, int swingticks);
	void destroy_patternplayer(patternplayer* pp);
	bool reset_pending_connection_commit_events(int connid);
	void reset_pending_plugin_commit_events(int pluginid);
	void reset_pending_patternevent_commit_events(int patternid);
};

}
