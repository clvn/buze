require "Gear.Scripts.LuaView.Checkbox"

-- global variables made available by the runtime:
--     context     - the fbgui context
--     player      - the armstrong player instance
--     audiodriver - the armstrong audiodriver
--     mainframe   - the buze mainframe instance
--     script_path - a string containing the path of the current script

local view = nil

local function on_idle(data)
	local heartlist = fbgui_node:find(view, "#heart")
	local heart = fbgui_node_list:get_item(heartlist, 0)
	
	local bpm = zzub_player:get_bpm(player)
	local bpmlist = fbgui_node:find(view, "#bpm")
	fbgui_node_list:set_inner_text(bpmlist, context, bpm)

	local samplerate = zzub_audiodriver:get_samplerate(audiodriver)
	local songtime = zzub_player:get_position_samples(player) / samplerate
	local counter = songtime

	local phase = counter * math.pi * 2 * bpm / 60 
	local size = math.sin(phase) * 10 + 10
	local pos = math.ceil(10 - (size / 2))
	fbgui_node:style(heart, context, "position:relative; left:" .. pos .. "%; top:" .. pos .. "%; width:" .. (80+size) .. "%; height:" .. (80+size) .. "%")
end

local function on_load(data)
end

local function on_quit(data)
end

-- register fonts
fbgui_context:register_font(context, "Regular", "Gear/Scripts/LuaView/Tahoma.ttf")
fbgui_context:register_font(context, "Fixed", "Gear/Scripts/LuaView/Perfect DOS VGA 437 Win.ttf")

-- load ui
fbgui_context:load_stylesheet(context, script_path .. "/main.nss")
view = fbgui_context:load_markup(context, script_path .. "/main.nml")

-- bind events
fbgui_context:bind(context, "root", "idle", on_idle)
fbgui_context:bind(context, "root", "load", on_load)
fbgui_context:bind(context, "root", "quit", on_quit)

-- initialize/run
fbgui_context:set_idle_modulus(context, 1)
fbgui_context:run(context, view)
