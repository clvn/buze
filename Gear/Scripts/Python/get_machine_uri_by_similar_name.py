from pprint import pprint

doc = mainframe.get_document()
player = doc.get_player()

# uri are important while scripting armstrong, this makes it easy for getting
# machines by name, or similar name. use this while you plan your script.

def getURI(plugin_name):
	uriList = []
	for i in range(player.get_pluginloader_count()):
		uri = player.get_pluginloader(i).get_uri()
		if uri.lower().find(plugin_name) >= 0:
			uriList.append(uri)
	return uriList

pprint(getURI("dist"))
