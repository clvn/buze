#pragma once

namespace zzub {

struct multithreadworker;

// commit_event = sent as part of the "barrier" audio event

enum commit_event_type {
	commit_event_set_parameter,
	commit_event_process_events,
	commit_event_set_state_format,
	commit_event_set_loop_info,
	commit_event_set_graph,
	commit_event_insert_connection,
	commit_event_delete_connection,
	commit_event_set_audio_connection,
	commit_event_set_attribute,
	commit_event_set_mutestate,
	commit_event_insert_patternevent,
	commit_event_remove_patternevent,
	commit_event_update_patternevent,
	commit_event_insert_plugin,
	commit_event_delete_plugin,
	commit_event_set_tpb,
	commit_event_set_bpm,
	commit_event_set_swing,
	commit_event_set_swing_ticks,
	commit_event_orderlist_timeshift,
	commit_event_type_set_plugin_latency,
	commit_event_type_set_plugin_stream_source,
};

struct commit_event_data {
	struct commit_event_data_set_parameter {
		int pluginid;
		int group, track, column, value;
		bool record;
	};

	struct commit_event_data_set_state_format {
		metaplugin* oldplugin;
		metaplugin* newplugin;
	};

	struct commit_event_data_set_graph {
		std::vector<metaplugin*>* plugins;
		std::vector<metaconnection*>* backedges;
	};

	struct commit_event_data_insert_connection {
		metaconnection* conn;
		metaplugin* to_plugin;
		metaplugin* from_plugin;
	};

	struct commit_event_data_delete_connection {
		metaconnection* conn;
		metaplugin* to_plugin;
		metaplugin* from_plugin;
	};

	struct commit_event_data_process_events {
		metaplugin* plugin;
	};

	struct commit_event_data_set_audio_connection {
		int connid;
	};

	struct commit_event_data_set_attribute {
		metaplugin* plugin;
		int index;
		int value;
	};

	struct commit_event_data_set_mutestate {
		std::map<mutetrackkeytype, int>* mutestate;
	};

	struct commit_event_data_insert_patternevent {
		int patternid;
		pattern_event* ev;
	};

	struct commit_event_data_remove_patternevent {
		int patternid;
		int id;
		pattern_event* ev;
	};

	struct commit_event_data_update_patternevent {
		int patternid;
		int id;
		int time;
		int value;
		int pluginid;
		int group;
		int track;
		int column;
	};

	struct commit_event_data_insert_plugin {
		zzub::metaplugin* plugin;
	};

	struct commit_event_data_delete_plugin {
		zzub::metaplugin* plugin;
	};

	struct commit_event_data_set_tpb {
		int tpb;
	};

	struct commit_event_data_set_bpm {
		int bpm;
	};

	struct commit_event_data_set_swing {
		float swing_amount;
	};

	struct commit_event_data_set_swing_ticks {
		int swing_ticks;
	};

	struct commit_event_data_orderlist_timeshift {
		int song_id;
		int index;
		int timeshift;
	};

	struct commit_event_data_set_plugin_latency {
		int plugin_id;
		int latency;
	};

	struct commit_event_data_set_plugin_stream_source {
		metaplugin* plugin;
		std::string* source;
	};

	commit_event_type type;
	union {
		commit_event_data_set_parameter set_parameter;
		commit_event_data_set_state_format set_state_format;
		commit_event_data_set_graph set_graph;
		commit_event_data_insert_connection insert_connection;
		commit_event_data_delete_connection delete_connection;
		commit_event_data_process_events process_events;
		commit_event_data_set_audio_connection set_audio_connection;
		commit_event_data_set_attribute set_attribute;
		commit_event_data_set_mutestate set_mutestate;
		commit_event_data_insert_patternevent insert_patternevent;
		commit_event_data_remove_patternevent remove_patternevent;
		commit_event_data_update_patternevent update_patternevent;
		commit_event_data_insert_plugin insert_plugin;
		commit_event_data_delete_plugin delete_plugin;
		commit_event_data_set_tpb set_tpb;
		commit_event_data_set_bpm set_bpm;
		commit_event_data_set_swing set_swing;
		commit_event_data_set_swing_ticks set_swing_ticks;
		commit_event_data_orderlist_timeshift orderlist_timeshift;
		commit_event_data_set_plugin_latency set_plugin_latency;
		commit_event_data_set_plugin_stream_source set_plugin_stream_source;
	};
};

// audio_event = events sent from user thread to audio thread
enum audio_event_type {
	audio_event_type_set_state,
	audio_event_type_play_note,
	audio_event_type_set_position,
	audio_event_type_set_midi_plugin,
	audio_event_type_alter_parameter,
	audio_event_type_barrier,
	audio_event_type_process_events,
	audio_event_type_samplerate_changed,
	audio_event_type_play_pattern,
	audio_event_type_queue_index,
	audio_event_type_order_position,
	audio_event_type_midi_event,
	audio_event_type_note_event,
	audio_event_type_set_thread_count,
};

struct audio_event_data {
	struct audio_event_data_state_change {
		player_state state;
		int stop_row;
	};

	struct audio_event_data_play_note {
		metaplugin* plugin;
		int note, prevnote, velocity;
	};

	struct audio_event_data_set_position {
		int orderindex;
		int position;
	};

	struct audio_event_data_set_midi_plugin {
		int pluginid;
	};

	struct audio_event_data_alter_parameter {
		int pluginid;
		int group, track, column, value;
		bool record;
	};

	struct audio_event_data_barrier {
		std::vector<commit_event_data>* commit_events;
	};

	struct audio_event_data_process_events {
		metaplugin* plugin;
	};

	struct audio_event_data_samplerate_changed {
		int samplerate;
	};

	struct audio_event_data_play_pattern {
		int pattern_id;
		int row;
		int stoprow;
	};

	struct audio_event_data_queue_index {
		int index;
	};

	struct audio_event_data_order_position {
		int index;
	};

	struct audio_event_data_midi_event {
		int plugin_id;
		zzub::midi_message message;
	};

	struct audio_event_data_note_event {
		int plugin_id;
		zzub::note_message message;
	};

	struct audio_event_data_set_thread_count {
		int thread_count;
	};

	audio_event_type type;

	union {
		audio_event_data_state_change state_change;
		audio_event_data_play_note play_note;
		audio_event_data_set_position set_position;
		audio_event_data_set_midi_plugin set_midi_plugin;
		audio_event_data_alter_parameter alter_parameter;
		audio_event_data_barrier barrier;
		audio_event_data_process_events process_events;
		audio_event_data_samplerate_changed samplerate_changed;
		audio_event_data_play_pattern play_pattern;
		audio_event_data_queue_index queue_index;
		audio_event_data_order_position order_position;
		audio_event_data_midi_event midi_event;
		audio_event_data_note_event note_event;
		audio_event_data_set_thread_count set_thread_count;
	};
};

// user_event = events sent from audio thread to user thread
enum user_event_type {
	user_event_type_parameter_change,
	user_event_type_state_change,
	user_event_type_midi_control,
	user_event_type_committed,
	user_event_type_post_barrier,
	user_event_type_order_change,
	user_event_type_order_queue_change,
	user_event_type_tempo_change,
	user_event_type_reset_samples,
	user_event_type_append_samples,
	user_event_type_threads_change,
	user_event_type_infinite_pattern_recursion,
	user_event_type_samplerate_changed,
	user_event_type_latency_changed,
	user_event_type_device_reset,
};

struct user_event_data {
	struct user_event_data_parameter_change {
		int id, group, track, column, value, automation_pattern, automation_timestamp;
	};
	struct user_event_data_state_change {
		player_state state;
	};
	struct user_event_data_midi_control {
		unsigned char status, data1, data2;
	};
	struct user_event_data_committed {
		commit_event_data data;
	};
	struct user_event_data_order_change {
		int orderindex;
	};
	struct user_event_data_tempo_change {
		int bpm, tpb;
		float swing;
		int swing_ticks;
	};
	struct user_event_data_post_barrier {
		std::vector<commit_event_data>* commit_events;
	};
	struct user_event_data_reset_samples {
		int wave;
		int wavelevel;
		int format;
		int channels;
	};
	struct user_event_data_append_samples {
		struct chunk {
			std::vector<boost::shared_array<float> > buffer;
			int numsamples;
		};
		int wave;
		int wavelevel;
		std::vector<chunk>* buffer;
		std::vector<int>* slices;
		int numsamples;
	};
	struct user_event_data_threads_change {
		int thread_count;
		std::vector<boost::shared_ptr<multithreadworker> >* old_threads;
	};

	struct user_event_data_samplerate_changed {
		int samplerate;
	};

	user_event_type type;
	union {
		user_event_data_parameter_change parameter_change;
		user_event_data_state_change state_change;
		user_event_data_midi_control midi_control;
		user_event_data_committed committed;
		user_event_data_order_change order_change;
		user_event_data_tempo_change tempo_change;
		user_event_data_post_barrier post_barrier;
		user_event_data_reset_samples reset_samples;
		user_event_data_append_samples append_samples;
		user_event_data_threads_change threads_change;
		user_event_data_samplerate_changed samplerate_changed;
	};
};

struct user_event_listener {
	virtual bool user_event(user_event_data& data) = 0;
};

}
