require "Gear.Scripts.LuaView.Checkbox"
require "Gear.Scripts.LuaView.List"
require "Gear.Scripts.LuaView.Spin"
require "Gear.Scripts.LuaView.Slider"

-- global variables made available by the runtime:
--     context     - the fbgui context
--     player      - the armstrong player instance
--     audiodriver - the armstrong audiodriver
--     mainframe   - the buze mainframe instance
--     script_path - a string containing the path of the current script

local view = nil
local starttime = os.clock()
local canvas = nil

local function on_idle(data)
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#numpluginloaders"), context, zzub_player:get_pluginloader_count(player))
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#numplugins"), context, zzub_player:get_plugin_count(player))
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#numpatterns"), context, zzub_player:get_pattern_count(player))
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#numpatternformats"), context, zzub_player:get_pattern_format_count(player))
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#numorders"), context, zzub_player:get_order_length(player))
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#samplerate"), context, zzub_audiodriver:get_samplerate(audiodriver))
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#buffersize"), context, zzub_audiodriver:get_buffersize(audiodriver))
	
	local currentorder = zzub_player:get_position_order(player)
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#currentorder"), context, currentorder)
	
	local currentpattern = ""
	local orderpattern = zzub_player:get_order_pattern(player, currentorder)
	if orderpattern ~= nil then
		currentpattern = zzub_pattern:get_name(orderpattern)
	else
		currentpattern = "(null)"
	end
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#currentpattern"), context, currentpattern)
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#currentrow"), context, zzub_player:get_position_row(player))

	local songstate = zzub_player:get_state(player)
	local statename = "unknown"
	if songstate == zzub_player_state.playing then
		statename = "playing"
	elseif songstate == zzub_player_state.stopped then
		statename = "stopped"
	elseif songstate == zzub_player_state.muted then
		statename = "muted"
	elseif songstate == zzub_player_state.released then
		statename = "released"
	end
	fbgui_node_list:set_inner_text(fbgui_node:find(view, "#songstate"), context, statename)

	if canvas ~= nil then
		fbgui_context:invalidate(context, canvas)
	end
end

local function on_load(data)
end

local function on_canvas_paint(data)
	local g = fbgui_context:get_graphics(context)
	local target = fbgui_event_data:get_target(data)
	
	canvas = target
	
	local width = fbgui_node:get_width(target) / 2
	local height = fbgui_node:get_height(target) / 2

	local left = width + fbgui_node:get_screen_left(target)
	local top = height + fbgui_node:get_screen_top(target)

	local painttime = (os.clock() - starttime) * 1

	for i = 0, 30 do
		local frametime = painttime + (i / 10)
		
		local x1 = math.cos(frametime) * width
		local y1 = math.sin(frametime) * height

		local x2 = -x1 + math.cos(frametime / 4) * width / 2
		local y2 = -y1 + math.sin(frametime / 3) * height
		
		local colorbase = i / 30
		
		colorbase = colorbase * colorbase * colorbase
		colorbase = math.ceil(255 - (colorbase * 255))
		
		local color = bit32.bor(colorbase, bit32.lshift(255, 8), bit32.lshift(colorbase, 16), bit32.lshift(255, 24))
		fbgui_graphics:set_color(g, color)
		fbgui_graphics:draw_line(g, left + x1, top + y1, left + x2, top + y2)
	end
	
	fbgui_graphics:set_color(g, 0)
	fbgui_graphics:draw_string(g, left, top, "Time: " .. painttime)
	
	--local x2 = fbgui_node:get_screen_left(n2)
	--local y2 = fbgui_node:get_screen_top(n2)
	
	--local w = fbgui_node:get_width(n1) / 2
	--local h = fbgui_node:get_height(n1) / 2

end

local function on_quit(data)
end

local function on_player_callback(player, plugin, data, tag)
	local type = zzub_event_data:get_type(data)
	
	if type == zzub_event_type.update_pluginparameter then
		local updatedata = zzub_event_data:get_update_pluginparameter(data)
		local plugin = zzub_event_data_update_plugin_parameter:get_plugin(updatedata)
		local group = zzub_event_data_update_plugin_parameter:get_group(updatedata)
		local track = zzub_event_data_update_plugin_parameter:get_track(updatedata)
		local param = zzub_event_data_update_plugin_parameter:get_param(updatedata)
		local value = zzub_event_data_update_plugin_parameter:get_value(updatedata)
		-- knows the changed plugin, parameter and value here
		fbgui_node_list:set_inner_text(fbgui_node:find(view, "#lastparam"), context, value)
	elseif type == zzub_event_type.double_click then
		-- must return -1 (unhandled) on the double_click event, else doubleclicking in the machine view might not work
		return -1 
	end	
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

fbgui_context:bind(context, "#canvas", "paint", on_canvas_paint)

-- armstrong callbacks are automatically disconnected when the view is closed
zzub_player:add_callback(player, on_player_callback, 0)

-- initialize/run
fbgui_context:set_idle_modulus(context, 1)
fbgui_context:run(context, view)
