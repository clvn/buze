# this can be used to convert string representation of notes to the 
# integer representation used internally by buze.
# - to note: is for single notes
# - from_chord: uses multiple calls to to_note to return a list of ints.
# Original by calvin, modified by zeffii.

def to_note(s):
    if s == "off":
        return 255
    if s == "cut":
        return 254

    try:
        octave = int(s[2:])
    except:
        return -1

    notepart = s[:2]
    notevalue = -1

    notes = [ "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" ]
    notes2 = [ "C-", "C#", "D-", "Eb", "E-", "F-", "F#", "G-", "Ab", "A-", "Bb", "B-" ]

    if notepart in notes2:
        notes = notes2
        
    if notepart not in notes:
        return -1

    for index, item in enumerate(notes):
        if item == notepart:
            notevalue = index
            break

    if notevalue == -1 or octave == -1:
        return -1

    return (notevalue + octave * 16) + 1


def from_chord(notes):
    note_list = []
    for note in notes:
        note_list.append(to_note(note))
    return note_list

  