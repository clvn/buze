#pragma once

#include <string>
using std::string;
#include <sstream>
using std::stringstream;
#include <vector>
using std::vector;
#include <map>
using std::map;
#include "boost/unordered_map.hpp"
using boost::unordered_map;
#include "boost/array.hpp"
using boost::array;
#include "boost/function.hpp"
using boost::function2;
#include <algorithm>
using std::rotate;
#include <cassert>

#include "Utils.h"
///

template <class X, size_t N> char (&array_size_helper(X(&)[N]))[N];
#define array_size(a) (sizeof(array_size_helper(a)))

// ---------------------------------------------------------------------------------------------------------------

namespace Harmony {

// ---------------------------------------------------------------------------------------------------------------
// H-SYSTEM
// ---------------------------------------------------------------------------------------------------------------

enum Natural {
	Cn, Dn, En, Fn, Gn, An, Bn,
};

static array<int, 14> const majorsteps = {
//	1	2	3	4	5	6	7	// Deg
	0,  2,  4,  5,  7,  9,  11,	// Oct
	12, 14, 16, 17, 19, 21, 23,	// Oct+1
};

enum Note {
	Z_ = -1,
	Cd, Cb, C_, Cs, Cx,
	Dd, Db, D_, Ds, Dx,
	Ed, Eb, E_, Es, Ex,
	Fd, Fb, F_, Fs, Fx,
	Gd, Gb, G_, Gs, Gx,
	Ad, Ab, A_, As, Ax,
	Bd, Bb, B_, Bs, Bx,
};

struct MetaInfo {
	Natural natural;
	int sign;
};

static MetaInfo const metainfos[12][3] = {	// NM	0	1	2
	{ { Cn,  0 }, { Dn, -2 }, { Bn, +1 } },	// 0	C	Dd	B#
	{ { Cn, +1 }, { Dn, -1 }, { Bn, +2 } },	// 1	C#	Db	Bx
	{ { Dn,  0 }, { En, -2 }, { Cn, +2 } },	// 2	D	Ed	Cx
	{ { En, -1 }, { Fn, -2 }, { Dn, +1 } },	// 3	Eb	Fd	D#
	{ { En,  0 }, { Fn, -1 }, { Dn, +2 } },	// 4	E	Fb	Dx
	{ { Fn,  0 }, { Gn, -2 }, { En, +1 } },	// 5	F	Gd	E#
	{ { Fn, +1 }, { Gn, -1 }, { En, +2 } },	// 6	F#	Gb	Ex
	{ { Gn,  0 }, { An, -2 }, { Fn, +2 } },	// 7	G	Ad	Fx
	{ { An, -1 }, { Bn, -3 }, { Gn, +1 } },	// 8	Ab	--	G#
	{ { An,  0 }, { Bn, -2 }, { Gn, +2 } },	// 9	A	Bd	Gx
	{ { Bn, -1 }, { Cn, -2 }, { An, +1 } },	// 10	Bb	Cd	A#
	{ { Bn,  0 }, { Cn, -1 }, { An, +2 } },	// 11	B	Cb	Ax
};

struct NoteInfo {
	char const* name;
	Note note;
	Natural natural;
	int adjust;
	int step;
	int meta;
};

static NoteInfo const noteinfos[35] = {
	{ "Cd", Cd, Cn, -2, 10, 1 },
	{ "Cb", Cb, Cn, -1, 11, 1 },
	{ "C" , C_, Cn,  0, 0 , 0 },
	{ "C#", Cs, Cn, +1, 1 , 0 },
	{ "Cx", Cx, Cn, +2, 2 , 2 },
	{ "Dd", Dd, Dn, -2, 0 , 1 },
	{ "Db", Db, Dn, -1, 1 , 1 },
	{ "D" , D_, Dn,  0, 2 , 0 },
	{ "D#", Ds, Dn, +1, 3 , 2 },
	{ "Dx", Dx, Dn, +2, 4 , 2 },
	{ "Ed", Ed, En, -2, 2 , 1 },
	{ "Eb", Eb, En, -1, 3 , 0 },
	{ "E" , E_, En,  0, 4 , 0 },
	{ "E#", Es, En, +1, 5 , 2 },
	{ "Ex", Ex, En, +1, 6 , 2 },
	{ "Fd", Fd, Fn, -2, 3 , 1 },
	{ "Fb", Fb, Fn, -1, 4 , 1 },
	{ "F" , F_, Fn,  0, 5 , 0 },
	{ "F#", Fs, Fn, +1, 6 , 0 },
	{ "Fx", Fx, Fn, +2, 7 , 2 },
	{ "Gd", Gd, Gn, -2, 5 , 1 },
	{ "Gb", Gb, Gn, -1, 6 , 1 },
	{ "G" , G_, Gn,  0, 7 , 0 },
	{ "G#", Gs, Gn, +1, 8 , 2 },
	{ "Gx", Gx, Gn, +2, 9 , 2 },
	{ "Ad", Ad, An, -2, 7 , 1 },
	{ "Ab", Ab, An, -1, 8 , 0 },
	{ "A" , A_, An,  0, 9 , 0 },
	{ "A#", As, An, +1, 10, 2 },
	{ "Ax", Ax, An, +2, 11, 2 },
	{ "Bd", Bd, Bn, -2, 9 , 1 },
	{ "Bb", Bb, Bn, -1, 10, 0 },
	{ "B" , B_, Bn,  0, 11, 0 },
	{ "B#", Bs, Bn, +1, 0 , 2 },
	{ "Bx", Bx, Bn, +2, 1 , 2 },
};

static NoteInfo const (&noteinfos_grouped)[7][5] = *(NoteInfo(*)[7][5])noteinfos;

enum Degree {
	_Z = 0,
	d1, b1, _1, s1, x1,
	d2, b2, _2, s2, x2,
	d3, b3, _3, s3, x3,
	d4, b4, _4, s4, x4,
	d5, b5, _5, s5, x5,
	d6, b6, _6, s6, x6,
	d7, b7, _7, s7, x7,
};

struct DegreeInfo {
	char const* name;
	Degree degree;
	int interval;
	int adjust;
};

static DegreeInfo const degreeinfos[] = {
	{ "_Z", _Z, 1, 0  },
	{ "d1", d1, 1, -2 }, { "b1", b1, 1, -1 }, { "1" , _1, 1, 0  }, { "#1", s1, 1, +1 }, { "x1", x1, 1, +2 },
	{ "d2", d2, 2, -2 }, { "b2", b2, 2, -1 }, { "2" , _2, 2, 0  }, { "#2", s2, 2, +1 }, { "x2", x2, 2, +2 },
	{ "d3", d3, 3, -2 }, { "b3", b3, 3, -1 }, { "3" , _3, 3, 0  }, { "#3", s3, 3, +1 }, { "x3", x3, 3, +2 },
	{ "d4", d4, 4, -2 }, { "b4", b4, 4, -1 }, { "4" , _4, 4, 0  }, { "#4", s4, 4, +1 }, { "x4", x4, 4, +2 },
	{ "d5", d5, 5, -2 }, { "b5", b5, 5, -1 }, { "5" , _5, 5, 0  }, { "#5", s5, 5, +1 }, { "x5", x5, 5, +2 },
	{ "d6", d6, 6, -2 }, { "b6", b6, 6, -1 }, { "6" , _6, 6, 0  }, { "#6", s6, 6, +1 }, { "x6", x6, 6, +2 },
	{ "d7", d7, 7, -2 }, { "b7", b7, 7, -1 }, { "7" , _7, 7, 0  }, { "#7", s7, 7, +1 }, { "x7", x7, 7, +2 },
};

namespace Symbols {
	enum T {
		// Special
		NullSym = -1,
		Off,
		// Keys
		Cd, Cb, C_, Cs, Cx,
		Dd, Db, D_, Ds, Dx,
		Ed, Eb, E_, Es, Ex,
		Fd, Fb, F_, Fs, Fx,
		Gd, Gb, G_, Gs, Gx,
		Ad, Ab, A_, As, Ax,
		Bd, Bb, B_, Bs, Bx,
		cd, cb, c_, cs, cx,
		dd, db, d_, ds, dx,
		ed, eb, e_, es, ex,
		fd, fb, f_, fs, fx,
		gd, gb, g_, gs, gx,
		ad, ab, a_, as, ax,
		bd, bb, b_, bs, bx,
		// Key Transposers
		Cd_t, Cb_t, C__t, Cs_t, Cx_t,
		Dd_t, Db_t, D__t, Ds_t, Dx_t,
		Ed_t, Eb_t, E__t, Es_t, Ex_t,
		Fd_t, Fb_t, F__t, Fs_t, Fx_t,
		Gd_t, Gb_t, G__t, Gs_t, Gx_t,
		Ad_t, Ab_t, A__t, As_t, Ax_t,
		Bd_t, Bb_t, B__t, Bs_t, Bx_t,
		// Degree Transposers
		d1_t, b1_t, _1_t, s1_t, x1_t,
		d2_t, b2_t, _2_t, s2_t, x2_t,
		d3_t, b3_t, _3_t, s3_t, x3_t,
		d4_t, b4_t, _4_t, s4_t, x4_t,
		d5_t, b5_t, _5_t, s5_t, x5_t,
		d6_t, b6_t, _6_t, s6_t, x6_t,
		d7_t, b7_t, _7_t, s7_t, x7_t,
		d9_t, b9_t, _9_t, s9_t, x9_t,
		// Macros
		Ion, Dor, Phr, Lyd, Mix, Aeo, Loc,
		// Chromatic Chords
		I_c, bII_c, II_c, bIII_c, III_c, IV_c, sIV_c, V_c, bVI_c, VI_c, bVII_c, VII_c,
		i_c, bii_c, ii_c, biii_c, iii_c, iv_c, siv_c, v_c, bvi_c, vi_c, bvii_c, vii_c,
		// Diatonic Chords
		I_d, II_d, III_d, IV_d, V_d, VI_d, VII_d,
		// Harmonic Chords
		I_h, II_h, III_h, IV_h, V_h, VI_h, VII_h,
		i_h, ii_h, iii_h, iv_h, v_h, vi_h, vii_h,
		// Degree modifiers
		        _1,         no1,
		d2, b2, _2, s2, x2, no2,
		d3, b3, _3, s3, x3, no3,
		d4, b4, _4, s4, x4, no4,
		d5, b5, _5, s5, x5, no5,
		d6, b6, _6, s6, x6, no6,
		d7, b7, _7, s7, x7, no7,
		d9, b9, _9, s9, x9, no9,
		// Special Modifiers
		sus2, sus4, mode, root, none,
		// Rotations
		r1, r2, r3, r4, r5, r6, r7,
		// Bass
		bass1, bass2, bass3, bass4, bass5, bass6, bass7,
		// Passing Tones
		none_p,
		b1_p, s1_p,
		b2_p, s2_p,
		b3_p, s3_p,
		b4_p, s4_p,
		b5_p, s5_p,
		b6_p, s6_p,
		b7_p, s7_p,
		b9_p, s9_p,
		nob1_p, nos1_p,
		nob2_p, nos2_p,
		nob3_p, nos3_p,
		nob4_p, nos4_p,
		nob5_p, nos5_p,
		nob6_p, nos6_p,
		nob7_p, nos7_p,
		nob9_p, nos9_p,
	};
}

struct SymbolInfo {
	Symbols::T symbol;
	char const* name;
	Note change_key;
	Degree transpose_key;
	bool pre_reset_mode;
	int rotate_mode;
	bool transpose_mode;
	bool post_reset_mode;
	Degree modify_degrees[7];
	bool clear_active;
	int add_active[7];
	int remove_active[7];
	int change_bass;
	bool clear_passing;
	Degree add_passing[7];
	Degree remove_passing[7];
};

static SymbolInfo const symbolinfos[] = {
	// Symbol          Name    En  Xpo PreRs  Ro PostRs Modify      RsAact AddAct           RemA Bass

	// Special ----------------------------------------------------------------------------------------------------------------
	{ Symbols::Off   , "====", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	// Keys -------------------------------------------------------------------------------------------------------------------
	{ Symbols::Cd    , "Cd"  , Cd, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Cb    , "Cb"  , Cb, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::C_    , "C"   , C_, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Cs    , "C#"  , Cs, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Cx    , "Cx"  , Cx, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Dd    , "Dd"  , Dd, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Db    , "Db"  , Db, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::D_    , "D"   , D_, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Ds    , "D#"  , Ds, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Dx    , "Dx"  , Dx, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Ed    , "Ed"  , Ed, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Eb    , "Eb"  , Eb, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::E_    , "E"   , E_, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Es    , "E#"  , Es, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Ex    , "Ex"  , Ex, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Fd    , "Fd"  , Fd, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Fb    , "Fb"  , Fb, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::F_    , "F"   , F_, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Fs    , "F#"  , Fs, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Fx    , "Fx"  , Fx, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Gd    , "Gd"  , Gd, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Gb    , "Gb"  , Gb, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::G_    , "G"   , G_, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Gs    , "G#"  , Gs, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Gx    , "Gx"  , Gx, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Ad    , "Ad"  , Ad, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Ab    , "Ab"  , Ab, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::A_    , "A"   , A_, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::As    , "A#"  , As, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Ax    , "Ax"  , Ax, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Bd    , "Bd"  , Bd, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Bb    , "Bb"  , Bb, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::B_    , "B"   , B_, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Bs    , "B#"  , Bs, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Bx    , "Bx"  , Bx, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::cd    , "cd"  , Cd, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::cb    , "cb"  , Cb, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::c_    , "c"   , C_, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::cs    , "c#"  , Cs, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::cx    , "cx"  , Cx, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::dd    , "dd"  , Dd, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::db    , "db"  , Db, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::d_    , "d"   , D_, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::ds    , "d#"  , Ds, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::dx    , "dx"  , Dx, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::ed    , "ed"  , Ed, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::eb    , "eb"  , Eb, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::e_    , "e"   , E_, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::es    , "e#"  , Es, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::ex    , "ex"  , Ex, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::fd    , "fd"  , Fd, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::fb    , "fb"  , Fb, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::f_    , "f"   , F_, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::fs    , "f#"  , Fs, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::fx    , "fx"  , Fx, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::gd    , "gd"  , Gd, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::gb    , "gb"  , Gb, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::g_    , "g"   , G_, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::gs    , "g#"  , Gs, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::gx    , "gx"  , Gx, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::ad    , "ad"  , Ad, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::ab    , "ab"  , Ab, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::a_    , "a"   , A_, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::as    , "a#"  , As, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::ax    , "ax"  , Ax, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::bd    , "bd"  , Bd, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::bb    , "bb"  , Bb, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::b_    , "b"   , B_, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::bs    , "b#"  , Bs, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::bx    , "bx"  , Bx, _Z, true , 1, false, false, {b3,b6,b7}, true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	// Key Transposing --------------------------------------------------------------------------------------------------------
	{ Symbols::Cd_t  , ":Cd" , Cd, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Cb_t  , ":Cb" , Cb, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::C__t  , ":C"  , C_, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Cs_t  , ":Cs" , Cs, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Cx_t  , ":Cx" , Cx, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Dd_t  , ":Dd" , Dd, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Db_t  , ":Db" , Db, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::D__t  , ":D"  , D_, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Ds_t  , ":Ds" , Ds, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Dx_t  , ":Dx" , Dx, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Ed_t  , ":Ed" , Ed, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Eb_t  , ":Eb" , Eb, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::E__t  , ":E"  , E_, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Es_t  , ":Es" , Es, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Ex_t  , ":Ex" , Ex, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Fd_t  , ":Fd" , Fd, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Fb_t  , ":Fb" , Fb, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::F__t  , ":F"  , F_, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Fs_t  , ":Fs" , Fs, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Fx_t  , ":Fx" , Fx, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Gd_t  , ":Gd" , Gd, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Gb_t  , ":Gb" , Gb, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::G__t  , ":G"  , G_, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Gs_t  , ":Gs" , Gs, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Gx_t  , ":Gx" , Gx, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Ad_t  , ":Ad" , Ad, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Ab_t  , ":Ab" , Ab, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::A__t  , ":A"  , A_, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::As_t  , ":As" , As, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Ax_t  , ":Ax" , Ax, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Bd_t  , ":Bd" , Bd, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Bb_t  , ":Bb" , Bb, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::B__t  , ":B"  , B_, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Bs_t  , ":Bs" , Bs, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	{ Symbols::Bx_t  , ":Bx" , Bx, _Z, false, 1, false, false, {}        , false, {}             , {} , 1, false, {}  , {}   },
	// Degree Transposing -----------------------------------------------------------------------------------------------------
	{ Symbols::d1_t  , ":d1" , Z_, d1, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::b1_t  , ":b1" , Z_, b1, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::_1_t  , ":1"  , Z_, _1, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::s1_t  , ":#1" , Z_, s1, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::x1_t  , ":x1" , Z_, x1, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::d2_t  , ":d2" , Z_, d2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::b2_t  , ":b2" , Z_, b2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::_2_t  , ":2"  , Z_, _2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::s2_t  , ":#2" , Z_, s2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::x2_t  , ":x2" , Z_, x2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::d3_t  , ":d3" , Z_, d3, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::b3_t  , ":b3" , Z_, b3, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::_3_t  , ":3"  , Z_, _3, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::s3_t  , ":#3" , Z_, s3, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::x3_t  , ":x3" , Z_, x3, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::d4_t  , ":d4" , Z_, d4, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::b4_t  , ":b4" , Z_, b4, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::_4_t  , ":4"  , Z_, _4, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::s4_t  , ":#4" , Z_, s4, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::x4_t  , ":x4" , Z_, x4, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::d5_t  , ":d5" , Z_, d5, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::b5_t  , ":b5" , Z_, b5, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::_5_t  , ":5"  , Z_, _5, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::s5_t  , ":#5" , Z_, s5, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::x5_t  , ":x5" , Z_, x5, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::d6_t  , ":d6" , Z_, d6, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::b6_t  , ":b6" , Z_, b6, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::_6_t  , ":6"  , Z_, _6, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::s6_t  , ":#6" , Z_, s6, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::x6_t  , ":x6" , Z_, x6, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::d7_t  , ":d7" , Z_, d7, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::b7_t  , ":b7" , Z_, b7, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::_7_t  , ":7"  , Z_, _7, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::s7_t  , ":#7" , Z_, s7, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::x7_t  , ":x7" , Z_, x7, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	// ---
	{ Symbols::d9_t  , ":d9" , Z_, d2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::b9_t  , ":b9" , Z_, b2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::_9_t  , ":9"  , Z_, _2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::s9_t  , ":#9" , Z_, s2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::x9_t  , ":x9" , Z_, x2, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	// Modes ------------------------------------------------------------------------------------------------------------------
	{ Symbols::Ion   , "Ion" , Z_, _Z, true , 1, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Dor   , "Dor" , Z_, _Z, true , 2, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Phr   , "Phr" , Z_, _Z, true , 3, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Lyd   , "Lyd" , Z_, _Z, true , 4, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Mix   , "Mix" , Z_, _Z, true , 5, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Aeo   , "Aeo" , Z_, _Z, true , 6, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	{ Symbols::Loc   , "Loc" , Z_, _Z, true , 7, false, false, {}        , true , {1,2,3,4,5,6,7}, {} , 1, true , {}  , {}   },
	// Chromatic Chords -------------------------------------------------------------------------------------------------------
	{ Symbols::I_c   , "I"   , Z_, _1, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::bII_c , "bII" , Z_, b2, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::II_c  , "II"  , Z_, _2, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::bIII_c, "bIII", Z_, b3, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::III_c , "III" , Z_, _3, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::IV_c  , "IV"  , Z_, _4, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::sIV_c , "#IV" , Z_, s4, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::V_c   , "V"   , Z_, _5, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::bVI_c , "bVI" , Z_, b6, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::VI_c  , "VI"  , Z_, _6, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::bVII_c, "bVII", Z_, b7, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::VII_c , "VII" , Z_, _7, true , 1, false, false, {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::i_c   , "i"   , Z_, _1, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::bii_c , "bii" , Z_, b2, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::ii_c  , "ii"  , Z_, _2, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::biii_c, "biii", Z_, b3, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::iii_c , "iii" , Z_, _3, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::iv_c  , "iv"  , Z_, _4, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::siv_c , "#iv" , Z_, s4, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::v_c   , "v"   , Z_, _5, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::bvi_c , "bvi" , Z_, b6, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::vi_c  , "vi"  , Z_, _6, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::bvii_c, "bvii", Z_, b7, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::vii_c , "vii" , Z_, _7, true , 1, false, false, {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	// Diatonic Chords --------------------------------------------------------------------------------------------------------
	{ Symbols::I_d   , "^I"  , Z_, _Z, false, 1, true , false, {}        , true , {1,3,5}        , {} , 1, false, {}  , {}   },
	{ Symbols::II_d  , "^II" , Z_, _Z, false, 2, true , false, {}        , true , {1,3,5}        , {} , 1, false, {}  , {}   },
	{ Symbols::III_d , "^III", Z_, _Z, false, 3, true , false, {}        , true , {1,3,5}        , {} , 1, false, {}  , {}   },
	{ Symbols::IV_d  , "^IV" , Z_, _Z, false, 4, true , false, {}        , true , {1,3,5}        , {} , 1, false, {}  , {}   },
	{ Symbols::V_d   , "^V"  , Z_, _Z, false, 5, true , false, {}        , true , {1,3,5}        , {} , 1, false, {}  , {}   },
	{ Symbols::VI_d  , "^VI" , Z_, _Z, false, 6, true , false, {}        , true , {1,3,5}        , {} , 1, false, {}  , {}   },
	{ Symbols::VII_d , "^VII", Z_, _Z, false, 7, true , false, {}        , true , {1,3,5}        , {} , 1, false, {}  , {}   },
	// Harmonic Chords --------------------------------------------------------------------------------------------------------
	{ Symbols::I_h   , "`I"  , Z_, _Z, false, 1, true , true , {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::II_h  , "`II" , Z_, _Z, false, 2, true , true , {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::III_h , "`III", Z_, _Z, false, 3, true , true , {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::IV_h  , "`IV" , Z_, _Z, false, 4, true , true , {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::V_h   , "`V"  , Z_, _Z, false, 5, true , true , {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::VI_h  , "`VI" , Z_, _Z, false, 6, true , true , {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::VII_h , "`VII", Z_, _Z, false, 7, true , true , {}        , true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::i_h   , "`i"  , Z_, _Z, false, 1, true , true , {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::ii_h  , "`ii" , Z_, _Z, false, 2, true , true , {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::iii_h , "`iii", Z_, _Z, false, 3, true , true , {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::iv_h  , "`iv" , Z_, _Z, false, 4, true , true , {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::v_h   , "`v"  , Z_, _Z, false, 5, true , true , {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::vi_h  , "`vi" , Z_, _Z, false, 6, true , true , {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	{ Symbols::vii_h , "`vii", Z_, _Z, false, 7, true , true , {b3,b6,b7}, true , {1,3,5}        , {} , 1, true , {}  , {}   },
	// Degree Modifiers -------------------------------------------------------------------------------------------------------
	{ Symbols::_1    , "1"   , Z_, _Z, false, 1, false, false, {}        , false, {1}            , {} , 0, false, {}  , {}   },
	{ Symbols::no1   , "-1"  , Z_, _Z, false, 1, false, false, {}        , false, {}             , {1}, 0, false, {}  , {}   },
	{ Symbols::d2    , "d2"  , Z_, _Z, false, 1, false, false, {d2}      , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::b2    , "b2"  , Z_, _Z, false, 1, false, false, {b2}      , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::_2    , "2"   , Z_, _Z, false, 1, false, false, {}        , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::s2    , "#2"  , Z_, _Z, false, 1, false, false, {s2}      , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::x2    , "x2"  , Z_, _Z, false, 1, false, false, {x2}      , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::no2   , "-2"  , Z_, _Z, false, 1, false, false, {}        , false, {}             , {2}, 0, false, {}  , {}   },
	{ Symbols::d3    , "d3"  , Z_, _Z, false, 1, false, false, {d3}      , false, {3}            , {} , 0, false, {}  , {}   },
	{ Symbols::b3    , "b3"  , Z_, _Z, false, 1, false, false, {b3}      , false, {3}            , {} , 0, false, {}  , {}   },
	{ Symbols::_3    , "3"   , Z_, _Z, false, 1, false, false, {}        , false, {3}            , {} , 0, false, {}  , {}   },
	{ Symbols::s3    , "#3"  , Z_, _Z, false, 1, false, false, {s3}      , false, {3}            , {} , 0, false, {}  , {}   },
	{ Symbols::x3    , "x3"  , Z_, _Z, false, 1, false, false, {x3}      , false, {3}            , {} , 0, false, {}  , {}   },
	{ Symbols::no3   , "-3"  , Z_, _Z, false, 1, false, false, {}        , false, {}             , {3}, 0, false, {}  , {}   },
	{ Symbols::d4    , "d4"  , Z_, _Z, false, 1, false, false, {d4}      , false, {4}            , {} , 0, false, {}  , {}   },
	{ Symbols::b4    , "b4"  , Z_, _Z, false, 1, false, false, {b4}      , false, {4}            , {} , 0, false, {}  , {}   },
	{ Symbols::_4    , "4"   , Z_, _Z, false, 1, false, false, {}        , false, {4}            , {} , 0, false, {}  , {}   },
	{ Symbols::s4    , "#4"  , Z_, _Z, false, 1, false, false, {s4}      , false, {4}            , {} , 0, false, {}  , {}   },
	{ Symbols::x4    , "x4"  , Z_, _Z, false, 1, false, false, {x4}      , false, {4}            , {} , 0, false, {}  , {}   },
	{ Symbols::no4   , "-4"  , Z_, _Z, false, 1, false, false, {}        , false, {}             , {4}, 0, false, {}  , {}   },
	{ Symbols::d5    , "d5"  , Z_, _Z, false, 1, false, false, {d5}      , false, {5}            , {} , 0, false, {}  , {}   },
	{ Symbols::b5    , "b5"  , Z_, _Z, false, 1, false, false, {b5}      , false, {5}            , {} , 0, false, {}  , {}   },
	{ Symbols::_5    , "5"   , Z_, _Z, false, 1, false, false, {}        , false, {5}            , {} , 0, false, {}  , {}   },
	{ Symbols::s5    , "#5"  , Z_, _Z, false, 1, false, false, {s5}      , false, {5}            , {} , 0, false, {}  , {}   },
	{ Symbols::x5    , "x5"  , Z_, _Z, false, 1, false, false, {x5}      , false, {5}            , {} , 0, false, {}  , {}   },
	{ Symbols::no5   , "-5"  , Z_, _Z, false, 1, false, false, {}        , false, {}             , {5}, 0, false, {}  , {}   },
	{ Symbols::d6    , "d6"  , Z_, _Z, false, 1, false, false, {d6}      , false, {6}            , {} , 0, false, {}  , {}   },
	{ Symbols::b6    , "b6"  , Z_, _Z, false, 1, false, false, {b6}      , false, {6}            , {} , 0, false, {}  , {}   },
	{ Symbols::_6    , "6"   , Z_, _Z, false, 1, false, false, {}        , false, {6}            , {} , 0, false, {}  , {}   },
	{ Symbols::s6    , "#6"  , Z_, _Z, false, 1, false, false, {s6}      , false, {6}            , {} , 0, false, {}  , {}   },
	{ Symbols::x6    , "x6"  , Z_, _Z, false, 1, false, false, {x6}      , false, {6}            , {} , 0, false, {}  , {}   },
	{ Symbols::no6   , "-6"  , Z_, _Z, false, 1, false, false, {}        , false, {}             , {6}, 0, false, {}  , {}   },
	{ Symbols::d7    , "d7"  , Z_, _Z, false, 1, false, false, {d7}      , false, {7}            , {} , 0, false, {}  , {}   },
	{ Symbols::b7    , "b7"  , Z_, _Z, false, 1, false, false, {b7}      , false, {7}            , {} , 0, false, {}  , {}   },
	{ Symbols::_7    , "7"   , Z_, _Z, false, 1, false, false, {}        , false, {7}            , {} , 0, false, {}  , {}   },
	{ Symbols::s7    , "#7"  , Z_, _Z, false, 1, false, false, {s7}      , false, {7}            , {} , 0, false, {}  , {}   },
	{ Symbols::x7    , "x7"  , Z_, _Z, false, 1, false, false, {x7}      , false, {7}            , {} , 0, false, {}  , {}   },
	{ Symbols::no7   , "-7"  , Z_, _Z, false, 1, false, false, {}        , false, {}             , {7}, 0, false, {}  , {}   },
	// ---
	{ Symbols::d9    , "d9"  , Z_, _Z, false, 1, false, false, {d2}      , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::b9    , "b9"  , Z_, _Z, false, 1, false, false, {b2}      , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::_9    , "9"   , Z_, _Z, false, 1, false, false, {}        , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::s9    , "#9"  , Z_, _Z, false, 1, false, false, {s2}      , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::x9    , "x9"  , Z_, _Z, false, 1, false, false, {x2}      , false, {2}            , {} , 0, false, {}  , {}   },
	{ Symbols::no9   , "-9"  , Z_, _Z, false, 1, false, false, {}        , false, {}             , {2}, 0, false, {}  , {}   },
	// Macros -----------------------------------------------------------------------------------------------------------------
	{ Symbols::sus2  , "s2"  , Z_, _Z, false, 1, false, false, {}        , false, {2}            , {3}, 0, false, {}  , {}   },
	{ Symbols::sus4  , "s4"  , Z_, _Z, false, 1, false, false, {}        , false, {4}            , {3}, 0, false, {}  , {}   },
	{ Symbols::mode  , "mode", Z_, _Z, false, 1, false, false, {}        , false, {1,2,3,4,5,6,7}, {} , 0, false, {}  , {}   },
	{ Symbols::root  , "root", Z_, _Z, false, 1, false, false, {}        , true , {1}            , {} , 0, false, {}  , {}   },
	{ Symbols::none  , "none", Z_, _Z, false, 1, false, false, {}        , true , {}             , {} , 0, false, {}  , {}   },
	// Rotations --------------------------------------------------------------------------------------------------------------
	{ Symbols::r1    , "r1"  , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::r2    , "r2"  , Z_, _Z, false, 2, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::r3    , "r3"  , Z_, _Z, false, 3, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::r4    , "r4"  , Z_, _Z, false, 4, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::r5    , "r5"  , Z_, _Z, false, 5, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::r6    , "r6"  , Z_, _Z, false, 6, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	{ Symbols::r7    , "r7"  , Z_, _Z, false, 7, false, false, {}        , false, {}             , {} , 0, false, {}  , {}   },
	// Diatonic Bass ----------------------------------------------------------------------------------------------------------
	{ Symbols::bass1 , "/^1" , Z_, _Z, false, 1, false, false, {}        , false, {1}            , {} , 1, false, {}  , {}   },
	{ Symbols::bass2 , "/^2" , Z_, _Z, false, 1, false, false, {}        , false, {2}            , {} , 2, false, {}  , {}   },
	{ Symbols::bass3 , "/^3" , Z_, _Z, false, 1, false, false, {}        , false, {3}            , {} , 3, false, {}  , {}   },
	{ Symbols::bass4 , "/^4" , Z_, _Z, false, 1, false, false, {}        , false, {4}            , {} , 4, false, {}  , {}   },
	{ Symbols::bass5 , "/^5" , Z_, _Z, false, 1, false, false, {}        , false, {5}            , {} , 5, false, {}  , {}   },
	{ Symbols::bass6 , "/^6" , Z_, _Z, false, 1, false, false, {}        , false, {6}            , {} , 6, false, {}  , {}   },
	{ Symbols::bass7 , "/^7" , Z_, _Z, false, 1, false, false, {}        , false, {7}            , {} , 7, false, {}  , {}   },
	// Passing Tones ----------------------------------------------------------------------------------------------------------
	{ Symbols::none_p, "~"   , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, true , {}  , {}   },
	{ Symbols::b1_p  , "~b1" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {b1}, {}   },
	{ Symbols::s1_p  , "~#1" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {s1}, {}   },
	{ Symbols::b2_p  , "~b2" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {b2}, {}   },
	{ Symbols::s2_p  , "~#2" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {s2}, {}   },
	{ Symbols::b3_p  , "~b3" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {b3}, {}   },
	{ Symbols::s3_p  , "~#3" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {s3}, {}   },
	{ Symbols::b4_p  , "~b4" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {b4}, {}   },
	{ Symbols::s4_p  , "~#4" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {s4}, {}   },
	{ Symbols::b5_p  , "~b5" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {b5}, {}   },
	{ Symbols::s5_p  , "~#5" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {s5}, {}   },
	{ Symbols::b6_p  , "~b6" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {b6}, {}   },
	{ Symbols::s6_p  , "~#6" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {s6}, {}   },
	{ Symbols::b7_p  , "~b7" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {b7}, {}   },
	{ Symbols::s7_p  , "~#7" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {s7}, {}   },
	{ Symbols::b9_p  , "~b9" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {b2}, {}   },
	{ Symbols::s9_p  , "~#9" , Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {s2}, {}   },
	{ Symbols::nob1_p, "-~b1", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {b1} },
	{ Symbols::nos1_p, "-~#1", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {s1} },
	{ Symbols::nob2_p, "-~b2", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {b2} },
	{ Symbols::nos2_p, "-~#2", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {s2} },
	{ Symbols::nob3_p, "-~b3", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {b3} },
	{ Symbols::nos3_p, "-~#3", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {s3} },
	{ Symbols::nob4_p, "-~b4", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {b4} },
	{ Symbols::nos4_p, "-~#4", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {s4} },
	{ Symbols::nob5_p, "-~b5", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {b5} },
	{ Symbols::nos5_p, "-~#5", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {s5} },
	{ Symbols::nob6_p, "-~b6", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {b6} },
	{ Symbols::nos6_p, "-~#6", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {s6} },
	{ Symbols::nob7_p, "-~b7", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {b7} },
	{ Symbols::nos7_p, "-~#7", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {s7} },
	{ Symbols::nob9_p, "-~b9", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {b2} },
	{ Symbols::nos9_p, "-~#9", Z_, _Z, false, 1, false, false, {}        , false, {}             , {} , 0, false, {}  , {s2} },
};

static int const symbolinfos_count = array_size(symbolinfos);

struct SignedNote {
	SignedNote() {}
	SignedNote(int natural_0, int sign) : natural_0(natural_0), sign(sign) {}
	int natural_0;
	int sign;
};

struct Context {
	Symbols::T sym;
	Context* parent;

	struct Data {
		SignedNote root;
		array<int, 7> adjust_degrees;
		array<int, 7> active_degrees;
		array<int, 7> passing_flats;
		array<int, 7> passing_sharps;
		int bass;
	} data;

	Context() : sym(Symbols::NullSym), parent(0), data() {}
	Context(Symbols::T sym, Context* parent) : sym(sym), parent(parent) {}
};

enum MetaSetProperty {
	outside_tone = -1,
	active_tone  = 0,
	root_tone    = 1,
	mode_tone    = 2,
	bass_tone    = 3,
	passing_tone = 4,
};

struct MetaSet {
	MetaSet() { memset(&notes[0], outside_tone, (sizeof(int) * 12 * 3)); }
	int notes[12][3];
};

// ---------------------------------------------------------------------------------------------------------------
// KEYSIGINFO
// ---------------------------------------------------------------------------------------------------------------

struct Key {
	const char* name;
	Degree degrees[20];
};

static const Key keys[] = {
	{ "Chromatic Balanced", { _1, s1, _2, b3, _3, _4, s4, _5, b6, _6, b7, _7 } },
	{ "Chromatic Sharped", { _1, s1, _2, s2, _3, _4, s4, _5, s5, _6, s6, _7 } },
	{ "Chromatic Flatted", { _1, b2, _2, b3, _3, _4, b5, _5, b6, _6, b7, _7 } },
	{ "Chromatic Keyed", { _1, b2, _2, b3, _3, _4, s4, _5, b6, _6, b7, _7 } },
	{ "Ionian (Major)", { _1, _2, _3, _4, _5, _6, _7 } },
	{ "Dorian", { _1, _2, b3, _4, _5, _6, b7 } },
	{ "Phrygian", { _1, b2, b3, _4, _5, b6, b7 } },
	{ "Lydian", { _1, _2, _3, s4, _5, _6, _7 } },
	{ "Mixolydian", { _1, _2, _3, _4, _5, _6, b7 } },
	{ "Aeolian (Minor)", { _1, _2, b3, _4, _5, b6, b7 } },
	{ "Locrian", { _1, b2, b3, _4, b5, b6, b7 } },
	{ "Major Blues", { _1, _2, b3, _3, _5, _6, b7 } },
	{ "Minor Blues", { _1, b3, _4, s4, _5, b7 } },
	{ "Major Pentatonic", { _1, _2, _3, _5, _6 } },
	{ "Minor Pentatonic", { _1, b3, _4, _5, b7 } },
};
static const int keys_count = array_size(keys);

#include "boost/unordered_map.hpp"

class KeySigInfo
{
  private:

	KeySigInfo() {
		SetupMaps();
	}

	~KeySigInfo() {}

	void SetupMaps() {
		for (int i = 0; i < 35; ++i) {
			note_map[noteinfos[i].name] = noteinfos[i].note;
		}

		for (int i = 0; i < keys_count; ++i) {
			key_map[keys[i].name] = keys[i];
			keynames.push_back(keys[i].name);
		}

		for (int i = 0; i < 35; ++i) {
			NoteInfo const& ni = noteinfos[i];
			if (0
				|| (ni.adjust == -1)
				|| (ni.adjust ==  0)
				|| (ni.adjust == +1)
			) {
				commons.push_back(ni.name);
			}
		}
	}

  public:

	static KeySigInfo& instance() {
		static KeySigInfo obj;
		return obj;
	}

	typedef boost::unordered_map<std::string, Note> note_map_t;
	note_map_t note_map;
	typedef boost::unordered_map<std::string, Key> key_map_t;
	key_map_t key_map;
	typedef std::vector<std::string> commons_t;
	commons_t commons;
	typedef std::vector<std::string> keynames_t;
	keynames_t keynames;
	typedef std::vector<int> metavect_t;

	metavect_t GetKeySigMetas(std::string const& root_str, std::string const& key_str, bool chromatic) {
		metavect_t mv(12, -1);

		Note root = note_map[root_str];

		if (chromatic) { // set-merges a keysig with the Keyed-set to convert any key to chromatic
			std::vector<Note> vnChrom = GetKeySig(root, key_map["Chromatic Keyed"]);
			for (std::vector<Note>::iterator i = vnChrom.begin(); i != vnChrom.end(); ++i) {
				if (*i != Z_) {
					NoteInfo const* ni = &noteinfos[*i];
					mv[ni->step] = ni->meta;
				}
			}
		}

		std::vector<Note> vn = GetKeySig(root, key_map[key_str]);
		for (std::vector<Note>::iterator i = vn.begin(); i != vn.end(); ++i) {
			if (*i != Z_) {
				NoteInfo const* ni = &noteinfos[*i];
				mv[ni->step] = ni->meta;
			}
		}

		return mv;
	}

	std::vector<Note> GetKeySig(Note root, Key key) {
		std::vector<Note> v;

		int root_natural_0 = noteinfos[root].natural;
		if ((root_natural_0 == 0) && (noteinfos[root].adjust < 0))
			root_natural_0 += 7;
		int root_step = noteinfos[root].step;
		if ((root_natural_0 == 6) && (noteinfos[root].adjust > 0))
			root_step += 12;

		for (int i = 0; ; ++i) {
			Degree degree = key.degrees[i];
			if (degree == _Z) break;

			DegreeInfo const& di = degreeinfos[degree];

			int interval_0 = di.interval - 1;
			int natural_0_xposed = root_natural_0 + interval_0;
			int interval_stepoffset = majorsteps[interval_0] + di.adjust;
			int adjust = (root_step + interval_stepoffset) - majorsteps[natural_0_xposed];

			Note got_note;
			if (adjust >= -2 && adjust <= 2)
				got_note = noteinfos_grouped[natural_0_xposed % 7][adjust + 2].note;
			else
				got_note = Z_;

			v.push_back(got_note);
		}

		return v;
	}
};

// ---------------------------------------------------------------------------------------------------------------
// INFO
// ---------------------------------------------------------------------------------------------------------------

class Info
{
  private:

	Info() 
	:
		root_ctx(Symbols::C_, 0)
	{
		Eval(&root_ctx);
		root_metaset = GetMetaSet(&root_ctx);

		for (int i = 0; i < symbolinfos_count; ++i) {
			SymbolInfo const& s = symbolinfos[i];
			sym_map[s.name] = s.symbol;
		}
	}

	~Info() {}

  public:

	Context root_ctx;
	MetaSet root_metaset;

	typedef map<string, Symbols::T> sym_map_t;
	sym_map_t sym_map;

	static Info& instance() {
		static Info obj;
		return obj;
	}

	typedef vector<SignedNote> note_vec_t;

	note_vec_t GetContextNotes(Context* ctx) {
		Context::Data& d = ctx->data;

		note_vec_t note_vec;

		for (int i = 0; i < 7; ++i) {
			if (d.passing_flats[i] == 1) {
				int natural = i + 1;
				int sign = d.adjust_degrees[i];
				SignedNote got_note = Transpose(d.root, natural, -1);
				note_vec.push_back(got_note);
			}
			if (d.active_degrees[i] == 1) {
				int natural = i + 1;
				int sign = d.adjust_degrees[i];
				SignedNote got_note = Transpose(d.root, natural, sign);
				note_vec.push_back(got_note);
			}
			if (d.passing_sharps[i] == 1) {
				int natural = i + 1;
				SignedNote got_note = Transpose(d.root, natural, 1);
				note_vec.push_back(got_note);
			}
		}

		return note_vec;
	}

	MetaSet GetMetaSet(Context* ctx) {
		Context::Data& d = ctx->data;

		MetaSet metaset;

		for (int i = 0; i < 7; ++i) {
			int ctx_sign = d.adjust_degrees[i];
			int ctx_natural = i + 1;
			SignedNote got_note = Transpose(d.root, ctx_natural, ctx_sign);
			int natural_0 = got_note.natural_0;
			int sign = got_note.sign;

			if ((sign >= -2) && (sign <= 2)) {
				MetaSetProperty prop;
				if (i == 0)
					prop = root_tone;
				else if (d.active_degrees[i] == 1)
					prop = active_tone;
				else
					prop = mode_tone;

				NoteInfo const& ni = noteinfos_grouped[natural_0][sign + 2];
				metaset.notes[ni.step][ni.meta] = (int)prop;
			}
		}

		return metaset;
	}

	SignedNote Transpose(SignedNote const& note, Degree deg) {
		DegreeInfo const& di = degreeinfos[deg];
		return Transpose(note, di.interval, di.adjust);
	}

	SignedNote Transpose(SignedNote const& note, int interval, int adjust) {
		int interval_0 = interval - 1;
		int natural_0_xposed = note.natural_0 + interval_0;

		int major_stepoffset = majorsteps[interval_0];
		int mode_stepoffset = majorsteps[natural_0_xposed] - majorsteps[note.natural_0];
		int key_adjust = major_stepoffset - mode_stepoffset;

		int natural_0 = natural_0_xposed % 7;
		int sign = note.sign + adjust + key_adjust;

		return SignedNote(natural_0, sign);
	}

	void Eval(Context* ctx) {
		Context::Data& d = ctx->data;
		if (ctx->parent)
			d = ctx->parent->data;

		if (ctx->sym == Symbols::NullSym)
			return;
		if (ctx->sym == Symbols::Off)
			return;

		SymbolInfo const& s = symbolinfos[ctx->sym];

		// Change Key
		if (s.change_key != Z_) {
			NoteInfo const& ni = noteinfos[s.change_key];
			d.root = SignedNote(ni.natural, ni.adjust);
		}

		// Transpose Key
		if (s.transpose_key != _Z) {
			d.root = Transpose(d.root, s.transpose_key);
		}

		// Pre Reset Mode
		if (s.pre_reset_mode) {
			d.adjust_degrees.assign(0);
		}

		// Rotate Mode
		if (s.rotate_mode != 1) {///
			array<int, 7> real_mode;
			int rotate_idx = s.rotate_mode - 1;

			if (s.transpose_mode)
				d.root = Transpose(d.root, rotate_idx + 1, d.adjust_degrees[rotate_idx]);

			for (int i = 0; i < 7; ++i) {
				real_mode[i] = majorsteps[i] + d.adjust_degrees[i];
				if (i < rotate_idx) real_mode[i] += 12;
			}
			int new_base = real_mode[rotate_idx];
			rotate(&real_mode[0], &real_mode[rotate_idx], &real_mode[6] + 1);
			for (int i = 0; i < 7; ++i) {
				d.adjust_degrees[i] = (real_mode[i] - new_base) - majorsteps[i];
			}
		}

		// Post Reset Mode
		if (s.post_reset_mode) {
			d.adjust_degrees.assign(0);
		}

		// Modify Degrees
		for (int i = 0; i < 7; ++i) {
			int x = s.modify_degrees[i];
			if (x == _1) break;
			DegreeInfo const& di = degreeinfos[x];
			int degree_idx = di.interval - 1;
			d.adjust_degrees[degree_idx] += di.adjust;
		}

		// Clear Active
		if (s.clear_active) {
			d.active_degrees.assign(0);
		}

		// Add Active
		for (int i = 0; i < 7; ++i) {
			int x = s.add_active[i];
			if (x == 0) break;
			int degree_idx = x - 1;
			d.active_degrees[degree_idx] = 1;
		}

		// Remove Active
		for (int i = 0; i < 7; ++i) {
			int x = s.remove_active[i];
			if (x == 0) break;
			int degree_idx = x - 1;
			d.active_degrees[degree_idx] = 0;
		}

		// Set Bass
		if (s.change_bass != 0) {
			d.bass = s.change_bass;
		}

		// Clear Passing
		if (s.clear_passing) {
			d.passing_flats.assign(_Z);
			d.passing_sharps.assign(_Z);
		}

		// Add Passing
		for (int i = 0; i < 7; ++i) {
			Degree x = s.add_passing[i];
			if (x == _Z) break;
			DegreeInfo const& di = degreeinfos[x];
			int natural_idx = di.interval - 1;
			if (di.adjust == -1)
				d.passing_flats[natural_idx] = 1;
			else
				d.passing_sharps[natural_idx] = 1;
		}

		// Remove Passing
		for (int i = 0; i < 7; ++i) {
			Degree x = s.remove_passing[i];
			if (x == _Z) break;
			DegreeInfo const& di = degreeinfos[x];
			int natural_idx = di.interval - 1;
			if (di.adjust == -1)
				d.passing_flats[natural_idx] = 0;
			else
				d.passing_sharps[natural_idx] = 0;
		}
	}
};

struct HSys
{
	typedef map<int, MetaSet> resolve_cache_t;
	typedef std::pair<int, MetaSet> resolve_pair_t;
	resolve_cache_t resolve_cache;
	typedef map<int, Context> context_map_t;
	typedef std::pair<int, Context> ctx_pair_t;
	typedef vector<context_map_t> tracks_t;
	tracks_t tracks;
	function2<void, int, int> invalidate;
	//function<void (int, int)> invalidate;

	void SetTrackCount(int track_count) {
		tracks.resize(track_count);

		/// insert new null contexts
	}

	void Reset() {
		for (int xi = 0; xi < (int)tracks.size(); ++xi) {
			context_map_t& cmap = tracks[xi];
			cmap.clear();
		}
		resolve_cache.clear();
	}

	void Add(int x, int y, Symbols::T sym) {
		context_map_t& cmap = tracks[x];
		Context* ctx = FindContextAt(x, y);
		if (ctx == 0) {
			Context* parent = FindParentOf(x, y);
			ctx = &cmap.insert(ctx_pair_t(y, Context(sym, parent))).first->second;
			TakeChildren(x, y, ctx);
			CreateNullRow(x, y, ctx);
		} else
		if (ctx->sym == Symbols::NullSym) {
			ctx->sym = sym;
		} else {
			assert(false);
		}

		EvalContext(ctx, x, y);
	}

	void CreateNullRow(int x, int y, Context* chain) {
		for (int xi = x + 1; xi < (int)tracks.size(); ++xi) {
			Context* ctx = FindContextAt(xi, y);
			if (ctx == 0) {
				context_map_t& cmap = tracks[xi];
				chain = &cmap.insert(ctx_pair_t(y, Context(Symbols::NullSym, chain))).first->second;
				TakeChildren(xi, y, chain);
			} else
			if (ctx->sym == Symbols::NullSym) {
				assert(false);
			} else {
				ctx->parent = chain;
				break;
			}
		}
	}

	void TakeChildren(int x, int y, Context* ctx) {
		int child_col = x + 1;
		if (child_col < (int)tracks.size()) {
			int next_ctx_row = GetNextContextRow(x, y);
			context_map_t& cmap = tracks[child_col];
			context_map_t::iterator j = cmap.lower_bound(y);

			context_map_t::iterator j_end;
			if (next_ctx_row != -1)
				j_end = cmap.upper_bound(next_ctx_row - 1);
			else
				j_end = cmap.end();

			for (; j != j_end; ++j) {
				j->second.parent = ctx;
			}
		}
	}

	void CacheInsert(Context* ctx, int y) {
		resolve_cache[y] = Info::instance().GetMetaSet(ctx);
	}

	void CacheRemove(int y) {
		resolve_cache_t::iterator j = resolve_cache.find(y);
		resolve_cache.erase(j);
	}

	MetaSet* CacheResolveRow(int y) {
		resolve_cache_t::iterator j = resolve_cache.upper_bound(y);
		if ((j != resolve_cache.begin()) && (resolve_cache.size() != 0)) {
			return &(--j)->second;
		}

		return &Info::instance().root_metaset;
	}

	void EvalContext(Context* ctx, int x, int y) {
		Info::instance().Eval(ctx);
		int last_track = (int)tracks.size() - 1;
		if (x == last_track)
			CacheInsert(ctx, y);
		EvalChildrenRect(x, y);
	}

	void EvalChildrenRect(int x, int y) {
		int next_ctx_row = GetNextContextRow(x, y);
		invalidate(y, next_ctx_row);
		int last_track = (int)tracks.size() - 1;
		for (int xi = x + 1; xi < (int)tracks.size(); ++xi) {
			context_map_t& cmap = tracks[xi];
 			context_map_t::iterator j = cmap.lower_bound(y);

 			context_map_t::iterator j_end;
 			if (next_ctx_row != -1)
 				j_end = cmap.upper_bound(next_ctx_row - 1);
 			else
 				j_end = cmap.end();

			for (; j != j_end; ++j) {
				Context* ctx = &j->second;
				Info::instance().Eval(ctx);
				if (xi == last_track)
					CacheInsert(ctx, j->first);
			}
		}
	}

	void Remove(int x, int y) {
		Context* left_ctx = 0;

		if (x != 0) {
			left_ctx = FindContextAt(x - 1, y);
			if (left_ctx != 0) {
				Context* ctx = FindContextAt(x, y);
				ctx->sym = Symbols::NullSym;
				EvalContext(ctx, x, y);
			}
		}

		if (left_ctx == 0) {
			context_map_t& cmap = tracks[x];
			context_map_t::iterator j = cmap.find(y);
			int last_track = (int)tracks.size() - 1;
			cmap.erase(j);
			if (x == last_track)
				CacheRemove(y);
			Context* newparent = FindParentOf(x + 1, y);
			TakeChildren(x, y, newparent);
			RemoveNullRow(x, y);
			EvalChildrenRect(x, y);
		}
	}

	void RemoveNullRow(int x, int y) {
		int last_track = (int)tracks.size() - 1;
		for (int xi = x + 1; xi < (int)tracks.size(); ++xi) {
			context_map_t& cmap = tracks[xi];
			context_map_t::iterator j = cmap.find(y);
			if (j->second.sym == Symbols::NullSym) {
				cmap.erase(j);
				if (xi == last_track)
					CacheRemove(y);
				Context* newparent = FindParentOf(xi + 1, y);
				TakeChildren(xi, y, newparent);
			} else {
				return;
			}
		}
	}

	void Update(int x, int y, Symbols::T sym) {///crash if time is updated.
		Context* ctx = FindContextAt(x, y);
		ctx->sym = sym;
		EvalContext(ctx, x, y);
	}

	Context* FindParentOf(int x, int y) {
		if (x == 0) return &Info::instance().root_ctx;
		Context* ctx = FindContextAboveOrAt(x - 1, y);
		return ctx ? ctx : &Info::instance().root_ctx;
	}

	Context* FindContextAt(int x, int y) {
		context_map_t& cmap = tracks[x];
		context_map_t::iterator j = cmap.find(y);
		if (j != cmap.end()) {
			return &j->second;
		} else {
			return 0;
		}
	}

	Context* FindContextAboveOrAt(int x, int y) {
		context_map_t& cmap = tracks[x];
		context_map_t::iterator j = cmap.upper_bound(y);
		if ((j != cmap.begin()) && (cmap.size() != 0)) {
			return &(--j)->second;
		} else {
			return 0;
		}
	}

	int GetNextContextRow(int x, int y) {
		if (tracks.empty()) return 1;
		context_map_t& cmap = tracks[x];
		context_map_t::iterator j = cmap.lower_bound(y + 1);
		if (j != cmap.end())
			return j->first;
		else
			return -1;
	}

	Context* ResolveRow(int y) {
		int last_track = (int)tracks.size() - 1;
		Context* ctx = FindContextAboveOrAt(last_track, y);
		return ctx ? ctx : &Info::instance().root_ctx;
	}
};

// ---------------------------------------------------------------------------------------------------------------

} // END namespace Harmony
