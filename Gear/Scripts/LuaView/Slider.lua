-- handles mouse events for scrolling lists made of the tags list, lbody, lbodyscroll, scrollhandle and associated stylesheet

local slider_dragging = false
local slider_mouse_start = 0
local slider_handle_start = 0
local slider_handle = nil
local slider_position = 0
local slider_handle_max = 0
local slider_page_size = 0
local slider_range_size = 0
local slider_target_position = 0

local function on_slider_load(data)
	local target = fbgui_event_data:get_target(data)
	
	fbgui_node:set_attribute(target, context, "last-scroll-position", -1)
	local target_position = tonumber(fbgui_node:get_attribute(target, "scroll-position"))
	if target_position == nil then
		fbgui_node:set_attribute(target, context, "scroll-position", 0)
	end
end

local function on_slider_resize(data)
	local target = fbgui_event_data:get_target(data)	
	fbgui_node:set_attribute(target, context, "last-scroll-position", -1)
	--on_slider_attribute
end

local function on_slider_mousedown(data)
	slider_mouse_start = fbgui_event_data:get_x(data)

	local target = fbgui_event_data:get_target(data)
	
	local list_element = target --fbgui_node:get_parent(target)
	slider_handle = fbgui_node_list:get_item(fbgui_node:find(list_element, "sliderhandle"), 0)
	slider_handle_max = fbgui_node:get_width(target) - fbgui_node:get_width(slider_handle)
	slider_handle_start = fbgui_node:get_screen_left(slider_handle) - fbgui_node:get_screen_left(list_element)
	slider_position = slider_handle_start
	
	slider_page_size = tonumber(fbgui_node:get_attribute(target, "slider-page"))
	slider_range_size = tonumber(fbgui_node:get_attribute(target, "slider-range"))

	if slider_page_size == nil or slider_range_size == nil then
		io.write("ERROR: slider missing slider-page and/or slider-range attributes\n")
		return
	end

		-- local slider_range = slider_range_size - slider_page_size
		-- local slider_area_position = math.floor(slider_position  * slider_range / slider_handle_max)
		-- fbgui_node:set_attribute(target, context, "scroll-position", slider_area_position)

	fbgui_context:capture(context, target)
	slider_dragging = true
	return true
end

local function on_slider_mouseup(data)
	fbgui_context:release_capture(context)
	slider_dragging = false
	return true
end

local function on_slider_mousemove(data)
	local target = fbgui_event_data:get_target(data)
	if not slider_dragging then
		return true
	end
	
	local movediff = fbgui_event_data:get_x(data) - slider_mouse_start

	if slider_handle_start + movediff ~= slider_position then
		slider_position = slider_handle_start + movediff
		if slider_position  < 0 then
			slider_position = 0
		end
		if slider_position > slider_handle_max then
			slider_position = slider_handle_max
		end

		-- position the scroll handle
		fbgui_node:style(slider_handle, context, "left:" .. slider_position)

		-- trigger a "scroll-track" event for generic scrolling support
		local slider_range = slider_range_size - slider_page_size
		slider_target_position = math.floor(slider_position  * slider_range / slider_handle_max)

		fbgui_node:set_attribute(target, context, "scroll-position", slider_target_position)
		--fbgui_node:set_attribute(target, context, "last-scroll-position", slider_target_position)
		--fbgui_context:trigger(context, target, "scroll-track", data)		
	end
	return true
end

local function on_slider_attribute(data)

	io.write("attribute on slider!\n")

	local target = fbgui_event_data:get_target(data)
	local target_position = tonumber(fbgui_node:get_attribute(target, "scroll-position"))
	local last_target_position = tonumber(fbgui_node:get_attribute(target, "last-scroll-position"))

	if target_position ~= last_target_position then
		slider_handle = fbgui_node_list:get_item(fbgui_node:find(target, "sliderhandle"), 0)
		slider_handle_max = fbgui_node:get_width(target) - fbgui_node:get_width(slider_handle)

		slider_page_size = tonumber(fbgui_node:get_attribute(target, "slider-page"))
		slider_range_size = tonumber(fbgui_node:get_attribute(target, "slider-range"))

		--io.write("scrollbar changed externally")
		slider_target_position = target_position
		local slider_range = slider_range_size - slider_page_size
		slider_position = math.floor(target_position * slider_handle_max / slider_range)
		
		--io.write("da scroll: " .. slider_target_position .. ", new top= " .. slider_position)
		
		fbgui_node:set_attribute(target, context, "last-scroll-position", slider_target_position)
		fbgui_node:style(slider_handle, context, "left:" .. slider_position)
		if slider_dragging then
			fbgui_context:trigger(context, target, "slider-track", data)
		end
	end
	
	-- position the scroll handle
	--fbgui_node:style(slider_handle, context, "top:" .. slider_position)


end


fbgui_context:load_stylesheet(context, "Gear/Scripts/LuaView/Slider.nss")
fbgui_context:bind(context, "slider", "load", on_slider_load)
fbgui_context:bind(context, "slider", "post-resize", on_slider_resize)
fbgui_context:bind(context, "slider", "lbuttondown", on_slider_mousedown)
fbgui_context:bind(context, "slider", "lbuttonup", on_slider_mouseup)
fbgui_context:bind(context, "slider", "mousemove", on_slider_mousemove)
fbgui_context:bind(context, "slider", "attributes-changed", on_slider_attribute)
