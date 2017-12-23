import armstrong
import buze
import os

doc = mainframe.get_document()
player = doc.get_player()



def load_samples_from_bank(full_path, sample_list, start_index):
    '''
    Convenience method for loading from a list in a given directory, to wavetable
    
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



'''E x a m p l e   u s a g e '''

# Load a sample list
start_index = 0
full_path_to_bank = os.path.join(os.getcwd(), "Samples")
sample_list = ['BT7AADA.WAV', 'HHCD4.WAV', 'HHODA.WAV', 'RIDED0.WAV', '-',
                    'mth_hit_clipper.wav', 'mth_hat_wook.wav', '-', 'mth_bd _tuned.wav']
load_samples_from_bank(full_path_to_bank, sample_list, start_index)
