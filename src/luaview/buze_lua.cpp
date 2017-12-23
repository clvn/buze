#include <zzub/zzub.h>
#include <buze/buze.h>
#include <cassert>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

extern void report_errors(lua_State *L, int status);

struct buze_callback_data {
	lua_State *L;
	int functionref;
	const void* ptr;
};

buze_callback_data buze_callback_datas[500];

template <int N>
int buze_callback_template(buze_window_t* sender, int hint, void* param, void* tag) {
	lua_State* L = buze_callback_datas[N].L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, buze_callback_datas[N].functionref);
	if (sender != NULL)
		lua_pushlightuserdata(L, sender);
	else
		lua_pushnil(L);
	lua_pushnumber(L, hint);
	lua_pushlightuserdata(L, param);
	lua_pushlightuserdata(L, tag);
	int status = lua_pcall(L, 4, 1, 0);
	report_errors(L, status);
	return lua_tonumber(L, -1);
}

buze_callback_t buze_callback_callbacks[] = {
	&buze_callback_template<0>,
	&buze_callback_template<1>,
	&buze_callback_template<2>,
	&buze_callback_template<3>,
	&buze_callback_template<4>,
	&buze_callback_template<5>,
	&buze_callback_template<6>,
	&buze_callback_template<7>,
	&buze_callback_template<8>,
	&buze_callback_template<9>,
	&buze_callback_template<10>,
	&buze_callback_template<11>,
	&buze_callback_template<12>,
	&buze_callback_template<13>,
	&buze_callback_template<14>,
	&buze_callback_template<15>,
	&buze_callback_template<16>,
	&buze_callback_template<17>,
	&buze_callback_template<18>,
	&buze_callback_template<19>,
	&buze_callback_template<20>,
	&buze_callback_template<21>,
	&buze_callback_template<22>,
	&buze_callback_template<23>,
	&buze_callback_template<24>,
	&buze_callback_template<25>,
	&buze_callback_template<26>,
	&buze_callback_template<27>,
	&buze_callback_template<28>,
	&buze_callback_template<29>,
	&buze_callback_template<30>,
	&buze_callback_template<31>,
	&buze_callback_template<32>,
	&buze_callback_template<33>,
	&buze_callback_template<34>,
	&buze_callback_template<35>,
	&buze_callback_template<36>,
	&buze_callback_template<37>,
	&buze_callback_template<38>,
	&buze_callback_template<39>,
	&buze_callback_template<40>,
	&buze_callback_template<41>,
	&buze_callback_template<42>,
	&buze_callback_template<43>,
	&buze_callback_template<44>,
	&buze_callback_template<45>,
	&buze_callback_template<46>,
	&buze_callback_template<47>,
	&buze_callback_template<48>,
	&buze_callback_template<49>,
	&buze_callback_template<50>,
	&buze_callback_template<51>,
	&buze_callback_template<52>,
	&buze_callback_template<53>,
	&buze_callback_template<54>,
	&buze_callback_template<55>,
	&buze_callback_template<56>,
	&buze_callback_template<57>,
	&buze_callback_template<58>,
	&buze_callback_template<59>,
	&buze_callback_template<60>,
	&buze_callback_template<61>,
	&buze_callback_template<62>,
	&buze_callback_template<63>,
	&buze_callback_template<64>,
	&buze_callback_template<65>,
	&buze_callback_template<66>,
	&buze_callback_template<67>,
	&buze_callback_template<68>,
	&buze_callback_template<69>,
	&buze_callback_template<70>,
	&buze_callback_template<71>,
	&buze_callback_template<72>,
	&buze_callback_template<73>,
	&buze_callback_template<74>,
	&buze_callback_template<75>,
	&buze_callback_template<76>,
	&buze_callback_template<77>,
	&buze_callback_template<78>,
	&buze_callback_template<79>,
	&buze_callback_template<80>,
	&buze_callback_template<81>,
	&buze_callback_template<82>,
	&buze_callback_template<83>,
	&buze_callback_template<84>,
	&buze_callback_template<85>,
	&buze_callback_template<86>,
	&buze_callback_template<87>,
	&buze_callback_template<88>,
	&buze_callback_template<89>,
	&buze_callback_template<90>,
	&buze_callback_template<91>,
	&buze_callback_template<92>,
	&buze_callback_template<93>,
	&buze_callback_template<94>,
	&buze_callback_template<95>,
	&buze_callback_template<96>,
	&buze_callback_template<97>,
	&buze_callback_template<98>,
	&buze_callback_template<99>,
	&buze_callback_template<100>,
	&buze_callback_template<101>,
	&buze_callback_template<102>,
	&buze_callback_template<103>,
	&buze_callback_template<104>,
	&buze_callback_template<105>,
	&buze_callback_template<106>,
	&buze_callback_template<107>,
	&buze_callback_template<108>,
	&buze_callback_template<109>,
	&buze_callback_template<110>,
	&buze_callback_template<111>,
	&buze_callback_template<112>,
	&buze_callback_template<113>,
	&buze_callback_template<114>,
	&buze_callback_template<115>,
	&buze_callback_template<116>,
	&buze_callback_template<117>,
	&buze_callback_template<118>,
	&buze_callback_template<119>,
	&buze_callback_template<120>,
	&buze_callback_template<121>,
	&buze_callback_template<122>,
	&buze_callback_template<123>,
	&buze_callback_template<124>,
	&buze_callback_template<125>,
	&buze_callback_template<126>,
	&buze_callback_template<127>,
	&buze_callback_template<128>,
	&buze_callback_template<129>,
	&buze_callback_template<130>,
	&buze_callback_template<131>,
	&buze_callback_template<132>,
	&buze_callback_template<133>,
	&buze_callback_template<134>,
	&buze_callback_template<135>,
	&buze_callback_template<136>,
	&buze_callback_template<137>,
	&buze_callback_template<138>,
	&buze_callback_template<139>,
	&buze_callback_template<140>,
	&buze_callback_template<141>,
	&buze_callback_template<142>,
	&buze_callback_template<143>,
	&buze_callback_template<144>,
	&buze_callback_template<145>,
	&buze_callback_template<146>,
	&buze_callback_template<147>,
	&buze_callback_template<148>,
	&buze_callback_template<149>,
	&buze_callback_template<150>,
	&buze_callback_template<151>,
	&buze_callback_template<152>,
	&buze_callback_template<153>,
	&buze_callback_template<154>,
	&buze_callback_template<155>,
	&buze_callback_template<156>,
	&buze_callback_template<157>,
	&buze_callback_template<158>,
	&buze_callback_template<159>,
	&buze_callback_template<160>,
	&buze_callback_template<161>,
	&buze_callback_template<162>,
	&buze_callback_template<163>,
	&buze_callback_template<164>,
	&buze_callback_template<165>,
	&buze_callback_template<166>,
	&buze_callback_template<167>,
	&buze_callback_template<168>,
	&buze_callback_template<169>,
	&buze_callback_template<170>,
	&buze_callback_template<171>,
	&buze_callback_template<172>,
	&buze_callback_template<173>,
	&buze_callback_template<174>,
	&buze_callback_template<175>,
	&buze_callback_template<176>,
	&buze_callback_template<177>,
	&buze_callback_template<178>,
	&buze_callback_template<179>,
	&buze_callback_template<180>,
	&buze_callback_template<181>,
	&buze_callback_template<182>,
	&buze_callback_template<183>,
	&buze_callback_template<184>,
	&buze_callback_template<185>,
	&buze_callback_template<186>,
	&buze_callback_template<187>,
	&buze_callback_template<188>,
	&buze_callback_template<189>,
	&buze_callback_template<190>,
	&buze_callback_template<191>,
	&buze_callback_template<192>,
	&buze_callback_template<193>,
	&buze_callback_template<194>,
	&buze_callback_template<195>,
	&buze_callback_template<196>,
	&buze_callback_template<197>,
	&buze_callback_template<198>,
	&buze_callback_template<199>,
	&buze_callback_template<200>,
	&buze_callback_template<201>,
	&buze_callback_template<202>,
	&buze_callback_template<203>,
	&buze_callback_template<204>,
	&buze_callback_template<205>,
	&buze_callback_template<206>,
	&buze_callback_template<207>,
	&buze_callback_template<208>,
	&buze_callback_template<209>,
	&buze_callback_template<210>,
	&buze_callback_template<211>,
	&buze_callback_template<212>,
	&buze_callback_template<213>,
	&buze_callback_template<214>,
	&buze_callback_template<215>,
	&buze_callback_template<216>,
	&buze_callback_template<217>,
	&buze_callback_template<218>,
	&buze_callback_template<219>,
	&buze_callback_template<220>,
	&buze_callback_template<221>,
	&buze_callback_template<222>,
	&buze_callback_template<223>,
	&buze_callback_template<224>,
	&buze_callback_template<225>,
	&buze_callback_template<226>,
	&buze_callback_template<227>,
	&buze_callback_template<228>,
	&buze_callback_template<229>,
	&buze_callback_template<230>,
	&buze_callback_template<231>,
	&buze_callback_template<232>,
	&buze_callback_template<233>,
	&buze_callback_template<234>,
	&buze_callback_template<235>,
	&buze_callback_template<236>,
	&buze_callback_template<237>,
	&buze_callback_template<238>,
	&buze_callback_template<239>,
	&buze_callback_template<240>,
	&buze_callback_template<241>,
	&buze_callback_template<242>,
	&buze_callback_template<243>,
	&buze_callback_template<244>,
	&buze_callback_template<245>,
	&buze_callback_template<246>,
	&buze_callback_template<247>,
	&buze_callback_template<248>,
	&buze_callback_template<249>,
	&buze_callback_template<250>,
	&buze_callback_template<251>,
	&buze_callback_template<252>,
	&buze_callback_template<253>,
	&buze_callback_template<254>,
	&buze_callback_template<255>,
	&buze_callback_template<256>,
	&buze_callback_template<257>,
	&buze_callback_template<258>,
	&buze_callback_template<259>,
	&buze_callback_template<260>,
	&buze_callback_template<261>,
	&buze_callback_template<262>,
	&buze_callback_template<263>,
	&buze_callback_template<264>,
	&buze_callback_template<265>,
	&buze_callback_template<266>,
	&buze_callback_template<267>,
	&buze_callback_template<268>,
	&buze_callback_template<269>,
	&buze_callback_template<270>,
	&buze_callback_template<271>,
	&buze_callback_template<272>,
	&buze_callback_template<273>,
	&buze_callback_template<274>,
	&buze_callback_template<275>,
	&buze_callback_template<276>,
	&buze_callback_template<277>,
	&buze_callback_template<278>,
	&buze_callback_template<279>,
	&buze_callback_template<280>,
	&buze_callback_template<281>,
	&buze_callback_template<282>,
	&buze_callback_template<283>,
	&buze_callback_template<284>,
	&buze_callback_template<285>,
	&buze_callback_template<286>,
	&buze_callback_template<287>,
	&buze_callback_template<288>,
	&buze_callback_template<289>,
	&buze_callback_template<290>,
	&buze_callback_template<291>,
	&buze_callback_template<292>,
	&buze_callback_template<293>,
	&buze_callback_template<294>,
	&buze_callback_template<295>,
	&buze_callback_template<296>,
	&buze_callback_template<297>,
	&buze_callback_template<298>,
	&buze_callback_template<299>,
	&buze_callback_template<300>,
	&buze_callback_template<301>,
	&buze_callback_template<302>,
	&buze_callback_template<303>,
	&buze_callback_template<304>,
	&buze_callback_template<305>,
	&buze_callback_template<306>,
	&buze_callback_template<307>,
	&buze_callback_template<308>,
	&buze_callback_template<309>,
	&buze_callback_template<310>,
	&buze_callback_template<311>,
	&buze_callback_template<312>,
	&buze_callback_template<313>,
	&buze_callback_template<314>,
	&buze_callback_template<315>,
	&buze_callback_template<316>,
	&buze_callback_template<317>,
	&buze_callback_template<318>,
	&buze_callback_template<319>,
	&buze_callback_template<320>,
	&buze_callback_template<321>,
	&buze_callback_template<322>,
	&buze_callback_template<323>,
	&buze_callback_template<324>,
	&buze_callback_template<325>,
	&buze_callback_template<326>,
	&buze_callback_template<327>,
	&buze_callback_template<328>,
	&buze_callback_template<329>,
	&buze_callback_template<330>,
	&buze_callback_template<331>,
	&buze_callback_template<332>,
	&buze_callback_template<333>,
	&buze_callback_template<334>,
	&buze_callback_template<335>,
	&buze_callback_template<336>,
	&buze_callback_template<337>,
	&buze_callback_template<338>,
	&buze_callback_template<339>,
	&buze_callback_template<340>,
	&buze_callback_template<341>,
	&buze_callback_template<342>,
	&buze_callback_template<343>,
	&buze_callback_template<344>,
	&buze_callback_template<345>,
	&buze_callback_template<346>,
	&buze_callback_template<347>,
	&buze_callback_template<348>,
	&buze_callback_template<349>,
	&buze_callback_template<350>,
	&buze_callback_template<351>,
	&buze_callback_template<352>,
	&buze_callback_template<353>,
	&buze_callback_template<354>,
	&buze_callback_template<355>,
	&buze_callback_template<356>,
	&buze_callback_template<357>,
	&buze_callback_template<358>,
	&buze_callback_template<359>,
	&buze_callback_template<360>,
	&buze_callback_template<361>,
	&buze_callback_template<362>,
	&buze_callback_template<363>,
	&buze_callback_template<364>,
	&buze_callback_template<365>,
	&buze_callback_template<366>,
	&buze_callback_template<367>,
	&buze_callback_template<368>,
	&buze_callback_template<369>,
	&buze_callback_template<370>,
	&buze_callback_template<371>,
	&buze_callback_template<372>,
	&buze_callback_template<373>,
	&buze_callback_template<374>,
	&buze_callback_template<375>,
	&buze_callback_template<376>,
	&buze_callback_template<377>,
	&buze_callback_template<378>,
	&buze_callback_template<379>,
	&buze_callback_template<380>,
	&buze_callback_template<381>,
	&buze_callback_template<382>,
	&buze_callback_template<383>,
	&buze_callback_template<384>,
	&buze_callback_template<385>,
	&buze_callback_template<386>,
	&buze_callback_template<387>,
	&buze_callback_template<388>,
	&buze_callback_template<389>,
	&buze_callback_template<390>,
	&buze_callback_template<391>,
	&buze_callback_template<392>,
	&buze_callback_template<393>,
	&buze_callback_template<394>,
	&buze_callback_template<395>,
	&buze_callback_template<396>,
	&buze_callback_template<397>,
	&buze_callback_template<398>,
	&buze_callback_template<399>,
	&buze_callback_template<400>,
	&buze_callback_template<401>,
	&buze_callback_template<402>,
	&buze_callback_template<403>,
	&buze_callback_template<404>,
	&buze_callback_template<405>,
	&buze_callback_template<406>,
	&buze_callback_template<407>,
	&buze_callback_template<408>,
	&buze_callback_template<409>,
	&buze_callback_template<410>,
	&buze_callback_template<411>,
	&buze_callback_template<412>,
	&buze_callback_template<413>,
	&buze_callback_template<414>,
	&buze_callback_template<415>,
	&buze_callback_template<416>,
	&buze_callback_template<417>,
	&buze_callback_template<418>,
	&buze_callback_template<419>,
	&buze_callback_template<420>,
	&buze_callback_template<421>,
	&buze_callback_template<422>,
	&buze_callback_template<423>,
	&buze_callback_template<424>,
	&buze_callback_template<425>,
	&buze_callback_template<426>,
	&buze_callback_template<427>,
	&buze_callback_template<428>,
	&buze_callback_template<429>,
	&buze_callback_template<430>,
	&buze_callback_template<431>,
	&buze_callback_template<432>,
	&buze_callback_template<433>,
	&buze_callback_template<434>,
	&buze_callback_template<435>,
	&buze_callback_template<436>,
	&buze_callback_template<437>,
	&buze_callback_template<438>,
	&buze_callback_template<439>,
	&buze_callback_template<440>,
	&buze_callback_template<441>,
	&buze_callback_template<442>,
	&buze_callback_template<443>,
	&buze_callback_template<444>,
	&buze_callback_template<445>,
	&buze_callback_template<446>,
	&buze_callback_template<447>,
	&buze_callback_template<448>,
	&buze_callback_template<449>,
	&buze_callback_template<450>,
	&buze_callback_template<451>,
	&buze_callback_template<452>,
	&buze_callback_template<453>,
	&buze_callback_template<454>,
	&buze_callback_template<455>,
	&buze_callback_template<456>,
	&buze_callback_template<457>,
	&buze_callback_template<458>,
	&buze_callback_template<459>,
	&buze_callback_template<460>,
	&buze_callback_template<461>,
	&buze_callback_template<462>,
	&buze_callback_template<463>,
	&buze_callback_template<464>,
	&buze_callback_template<465>,
	&buze_callback_template<466>,
	&buze_callback_template<467>,
	&buze_callback_template<468>,
	&buze_callback_template<469>,
	&buze_callback_template<470>,
	&buze_callback_template<471>,
	&buze_callback_template<472>,
	&buze_callback_template<473>,
	&buze_callback_template<474>,
	&buze_callback_template<475>,
	&buze_callback_template<476>,
	&buze_callback_template<477>,
	&buze_callback_template<478>,
	&buze_callback_template<479>,
	&buze_callback_template<480>,
	&buze_callback_template<481>,
	&buze_callback_template<482>,
	&buze_callback_template<483>,
	&buze_callback_template<484>,
	&buze_callback_template<485>,
	&buze_callback_template<486>,
	&buze_callback_template<487>,
	&buze_callback_template<488>,
	&buze_callback_template<489>,
	&buze_callback_template<490>,
	&buze_callback_template<491>,
	&buze_callback_template<492>,
	&buze_callback_template<493>,
	&buze_callback_template<494>,
	&buze_callback_template<495>,
	&buze_callback_template<496>,
	&buze_callback_template<497>,
	&buze_callback_template<498>,
	&buze_callback_template<499>,
};

buze_callback_t buze_callback_alloc(lua_State* L, int index) {
	const void* ptr = lua_topointer(L, index);
	assert(ptr != 0);
	for (int i = 0; i < 500; ++i) {
		if (ptr == buze_callback_datas[i].ptr) {
			return buze_callback_callbacks[i];
		}
	}
	lua_pushvalue(L, index);
	int functionref = luaL_ref(L, LUA_REGISTRYINDEX);
	for (int i = 0; i < 500; ++i) {
		if (buze_callback_datas[i].L == 0) {
			buze_callback_datas[i].L = L;
			buze_callback_datas[i].ptr = ptr;
			buze_callback_datas[i].functionref = functionref;
			return buze_callback_callbacks[i];
		}
	}
	return 0;
}

void buze_callback_clear(lua_State* L) {
	for (int i = 0; i < 500; ++i) {
		if (L == buze_callback_datas[i].L) {
			buze_callback_datas[i].L = 0;
			buze_callback_datas[i].ptr = 0;
			buze_callback_datas[i].functionref = 0;
		}
	}
}

static int change_pattern_order_get_index(lua_State *L) {
	buze_event_data_change_pattern_order_t* self = (buze_event_data_change_pattern_order_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ChangePatternOrder:index");
		return 0;
	}

	int _luaresult = self->index;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg change_pattern_order_lib[] = {
	{ "get_index", change_pattern_order_get_index }, 
	{ NULL, NULL }
};

static int change_pattern_row_get_row(lua_State *L) {
	buze_event_data_change_pattern_row_t* self = (buze_event_data_change_pattern_row_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ChangePatternRow:row");
		return 0;
	}

	int _luaresult = self->row;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg change_pattern_row_lib[] = {
	{ "get_row", change_pattern_row_get_row }, 
	{ NULL, NULL }
};

static int show_machine_parameter_get_plugin(lua_State *L) {
	buze_event_data_show_machine_parameter_t* self = (buze_event_data_show_machine_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowMachineParameter:plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int show_machine_parameter_get_mode(lua_State *L) {
	buze_event_data_show_machine_parameter_t* self = (buze_event_data_show_machine_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowMachineParameter:mode");
		return 0;
	}

	int _luaresult = self->mode;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int show_machine_parameter_get_x(lua_State *L) {
	buze_event_data_show_machine_parameter_t* self = (buze_event_data_show_machine_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowMachineParameter:x");
		return 0;
	}

	int _luaresult = self->x;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int show_machine_parameter_get_y(lua_State *L) {
	buze_event_data_show_machine_parameter_t* self = (buze_event_data_show_machine_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowMachineParameter:y");
		return 0;
	}

	int _luaresult = self->y;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg show_machine_parameter_lib[] = {
	{ "get_plugin", show_machine_parameter_get_plugin }, 
	{ "get_mode", show_machine_parameter_get_mode }, 
	{ "get_x", show_machine_parameter_get_x }, 
	{ "get_y", show_machine_parameter_get_y }, 
	{ NULL, NULL }
};

static int show_pattern_get_pattern(lua_State *L) {
	buze_event_data_show_pattern_t* self = (buze_event_data_show_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowPattern:pattern");
		return 0;
	}

	zzub_pattern_t* _luaresult = self->pattern;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int show_pattern_get_change_pattern(lua_State *L) {
	buze_event_data_show_pattern_t* self = (buze_event_data_show_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowPattern:change_pattern");
		return 0;
	}

	int _luaresult = self->change_pattern;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int show_pattern_get_reset_stack(lua_State *L) {
	buze_event_data_show_pattern_t* self = (buze_event_data_show_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowPattern:reset_stack");
		return 0;
	}

	int _luaresult = self->reset_stack;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int show_pattern_get_editor_id(lua_State *L) {
	buze_event_data_show_pattern_t* self = (buze_event_data_show_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowPattern:editor_id");
		return 0;
	}

	int _luaresult = self->editor_id;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg show_pattern_lib[] = {
	{ "get_pattern", show_pattern_get_pattern }, 
	{ "get_change_pattern", show_pattern_get_change_pattern }, 
	{ "get_reset_stack", show_pattern_get_reset_stack }, 
	{ "get_editor_id", show_pattern_get_editor_id }, 
	{ NULL, NULL }
};

static int show_pattern_format_get_pattern_format(lua_State *L) {
	buze_event_data_show_pattern_format_t* self = (buze_event_data_show_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowPatternFormat:pattern_format");
		return 0;
	}

	zzub_pattern_format_t* _luaresult = self->pattern_format;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg show_pattern_format_lib[] = {
	{ "get_pattern_format", show_pattern_format_get_pattern_format }, 
	{ NULL, NULL }
};

static int show_properties_get_type(lua_State *L) {
	buze_event_data_show_properties_t* self = (buze_event_data_show_properties_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowProperties:type");
		return 0;
	}

	int _luaresult = self->type;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int show_properties_get_return_view(lua_State *L) {
	buze_event_data_show_properties_t* self = (buze_event_data_show_properties_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowProperties:return_view");
		return 0;
	}

	buze_window_t* _luaresult = self->return_view;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int show_properties_get_plugin(lua_State *L) {
	buze_event_data_show_properties_t* self = (buze_event_data_show_properties_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowProperties:plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int show_properties_get_plugin_group(lua_State *L) {
	buze_event_data_show_properties_t* self = (buze_event_data_show_properties_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowProperties:plugin_group");
		return 0;
	}

	zzub_plugin_group_t* _luaresult = self->plugin_group;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int show_properties_get_connection(lua_State *L) {
	buze_event_data_show_properties_t* self = (buze_event_data_show_properties_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowProperties:connection");
		return 0;
	}

	zzub_connection_t* _luaresult = self->connection;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int show_properties_get_pattern(lua_State *L) {
	buze_event_data_show_properties_t* self = (buze_event_data_show_properties_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowProperties:pattern");
		return 0;
	}

	zzub_pattern_t* _luaresult = self->pattern;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int show_properties_get_pattern_format(lua_State *L) {
	buze_event_data_show_properties_t* self = (buze_event_data_show_properties_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowProperties:pattern_format");
		return 0;
	}

	zzub_pattern_format_t* _luaresult = self->pattern_format;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int show_properties_get_wave(lua_State *L) {
	buze_event_data_show_properties_t* self = (buze_event_data_show_properties_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowProperties:wave");
		return 0;
	}

	zzub_wave_t* _luaresult = self->wave;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int show_properties_get_wavelevel(lua_State *L) {
	buze_event_data_show_properties_t* self = (buze_event_data_show_properties_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ShowProperties:wavelevel");
		return 0;
	}

	zzub_wavelevel_t* _luaresult = self->wavelevel;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg show_properties_lib[] = {
	{ "get_type", show_properties_get_type }, 
	{ "get_return_view", show_properties_get_return_view }, 
	{ "get_plugin", show_properties_get_plugin }, 
	{ "get_plugin_group", show_properties_get_plugin_group }, 
	{ "get_connection", show_properties_get_connection }, 
	{ "get_pattern", show_properties_get_pattern }, 
	{ "get_pattern_format", show_properties_get_pattern_format }, 
	{ "get_wave", show_properties_get_wave }, 
	{ "get_wavelevel", show_properties_get_wavelevel }, 
	{ NULL, NULL }
};

static int event_data_get_change_pattern_order(lua_State *L) {
	buze_event_data_t* self = (buze_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:change_pattern_order");
		return 0;
	}

	buze_event_data_change_pattern_order_t* _luaresult = &self->change_pattern_order;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_change_pattern_row(lua_State *L) {
	buze_event_data_t* self = (buze_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:change_pattern_row");
		return 0;
	}

	buze_event_data_change_pattern_row_t* _luaresult = &self->change_pattern_row;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_show_properties(lua_State *L) {
	buze_event_data_t* self = (buze_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:show_properties");
		return 0;
	}

	buze_event_data_show_properties_t* _luaresult = &self->show_properties;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_show_parameters(lua_State *L) {
	buze_event_data_t* self = (buze_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:show_parameters");
		return 0;
	}

	buze_event_data_show_machine_parameter_t* _luaresult = &self->show_parameters;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_show_pattern(lua_State *L) {
	buze_event_data_t* self = (buze_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:show_pattern");
		return 0;
	}

	buze_event_data_show_pattern_t* _luaresult = &self->show_pattern;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_show_pattern_format(lua_State *L) {
	buze_event_data_t* self = (buze_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:show_pattern_format");
		return 0;
	}

	buze_event_data_show_pattern_format_t* _luaresult = &self->show_pattern_format;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg event_data_lib[] = {
	{ "get_change_pattern_order", event_data_get_change_pattern_order }, 
	{ "get_change_pattern_row", event_data_get_change_pattern_row }, 
	{ "get_show_properties", event_data_get_show_properties }, 
	{ "get_show_parameters", event_data_get_show_parameters }, 
	{ "get_show_pattern", event_data_get_show_pattern }, 
	{ "get_show_pattern_format", event_data_get_show_pattern_format }, 
	{ NULL, NULL }
};

static int application_create(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Application:create");
		return 0;
	}
	buze_host_module_t* module = (buze_host_module_t*)lua_touserdata(L, 2);
	const char* globalPath = luaL_tolstring(L, 3, &len);
	const char* userPath = luaL_tolstring(L, 4, &len);
	const char* tempPath = luaL_tolstring(L, 5, &len);
	buze_application_t* _luaresult = buze_application_create(module, globalPath, userPath, tempPath);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int application_get_host_module(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Application:get_host_module");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:get_host_module");
		return 0;
	}

	buze_host_module_t* _luaresult = buze_application_get_host_module(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int application_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Application:destroy");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:destroy");
		return 0;
	}

	buze_application_destroy(self);
	return 0;
}

static int application_initialize(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Application:initialize");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:initialize");
		return 0;
	}

	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 3);
	zzub_audiodriver_t* driver = (zzub_audiodriver_t*)lua_touserdata(L, 4);
	buze_application_initialize(self, player, driver);
	return 0;
}

static int application_open_midi_devices_from_config(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Application:open_midi_devices_from_config");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:open_midi_devices_from_config");
		return 0;
	}

	int _luaresult = buze_application_open_midi_devices_from_config(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int application_create_audio_device_from_config(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Application:create_audio_device_from_config");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:create_audio_device_from_config");
		return 0;
	}

	int _luaresult = buze_application_create_audio_device_from_config(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int application_create_audio_device(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Application:create_audio_device");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:create_audio_device");
		return 0;
	}

	const char* outdevicename = luaL_tolstring(L, 3, &len);
	const char* indevicename = luaL_tolstring(L, 4, &len);
	int rate = lua_tonumber(L, 5);
	int buffersize = lua_tonumber(L, 6);
	int masterchannel = lua_tonumber(L, 7);
	int save = lua_toboolean(L, 8);
	int _luaresult = buze_application_create_audio_device(self, outdevicename, indevicename, rate, buffersize, masterchannel, save);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int application_enable_silent_driver(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Application:enable_silent_driver");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:enable_silent_driver");
		return 0;
	}

	int enable = lua_toboolean(L, 3);
	buze_application_enable_silent_driver(self, enable);
	return 0;
}

static int application_get_audio_driver(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Application:get_audio_driver");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:get_audio_driver");
		return 0;
	}

	zzub_audiodriver_t* _luaresult = buze_application_get_audio_driver(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int application_release_audio_driver(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Application:release_audio_driver");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:release_audio_driver");
		return 0;
	}

	buze_application_release_audio_driver(self);
	return 0;
}

static int application_get_configuration(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Application:get_configuration");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:get_configuration");
		return 0;
	}

	buze_configuration_t* _luaresult = buze_application_get_configuration(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int application_map_path(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Application:map_path");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:map_path");
		return 0;
	}

	const char* path = luaL_tolstring(L, 3, &len);
	int type = lua_tonumber(L, 4);
	const char* _luaresult = buze_application_map_path(self, path, type);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int application_show_wait_window(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Application:show_wait_window");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:show_wait_window");
		return 0;
	}

	buze_application_show_wait_window(self);
	return 0;
}

static int application_set_wait_text(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Application:set_wait_text");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:set_wait_text");
		return 0;
	}

	const char* text = luaL_tolstring(L, 3, &len);
	buze_application_set_wait_text(self, text);
	return 0;
}

static int application_hide_wait_window(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Application:hide_wait_window");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:hide_wait_window");
		return 0;
	}

	void* focusWnd = lua_touserdata(L, 3);
	buze_application_hide_wait_window(self, focusWnd);
	return 0;
}

static int application_get_theme_color(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Application:get_theme_color");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:get_theme_color");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	unsigned int _luaresult = buze_application_get_theme_color(self, name);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int application_get_theme_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Application:get_theme_name");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:get_theme_name");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	const char* _luaresult = buze_application_get_theme_name(self, index);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int application_get_theme_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Application:get_theme_count");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:get_theme_count");
		return 0;
	}

	int _luaresult = buze_application_get_theme_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int application_load_theme(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Application:load_theme");
		return 0;
	}
	buze_application_t* self = (buze_application_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Application:load_theme");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	buze_application_load_theme(self, name);
	return 0;
}

static const luaL_Reg application_lib[] = {
	{ "create", application_create },
	{ "get_host_module", application_get_host_module },
	{ "destroy", application_destroy },
	{ "initialize", application_initialize },
	{ "open_midi_devices_from_config", application_open_midi_devices_from_config },
	{ "create_audio_device_from_config", application_create_audio_device_from_config },
	{ "create_audio_device", application_create_audio_device },
	{ "enable_silent_driver", application_enable_silent_driver },
	{ "get_audio_driver", application_get_audio_driver },
	{ "release_audio_driver", application_release_audio_driver },
	{ "get_configuration", application_get_configuration },
	{ "map_path", application_map_path },
	{ "show_wait_window", application_show_wait_window },
	{ "set_wait_text", application_set_wait_text },
	{ "hide_wait_window", application_hide_wait_window },
	{ "get_theme_color", application_get_theme_color },
	{ "get_theme_name", application_get_theme_name },
	{ "get_theme_count", application_get_theme_count },
	{ "load_theme", application_load_theme },
	{ NULL, NULL }
};

static int main_frame_initialize(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for MainFrame:initialize");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:initialize");
		return 0;
	}

	void* parentwnd = lua_touserdata(L, 3);
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 4);
	int _luaresult = buze_main_frame_initialize(self, parentwnd, player);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int main_frame_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:destroy");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:destroy");
		return 0;
	}

	buze_main_frame_destroy(self);
	return 0;
}

static int main_frame_register_window_factory(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for MainFrame:register_window_factory");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:register_window_factory");
		return 0;
	}

	buze_window_factory_t* info = (buze_window_factory_t*)lua_touserdata(L, 3);
	buze_main_frame_register_window_factory(self, info);
	return 0;
}

static int main_frame_get_wnd(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_wnd");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_wnd");
		return 0;
	}

	void* _luaresult = buze_main_frame_get_wnd(self);
	lua_pushlightuserdata(L, _luaresult);
	return 1;
}

static int main_frame_add_timer_handler(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for MainFrame:add_timer_handler");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:add_timer_handler");
		return 0;
	}

	buze_window_t* wnd = (buze_window_t*)lua_touserdata(L, 3);
	buze_main_frame_add_timer_handler(self, wnd);
	return 0;
}

static int main_frame_remove_timer_handler(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for MainFrame:remove_timer_handler");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:remove_timer_handler");
		return 0;
	}

	buze_window_t* wnd = (buze_window_t*)lua_touserdata(L, 3);
	buze_main_frame_remove_timer_handler(self, wnd);
	return 0;
}

static int main_frame_viewstack_insert(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for MainFrame:viewstack_insert");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:viewstack_insert");
		return 0;
	}

	buze_window_t* wnd = (buze_window_t*)lua_touserdata(L, 3);
	buze_main_frame_viewstack_insert(self, wnd);
	return 0;
}

static int main_frame_get_accelerators(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for MainFrame:get_accelerators");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_accelerators");
		return 0;
	}

	const char* viewname = luaL_tolstring(L, 3, &len);
	void* _luaresult = buze_main_frame_get_accelerators(self, viewname);
	lua_pushlightuserdata(L, _luaresult);
	return 1;
}

static int main_frame_get_document(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_document");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_document");
		return 0;
	}

	buze_document_t* _luaresult = buze_main_frame_get_document(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int main_frame_get_application(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_application");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_application");
		return 0;
	}

	buze_application_t* _luaresult = buze_main_frame_get_application(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int main_frame_get_player(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_player");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_player");
		return 0;
	}

	zzub_player_t* _luaresult = buze_main_frame_get_player(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int main_frame_get_focused_view(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_focused_view");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_focused_view");
		return 0;
	}

	buze_window_t* _luaresult = buze_main_frame_get_focused_view(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int main_frame_is_float_view(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for MainFrame:is_float_view");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:is_float_view");
		return 0;
	}

	buze_window_t* wnd = (buze_window_t*)lua_touserdata(L, 3);
	int _luaresult = buze_main_frame_is_float_view(self, wnd);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int main_frame_set_focus_to(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for MainFrame:set_focus_to");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:set_focus_to");
		return 0;
	}

	buze_window_t* wnd = (buze_window_t*)lua_touserdata(L, 3);
	buze_main_frame_set_focus_to(self, wnd);
	return 0;
}

static int main_frame_get_view(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for MainFrame:get_view");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_view");
		return 0;
	}

	const char* viewname = luaL_tolstring(L, 3, &len);
	int viewid = lua_tonumber(L, 4);
	buze_window_t* _luaresult = buze_main_frame_get_view(self, viewname, viewid);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int main_frame_open_view(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for MainFrame:open_view");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:open_view");
		return 0;
	}

	const char* viewname = luaL_tolstring(L, 3, &len);
	const char* label = luaL_tolstring(L, 4, &len);
	int viewid = lua_tonumber(L, 5);
	int x = lua_tonumber(L, 6);
	int y = lua_tonumber(L, 7);
	buze_window_t* _luaresult = buze_main_frame_open_view(self, viewname, label, viewid, x, y);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int main_frame_close_view(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for MainFrame:close_view");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:close_view");
		return 0;
	}

	buze_window_t* wnd = (buze_window_t*)lua_touserdata(L, 3);
	buze_main_frame_close_view(self, wnd);
	return 0;
}

static int main_frame_get_open_filename(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_open_filename");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_open_filename");
		return 0;
	}

	const char* _luaresult = buze_main_frame_get_open_filename(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int main_frame_get_save_filename(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_save_filename");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_save_filename");
		return 0;
	}

	const char* _luaresult = buze_main_frame_get_save_filename(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int main_frame_get_plugin_menu_create(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_plugin_menu_create");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_plugin_menu_create");
		return 0;
	}

	void* _luaresult = buze_main_frame_get_plugin_menu_create(self);
	lua_pushlightuserdata(L, _luaresult);
	return 1;
}

static int main_frame_get_plugin_menu_insert_after(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_plugin_menu_insert_after");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_plugin_menu_insert_after");
		return 0;
	}

	void* _luaresult = buze_main_frame_get_plugin_menu_insert_after(self);
	lua_pushlightuserdata(L, _luaresult);
	return 1;
}

static int main_frame_get_plugin_menu_insert_before(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_plugin_menu_insert_before");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_plugin_menu_insert_before");
		return 0;
	}

	void* _luaresult = buze_main_frame_get_plugin_menu_insert_before(self);
	lua_pushlightuserdata(L, _luaresult);
	return 1;
}

static int main_frame_get_plugin_menu_replace(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_plugin_menu_replace");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_plugin_menu_replace");
		return 0;
	}

	void* _luaresult = buze_main_frame_get_plugin_menu_replace(self);
	lua_pushlightuserdata(L, _luaresult);
	return 1;
}

static int main_frame_get_main_menu(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_main_menu");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_main_menu");
		return 0;
	}

	void* _luaresult = buze_main_frame_get_main_menu(self);
	lua_pushlightuserdata(L, _luaresult);
	return 1;
}

static int main_frame_add_menu_keys(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for MainFrame:add_menu_keys");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:add_menu_keys");
		return 0;
	}

	const char* viewname = luaL_tolstring(L, 3, &len);
	void* menu = lua_touserdata(L, 4);
	buze_main_frame_add_menu_keys(self, viewname, menu);
	return 0;
}

static int main_frame_register_event(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:register_event");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:register_event");
		return 0;
	}

	int _luaresult = buze_main_frame_register_event(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int main_frame_register_accelerator_event(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for MainFrame:register_accelerator_event");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:register_accelerator_event");
		return 0;
	}

	const char* viewname = luaL_tolstring(L, 3, &len);
	const char* defaulthotkey = luaL_tolstring(L, 4, &len);
	int evcode = lua_tonumber(L, 5);
	unsigned short _luaresult = buze_main_frame_register_accelerator_event(self, viewname, defaulthotkey, evcode);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int main_frame_register_accelerator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for MainFrame:register_accelerator");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:register_accelerator");
		return 0;
	}

	const char* viewname = luaL_tolstring(L, 3, &len);
	const char* name = luaL_tolstring(L, 4, &len);
	const char* defaulthotkey = luaL_tolstring(L, 5, &len);
	unsigned short id = lua_tonumber(L, 6);
	buze_main_frame_register_accelerator(self, viewname, name, defaulthotkey, id);
	return 0;
}

static int main_frame_show_plugin_parameters(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for MainFrame:show_plugin_parameters");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:show_plugin_parameters");
		return 0;
	}

	zzub_plugin_t* p = (zzub_plugin_t*)lua_touserdata(L, 3);
	int modehint = lua_tonumber(L, 4);
	int x = lua_tonumber(L, 5);
	int y = lua_tonumber(L, 6);
	buze_main_frame_show_plugin_parameters(self, p, modehint, x, y);
	return 0;
}

static int main_frame_get_keyjazz_map(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_keyjazz_map");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_keyjazz_map");
		return 0;
	}

	void* _luaresult = buze_main_frame_get_keyjazz_map(self);
	lua_pushlightuserdata(L, _luaresult);
	return 1;
}

static int main_frame_get_view_by_wnd(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for MainFrame:get_view_by_wnd");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_view_by_wnd");
		return 0;
	}

	void* wnd = lua_touserdata(L, 3);
	buze_window_t* _luaresult = buze_main_frame_get_view_by_wnd(self, wnd);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int main_frame_get_program_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for MainFrame:get_program_name");
		return 0;
	}
	buze_main_frame_t* self = (buze_main_frame_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MainFrame:get_program_name");
		return 0;
	}

	const char* _luaresult = buze_main_frame_get_program_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static const luaL_Reg main_frame_lib[] = {
	{ "initialize", main_frame_initialize },
	{ "destroy", main_frame_destroy },
	{ "register_window_factory", main_frame_register_window_factory },
	{ "get_wnd", main_frame_get_wnd },
	{ "add_timer_handler", main_frame_add_timer_handler },
	{ "remove_timer_handler", main_frame_remove_timer_handler },
	{ "viewstack_insert", main_frame_viewstack_insert },
	{ "get_accelerators", main_frame_get_accelerators },
	{ "get_document", main_frame_get_document },
	{ "get_application", main_frame_get_application },
	{ "get_player", main_frame_get_player },
	{ "get_focused_view", main_frame_get_focused_view },
	{ "is_float_view", main_frame_is_float_view },
	{ "set_focus_to", main_frame_set_focus_to },
	{ "get_view", main_frame_get_view },
	{ "open_view", main_frame_open_view },
	{ "close_view", main_frame_close_view },
	{ "get_open_filename", main_frame_get_open_filename },
	{ "get_save_filename", main_frame_get_save_filename },
	{ "get_plugin_menu_create", main_frame_get_plugin_menu_create },
	{ "get_plugin_menu_insert_after", main_frame_get_plugin_menu_insert_after },
	{ "get_plugin_menu_insert_before", main_frame_get_plugin_menu_insert_before },
	{ "get_plugin_menu_replace", main_frame_get_plugin_menu_replace },
	{ "get_main_menu", main_frame_get_main_menu },
	{ "add_menu_keys", main_frame_add_menu_keys },
	{ "register_event", main_frame_register_event },
	{ "register_accelerator_event", main_frame_register_accelerator_event },
	{ "register_accelerator", main_frame_register_accelerator },
	{ "show_plugin_parameters", main_frame_show_plugin_parameters },
	{ "get_keyjazz_map", main_frame_get_keyjazz_map },
	{ "get_view_by_wnd", main_frame_get_view_by_wnd },
	{ "get_program_name", main_frame_get_program_name },
	{ NULL, NULL }
};

static int window_get_wnd(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Window:get_wnd");
		return 0;
	}
	buze_window_t* self = (buze_window_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Window:get_wnd");
		return 0;
	}

	void* _luaresult = buze_window_get_wnd(self);
	lua_pushlightuserdata(L, _luaresult);
	return 1;
}

static const luaL_Reg window_lib[] = {
	{ "get_wnd", window_get_wnd },
	{ NULL, NULL }
};

static int document_add_view(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:add_view");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:add_view");
		return 0;
	}

	buze_event_handler_t* view = (buze_event_handler_t*)lua_touserdata(L, 3);
	buze_document_add_view(self, view);
	return 0;
}

static int document_remove_view(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:remove_view");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:remove_view");
		return 0;
	}

	buze_event_handler_t* view = (buze_event_handler_t*)lua_touserdata(L, 3);
	buze_document_remove_view(self, view);
	return 0;
}

static int document_notify_views(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Document:notify_views");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:notify_views");
		return 0;
	}

	buze_window_t* sender = (buze_window_t*)lua_touserdata(L, 3);
	int hint = lua_tonumber(L, 4);
	void* param = lua_touserdata(L, 5);
	buze_document_notify_views(self, sender, hint, param);
	return 0;
}

void(*luacallback_buze_document_add_callback)(lua_State* L, buze_document_t* document, buze_callback_t callback, void* tag);

static int document_add_callback(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Document:add_callback");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:add_callback");
		return 0;
	}

	buze_callback_t callback = buze_callback_alloc(L, 3);
	void* tag = lua_touserdata(L, 4);
	if (luacallback_buze_document_add_callback) luacallback_buze_document_add_callback(L, self,  callback,  tag);
	buze_document_add_callback(self, callback, tag);
	return 0;
}

void(*luacallback_buze_document_remove_callback)(lua_State* L, buze_document_t* document, buze_callback_t callback, void* tag);

static int document_remove_callback(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Document:remove_callback");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:remove_callback");
		return 0;
	}

	buze_callback_t callback = buze_callback_alloc(L, 3);
	void* tag = lua_touserdata(L, 4);
	if (luacallback_buze_document_remove_callback) luacallback_buze_document_remove_callback(L, self,  callback,  tag);
	buze_document_remove_callback(self, callback, tag);
	return 0;
}

static int document_get_octave(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_octave");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_octave");
		return 0;
	}

	int _luaresult = buze_document_get_octave(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_set_octave(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:set_octave");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:set_octave");
		return 0;
	}

	int oct = lua_tonumber(L, 3);
	buze_document_set_octave(self, oct);
	return 0;
}

static int document_get_plugin_non_song(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:get_plugin_non_song");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_plugin_non_song");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int _luaresult = buze_document_get_plugin_non_song(self, plugin);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_set_plugin_non_song(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Document:set_plugin_non_song");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:set_plugin_non_song");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int state = lua_toboolean(L, 4);
	buze_document_set_plugin_non_song(self, plugin, state);
	return 0;
}

static int document_get_plugin_parameter_view_mode(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:get_plugin_parameter_view_mode");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_plugin_parameter_view_mode");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int _luaresult = buze_document_get_plugin_parameter_view_mode(self, plugin);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_set_plugin_parameter_view_mode(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Document:set_plugin_parameter_view_mode");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:set_plugin_parameter_view_mode");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int mode = lua_tonumber(L, 4);
	buze_document_set_plugin_parameter_view_mode(self, plugin, mode);
	return 0;
}

static int document_get_plugin_last_preset(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:get_plugin_last_preset");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_plugin_last_preset");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	const char* _luaresult = buze_document_get_plugin_last_preset(self, plugin);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int document_set_plugin_last_preset(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Document:set_plugin_last_preset");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:set_plugin_last_preset");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	const char* preset = luaL_tolstring(L, 4, &len);
	buze_document_set_plugin_last_preset(self, plugin, preset);
	return 0;
}

static int document_get_player(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_player");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_player");
		return 0;
	}

	zzub_player_t* _luaresult = buze_document_get_player(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_get_stream_plugin_uri_for_file(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:get_stream_plugin_uri_for_file");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_stream_plugin_uri_for_file");
		return 0;
	}

	const char* path = luaL_tolstring(L, 3, &len);
	const char* _luaresult = buze_document_get_stream_plugin_uri_for_file(self, path);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int document_play_plugin_note(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Document:play_plugin_note");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:play_plugin_note");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int note = lua_tonumber(L, 4);
	int prevnote = lua_tonumber(L, 5);
	buze_document_play_plugin_note(self, plugin, note, prevnote);
	return 0;
}

static int document_play_stream(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Document:play_stream");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:play_stream");
		return 0;
	}

	int note = lua_tonumber(L, 3);
	int offset = lua_tonumber(L, 4);
	int length = lua_tonumber(L, 5);
	const char* pluginuri = luaL_tolstring(L, 6, &len);
	const char* dataurl = luaL_tolstring(L, 7, &len);
	buze_document_play_stream(self, note, offset, length, pluginuri, dataurl);
	return 0;
}

static int document_keyjazz_key_down(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Document:keyjazz_key_down");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:keyjazz_key_down");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int wParam = lua_tonumber(L, 4);
	int note = lua_tonumber(L, 5);
	buze_document_keyjazz_key_down(self, plugin, wParam, note);
	return 0;
}

static int document_keyjazz_key_up(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Document:keyjazz_key_up");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:keyjazz_key_up");
		return 0;
	}

	int wParam = lua_tonumber(L, 3);
	int note;
	zzub_plugin_t* plugin;
	int _luaresult = buze_document_keyjazz_key_up(self, wParam, &note, &plugin);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_keyjazz_release(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:keyjazz_release");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:keyjazz_release");
		return 0;
	}

	int sendnoteoffs = lua_toboolean(L, 3);
	buze_document_keyjazz_release(self, sendnoteoffs);
	return 0;
}

static int document_get_stream_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_stream_plugin");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_stream_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = buze_document_get_stream_plugin(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_delete_stream_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:delete_stream_plugin");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:delete_stream_plugin");
		return 0;
	}

	buze_document_delete_stream_plugin(self);
	return 0;
}

static int document_get_configuration(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_configuration");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_configuration");
		return 0;
	}

	buze_configuration_t* _luaresult = buze_document_get_configuration(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_get_solo_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_solo_plugin");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_solo_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = buze_document_get_solo_plugin(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_set_solo_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Document:set_solo_plugin");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:set_solo_plugin");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int state = lua_toboolean(L, 4);
	buze_document_set_solo_plugin(self, plugin, state);
	return 0;
}

static int document_get_plugin_helpfile(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:get_plugin_helpfile");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_plugin_helpfile");
		return 0;
	}

	zzub_pluginloader_t* loader = (zzub_pluginloader_t*)lua_touserdata(L, 3);
	const char* _luaresult = buze_document_get_plugin_helpfile(self, loader);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int document_import_song(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Document:import_song");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:import_song");
		return 0;
	}

	const char* filename = luaL_tolstring(L, 3, &len);
	int flags = lua_tonumber(L, 4);
	float x = lua_tonumber(L, 5);
	float y = lua_tonumber(L, 6);
	char* errormessages;
	int outsize = lua_tonumber(L, 8);
	int _luaresult = buze_document_import_song(self, filename, flags, x, y, errormessages, outsize);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_load_plugin_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:load_plugin_index");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:load_plugin_index");
		return 0;
	}

	int _luaresult = buze_document_load_plugin_index(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_get_plugin_index_item_by_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:get_plugin_index_item_by_index");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_plugin_index_item_by_index");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	buze_plugin_index_item_t* _luaresult = buze_document_get_plugin_index_item_by_index(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_get_plugin_index_root(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_plugin_index_root");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_plugin_index_root");
		return 0;
	}

	buze_plugin_index_item_t* _luaresult = buze_document_get_plugin_index_root(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_is_dirty(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:is_dirty");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:is_dirty");
		return 0;
	}

	int _luaresult = buze_document_is_dirty(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_set_current_file(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:set_current_file");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:set_current_file");
		return 0;
	}

	const char* fullpath = luaL_tolstring(L, 3, &len);
	buze_document_set_current_file(self, fullpath);
	return 0;
}

static int document_get_current_filename(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_filename");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_filename");
		return 0;
	}

	const char* _luaresult = buze_document_get_current_filename(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int document_get_current_path(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_path");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_path");
		return 0;
	}

	const char* _luaresult = buze_document_get_current_path(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int document_get_current_extension(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_extension");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_extension");
		return 0;
	}

	const char* _luaresult = buze_document_get_current_extension(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int document_clear_song(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:clear_song");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:clear_song");
		return 0;
	}

	buze_document_clear_song(self);
	return 0;
}

static int document_save_file(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Document:save_file");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:save_file");
		return 0;
	}

	const char* filename = luaL_tolstring(L, 3, &len);
	int withwaves = lua_toboolean(L, 4);
	int _luaresult = buze_document_save_file(self, filename, withwaves);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_create_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Document:create_plugin");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:create_plugin");
		return 0;
	}

	const char* uri = luaL_tolstring(L, 3, &len);
	const char* instrumentname = luaL_tolstring(L, 4, &len);
	float x = lua_tonumber(L, 5);
	float y = lua_tonumber(L, 6);
	zzub_plugin_group_t* plugingroup = (zzub_plugin_group_t*)lua_touserdata(L, 7);
	zzub_plugin_t* _luaresult = buze_document_create_plugin(self, uri, instrumentname, x, y, plugingroup);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_create_plugin_between(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Document:create_plugin_between");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:create_plugin_between");
		return 0;
	}

	zzub_plugin_t* to_plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	zzub_plugin_t* from_plugin = (zzub_plugin_t*)lua_touserdata(L, 4);
	const char* uri = luaL_tolstring(L, 5, &len);
	const char* instrumentname = luaL_tolstring(L, 6, &len);
	zzub_plugin_t* _luaresult = buze_document_create_plugin_between(self, to_plugin, from_plugin, uri, instrumentname);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_create_plugin_before(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Document:create_plugin_before");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:create_plugin_before");
		return 0;
	}

	zzub_plugin_t* srcplugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	const char* uri = luaL_tolstring(L, 4, &len);
	const char* instrumentname = luaL_tolstring(L, 5, &len);
	zzub_plugin_t* _luaresult = buze_document_create_plugin_before(self, srcplugin, uri, instrumentname);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_create_plugin_after(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Document:create_plugin_after");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:create_plugin_after");
		return 0;
	}

	zzub_plugin_t* srcplugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	const char* uri = luaL_tolstring(L, 4, &len);
	const char* instrumentname = luaL_tolstring(L, 5, &len);
	zzub_plugin_t* _luaresult = buze_document_create_plugin_after(self, srcplugin, uri, instrumentname);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_create_plugin_replace(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Document:create_plugin_replace");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:create_plugin_replace");
		return 0;
	}

	zzub_plugin_t* srcplugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	const char* uri = luaL_tolstring(L, 4, &len);
	const char* instrumentname = luaL_tolstring(L, 5, &len);
	zzub_plugin_t* _luaresult = buze_document_create_plugin_replace(self, srcplugin, uri, instrumentname);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_create_default_document(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:create_default_document");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:create_default_document");
		return 0;
	}

	buze_document_create_default_document(self);
	return 0;
}

static int document_create_default_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Document:create_default_format");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:create_default_format");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int simple = lua_toboolean(L, 4);
	zzub_pattern_format_t* _luaresult = buze_document_create_default_format(self, plugin, simple);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_extend_pattern_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Document:extend_pattern_format");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:extend_pattern_format");
		return 0;
	}

	zzub_pattern_format_t* format = (zzub_pattern_format_t*)lua_touserdata(L, 3);
	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 4);
	int simple = lua_toboolean(L, 5);
	buze_document_extend_pattern_format(self, format, plugin, simple);
	return 0;
}

static int document_delete_plugin_smart(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Document:delete_plugin_smart");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:delete_plugin_smart");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	buze_document_delete_plugin_smart(self, plugin);
	return 0;
}

static int document_get_current_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_plugin");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = buze_document_get_current_plugin(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_get_current_pattern(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_pattern");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_pattern");
		return 0;
	}

	zzub_pattern_t* _luaresult = buze_document_get_current_pattern(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_get_current_pattern_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_pattern_format");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_pattern_format");
		return 0;
	}

	zzub_pattern_format_t* _luaresult = buze_document_get_current_pattern_format(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_get_current_connection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_connection");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_connection");
		return 0;
	}

	zzub_connection_t* _luaresult = buze_document_get_current_connection(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_get_current_wave(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_wave");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_wave");
		return 0;
	}

	zzub_wave_t* _luaresult = buze_document_get_current_wave(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_get_current_wavelevel(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_wavelevel");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_wavelevel");
		return 0;
	}

	zzub_wavelevel_t* _luaresult = buze_document_get_current_wavelevel(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int document_import_wave(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Document:import_wave");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:import_wave");
		return 0;
	}

	const char* filename = luaL_tolstring(L, 3, &len);
	zzub_wave_t* target = (zzub_wave_t*)lua_touserdata(L, 4);
	int _luaresult = buze_document_import_wave(self, filename, target);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_get_current_order_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_order_index");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_order_index");
		return 0;
	}

	int _luaresult = buze_document_get_current_order_index(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_get_current_order_pattern_row(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_order_pattern_row");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_order_pattern_row");
		return 0;
	}

	int _luaresult = buze_document_get_current_order_pattern_row(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int document_get_current_pattern_row(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Document:get_current_pattern_row");
		return 0;
	}
	buze_document_t* self = (buze_document_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Document:get_current_pattern_row");
		return 0;
	}

	int _luaresult = buze_document_get_current_pattern_row(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg document_lib[] = {
	{ "add_view", document_add_view },
	{ "remove_view", document_remove_view },
	{ "notify_views", document_notify_views },
	{ "add_callback", document_add_callback },
	{ "remove_callback", document_remove_callback },
	{ "get_octave", document_get_octave },
	{ "set_octave", document_set_octave },
	{ "get_plugin_non_song", document_get_plugin_non_song },
	{ "set_plugin_non_song", document_set_plugin_non_song },
	{ "get_plugin_parameter_view_mode", document_get_plugin_parameter_view_mode },
	{ "set_plugin_parameter_view_mode", document_set_plugin_parameter_view_mode },
	{ "get_plugin_last_preset", document_get_plugin_last_preset },
	{ "set_plugin_last_preset", document_set_plugin_last_preset },
	{ "get_player", document_get_player },
	{ "get_stream_plugin_uri_for_file", document_get_stream_plugin_uri_for_file },
	{ "play_plugin_note", document_play_plugin_note },
	{ "play_stream", document_play_stream },
	{ "keyjazz_key_down", document_keyjazz_key_down },
	{ "keyjazz_key_up", document_keyjazz_key_up },
	{ "keyjazz_release", document_keyjazz_release },
	{ "get_stream_plugin", document_get_stream_plugin },
	{ "delete_stream_plugin", document_delete_stream_plugin },
	{ "get_configuration", document_get_configuration },
	{ "get_solo_plugin", document_get_solo_plugin },
	{ "set_solo_plugin", document_set_solo_plugin },
	{ "get_plugin_helpfile", document_get_plugin_helpfile },
	{ "import_song", document_import_song },
	{ "load_plugin_index", document_load_plugin_index },
	{ "get_plugin_index_item_by_index", document_get_plugin_index_item_by_index },
	{ "get_plugin_index_root", document_get_plugin_index_root },
	{ "is_dirty", document_is_dirty },
	{ "set_current_file", document_set_current_file },
	{ "get_current_filename", document_get_current_filename },
	{ "get_current_path", document_get_current_path },
	{ "get_current_extension", document_get_current_extension },
	{ "clear_song", document_clear_song },
	{ "save_file", document_save_file },
	{ "create_plugin", document_create_plugin },
	{ "create_plugin_between", document_create_plugin_between },
	{ "create_plugin_before", document_create_plugin_before },
	{ "create_plugin_after", document_create_plugin_after },
	{ "create_plugin_replace", document_create_plugin_replace },
	{ "create_default_document", document_create_default_document },
	{ "create_default_format", document_create_default_format },
	{ "extend_pattern_format", document_extend_pattern_format },
	{ "delete_plugin_smart", document_delete_plugin_smart },
	{ "get_current_plugin", document_get_current_plugin },
	{ "get_current_pattern", document_get_current_pattern },
	{ "get_current_pattern_format", document_get_current_pattern_format },
	{ "get_current_connection", document_get_current_connection },
	{ "get_current_wave", document_get_current_wave },
	{ "get_current_wavelevel", document_get_current_wavelevel },
	{ "import_wave", document_import_wave },
	{ "get_current_order_index", document_get_current_order_index },
	{ "get_current_order_pattern_row", document_get_current_order_pattern_row },
	{ "get_current_pattern_row", document_get_current_pattern_row },
	{ NULL, NULL }
};

static int plugin_index_item_get_type(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIndexItem:get_type");
		return 0;
	}
	buze_plugin_index_item_t* self = (buze_plugin_index_item_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIndexItem:get_type");
		return 0;
	}

	int _luaresult = buze_plugin_index_item_get_type(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_index_item_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIndexItem:get_name");
		return 0;
	}
	buze_plugin_index_item_t* self = (buze_plugin_index_item_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIndexItem:get_name");
		return 0;
	}

	const char* _luaresult = buze_plugin_index_item_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_index_item_is_hidden(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIndexItem:is_hidden");
		return 0;
	}
	buze_plugin_index_item_t* self = (buze_plugin_index_item_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIndexItem:is_hidden");
		return 0;
	}

	int _luaresult = buze_plugin_index_item_is_hidden(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_index_item_is_preloaded(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIndexItem:is_preloaded");
		return 0;
	}
	buze_plugin_index_item_t* self = (buze_plugin_index_item_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIndexItem:is_preloaded");
		return 0;
	}

	int _luaresult = buze_plugin_index_item_is_preloaded(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_index_item_get_filename(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIndexItem:get_filename");
		return 0;
	}
	buze_plugin_index_item_t* self = (buze_plugin_index_item_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIndexItem:get_filename");
		return 0;
	}

	const char* _luaresult = buze_plugin_index_item_get_filename(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_index_item_get_instrumentname(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIndexItem:get_instrumentname");
		return 0;
	}
	buze_plugin_index_item_t* self = (buze_plugin_index_item_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIndexItem:get_instrumentname");
		return 0;
	}

	const char* _luaresult = buze_plugin_index_item_get_instrumentname(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_index_item_get_sub_item(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PluginIndexItem:get_sub_item");
		return 0;
	}
	buze_plugin_index_item_t* self = (buze_plugin_index_item_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIndexItem:get_sub_item");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	buze_plugin_index_item_t* _luaresult = buze_plugin_index_item_get_sub_item(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_index_item_get_sub_item_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIndexItem:get_sub_item_count");
		return 0;
	}
	buze_plugin_index_item_t* self = (buze_plugin_index_item_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIndexItem:get_sub_item_count");
		return 0;
	}

	int _luaresult = buze_plugin_index_item_get_sub_item_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_index_item_get_separator_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIndexItem:get_separator_id");
		return 0;
	}
	buze_plugin_index_item_t* self = (buze_plugin_index_item_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIndexItem:get_separator_id");
		return 0;
	}

	const char* _luaresult = buze_plugin_index_item_get_separator_id(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static const luaL_Reg plugin_index_item_lib[] = {
	{ "get_type", plugin_index_item_get_type },
	{ "get_name", plugin_index_item_get_name },
	{ "is_hidden", plugin_index_item_is_hidden },
	{ "is_preloaded", plugin_index_item_is_preloaded },
	{ "get_filename", plugin_index_item_get_filename },
	{ "get_instrumentname", plugin_index_item_get_instrumentname },
	{ "get_sub_item", plugin_index_item_get_sub_item },
	{ "get_sub_item_count", plugin_index_item_get_sub_item_count },
	{ "get_separator_id", plugin_index_item_get_separator_id },
	{ NULL, NULL }
};

static int configuration_add_sample_path(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Configuration:add_sample_path");
		return 0;
	}
	buze_configuration_t* self = (buze_configuration_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Configuration:add_sample_path");
		return 0;
	}

	const char* path = luaL_tolstring(L, 3, &len);
	buze_configuration_add_sample_path(self, path);
	return 0;
}

static int configuration_remove_sample_path(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Configuration:remove_sample_path");
		return 0;
	}
	buze_configuration_t* self = (buze_configuration_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Configuration:remove_sample_path");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	buze_configuration_remove_sample_path(self, index);
	return 0;
}

static int configuration_get_sample_path_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Configuration:get_sample_path_count");
		return 0;
	}
	buze_configuration_t* self = (buze_configuration_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Configuration:get_sample_path_count");
		return 0;
	}

	int _luaresult = buze_configuration_get_sample_path_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int configuration_get_sample_path(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Configuration:get_sample_path");
		return 0;
	}
	buze_configuration_t* self = (buze_configuration_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Configuration:get_sample_path");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	const char* _luaresult = buze_configuration_get_sample_path(self, index);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int configuration_get_fixed_width_font(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Configuration:get_fixed_width_font");
		return 0;
	}
	buze_configuration_t* self = (buze_configuration_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Configuration:get_fixed_width_font");
		return 0;
	}

	const char* _luaresult = buze_configuration_get_fixed_width_font(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int configuration_get_toolbars_locked(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Configuration:get_toolbars_locked");
		return 0;
	}
	buze_configuration_t* self = (buze_configuration_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Configuration:get_toolbars_locked");
		return 0;
	}

	int _luaresult = buze_configuration_get_toolbars_locked(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg configuration_lib[] = {
	{ "add_sample_path", configuration_add_sample_path },
	{ "remove_sample_path", configuration_remove_sample_path },
	{ "get_sample_path_count", configuration_get_sample_path_count },
	{ "get_sample_path", configuration_get_sample_path },
	{ "get_fixed_width_font", configuration_get_fixed_width_font },
	{ "get_toolbars_locked", configuration_get_toolbars_locked },
	{ NULL, NULL }
};

int luaopen_buze(lua_State* L) {
	lua_newtable(L);
	lua_pushliteral(L, "version"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_setglobal(L, "buze");

	lua_newtable(L);
	lua_pushliteral(L, "update_new_document"); lua_pushnumber(L, 8192); lua_settable(L, -3);
	lua_pushliteral(L, "update_pre_save_document"); lua_pushnumber(L, 8193); lua_settable(L, -3);
	lua_pushliteral(L, "update_post_save_document"); lua_pushnumber(L, 8194); lua_settable(L, -3);
	lua_pushliteral(L, "update_pre_clear_document"); lua_pushnumber(L, 8195); lua_settable(L, -3);
	lua_pushliteral(L, "update_post_clear_document"); lua_pushnumber(L, 8196); lua_settable(L, -3);
	lua_pushliteral(L, "update_pre_open_document"); lua_pushnumber(L, 8197); lua_settable(L, -3);
	lua_pushliteral(L, "update_post_open_document"); lua_pushnumber(L, 8198); lua_settable(L, -3);
	lua_pushliteral(L, "update_pre_mixdown"); lua_pushnumber(L, 8199); lua_settable(L, -3);
	lua_pushliteral(L, "update_post_mixdown"); lua_pushnumber(L, 8200); lua_settable(L, -3);
	lua_pushliteral(L, "update_properties"); lua_pushnumber(L, 8201); lua_settable(L, -3);
	lua_pushliteral(L, "update_theme"); lua_pushnumber(L, 8202); lua_settable(L, -3);
	lua_pushliteral(L, "update_settings"); lua_pushnumber(L, 8203); lua_settable(L, -3);
	lua_pushliteral(L, "update_index"); lua_pushnumber(L, 8204); lua_settable(L, -3);
	lua_pushliteral(L, "change_pattern_order"); lua_pushnumber(L, 8300); lua_settable(L, -3);
	lua_pushliteral(L, "change_pattern_row"); lua_pushnumber(L, 8301); lua_settable(L, -3);
	lua_pushliteral(L, "show_parameter_view"); lua_pushnumber(L, 9000); lua_settable(L, -3);
	lua_pushliteral(L, "show_pattern_view"); lua_pushnumber(L, 9001); lua_settable(L, -3);
	lua_pushliteral(L, "show_pattern_format_view"); lua_pushnumber(L, 9002); lua_settable(L, -3);
	lua_pushliteral(L, "show_machine_view"); lua_pushnumber(L, 9003); lua_settable(L, -3);
	lua_pushliteral(L, "show_wavetable_view"); lua_pushnumber(L, 9004); lua_settable(L, -3);
	lua_pushliteral(L, "show_analyzer"); lua_pushnumber(L, 9006); lua_settable(L, -3);
	lua_pushliteral(L, "show_comment_view"); lua_pushnumber(L, 9007); lua_settable(L, -3);
	lua_pushliteral(L, "show_cpu_view"); lua_pushnumber(L, 9008); lua_settable(L, -3);
	lua_pushliteral(L, "show_filebrowser"); lua_pushnumber(L, 9009); lua_settable(L, -3);
	lua_pushliteral(L, "show_help_view"); lua_pushnumber(L, 9010); lua_settable(L, -3);
	lua_pushliteral(L, "show_history"); lua_pushnumber(L, 9011); lua_settable(L, -3);
	lua_pushliteral(L, "show_preferences"); lua_pushnumber(L, 9012); lua_settable(L, -3);
	lua_pushliteral(L, "show_properties"); lua_pushnumber(L, 9005); lua_settable(L, -3);
	lua_pushliteral(L, "show_all_machines"); lua_pushnumber(L, 9013); lua_settable(L, -3);
	lua_setglobal(L, "buze_event_type");

	lua_newtable(L);
	lua_pushliteral(L, "plugin"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "connection"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "pattern"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "pattern_format"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_pushliteral(L, "wave"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_pushliteral(L, "wave_level"); lua_pushnumber(L, 5); lua_settable(L, -3);
	lua_pushliteral(L, "plugin_group"); lua_pushnumber(L, 6); lua_settable(L, -3);
	lua_setglobal(L, "buze_property_type");

	lua_newtable(L);
	lua_pushliteral(L, "AppPath"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "UserPath"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_setglobal(L, "buze_path_type");

	luaL_newlib(L, change_pattern_order_lib);
	lua_setglobal(L, "buze_event_data_change_pattern_order");

	luaL_newlib(L, change_pattern_row_lib);
	lua_setglobal(L, "buze_event_data_change_pattern_row");

	luaL_newlib(L, show_machine_parameter_lib);
	lua_setglobal(L, "buze_event_data_show_machine_parameter");

	luaL_newlib(L, show_pattern_lib);
	lua_setglobal(L, "buze_event_data_show_pattern");

	luaL_newlib(L, show_pattern_format_lib);
	lua_setglobal(L, "buze_event_data_show_pattern_format");

	luaL_newlib(L, show_properties_lib);
	lua_setglobal(L, "buze_event_data_show_properties");

	luaL_newlib(L, event_data_lib);
	lua_setglobal(L, "buze_event_data");

	luaL_newlib(L, application_lib);
	lua_setglobal(L, "buze_application");

	luaL_newlib(L, main_frame_lib);
	lua_setglobal(L, "buze_main_frame");

	luaL_newlib(L, window_lib);
	lua_setglobal(L, "buze_window");

	luaL_newlib(L, document_lib);
	lua_setglobal(L, "buze_document");

	luaL_newlib(L, plugin_index_item_lib);
	lua_setglobal(L, "buze_plugin_index_item");

	luaL_newlib(L, configuration_lib);
	lua_setglobal(L, "buze_configuration");

	return 1;
}
void luaclose_buze(lua_State* L) {
	buze_callback_clear(L);
}
