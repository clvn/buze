#include "stdafx.h"
#include "resource.h"
#include "MixerView.h"
#include "utils.h"
#include "Keymaps.h"

#define VU_COLOR_LOW	0x00ff00	/* Green */
#define VU_COLOR_12DB	0x99CCFF	/* Yellow */
#define VU_COLOR_6DB	0x0000FF	/* Red */

CHostDllModule _Module;

// dupe from PatternFormatView.cpp
bool visibleParameter(zzub_plugin_t* plugin, int group, int column, std::vector<std::string>& parameterfilter) {

	if (parameterfilter.size() == 0) return true;

	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, 0, column);
	static char pcName[1024];
	strcpy(pcName, zzub_parameter_get_name(param));

	strlwr(pcName);

	std::vector<std::string> tokens;
	split(pcName, tokens, " ");

	int counter = 0;
	for (std::vector<std::string>::iterator i = tokens.begin(); i != tokens.end(); ++i) {
		for (std::vector<std::string>::iterator filterit = parameterfilter.begin(); filterit != parameterfilter.end(); ++filterit) {
			if (filterit->length() == 0) continue;
			counter++;
			if (i->find(*filterit) == 0) {
				return true;
			}
		}
	}
	return counter == 0;
}

// dupe from PatternFormatView.cpp
bool filterPlugin(const char* name, std::vector<std::string>& pluginfilter) {

	if (pluginfilter.size() == 0) return true;

	static char pcName[256];
	strncpy(pcName, name, 256);

	strlwr(pcName);

	std::vector<std::string> pluginname;
	split(pcName, pluginname, " ");

	int counter = 0;
	for (std::vector<std::string>::iterator i = pluginname.begin(); i != pluginname.end(); ++i) {
		for (std::vector<std::string>::iterator filterit = pluginfilter.begin(); filterit != pluginfilter.end(); ++filterit) {
			if (filterit->length() == 0) continue;
			counter++;
			if (i->find(*filterit) == 0) {
				return true;
			}
		}
	}
	return counter == 0;
}

//
// View
//

CMixerView::CMixerView(buze_main_frame_t* m, CMixerViewInfo* i)	
	:CViewImpl(m)
	,info(i)
{
	dragmode = dragtype_move_none;
	selectedtrack = -1;
	outplugin = 0;
	dirtystate = false;
	linked = true;
}

CMixerView::~CMixerView() {
}

void CMixerView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CMixerView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lres = DefWindowProc();
	int bWidth  = 90;
	int bHeight = 40;

	BOOL bLock = FALSE;
	HFONT defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	// TODO: the filters should be label-less, and instead have a search-icon and a shaded label inside the textbox when the textbox is empty
	pluginFilter.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PLUGINFILTER);
	pluginFilter.SetFont(defaultFont);
	pluginFilter.m_edit.SetWindowText(info->plugin_filter.c_str());
	insertToolbarBand(pluginFilter, "Plugin/Group Filter", 200, -1, true, bLock, false);

	parameterFilter.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PARAMETERFILTER);
	parameterFilter.SetFont(defaultFont);
	parameterFilter.m_edit.SetWindowText(info->parameter_filter.c_str());
	insertToolbarBand(parameterFilter, "Parameter Filter", 200, -1, true, bLock, false);

	dirtystate = true;

	buze_document_add_view(document, this);
	buze_main_frame_add_timer_handler(mainframe, this);
	buze_main_frame_viewstack_insert(mainframe, this); // true
	return lres;
}

const int trackwidth = 64;
const int trackmargin = 2;

void CMixerView::UpdateLayout() {
	RECT rcClient;
	GetClientRect(&rcClient);

	rcClient.top += getToolbarHeight();

	int maxinputs = 0;
	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		maxinputs = std::max((int)track->inplugins.size(), maxinputs);
	}

	int inputheight = maxinputs * 16;
	if (inputheight > 0) inputheight += 2; // 2px margin between inputs and eq

	std::vector<std::string> paramFilters;
	GetParameterFilter(paramFilters);

	int maxparameters = 0;
	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		maxparameters = std::max(track->knobcount, maxparameters);
	}

	// TODO: max number to fit on screen:
	//int maxfitparameters = (rcClient.bottom - rcClient.top - 16 - 1 - inputheight) / 16;
	//if (maxfitparameters < 0) maxfitparameters = 0;
	//maxparameters = std::min(maxfitparameters, maxparameters);

	int parameterheight = maxparameters * 16;
	if (parameterheight > 0) parameterheight += 2; // 2px margin between eqs and big slider

	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		int index = (int)std::distance(tracks.begin(), i);

		SetRect(&track->rcTrack, index * trackwidth, rcClient.top, index * trackwidth + trackwidth - trackmargin, rcClient.bottom );
		SetRect(&track->rcHeader, track->rcTrack.left + 1, track->rcTrack.top + 1, track->rcTrack.right - 1, track->rcTrack.top + 16);
		SetRect(&track->rcFooter, track->rcTrack.left + 1, track->rcTrack.bottom - 16, track->rcTrack.right - 1, track->rcTrack.bottom);

		int top = track->rcTrack.top + 1 + 16;

		for (std::vector<zzub_plugin_t*>::iterator j = track->inplugins.begin(); j != track->inplugins.end(); ++j) {
			int index = (int)std::distance(track->inplugins.begin(), j);
			SetRect(&track->rcInputs[index], track->rcTrack.left + 1, top + index * 16, track->rcTrack.right - 2, top + 16 + index * 16);
		}

		for (int j = 0; j < track->knobcount; j++) {
			SetRect(&track->knobs[j].rcKnob, track->rcTrack.left + 1, top + inputheight + j * 16, track->rcTrack.right - 2, top + inputheight + 16 + j * 16);
		}

		int ampwidth = trackwidth;// - 16 - 1;

		SetRect(&track->rcAmps[0], track->rcTrack.left + 1, top + inputheight + parameterheight, track->rcTrack.left + ampwidth / 2 - 1, track->rcTrack.bottom - 16);
		SetRect(&track->rcAmps[1], track->rcTrack.left + ampwidth / 2, top + inputheight + parameterheight, track->rcTrack.left + ampwidth - 2, track->rcTrack.bottom - 16);

/*		SetRect(&track->rcMute, 
			track->rcTrack.left + ampwidth + 2, 
			top + inputheight + parameterheight + 16 + 1, 
			track->rcTrack.left + trackwidth - 2, 
			top + inputheight + parameterheight + 16 + 1 + 16);

		SetRect(&track->rcSolo, 
			track->rcTrack.left + ampwidth + 2, 
			top + inputheight + parameterheight + 16 + 1 + 16 + 1, 
			track->rcTrack.left + trackwidth - 2, 
			top + inputheight + parameterheight + 16 + 1 + 16 + 1 + 16);*/
	}
}

LRESULT CMixerView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	UpdateLayout();
	return 0;
}

LRESULT CMixerView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	buze_main_frame_remove_timer_handler(mainframe, this);
	buze_document_remove_view(document, this);
	return 0;
}

LRESULT CMixerView::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if ((lParam & (1 << 30)) != 0) return 0;

	int note = keyboard_mapper::map_code_to_note(buze_document_get_octave(document), wParam);
	if (note > 0 && note != zzub_note_value_off && note != zzub_note_value_cut) note = midi_to_buzz_note(note);
	if (note == -1) return 0;

	zzub_plugin_t* plugin = zzub_player_get_midi_plugin(player);
	if (plugin == 0) return 0;

	buze_document_keyjazz_key_down(document, plugin, wParam, note);
	buze_document_play_plugin_note(document, plugin, note, 0);
	return 0;
}

LRESULT CMixerView::OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	int lastnote;
	zzub_plugin_t* lastplugin = 0;

	buze_document_keyjazz_key_up(document, wParam, &lastnote, &lastplugin);
	if (lastplugin == 0) return 0;

	buze_document_play_plugin_note(document, lastplugin, zzub_note_value_off, lastnote);
	return 0;
}

LRESULT CMixerView::OnBlur( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// do not kill keyjazz if the target focus is (a child of) the paramview or something that handles keyjazz!
	HWND hFocusWnd = (HWND)wParam;
	CView* view = buze_main_frame_get_view_by_wnd(mainframe, hFocusWnd);
	if (view != 0 && view->DoesKeyjazz())
		return 0;

	buze_document_keyjazz_release(document, true);		// reset keyboard keyjazz
	return 0;
}

// value 0..1
void CMixerView::DrawTrackSlider(CDC& dc, const std::string& label, RECT& rcSlider, float value, bool showValue) {
	dc.Draw3dRect(&rcSlider, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));
	
	int sliderwidth = (rcSlider.right - rcSlider.left) - 2;
	int valuewidth = (int)(value * (float)sliderwidth);
	dc.FillSolidRect(rcSlider.left + 1, rcSlider.top + 1, valuewidth, rcSlider.bottom - rcSlider.top - 2, 0x60F060);

	RECT rcLabel;
	SetRect(&rcLabel, rcSlider.left + 1, rcSlider.top + 1, rcSlider.right - 22, rcSlider.bottom - 1);
	dc.ExtTextOut(rcLabel.left, rcLabel.top, ETO_CLIPPED, &rcLabel, label.c_str(), (int)label.length(), NULL);

	if (showValue) {
		std::stringstream valuestrm;
		valuestrm << std::fixed << std::setprecision(1) << value;
		std::string valuestring = valuestrm.str();

		RECT rcValue;
		SetRect(&rcValue, rcSlider.right - 22, rcSlider.top + 1, rcSlider.right - 1, rcSlider.bottom - 1);
		dc.ExtTextOut(rcValue.left, rcValue.top, ETO_CLIPPED, &rcValue, valuestring.c_str(), (int)valuestring.length(), NULL);
	}

}

// peak = 0..1
// amp = 0..2
void CMixerView::DrawAmpSlider(CDC& dc, RECT& rcAmpRect, float peak, float amp) {
	dc.FillSolidRect(&rcAmpRect, 0);
	dc.Draw3dRect(&rcAmpRect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));
	//dc.Draw3dRect(&rcAmpRect, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));

	RECT rcAmp = rcAmpRect;
	InflateRect(&rcAmp, -1, -1);

	int vuheight = (rcAmp.bottom - rcAmp.top); // - 4; // -4 for the handle height - why??
	peak = std::min(std::max(peak * amp, 0.0f), 1.0f); // multiply by amp, we get the peaks of the plugin output and alter the connection amp parameter
	int peakheight = (int)(peak * (float)vuheight);

	int greenheight = std::min((int)(0.8 * (float)vuheight), peakheight);
	int yellowheight = std::min((int)(0.95 * (float)vuheight), peakheight);
	int redheight = std::min((int)(1 * (float)vuheight), peakheight);

	RECT rcGreen;
	RECT rcYellow;
	RECT rcRed;
	SetRect(&rcGreen, rcAmp.left, rcAmp.bottom - greenheight, rcAmp.right, rcAmp.bottom);
	SetRect(&rcYellow, rcAmp.left, rcAmp.bottom - yellowheight, rcAmp.right, rcGreen.top);
	SetRect(&rcRed, rcAmp.left, rcAmp.bottom - redheight, rcAmp.right, rcYellow.top);

	dc.FillSolidRect(&rcGreen, VU_COLOR_LOW);
	if (rcYellow.bottom > rcYellow.top)
		dc.FillSolidRect(&rcYellow, VU_COLOR_12DB);
	if (rcRed.bottom > rcRed.top)
		dc.FillSolidRect(&rcRed, VU_COLOR_6DB);

	//RECT rcPeak;
	//SetRect(&rcPeak, rcAmp.left, rcAmp.bottom - peakheight, rcAmp.right, rcAmp.bottom);
	//dc.FillSolidRect(&rcPeak, 0);

	int ampheight = (int)(amp * 0.5f * (float)vuheight);
	RECT rcHandle;
	SetRect(&rcHandle, rcAmp.left, rcAmp.bottom - ampheight - 4, rcAmp.right, rcAmp.bottom - ampheight);
	dc.FillSolidRect(&rcHandle, GetSysColor(COLOR_3DFACE));
	dc.Draw3dRect(&rcHandle, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
}

LRESULT CMixerView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

	if (dirtystate) {
		RestoreState();
		dirtystate = false;
	}

	CPaintDC paintDC(m_hWnd);

	RECT rcPaint;
	paintDC.GetClipBox(&rcPaint);

	CMemoryDC dc(paintDC, rcPaint);

	RECT rcClient;
	GetClientRect(&rcClient);

	dc.FillSolidRect(&rcClient, GetSysColor(COLOR_APPWORKSPACE));

	int bkmode = dc.SetBkMode(TRANSPARENT);
	CFontHandle font = dc.SelectFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		int index = (int)std::distance(tracks.begin(), i);
		const RECT rcTrack = track->rcTrack;

		RECT rcIntersect;
		if (IntersectRect(&rcIntersect, &rcPaint, &track->rcTrack) == 0)
			continue;

		dc.FillSolidRect(&rcTrack, GetSysColor(COLOR_3DFACE));
		dc.Draw3dRect(&rcTrack, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
		dc.ExtTextOut(track->rcHeader.left, track->rcHeader.top, ETO_CLIPPED, &track->rcHeader, track->name.c_str(), (int)track->name.length(), NULL);

		float value;

		// draw input knobs
		for (std::vector<zzub_plugin_t*>::iterator j = track->inplugins.begin(); j != track->inplugins.end(); ++j) {
			int index = (int)std::distance(track->inplugins.begin(), j);

			RECT rcIntersect;
			if (IntersectRect(&rcIntersect, &rcPaint, &track->rcInputs[index]) == 0)
				continue;

			const char* name = zzub_plugin_get_name(*j);
			// TODO: mono or stereo input value?
			zzub_plugin_t* connplug = track->inconnplugins[index];
			value = (float)(zzub_plugin_get_parameter_value(connplug, 2, 0, 0)) / 32768.0f;
			DrawTrackSlider(dc, name, track->rcInputs[index], value, false);
		}

		// draw eq knobs
		int paramcount = track->knobcount;
		for (int i = 0; i < paramcount; i++) {
			RECT rcIntersect;
			if (IntersectRect(&rcIntersect, &rcPaint, &track->knobs[i].rcKnob) == 0)
				continue;

			zzub_parameter_t* param = zzub_plugin_get_parameter(track->knobs[i].plugin, track->knobs[i].group, 0, track->knobs[i].column);
			int value_min = zzub_parameter_get_value_min(param);
			int value_max = zzub_parameter_get_value_max(param);
			int value_range = value_max - value_min;
			int value = zzub_plugin_get_parameter_value(track->knobs[i].plugin, track->knobs[i].group, 0, track->knobs[i].column);
			float slidervalue = (float)(value - value_min) / value_range;
			const char* name = zzub_parameter_get_name(param);
			DrawTrackSlider(dc, name, track->knobs[i].rcKnob, slidervalue, true);
		}

		// draw vu and l/r output amp sliders
		// slider value = amp track parameter of all output connections

		if (track->connplugin) {
			float ampL = (float)zzub_plugin_get_parameter_value(track->connplugin, 2, 0, 0) / 16384.0f;
			float ampR = (float)zzub_plugin_get_parameter_value(track->connplugin, 2, 1, 0) / 16384.0f;

			DrawAmpSlider(dc, track->rcAmps[0], track->lastpeak[0], ampL);
			DrawAmpSlider(dc, track->rcAmps[1], track->lastpeak[1], ampR);
		}

		/*int mutevalue = zzub_plugin_get_parameter_value(track->eqinplugin, 0, 0, 0);
		if (mutevalue != 0) {
			dc.FillSolidRect(&track->rcMute, GetSysColor(COLOR_BTNHIGHLIGHT));
		} else {
			dc.FillSolidRect(&track->rcMute, GetSysColor(COLOR_APPWORKSPACE));
		}*/

		std::string outname;
		if (track->outplugin) {
			outname = zzub_plugin_get_name(track->outplugin);
		} else {
			outname = "(no out)";
		}
		dc.ExtTextOut(track->rcFooter.left, track->rcFooter.top, ETO_CLIPPED, &track->rcFooter, outname.c_str(), (int)outname.length(), NULL);
	}

	dc.SelectFont(font);
	dc.SetBkMode(bkmode);

	return DefWindowProc();
}

LRESULT CMixerView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	int trackindex = pt.x / trackwidth;
	if (trackindex < 0) trackindex = -1;
	if (trackindex >= (int)tracks.size()) trackindex = -1;

	selectedtrack = trackindex;

	if (selectedtrack == -1) return 0;

	dragmode = dragtype_move_none;
	dragindex = -1;
	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		int index = (int)std::distance(tracks.begin(), i);

		for (std::vector<zzub_plugin_t*>::iterator j = track->inplugins.begin(); j != track->inplugins.end(); ++j) {
			int index = (int)std::distance(track->inplugins.begin(), j);
			if (PtInRect(&track->rcInputs[index], pt)) {
				dragmode = dragtype_move_input;
				dragindex = index;
				break;
			}
		}

		int paramcount = track->knobcount;
		for (int j = 0; j < paramcount; j++) {
			if (PtInRect(&track->knobs[j].rcKnob, pt)) {
				dragmode = dragtype_move_eq;
				dragindex = j;
				break;
			}
		}

		for (int j = 0; j < 2; j++) {
			if (PtInRect(&track->rcAmps[j], pt)) {
				dragmode = dragtype_move_amp;
				dragindex = j;
				break;
			}
		}

		if (dragmode != dragtype_move_none) break;
	}

	
	if (dragmode != dragtype_move_none)
		SetCapture();

	return 0;
}

LRESULT CMixerView::OnLButtonDblClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// if input click = open parameters
	return 0;
}

LRESULT CMixerView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (selectedtrack == -1) return 0;

	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	CMixerTrack* track = tracks[selectedtrack];

	if (dragmode == dragtype_move_input) {
		RECT rcAmp = track->rcInputs[dragindex];
		int amppos = pt.x - rcAmp.left;
		float ampvalue = (float)amppos / (float)(rcAmp.right - rcAmp.left);
		ampvalue = std::min(std::max(ampvalue, 0.0f), 1.0f);

		zzub_connection_t* conn = zzub_plugin_get_input_connection_by_type(track->eqinplugin, track->inplugins[dragindex], zzub_connection_type_audio);
		if (conn != 0) {
			zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(conn);
			for (int i = 0; i < zzub_plugin_get_track_count(connplug, 2); i++) {
				zzub_plugin_set_parameter_value_direct(connplug, 2, i, 0, (int)(ampvalue * 32768.0f), false);
			}
		}
	} else if (dragmode == dragtype_move_eq) {
		CMixerKnob& mixknob = track->knobs[dragindex];
		RECT rcKnob = mixknob.rcKnob;

		int knobpos = pt.x - rcKnob.left;
		float knobvalue = (float)knobpos / (float)(rcKnob.right - rcKnob.left);
		knobvalue = std::min(std::max(knobvalue, 0.0f), 1.0f);

		zzub_parameter_t* param = zzub_plugin_get_parameter(mixknob.plugin, mixknob.group, 0, mixknob.column);
		int value_min = zzub_parameter_get_value_min(param);
		int value_max = zzub_parameter_get_value_max(param);
		int value_range = value_max - value_min;
		int value = (int)((float)value_min + 0.5f + knobvalue * (float)value_range);

		zzub_plugin_set_parameter_value_direct(mixknob.plugin, mixknob.group, 0, mixknob.column, value, false);
	} else if (dragmode == dragtype_move_amp) {
		if (track->connplugin) {
			RECT rcAmp = track->rcAmps[dragindex];
			int amppos = rcAmp.bottom - pt.y;
			float ampvalue = (float)amppos / (float)(rcAmp.bottom - rcAmp.top);
			ampvalue = std::min(std::max(ampvalue, 0.0f), 1.0f);

			zzub_plugin_set_parameter_value_direct(track->connplugin, 2, dragindex, 0, (int)(ampvalue * 32768.0f), false);
			if (linked) {
				zzub_plugin_set_parameter_value_direct(track->connplugin, 2, 1-dragindex, 0, (int)(ampvalue * 32768.0f), false);
			}
		}
	}

	return 0;
}

LRESULT CMixerView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (selectedtrack == -1) return 0;

	if (dragmode == dragtype_move_eq) {
		// TODO: set_value no-direct for undo buffer
	}

	if (GetCapture() == m_hWnd)
		ReleaseCapture();

	dragmode = dragtype_move_none;
	return 0;
}

bool CMixerView::IsTrackPlugin(zzub_plugin_t* plugin) {
	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		if (plugin == track->eqinplugin || plugin == track->eqoutplugin || plugin == track->connplugin) return true;
		for (std::vector<zzub_plugin_t*>::iterator j = track->inconnplugins.begin(); j != track->inconnplugins.end(); ++j) {
			if (plugin == *j) return true;
		}
		for (int j = 0; j < track->knobcount; j++) {
			if (track->knobs[j].plugin == plugin) return true;
		}
	}
	return false;
}

bool CMixerView::IsCompatiblePlugin(zzub_plugin_t* plugin) {
	std::vector<std::string> pluginFilters;
	GetPluginFilter(pluginFilters);

	return filterPlugin(zzub_plugin_get_name(plugin), pluginFilters);
}

LRESULT CMixerView::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	ScreenToClient(&pt);
	int trackindex = pt.x / trackwidth;
	if (trackindex < 0) trackindex = -1;
	if (trackindex >= (int)tracks.size()) trackindex = -1;

	selectedtrack = trackindex;

	CMenu menu; 
	menu.CreatePopupMenu();

	CMenu inputMenu;
	inputMenu.CreatePopupMenu();

	CMenu outputMenu;
	outputMenu.CreatePopupMenu();

	CMenu trackMenu;
	trackMenu.CreatePopupMenu();

	int inputcounter = 0;
	int outputcounter = 0;
	int trackcounter = 0;

	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);

	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		unsigned int flags = zzub_plugin_get_flags(plugin);

		if (trackindex != -1) {
			if (flags & zzub_plugin_flag_has_audio_input) {
				zzub_connection_t* conn;
				if (tracks[selectedtrack]->outplugin != 0)
					conn = zzub_plugin_get_input_connection_by_type(plugin, tracks[selectedtrack]->eqoutplugin, zzub_connection_type_audio);
				else
					conn = 0;
				outputMenu.AppendMenu(MF_STRING|(conn != 0 ? MF_CHECKED : 0), (UINT_PTR)ID_SELECT_OUTPUT_FIRST + outputcounter, zzub_plugin_get_name(plugin));
				outputcounter ++;
			}

			if (flags & zzub_plugin_flag_has_audio_output) {
				zzub_connection_t* conn = zzub_plugin_get_input_connection_by_type(tracks[selectedtrack]->eqinplugin, plugin, zzub_connection_type_audio);
				inputMenu.AppendMenu(MF_STRING|(conn != 0 ? MF_CHECKED : 0), (UINT_PTR)ID_SELECT_INPUT_FIRST + inputcounter, zzub_plugin_get_name(plugin));
				inputcounter ++;
			}
		}

		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	bool isgroup = selectedtrack != -1 && tracks[selectedtrack]->eqinplugin != tracks[selectedtrack]->eqoutplugin;

	menu.AppendMenu(MF_POPUP|(trackcounter==0?MF_GRAYED:0), (UINT_PTR)trackMenu.m_hMenu, "Add Plugin To Strip");
	menu.AppendMenu(MF_STRING|(trackindex==-1||isgroup?MF_GRAYED:0), (UINT_PTR)ID_SHOW_EQ_PARAMETERS, "EQ Parameters");
	menu.AppendMenu(MF_STRING|(trackindex==-1?MF_GRAYED:0), (UINT_PTR)ID_SHOW_CONNECTION_PARAMETERS, "Output Parameters");
	menu.AppendMenu(MF_POPUP|(trackindex==-1?MF_GRAYED:0), (UINT_PTR)inputMenu.m_hMenu, "Select Track Input(s)");
	menu.AppendMenu(MF_POPUP|(trackindex==-1?MF_GRAYED:0), (UINT_PTR)outputMenu.m_hMenu, "Select Track Output");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING|(linked?MF_CHECKED:0), ID_LINKED_AMP, "Linked");

	ClientToScreen(&pt);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	return 0;
}

void CMixerView::ResetState() {
	selectedtrack = -1;
	dirtystate = true;
	Invalidate(FALSE);
}

bool CMixerView::GetAmpParameterRect(zzub_plugin_t* plugin, int group, int column, RECT* result) {
	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		if (plugin == track->connplugin && group == 2) {
			UnionRect(result, &track->rcAmps[0], &track->rcAmps[1]);
			return true;
		}
	}
	return false;
}

bool CMixerView::GetEqParameterRect(zzub_plugin_t* plugin, int group, int column, RECT* result) {
	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		for (int j = 0; j < track->knobcount; j++) {
			CMixerKnob& mixknob = track->knobs[j];
			if (mixknob.plugin == plugin && mixknob.group == group && mixknob.column == column) {
				*result = track->knobs[j].rcKnob;
				return true;
			}
		}
	}
	return false;
}

bool CMixerView::GetInputParameterRect(zzub_plugin_t* plugin, int group, int column, RECT* result) {
	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		
		for (std::vector<zzub_plugin_t*>::iterator j = track->inconnplugins.begin(); j != track->inconnplugins.end(); ++j) {
			int index = (int)std::distance(track->inconnplugins.begin(), j);
			
			if (*j == plugin) {
				*result = track->rcInputs[index];
				return true;
			}
		}
	}
	return false;
}

void CMixerView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	zzub_event_data_t* zzubdata = (zzub_event_data_t*)pHint;
	RECT rcParameter;
	switch (lHint) {
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_new_document:
		case zzub_event_type_update_song:
			pluginFilter.m_edit.SetWindowText(info->plugin_filter.c_str());
			parameterFilter.m_edit.SetWindowText(info->parameter_filter.c_str());
			ResetState();
			break;
		case zzub_event_type_insert_connection:
			if (IsTrackPlugin(zzubdata->insert_connection.from_plugin) ||
				IsTrackPlugin(zzubdata->insert_connection.to_plugin)) 
			{
				ResetState();
			}
			break;
		case zzub_event_type_delete_connection:
			if (IsTrackPlugin(zzubdata->delete_connection.from_plugin) ||
				IsTrackPlugin(zzubdata->delete_connection.to_plugin)) 
			{
				ResetState();
			}
			break;
		case zzub_event_type_insert_plugin:
			if (IsCompatiblePlugin(zzubdata->insert_plugin.plugin)) {
				ResetState();
			}
			break;
		case zzub_event_type_delete_plugin:
			if (IsTrackPlugin(zzubdata->delete_plugin.plugin)) {
				ResetState();
			}
			break;
		case zzub_event_type_update_pluginparameter:
			if (GetAmpParameterRect(zzubdata->update_pluginparameter.plugin, zzubdata->update_pluginparameter.group, zzubdata->update_pluginparameter.param, &rcParameter)) {
				InvalidateRect(&rcParameter, FALSE);
			} else if (GetEqParameterRect(zzubdata->update_pluginparameter.plugin, zzubdata->update_pluginparameter.group, zzubdata->update_pluginparameter.param, &rcParameter)) {
				InvalidateRect(&rcParameter, FALSE);
			} else if (GetInputParameterRect(zzubdata->update_pluginparameter.plugin, zzubdata->update_pluginparameter.group, zzubdata->update_pluginparameter.param, &rcParameter)) {
				InvalidateRect(&rcParameter, FALSE);
			}
			break;
	}
}

void CMixerView::UpdateTimer(int count) {
	if (dirtystate) return ;

	if (count % 2 != 0) return; // skip every 2nd frame

	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		int index = (int)std::distance(tracks.begin(), i);
		float l = zzub_plugin_get_last_peak((*i)->eqoutplugin, 0);
		float r = zzub_plugin_get_last_peak((*i)->eqoutplugin, 1);

		if (l != (*i)->lastpeak[0]) {
			(*i)->lastpeak[0] = l;
			InvalidateRect(&(*i)->rcAmps[0], FALSE);
		}
		if (r != (*i)->lastpeak[1]) {
			(*i)->lastpeak[1] = r;
			InvalidateRect(&(*i)->rcAmps[1], FALSE);
		}
	}	
}

LRESULT CMixerView::OnSelectInput(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	// (dis)connect input to eqplugin 
	if (selectedtrack == -1) return 0;

	CMixerTrack* track = tracks[selectedtrack];

	int inputindex = wID - ID_SELECT_INPUT_FIRST;

	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);

	int counter = 0;
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		int flags = zzub_plugin_get_flags(plugin);
		if (flags & zzub_plugin_flag_has_audio_output) {
			
			if (counter == inputindex) {

				zzub_connection_t* conn = zzub_plugin_get_input_connection_by_type(track->eqinplugin, plugin, zzub_connection_type_audio);

				if (conn == 0) {
					zzub_plugin_create_audio_connection(track->eqinplugin, plugin, 0, 2, -1, -1);
				} else {
					zzub_connection_destroy(conn);
				}
				zzub_player_history_commit(player, 0, 0, "Set Mixer Track Input");

				ResetState();
				break;
			}
			counter ++;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	return 0;
}


LRESULT CMixerView::OnSelectOutput(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	// (dis)connect input to eqplugin 
	if (selectedtrack == -1) return 0;

	CMixerTrack* track = tracks[selectedtrack];

	int outputindex = wID - ID_SELECT_OUTPUT_FIRST;

	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);

	int counter = 0;
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		int flags = zzub_plugin_get_flags(plugin);
		if (flags & zzub_plugin_flag_has_audio_input) {
			//zzub_connection_t* conn = zzub_plugin_get_input_connection_by_type(plugin, tracks[selectedtrack]->outplugin, zzub_connection_type_audio);
			
			if (counter == outputindex) {
				// we have the plugin - disconnect old connect new
				if (track->outconnection != 0)
					zzub_connection_destroy(track->outconnection);
				zzub_plugin_create_audio_connection(plugin, track->eqoutplugin, 0, 2, -1, -1);
				zzub_player_history_commit(player, 0, 0, "Set Mixer Track Output");

				ResetState();
				break;
			}

			counter ++;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	return 0;
}

LRESULT CMixerView::OnEqParameters(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (selectedtrack == -1) return 0;
	 
	buze_main_frame_show_plugin_parameters(mainframe, tracks[selectedtrack]->eqinplugin, parametermode_default, -1, -1);
	return 0;
}

LRESULT CMixerView::OnConnectionParameters(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (selectedtrack == -1) return 0;
	if (tracks[selectedtrack]->connplugin == 0) return 0;

	buze_main_frame_show_plugin_parameters(mainframe, tracks[selectedtrack]->connplugin, parametermode_default, -1, -1);
	return 0;
}

LRESULT CMixerView::OnLinkedAmp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	linked = !linked;
	return 0;
}

LRESULT CMixerView::OnPluginFilterChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	char pcText[1024];
	pluginFilter.m_edit.GetWindowText(pcText, 1024);

	info->plugin_filter = pcText;

	ResetState();
	return 0;
}

LRESULT CMixerView::OnParameterFilterChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	char pcText[1024];
	parameterFilter.m_edit.GetWindowText(pcText, 1024);

	info->parameter_filter = pcText;

	ResetState();
	return 0;
}

HWND CMixerView::GetHwnd() {
	return m_hWnd;
}

bool CMixerView::DoesKeyjazz() {
	return true;
}

void CMixerView::InsertPluginKnobs(CMixerTrack* track, zzub_plugin_t* eqplugin, std::vector<std::string>& paramFilters) {

	for (int j = 0; j < zzub_plugin_get_parameter_count(eqplugin, 1, 0); j++) {
		if (visibleParameter(eqplugin, 1, j, paramFilters)) {
			int knobindex = track->knobcount;
			if (knobindex >= 100)
				break;
			track->knobs[knobindex].plugin = eqplugin;
			track->knobs[knobindex].group = 1;
			track->knobs[knobindex].column = j;
			track->knobcount++;
		}
	}

}

void CMixerView::InsertPluginTrack(zzub_plugin_t* eqplugin) {
	// find first (and usually only)
	CMixerTrack* track = new CMixerTrack();
	track->name = zzub_plugin_get_name(eqplugin);
	track->eqinplugin = eqplugin;
	track->eqoutplugin = eqplugin;
	track->outplugin = 0;
	track->outconnection = 0;
	track->connplugin = 0;
	track->knobcount = 0;

	std::vector<std::string> paramFilters;
	// GetParameterFilter(paramFilters);

	InsertPluginKnobs(track, eqplugin, paramFilters);

	InitTrack(track);

	tracks.push_back(track);
}

// dupe from MachineView/MachineHelpers.cpp
zzub_plugin_t* zzub_plugin_group_get_input_plugin(zzub_plugin_group_t* layer) {
	zzub_plugin_t* result = 0;
	zzub_plugin_iterator_t* plugit = zzub_plugin_group_get_plugins(layer);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		if (zzub_plugin_get_flags(plugin) & zzub_plugin_flag_has_group_input) {
			result = plugin;
			break;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);
	return result;
}

// dupe from MachineView/MachineHelpers.cpp
zzub_plugin_t* zzub_plugin_group_get_output_plugin(zzub_plugin_group_t* layer) {
	zzub_plugin_t* result = 0;
	zzub_plugin_iterator_t* plugit = zzub_plugin_group_get_plugins(layer);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		if (zzub_plugin_get_flags(plugin) & zzub_plugin_flag_has_group_output) {
			result = plugin;
			break;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);
	return result;
}

void CMixerView::InsertPluginGroupTrack(zzub_plugin_group_t* eqplugingroup) {
	CMixerTrack* track = new CMixerTrack();
	track->name = zzub_plugin_group_get_name(eqplugingroup);
	track->eqinplugin = zzub_plugin_group_get_input_plugin(eqplugingroup);
	track->eqoutplugin = zzub_plugin_group_get_output_plugin(eqplugingroup);
	track->outplugin = 0;
	track->outconnection = 0;
	track->connplugin = 0;
	track->knobcount = 0;

	std::vector<std::string> paramFilters;
	GetParameterFilter(paramFilters);

	// add filtered knobs for all plugins inside the plugingroup
	zzub_plugin_iterator_t* plugit = zzub_plugin_group_get_plugins(eqplugingroup);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);

		unsigned int flags = zzub_plugin_get_flags(plugin);

		// hide master and group plugins
		const unsigned int pluginmask = zzub_plugin_flag_has_audio_input|zzub_plugin_flag_has_audio_output|zzub_plugin_flag_is_root|zzub_plugin_flag_has_group_input|zzub_plugin_flag_has_group_output;
		const unsigned int pluginfilter = zzub_plugin_flag_has_audio_input|zzub_plugin_flag_has_audio_output;
		if ((flags & pluginmask) == pluginfilter) {
			InsertPluginKnobs(track, plugin, paramFilters);
		}

		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	InitTrack(track);

	tracks.push_back(track);
}

void CMixerView::InitTrack(CMixerTrack* track) {

	for (int i = 0; i < zzub_plugin_get_output_connection_count(track->eqoutplugin); i++) {
		zzub_connection_t* conn = zzub_plugin_get_output_connection(track->eqoutplugin, i);
		if (zzub_connection_get_type(conn) == zzub_connection_type_audio) {
			track->outplugin = zzub_connection_get_to_plugin(conn);
			track->connplugin = zzub_connection_get_connection_plugin(conn);
			track->outconnection = conn;
			break;
		}
	}

	for (int i = 0; i < zzub_plugin_get_input_connection_count(track->eqinplugin); i++) {
		zzub_connection_t* conn = zzub_plugin_get_input_connection(track->eqinplugin, i);
		if (zzub_connection_get_type(conn) == zzub_connection_type_audio) {
			track->inplugins.push_back(zzub_connection_get_from_plugin(conn));
			track->inconnplugins.push_back(zzub_connection_get_connection_plugin(conn));
		}
	}

}

void CMixerView::ClearTracks() {
	for (std::vector<CMixerTrack*>::iterator i = tracks.begin(); i != tracks.end(); ++i) {
		CMixerTrack* track = *i;
		delete track;
	}
	tracks.clear();
}

void CMixerView::RestoreState() {

	ClearTracks();

	std::vector<std::string> pluginFilter;
	GetPluginFilter(pluginFilter);

	// insert tracks for matching plugins
	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		unsigned int flags = zzub_plugin_get_flags(plugin);

		// hide master and group plugins
		const unsigned int pluginmask = zzub_plugin_flag_has_audio_input|zzub_plugin_flag_has_audio_output|zzub_plugin_flag_is_root|zzub_plugin_flag_has_group_input|zzub_plugin_flag_has_group_output;
		const unsigned int pluginfilter = zzub_plugin_flag_has_audio_input|zzub_plugin_flag_has_audio_output;
		if ((flags & pluginmask) == pluginfilter) {

			if (filterPlugin(zzub_plugin_get_name(plugin), pluginFilter))
				InsertPluginTrack(plugin);
		}

		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	// insert tracks for matching groups
	zzub_plugin_group_iterator_t* groupit = zzub_player_get_plugin_group_iterator(player, 0);

	while (zzub_plugin_group_iterator_valid(groupit)) {
		zzub_plugin_group_t* plugingroup = zzub_plugin_group_iterator_current(groupit);
		if (filterPlugin(zzub_plugin_group_get_name(plugingroup), pluginFilter))
			InsertPluginGroupTrack(plugingroup);

		zzub_plugin_group_iterator_next(groupit);
	}
	zzub_plugin_group_iterator_destroy(groupit);

	UpdateLayout();
}


void CMixerView::GetPluginFilter(std::vector<std::string>& result) {
	std::string filter = info->plugin_filter;
	transform(filter.begin(), filter.end(), filter.begin(), (int(*)(int))std::tolower);
	split(filter, result, " ");
}

void CMixerView::GetParameterFilter(std::vector<std::string>& result) {
	std::string filter = info->parameter_filter;
	transform(filter.begin(), filter.end(), filter.begin(), (int(*)(int))std::tolower);
	split(filter, result, " ");
}
