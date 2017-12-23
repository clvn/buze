# param tests
import armstrong
from pprint import pprint

# if you are exploring armstrong, the last few lines here allows you
# print the methods that don't start with an underscore.

doc = mainframe.get_document()
player = doc.get_player()
plugin = player.get_plugin_by_name("Synth")

g_params = 1 
param_track = 0 # this g_params has only one track, namely '0'
param_number = 7 # currently, resonance

param = plugin.get_parameter(g_params, param_track, param_number)
print(param.get_value_min())
print(param.get_value_max())

methods = [method for method in dir(param) if not method.startswith('_')]
pprint(methods)