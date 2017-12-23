# this will print something like
# [3, 80, 100, 180, 3, 80, 180, .......(etc)...... 40, 40, 180, 20, 0]

import armstrong

doc = mainframe.get_document()
player = doc.get_player()
plugin = player.get_plugin_by_name("Infector44")

parameter_group = 1  # 1 = global params
global_vals = []
for track_n in range(plugin.get_track_count(parameter_group)):
	for param_n in range(plugin.get_parameter_count(parameter_group, track_n)):
	    value = plugin.get_parameter_value(parameter_group, track_n, param_n)
	    global_vals.append(value)
	
print(global_vals)