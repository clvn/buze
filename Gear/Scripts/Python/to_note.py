def to_note(s):
	notes = [ "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" ]

	if s == "off":
		return 255
	if s == "cut":
		return 254

	try:
		octave = int(s[2:])
		notevalue = notes.index(s[:2])
	except:
		return -1

	return notevalue + octave * 16

print(to_note("F#4"))
print(to_note("C#4"))
