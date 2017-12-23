-- checkbox widget

local function on_checkbox_paint(data)
	local target = fbgui_event_data:get_target(data)	
	local is_checked = fbgui_node:get_attribute(target, "value") == "true"
	
	if is_checked then
		local x = fbgui_node:get_screen_left(target)
		local y = fbgui_node:get_screen_top(target)
		local g = fbgui_context:get_graphics(context)
		
		local w = fbgui_node:get_width(target)
		local h = fbgui_node:get_height(target)
		
		fbgui_graphics:set_color(g, 0)
		fbgui_graphics:draw_line(g, x + 3, y + 3, x + 10, y + 10)
		fbgui_graphics:draw_line(g, x + 4, y + 3, x + 11, y + 10)

		fbgui_graphics:draw_line(g, x + w - 3, y + 3, x + w - 10, y + 10)
		fbgui_graphics:draw_line(g, x + w - 4, y + 3, x + w - 11, y + 10)

		fbgui_graphics:set_color(g, 0xffffff)
	end
end

local function on_checkbox_click(data)
	local target = fbgui_event_data:get_target(data)

	local is_checked = fbgui_node:get_attribute(target, "value") == "true"
	if is_checked then
		fbgui_node:set_attribute(target, context, "value", "false")
	else
		fbgui_node:set_attribute(target, context, "value", "true")
	end

	fbgui_context:invalidate(context, target)
	fbgui_context:trigger(context, target, "selectionchanged", data)
end


fbgui_context:load_stylesheet(context, "Gear/Scripts/LuaView/Checkbox.nss")
fbgui_context:bind(context, "checkbox", "paint", on_checkbox_paint)
fbgui_context:bind(context, "checkbox", "lbuttonup", on_checkbox_click)
