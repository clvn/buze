# Input format:
# | Frame | Freq Note/Abs WF ADSR Pul | Freq Note/Abs WF ADSR Pul | Freq Note/Abs WF ADSR Pul | FCut RC Typ V |
#|     0 | 0000  ... ..  00 0000 000 | 0000  ... ..  00 0000 000 | 0000  ... ..  00 0000 000 | 0000 00 Off F |
#|     1 | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | .... .. ... . |
#|     2 | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | .... .. ... . |
#|     3 | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | .... .. ... . |
#|     4 | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | .... .. ... . |
#|     5 | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | ....  ... ..  .. .... ... | .... .. ... . |
#|     6 | 0E18  G#3 AC  11 10C0 ... | 151F  D#4 B3  11 10C0 ... | 2187  B-4 BB  21 192C ... | .... .. ... . |

# issues:
#   - siddump wouldnt dump gate-changes per row, so it was slightly modified
#   - we now add a note-off on the previous row if there was a gate change/new-note
#     but there should ideally be a "trigger"-param or "slide" or something (fx 3?)


import sys

template_begin = """<?xml version="1.0" encoding="utf-8"?>
<xmix xmlns:xmix="http://www.zzub.org/ccm/xmix">
	<transport bpm="188" tpb="16" loopstart="0" loopend="3000" start="0" end="3000" />
	<pluginclasses>
		<pluginclass id="@zzub.org/master" is_root="true" has_audio_input="true" has_audio_output="true">
			<parameters>
				<global>
					<parameter id="1aa8098" name="Volume" type="word" minvalue="0" maxvalue="16384" novalue="65535" defvalue="0" state="true" index="0" />
					<parameter id="1aa80c0" name="BPM" type="word" minvalue="16" maxvalue="512" novalue="65535" defvalue="126" state="true" index="1" />
					<parameter id="1aa80f8" name="TPB" type="byte" minvalue="1" maxvalue="32" novalue="255" defvalue="4" state="true" index="2" />
				</global>
			</parameters>
		</pluginclass>
		<pluginclass id="@zzub.org/lunar/sid;1" has_audio_output="true">
			<data type="raw" base="manifest.xml" src="@zzub.org/lunar/sid;1/manifest.xml" />
			<data type="raw" base="sid.cpp" src="@zzub.org/lunar/sid;1/sid.cpp" />
			<data type="raw" base="sid.h" src="@zzub.org/lunar/sid;1/sid.h" />
			<data type="raw" base="sidemu.cc" src="@zzub.org/lunar/sid;1/sidemu.cc" />
			<data type="raw" base="sidemu.h" src="@zzub.org/lunar/sid;1/sidemu.h" />
			<parameters>
				<global>
					<parameter id="cutoff" name="Cutoff" type="word" minvalue="0" maxvalue="2047" novalue="65535" defvalue="1024" state="true" index="0" />
					<parameter id="reso" name="Resonance" type="byte" minvalue="0" maxvalue="15" novalue="255" defvalue="2" state="true" index="1" />
					<parameter id="filtmode" name="Filter Mode" type="byte" minvalue="0" maxvalue="3" novalue="255" defvalue="2" state="true" index="2" />
					<parameter id="vol" name="Volume" type="byte" minvalue="0" maxvalue="15" novalue="255" defvalue="15" state="true" index="3" />
				</global>
				<track>
					<parameter id="note" name="Note" type="note16" minvalue="1" maxvalue="156" novalue="0" defvalue="0" index="0" />
					<parameter id="effect" name="Effect" type="byte" minvalue="1" maxvalue="255" defvalue="0" novalue="0" index="1" />
					<parameter id="effectvalue" name="Value" type="byte" minvalue="1" maxvalue="255" defvalue="0" novalue="0" index="2" />
					<parameter id="pw" name="Pulse Width" type="word" minvalue="0" maxvalue="4095" novalue="65535" defvalue="2048" state="true" index="3" />
					<parameter id="wf" name="Waveform" type="byte" minvalue="0" maxvalue="3" novalue="255" defvalue="2" state="true" index="4" />
					<parameter id="filter" name="Filter Enable" type="switch" minvalue="0" maxvalue="1" novalue="255" defvalue="0" state="true" index="5" />
					<parameter id="ringmod" name="Ringmod" type="switch" minvalue="0" maxvalue="1" novalue="255" defvalue="0" state="true" index="6" />
					<parameter id="sync" name="Sync" type="switch" minvalue="0" maxvalue="1" novalue="255" defvalue="0" state="true" index="7" />
					<parameter id="attack" name="Attack" type="byte" minvalue="0" maxvalue="15" novalue="255" defvalue="2" state="true" index="8" />
					<parameter id="decay" name="Decay" type="byte" minvalue="0" maxvalue="15" novalue="255" defvalue="2" state="true" index="9" />
					<parameter id="sustain" name="Sustain" type="byte" minvalue="0" maxvalue="15" novalue="255" defvalue="10" state="true" index="10" />
					<parameter id="release" name="Release" type="byte" minvalue="0" maxvalue="15" novalue="255" defvalue="5" state="true" index="11" />
				</track>
			</parameters>
		</pluginclass>
	</pluginclasses>
	<plugins>
		<plugin id="0" name="Master" ref="@zzub.org/master">
			<position x="0" y="0" />
			<connections>
				<input id="18054100" ref="1" type="audio" amplitude="1" panning="0" />
			</connections>
			<init>
				<global>
					<n ref="1aa8098" v="0" />
					<n ref="1aa80c0" v="188" />
					<n ref="1aa80f8" v="16" />
				</global>
			</init>
			<midi />
			<sequences />
		</plugin>
		<plugin id="1" name="SID" ref="@zzub.org/lunar/sid;1">
			<position x="-0.364393" y="-0.459091" />
			<init>
				<global>
					<n ref="cutoff" v="1024" />
					<n ref="reso" v="2" />
					<n ref="filtmode" v="2" />
					<n ref="vol" v="15" />
				</global>
				<tracks>
					<track index="0">
						<n ref="note" v="83" />
						<n ref="pw" v="1609" />
						<n ref="wf" v="2" />
						<n ref="filter" v="0" />
						<n ref="ringmod" v="0" />
						<n ref="sync" v="0" />
						<n ref="attack" v="2" />
						<n ref="decay" v="2" />
						<n ref="sustain" v="10" />
						<n ref="release" v="0" />
					</track>
					<track index="1">
						<n ref="note" v="83" />
						<n ref="pw" v="1609" />
						<n ref="wf" v="2" />
						<n ref="filter" v="0" />
						<n ref="ringmod" v="0" />
						<n ref="sync" v="0" />
						<n ref="attack" v="2" />
						<n ref="decay" v="2" />
						<n ref="sustain" v="10" />
						<n ref="release" v="0" />
					</track>
					<track index="2">
						<n ref="note" v="83" />
						<n ref="pw" v="1609" />
						<n ref="wf" v="2" />
						<n ref="filter" v="0" />
						<n ref="ringmod" v="0" />
						<n ref="sync" v="0" />
						<n ref="attack" v="2" />
						<n ref="decay" v="2" />
						<n ref="sustain" v="10" />
						<n ref="release" v="0" />
					</track>
				</tracks>
			</init>
			<midi />
			<eventtracks>
				<events id="180514a8" name="00" length="188">"""
template_end = """
					<!--g>
						<e t="0" ref="171ac6b8" v="15" />
						<e t="0.5" ref="171ac6b8" v="2" />
						<e t="0.75" ref="171ac6b8" v="5" />
						<e t="1" ref="171ac6b8" v="15" />
						<e t="1.5" ref="171ac6b8" v="15" />
						<e t="2" ref="171ac6b8" v="15" />
						<e t="2.25" ref="171ac6b8" v="4" />
						<e t="2.5" ref="171ac6b8" v="15" />
						<e t="2.75" ref="171ac6b8" v="3" />
						<e t="3" ref="171ac6b8" v="15" />
					</g>
					<t index="0">
						<e t="0" ref="171ac7c0" v="65" />
						<e t="0" ref="171ac8b0" v="41" />
						<e t="0.25" ref="171ac8b0" v="35" />
						<e t="0.5" ref="171ac7c0" v="81" />
						<e t="0.5" ref="171ac8b0" v="190" />
						<e t="1" ref="171ac7c0" v="84" />
						<e t="1" ref="171ac8b0" v="737" />
						<e t="1.25" ref="171ac8b0" v="1388" />
						<e t="1.5" ref="171ac7c0" v="86" />
						<e t="1.5" ref="171ac8b0" v="1238" />
						<e t="2" ref="171ac7c0" v="84" />
						<e t="2" ref="171ac8b0" v="338" />
						<e t="2.25" ref="171ac8b0" v="1936" />
						<e t="2.5" ref="171ac7c0" v="83" />
						<e t="2.5" ref="171ac8b0" v="1609" />
						<e t="3" ref="171ac7c0" v="81" />
						<e t="3" ref="171ac8b0" v="753" />
						<e t="3.25" ref="171ac8b0" v="443" />
						<e t="3.5" ref="171ac7c0" v="255" />
					</t-->
				</events>
			</eventtracks>
			<sequences>
				<sequence index="0">
					<e t="0" ref="180514a8" />
				</sequence>
			</sequences>
		</plugin>
	</plugins>
	<instruments />
</xmix>"""

# a pattern is an array of four array, where each array contains list of changes in the respective track
# array0..2 = lunar SID track parameters
# array3 = lunar SID global parameters
# each change is an array of [frame, col, value ]
pattern = [ [], [], [], [] ]

# is there a built-in function to use instead of this?
def parse_int(v, base = 10):
	if type(v) == int: return v
	try:
		i = int(v, base)
	except ValueError:
		return -1
	except TypeError:
		return -1
	return i

def add_event(frame, track, col, value):
	ev = [ frame, col, value ]
	pattern[track].append(ev)
	#print "Event at " + str(frame) + ": " + str(track) + " " + str(col) + ": " + str(value)


def parse_note(note):
	notes = [ "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" ]
	index = 0
	for n in notes:
		if note[0:2] == n:
			oct = int(note[2:3])
			return index + oct * 16 + 1;
		index = index+1
	return -1;

for line in file("Compleeto.sid.txt",'r'):
	tracknumber = 0
	line = line.replace("  ", " ")	# remove double spaces
	line = line.replace("/", " ")		# fiks Note/Abs column so its split nicely for pretty printing, there are no other /'s anyway
	frame = -1
	for track in line.split('|'):
		s = track
		if tracknumber== 1:
			frame = parse_int(s.strip())
			if frame == -1: continue
			#print "Frame: '" + frame + "'"
		elif tracknumber >= 2 and tracknumber <= 4:		# voices
			voice = tracknumber - 2
			tok = 0
			row_has_note = False
			row_has_slide_up = False
			row_has_slide_down = False
			for cell in track.split(" "):
				if tok == 4:
					wfvalue = cell.strip()
					# NOTE: some of the wf bits are GATE, SYNC, RINGMOD, TEST
					if wfvalue != "..":
						# hex2int
						wfvalue = parse_int("0x" + wfvalue, 16);
						wf = wfvalue >> 4;
						if wf & 0x1: 
							wf = 0
						elif wf & 0x2: 
							wf = 1
						elif wf & 0x4:
							wf = 2
						elif wf & 0x8:
							wf = 3
						enabled = wfvalue & 1;
						#print ("WAT " + str(wfvalue))
						add_event(frame, voice, 4, wf)
						
#						if not enabled and not row_has_note:
#							add_event(frame-1, voice, 0, 0)	# note off
							#print "noteab"
						#print "Chjanged waveform"
				elif tok == 2:
					freqvalue = cell.strip()
					if freqvalue != "...":
						if freqvalue[0] == '(' and freqvalue[1] == '+':
							#print "Skippoing pitch up" + freqvalue
							row_has_slide_up = True
						elif freqvalue[0] == '(' and freqvalue[1] == '-':
							#print "Skippoing pitch down" + freqvalue
							row_has_slide_down = True
						else:
							if freqvalue[0] == '(':
								add_event(frame-1, voice, 0, 255)	# note off
								freqvalue = freqvalue[1:]
							note = parse_note(freqvalue)
							add_event(frame, voice, 0, note)
							row_has_note = True
						#print "Voice " + str(voice) + " Freq: '" + freqvalue + "'"
				elif tok == 3:
					slideval = cell.strip()
					slideval = parse_int("0x" + slideval[:len(slideval)-1], 16);
					if row_has_slide_up:
						add_event(frame, voice, 1, 0x01);
						add_event(frame, voice, 2, slideval);
						#print "Slide UP!" + str(slideval)
					elif row_has_slide_down:
						add_event(frame, voice, 1, 0x02);
						add_event(frame, voice, 2, slideval);
						#print "Slide DOWN!" + str(slideval)
				elif tok == 5:
					adsrvalue = cell.strip()
					if adsrvalue != "....":
						attack = parse_int("0x" + adsrvalue[0:1], 16)
						decay = parse_int("0x" + adsrvalue[1:2], 16)
						sustain = parse_int("0x" + adsrvalue[2:3], 16)
						release = parse_int("0x" + adsrvalue[3:4], 16)
						add_event(frame, voice, 8, attack)
						add_event(frame, voice, 9, decay)
						add_event(frame, voice, 10, sustain)
						add_event(frame, voice, 11, release)
						# WE NEED TO CHECK PREV VALUES!
						#print "ADHD"
				elif tok == 6:
					pw = cell.strip()
					if pw != "...":
						pw = parse_int("0x" + pw, 16)
						if pw == -1: continue
						add_event(frame, voice, 3, pw)

				tok = tok+1
		elif tracknumber == 5:
			tok = 0
			for cell in track.split(" "):
				if tok == 1:
					fcvalue = cell.strip()
					if fcvalue != "....":
						fcvalue = parse_int("0x" + fcvalue, 16) >> 5
						add_event(frame, 3, 0, fcvalue)
						# print "CUTOFF!" + str(fcvalue)
						#fclo = fcvalue & 0x3f # 1+2+4+8+16+32
						#fchi = (fcvalue >> 3) & 0xFF;
						#print "%i, %i" % (fclo, fchi)
				elif tok == 2:
					rcvalue = cell.strip()
					if rcvalue != "..":
						rcvalue = parse_int("0x" + rcvalue, 16);
						res = rcvalue >> 4
						filt = rcvalue & 0xF
						add_event(frame, 3, 1, res)

						# our sid plugin has a enable-filter-param per track instead of using teh global flags as on the chip
						if filt & 0x1:
							add_event(frame, 0, 5, 1)
						else:
							add_event(frame, 0, 5, 0)

						if filt & 0x2:
							add_event(frame, 1, 5, 1)
						else:
							add_event(frame, 1, 5, 0)

						if filt & 0x4:
							add_event(frame, 2, 5, 1)
						else:
							add_event(frame, 2, 5, 0)

						#add_event(frame, 3, 2, filt)
						#print "rc %i, res %i, filt %i" % (rcvalue, res, filt)
				elif tok == 3:
					ftype = cell.strip()
					if ftype == "Off":
						add_event(frame, 3, 2, 0)
					elif ftype == "Low":
						add_event(frame, 3, 2, 1)
					elif ftype == "Bnd":
						add_event(frame, 3, 2, 2)
					elif ftype == "L+B":
						add_event(frame, 3, 2, 1)	# param kludge, default to lowpass
					elif ftype == "Hi":
						add_event(frame, 3, 2, 3)
					elif ftype == "L+H":
						add_event(frame, 3, 2, 1)	# param kludge, default to lowpass
					elif ftype == "B+H":
						add_event(frame, 3, 2, 1)	# param kludge, default to lowpass
					elif ftype == "LBH":
						add_event(frame, 3, 2, 1)	# param kludge, default to lowpass
				elif tok == 4:
					vvalue = cell.strip();
					if vvalue != ".":
						vvalue = parse_int("0x" + cell.strip(), 16)
						if vvalue != -1:
							add_event(frame, 3, 3, vvalue)
						#print "volume %i, %s" % (vvalue, cell)
				tok = tok + 1
		tracknumber = tracknumber + 1

globalpararefs = ["cutoff", "reso", "filtmode", "vol" ]
trackpararefs = ["note", "effect", "effectvalue", "pw", "wf", "filter", "ringmod", "sync", "attack", "decay", "sustain", "release"];

def print_events(track, pararefs):
	for ev in track:
		frame = float(ev[0]) / 16;
		colnum = parse_int(ev[1])
		if colnum == -1: 
			#print "Ignored invalid column " + ev[1]
			continue
		col = pararefs[colnum];
		print "						<e t=\"" + str("%f" % frame) + "\" ref=\"" + col + "\" v=\"" + str(ev[2]) + "\"/>"

tracknumber = 0

print template_begin

for track in pattern:
	if tracknumber >= 0 and tracknumber <= 2:
		print "					<t index=\"" + str(tracknumber) + "\">"
		print_events(track, trackpararefs)
		print "					</t>"
	elif tracknumber == 3:
		print "					<g>"
		print_events(track, globalpararefs)
		print "					</g>"
	#print len(track)
	tracknumber = tracknumber + 1

print template_end
