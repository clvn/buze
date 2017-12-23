import ctypes
import armstrong
import buze

doc = mainframe.get_document()
player = doc.get_player()
#print(player)

plugcount = player.get_plugin_count()

plugit = player.get_plugin_iterator()

while plugit.valid():
	plugin = plugit.current()
	loader = plugin.get_pluginloader()
	print("id: " + str(plugin.get_id()) + ", name: " + plugin.get_name() + ", type=" + loader.get_name())
	#print(plugin)
	plugit.next()

print("plugin count = " + str(plugcount))
