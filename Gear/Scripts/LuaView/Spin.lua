-- spin control, hooks to the spin tag, expects li tags as children
-- set_attribute("index") to set the selected index
-- get_attribute("index") to retreive the currently selected index
-- generates "selectionchanged" event

local function on_spin_load(data)
	io.write("spin format\n")
	local target = fbgui_event_data:get_target(data)
	
	local items = fbgui_node:find(target, "li")
	local itemcount = fbgui_node_list:get_count(items)
	if itemcount == 0 then
		return
	end

	local index = fbgui_node:get_attribute(target, "index")
	if index ~= nil then
		io.write("indeks is " .. index .. "\n")
	end
	
	if index == nil then
		index = 0
		fbgui_node:set_attribute(target, context, "index", index)
	else
		index = tonumber(index)
	end
	
	if index >= itemcount then
		index = itemcount - 1
	end
	
	fbgui_node_list:style(items, context, "visible:false")
	
	local item = fbgui_node_list:get_item(items, index)
	fbgui_node:style(item, context, "visible:true")
	io.write("spin loaded")
end

local function on_spin_click(data)
	local target = fbgui_event_data:get_target(data)
	local items = fbgui_node:find(target, "li")
	local itemcount = fbgui_node_list:get_count(items)
	if itemcount == 0 then
		return
	end

	-- find which item is selected, 
	local index = tonumber(fbgui_node:get_attribute(target, "index"))
	index = index + 1
	if index >= itemcount then
		index = 0
	end
	fbgui_node:set_attribute(target, context, "index", index)

	fbgui_node_list:style(items, context, "visible:false")
	local item = fbgui_node_list:get_item(items, index)
	fbgui_node:style(item, context, "visible:true")
	
	fbgui_context:trigger(context, target, "selectionchanged", data)
end


fbgui_context:load_stylesheet(context, "Gear/Scripts/LuaView/Spin.nss")

fbgui_context:bind(context, "spin", "load", on_spin_load) -- load is called once, format every time the node (or a sibling or parent) is changed
fbgui_context:bind(context, "spin", "children-changed", on_spin_load) -- load is called once, format every time the node (or a sibling or parent) is changed
fbgui_context:bind(context, "spin", "attributes-changed", on_spin_load) -- load is called once, format every time the node (or a sibling or parent) is changed
fbgui_context:bind(context, "spin", "lbuttonup", on_spin_click)
