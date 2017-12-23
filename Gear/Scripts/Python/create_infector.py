import ctypes
import armstrong
import buze

doc = mainframe.get_document()
player = doc.get_player()

loader = player.get_pluginloader_by_name("@zzub.org/buzz2zzub/FSM+Infector")
newplug = player.create_plugin(None, 0, "Infector", loader)

player.history_commit(0, 0, "Created plugin")
