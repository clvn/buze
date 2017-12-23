from random import randint
import armstrong
import buze
import random
import sys
import os
from note_helper import to_note
from note_helper import from_chord

'''
A comprehensive collection of convenience methods.

This is a mere suggestion as to how you might use armstrong API. 
Your needs will inevitably vary, the functions are short enough 
to be digested rapidly by a newcomer. 

- deals with random and seed
- loads machines
- creates default pattern formats
- adds tracks to machines
- adds tracks to pattern formats
- loads samples
- connects machines
- fills patterns with data
- sets pattern length
- sets pattern resolution
- sets speed


'''




random.seed(5)
# assumption : that a master plugin ("Master")  is present in the document.
doc = mainframe.get_document()
player = doc.get_player()

# machine uris 
LUNAR_SYNTH = "@trac.zeitherrschaft.org/aldrin/lunar/generator/synth;1"
LUNAR_KICK = "@trac.zeitherrschaft.org/aldrin/lunar/generator/kick;1"
LUNAR_DELAY = "@trac.zeitherrschaft.org/aldrin/lunar/effect/delay;1"
MATILDE_2 = "@zzub.org/buzz2zzub/Matilde+Tracker2"
# FUNKY_VERB = "@zzub.org/buzz2zzub/Larsha+Funkyverb"

PATTERN_LENGTH = 64


def get_step_distance(skip_list):
    return skip_list[randint(0, len(skip_list)-1)]


def make_list(dicted_list):
    step_list = []
    for key, item in dicted_list.items():
        step_list += [key,]*item
    # print(step_list)
    return step_list


def get_skip_list(version):
    if version == 0:
        return make_list({0:3, 2:7, 4:5, 6:5, 8:2})
    if version == 1:
        return make_list({2:1, 4:6, 6:5, 8:2})
    if version == 2:
        return make_list({2:2, 4:5, 6:5, 8:2})
    if version == 3:
        return [2,4,6]


def get_tick_triggers():
    tick_triggers = []
    tick = 0

    while tick < PATTERN_LENGTH:
        if tick == 0:
            step = get_step_distance(get_skip_list(0))
            if step == 0:
                tick_triggers.append(tick)
                tick += step
                step = get_step_distance(get_skip_list(1))
            tick += step
            tick_triggers.append(tick)
        else:
            step = get_step_distance(get_skip_list(2))
            if tick+step < PATTERN_LENGTH:
                tick += step
                tick_triggers.append(tick)
            else:
                step = get_step_distance(get_skip_list(3))
                if tick+step < PATTERN_LENGTH:
                    tick += step
                    tick_triggers.append(tick)
                break

    return tick_triggers


def printInfo(format_name, plugin):
    num_gparams = plugin.get_parameter_count(1, 0)
    num_tparams = plugin.get_parameter_count(2, 0)
    print("Created: " + format_name)
    print("g_params: %d" % num_gparams)
    print("t_params (params in note track) %d" % num_tparams)


# take care of the machine loading and track count config
def create_machine(uri, position, tracks, name):
    loader = player.get_pluginloader_by_name(uri)
    newplug = player.create_plugin(None, 0, name, loader)
    newplug.set_position(*position)
    newplug.set_track_count(tracks)
    return newplug

  
# this handles the short notation "machine1 > machine2 > machine3" for chaining machines.
def connect_machines(chain):
    machines = chain.split(" > ")
    for machine in range(len(machines)-1):
        from_machine = player.get_plugin_by_name(machines[machine])
        to_machine = player.get_plugin_by_name(machines[machine+1])
        to_machine.create_audio_connection(from_machine, 0, 2, 0, 2)


def count_format_columns(format):
    format_iterator = format.get_iterator()
    column_counter = 0
    while format_iterator.valid():
        column_counter+=1
        format_iterator.next()
    format_iterator.destroy()
    return column_counter


def add_tracks_to_tparams(plugin, format, tracks_to_add):
    column_idx = count_format_columns(format)
    columns_per_tparams = plugin.get_parameter_count(2, 0)

    for i in range(tracks_to_add):
        for n in range(columns_per_tparams):
            format.add_column(plugin, group=2, track=1+i, column=n, idx=column_idx+n)
        column_idx+=columns_per_tparams
    return format


def add_track_from_group(format, plugin, group, track):
        column_idx = count_format_columns(format)
        num_params = plugin.get_parameter_count(group, track)
        for i in range(num_params):
            format.add_column(plugin, group, track, i, column_idx)
            column_idx += 1


def add_columns_to_format_from_plugin(format, plugin):
    # g_params
    add_track_from_group(format, plugin, 1, 0)
    # t_params
    num_tracks_in_tparams = plugin.get_track_count(2)
    for track in range(num_tracks_in_tparams):
        add_track_from_group(format, plugin, 2, track)


def create_simple_format_from_machine(plugin):
    plugin_name = plugin.get_name()
    format_name = "[default "+ plugin_name + "]"
    default_format = player.create_pattern_format(format_name)
    add_columns_to_format_from_plugin(default_format, plugin)

    printInfo(format_name, plugin)
    player.history_commit(0, 0, "Added Default %s Format" % format_name)
    return default_format


def add_subset_to_format_from_plugin(format, plugin, group, track, subset):
    '''
    format:     format to add to
    plugin:     plugin to add from
    group:      group to add from
    track:      track in group to add from
    subset:     list of 1 or more parameters to add.  f.ex [2,4,5,9] of lunar verb.
    '''
    column_idx = count_format_columns(format)
    for column in subset:
        format.add_column(plugin, group, track, column, column_idx)
        column_idx += 1


def create_new_pattern(pattern_format, pattern_name, pattern_length):
    new_pattern = player.create_pattern(pattern_format, pattern_name, pattern_length)
    player.history_commit(0, 0, "Added %s" % pattern_name)
    return new_pattern


# add_track_### and add_tracks_### would benefit from a rewrite.
def add_track_to_sequence_pattern(seq_plug, track, pattern_name):
    current_count = seq_plug.get_track_count(2)
    seq_plug.set_track_count(current_count + 1)

    seq_pat = player.get_pattern_by_name(pattern_name)
    seq_pat_format = seq_pat.get_format()

    # add_column(plugin, group, track, i, column_idx)
    current_count = seq_plug.get_track_count(2)
    seq_pat_format.add_column(seq_plug, 2, track, 0, current_count-1)


def add_tracks_to_sequence_pattern(num_tracks, pattern_name):
    seq_plug = player.get_plugin_by_name("Pattern")
    current_count = seq_plug.get_track_count(2)
    
    for i in range(num_tracks):
        track = current_count + i
        # print("add_track_to_sequence_pattern(track -> track= %d" % track)
        add_track_to_sequence_pattern(seq_plug, track, pattern_name)

    print('Tracks added: %d' % num_tracks)
    player.history_commit(0, 0, "Added %d tracks to Sequence Pattern 00" % num_tracks)


def between_locations(location1, location2):
    return ((location1[0]+location2[0])/2, (location1[1]+location2[1])/2)


def get_plugin_position(plugin_name):
    plugin = player.get_plugin_by_name(plugin_name)
    x = plugin.get_position_x()
    y = plugin.get_position_y()
    return (x,y)


def load_samples_from_bank(full_path, sample_list, start_index):
    '''
    full_path:      directory must exist, will return None early in case it doesn't 
    sample_list:    case sensitive, any incorrect file names will be treated the same as an empty string, skip index.
    start_index:    an int, within bounds 0 to 199 - (len(sample_list)) 
                    0 = wavetable index 01
    '''
    # check full path to see if it exists.
    if not os.path.isdir(full_path):
        print(full_path + ' appears invalid!')
        return None
    
    # sane and valid start_index?
    real_samples = 0
    if (start_index.__class__ == int 
        and start_index >= 0
        and start_index <= 199 - len(sample_list)):
        
        end_index = start_index + len(sample_list)
        sample_list_index = 0
        for i in range(start_index, end_index):
            sample_name = sample_list[sample_list_index]
            target = player.get_wave(i)
            file_and_path = os.path.join(full_path, sample_name)

            # invalid filename? we'll skip an index.
            if os.path.isfile(file_and_path): 
                doc.import_wave(file_and_path, target)
                real_samples += 1
            else:
                if sample_name in ('', ' ', '-', 'spacer', 'space'):
                    print('skipped a line - sample name implied spacer')
                else:
                    print(sample_name + 'was not found in '+ full_path)
                    
            sample_list_index += 1
    else:
        print('read the load_samples_from_bank description for valid input')
        return None
    print('imported %d samples' % real_samples)



    ''' S E T U P '''



# stored global parameters, could add everal states per list if modifying state during song.
delay_states = {'sweet_state' : [768, 4000, 2896, 4568], 
                        'progress_state' : [ ]}

# machine locations
synth_location = (-.25, -1)
kick_location = (-.5, 0)
sample_location = (-.25, 1)
master_location = get_plugin_position("Master")
delay_location = between_locations(synth_location, master_location)

# set song speed to tpb 8 / bpm 128
time_plug = player.get_plugin_by_name("Sequence")
time_plug.set_parameter_value(1,0,0,128,1)
time_plug.set_parameter_value(1,0,1,8,1)

# machine creation
synth = create_machine(LUNAR_SYNTH, synth_location, 6, "Synthline")
kicksynth = create_machine(LUNAR_KICK, kick_location, 1, "Kick")
matilde_one = create_machine(MATILDE_2, sample_location, 1, "Samples")
delay_one = create_machine(LUNAR_DELAY, delay_location, 1, "Delay_Syn")

# machine connecting
connect_machines("Synthline > Delay_Syn > Master")
connect_machines("Kick > Master")
connect_machines("Samples > Master")

# format creation
synthline_format = create_simple_format_from_machine(synth)
kick_format = create_simple_format_from_machine(kicksynth)
sample_format = create_simple_format_from_machine(matilde_one)

# Create the patterns
stabs_pattern = create_new_pattern(synthline_format, "Stabs", PATTERN_LENGTH)
kick_pattern = create_new_pattern(kick_format, "Kicks", PATTERN_LENGTH)
samples_pattern = create_new_pattern(sample_format, "Samples", PATTERN_LENGTH)

# Load a sample list
start_index = 0
full_path_to_bank = os.path.join(os.getcwd(), "Samples")
sample_list = ['BT7AADA.WAV', 'HHCD4.WAV', 'HHODA.WAV', 'RIDED0.WAV', '-',
                    'mth_hit_clipper.wav', 'mth_hat_wook.wav', '-', 'mth_bd _tuned.wav']
load_samples_from_bank(full_path_to_bank, sample_list, start_index)



''' F i l l i n g   P a t t e r n s '''



# fill the stab pattern
tick_list = get_tick_triggers()
synthID = synth.get_id()
for tick_event in range(PATTERN_LENGTH):
    if tick_event in tick_list:
        notes = from_chord(["C-3", "E-3", "B-3", "G-2"])
        for track, note in enumerate(notes):
            stabs_pattern.insert_value(synthID, 2, track, 0, tick_event, note, 0)
            stabs_pattern.insert_value(synthID, 2, track, 0, tick_event+1, to_note("off"), 0)
player.history_commit(0, 0, "Filled Stabs Pattern")


# fill the kick pattern
for tick_event in range(0, PATTERN_LENGTH, 8):
    kick_pattern.insert_value(kicksynth.get_id(), 2, 0, 0, tick_event, 1, 0)


# fill the samples pattern, // TODO maybe set active wave table slot instead..
sample_slot = 6 # 01
for tick_event in range(4, PATTERN_LENGTH, 8):
    samples_pattern.insert_value(matilde_one.get_id(), 2, 0, 0, tick_event, to_note("C-4"), 0)
    samples_pattern.insert_value(matilde_one.get_id(), 2, 0, 1, tick_event, sample_slot, 0)
    samples_pattern.insert_value(matilde_one.get_id(), 2, 0, 2, tick_event, 78, 0) # volume



''' S e t   E f f e c t   M a c h i n e s   s t a t e s '''



# set/init global parameters of delay_one only.
delay_machine_gparams = delay_one.get_parameter_count(1,0)
for param in range(delay_machine_gparams):
    value = delay_states['sweet_state'][param]
    delay_one.set_parameter_value(1, 0, param, value, 1)



''' S e t   G e n e r a t o r   s t a t e s ''' 



kick_params = [0, 9004, 0, 2500, 5847, 363, 5632, 250, 4200]
num_kick_params = kicksynth.get_parameter_count(1,0)
for param in range(num_kick_params):
    value = kick_params[param]
    kicksynth.set_parameter_value(1,0,param, value, 1)



''' A d d i n g   P a t t e r n s   t o   P a t t e r n   P l a y e r '''



add_tracks_to_sequence_pattern(5, "00")

# add stabs to the first track of the sequence_pattern, assumes presence of one track.
seq_plug = player.get_plugin_by_name("Pattern")
sequence_pattern = player.get_pattern_by_name("00")
sequence_pattern.insert_value(seq_plug.get_id(), 2, 0, 0, 0, stabs_pattern.get_id(), 0)
sequence_pattern.set_row_count(128)
sequence_pattern.set_display_resolution(16)
player.history_commit(0, 0, "Added Stabs Pattern to Sequence Pattern 00")

# add lunarkick track to Sequence
# insert_value(self, pluginid, group, track, column, time, value, meta):
sequence_pattern.insert_value(seq_plug.get_id(), 2, 1, 0, 0, kick_pattern.get_id(), 0)
player.history_commit(0, 0, "Added Kick Pattern to Sequence Pattern 00")

# add samples (hihat) to Sequence
sequence_pattern.insert_value(seq_plug.get_id(), 2, 2, 0, 0, samples_pattern.get_id(), 0)
player.history_commit(0, 0, "Added Hihats sample Pattern to Sequence Pattern 00")