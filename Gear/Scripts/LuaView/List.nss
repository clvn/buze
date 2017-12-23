list {
	layout:horizontal;
}

lbody {
	layout:vertical;
	border-top-width:1;
}

li {
	layout:horizontal;
	height:20;
	border-bottom-width:1;
	background-color:#444444;
	color:#ffffff;
}

li:odd {
	background-color:#383C38;
}

li:first, li:last {
	background-color:#ff00cc;
}

li.selected {
	background-color:#777787
}

li.highlight {
	background-color:#555555
}

li.highlight.selected {
	background-color:#777787
}

lbody > label {
	background-color:#d0d0d0;
	height:20;
}

lbodyscroll {
	layout:vertical;
	width:20; 
	background-color:#888888;
	border-right-color:#555555;
	border-right-width:1;
	border-left-color:#555555;
	border-left-width:1;
}
