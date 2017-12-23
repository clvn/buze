'''
some major assumption in this code, 
- master is present
- master is called 'Master'
- a generator called 'Synth' is present

'''

import armstrong

ZZUB_MASTER = "@zzub.org/master"

doc = mainframe.get_document()
player = doc.get_player()

plugin_in_question = player.get_plugin_by_name("Synth")
master = player.get_plugin_by_uri(ZZUB_MASTER)

# connect Synth to Master
master.create_audio_connection(plugin_in_question, 0, 2, 0, 2)