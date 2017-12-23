-- implements a list tag, must contain an lbody/li and optional lbodyscroll/scrollhandle
-- supports the class .singleselection

local function get_children_height(nodelist)
	local height = 0
	for i = 0, fbgui_node_list:get_count(nodelist) - 1 do
		local item = fbgui_node_list:get_item(nodelist, i)
		height = height + fbgui_node:get_height(item)
	end
	return height
end

local function list_resize(listelement)
	local lbodyscroll = fbgui_node_list:get_item(fbgui_node:find(listelement, "lbodyscroll"), 0)
	if lbodyscroll ~= nil then
		local lbodychildren = fbgui_node:find(listelement, "lbody > *")
		local scrollheight = get_children_height(lbodychildren)
		fbgui_node:set_attribute(lbodyscroll, context, "scroll-page", fbgui_node:get_height(lbodyscroll))
		fbgui_node:set_attribute(lbodyscroll, context, "scroll-range", scrollheight)
	end
end

local function on_list_resize(data)
	local target = fbgui_event_data:get_target(data)
	list_resize(target)
end

local function on_listbody_childrenchanged(data)
	local target = fbgui_event_data:get_target(data)
	local listelement = fbgui_node:get_parent(target)
	list_resize(listelement)
end

local function on_listbody_format(data)
	--io.write("setting lbody-index\n")
	local target = fbgui_event_data:get_target(data)
	local listelement = fbgui_node:get_parent(target)
	fbgui_node:set_attribute(listelement, context, "lbody-index", fbgui_node:get_index(target) - 1) -- index is 1-based
end

local function on_list_scroll(data)
	local target = fbgui_event_data:get_target(data)
	local listelement = fbgui_node:get_parent(target)
	local lbodyindex = tonumber(fbgui_node:get_attribute(listelement, "lbody-index"))
	local lbody = fbgui_node_list:get_item(fbgui_node:get_child_nodes(listelement), lbodyindex) -- NOTE: not slow, use predetermined index
	--local lbody = fbgui_node_list:get_item(fbgui_node:find(listelement, "lbody"), 0) -- NOTE: SLOW! scans the whole list
	
	local scroll_position = tonumber(fbgui_node:get_attribute(target, "scroll-position"))
	if scroll_position ~= nil then
		fbgui_node:style(lbody, context, "scroll-top:" .. scroll_position)
	end
end

local function on_mousemove(data)
	local target = fbgui_event_data:get_target(data)
	local lbody = fbgui_node:get_parent(target)

	local is_selected = fbgui_node:has_class(target, "highlight") ~= 0

	if not is_selected then
		fbgui_node_list:remove_class(fbgui_node:get_child_nodes(lbody), context, "highlight")
		fbgui_node:add_class(target, context, "highlight")
	end
end

fbgui_context:load_stylesheet(context, "Gear/Scripts/LuaView/List.nss")

fbgui_context:bind(context, "list", "post-resize", on_list_resize)
fbgui_context:bind(context, "list > lbody", "children-changed", on_listbody_childrenchanged)
fbgui_context:bind(context, "list > lbody", "format", on_listbody_format)
fbgui_context:bind(context, "list > lbodyscroll", "scroll-track", on_list_scroll)
fbgui_context:bind(context, "list > lbody > li", "mousemove", on_mousemove) -- TODO: use hover
