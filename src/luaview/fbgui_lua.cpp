#include <fbgui.h>
#include <cassert>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

extern void report_errors(lua_State *L, int status);

struct fbgui_callback_data {
	lua_State *L;
	int functionref;
	const void* ptr;
};

fbgui_callback_data fbgui_callback_datas[500];

template <int N>
int fbgui_callback_template(fbgui_event_data_t* data) {
	lua_State* L = fbgui_callback_datas[N].L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, fbgui_callback_datas[N].functionref);
	if (data != NULL)
		lua_pushlightuserdata(L, data);
	else
		lua_pushnil(L);
	int status = lua_pcall(L, 1, 1, 0);
	report_errors(L, status);
	return lua_toboolean(L, -1);
}

fbgui_callback_t fbgui_callback_callbacks[] = {
	&fbgui_callback_template<0>,
	&fbgui_callback_template<1>,
	&fbgui_callback_template<2>,
	&fbgui_callback_template<3>,
	&fbgui_callback_template<4>,
	&fbgui_callback_template<5>,
	&fbgui_callback_template<6>,
	&fbgui_callback_template<7>,
	&fbgui_callback_template<8>,
	&fbgui_callback_template<9>,
	&fbgui_callback_template<10>,
	&fbgui_callback_template<11>,
	&fbgui_callback_template<12>,
	&fbgui_callback_template<13>,
	&fbgui_callback_template<14>,
	&fbgui_callback_template<15>,
	&fbgui_callback_template<16>,
	&fbgui_callback_template<17>,
	&fbgui_callback_template<18>,
	&fbgui_callback_template<19>,
	&fbgui_callback_template<20>,
	&fbgui_callback_template<21>,
	&fbgui_callback_template<22>,
	&fbgui_callback_template<23>,
	&fbgui_callback_template<24>,
	&fbgui_callback_template<25>,
	&fbgui_callback_template<26>,
	&fbgui_callback_template<27>,
	&fbgui_callback_template<28>,
	&fbgui_callback_template<29>,
	&fbgui_callback_template<30>,
	&fbgui_callback_template<31>,
	&fbgui_callback_template<32>,
	&fbgui_callback_template<33>,
	&fbgui_callback_template<34>,
	&fbgui_callback_template<35>,
	&fbgui_callback_template<36>,
	&fbgui_callback_template<37>,
	&fbgui_callback_template<38>,
	&fbgui_callback_template<39>,
	&fbgui_callback_template<40>,
	&fbgui_callback_template<41>,
	&fbgui_callback_template<42>,
	&fbgui_callback_template<43>,
	&fbgui_callback_template<44>,
	&fbgui_callback_template<45>,
	&fbgui_callback_template<46>,
	&fbgui_callback_template<47>,
	&fbgui_callback_template<48>,
	&fbgui_callback_template<49>,
	&fbgui_callback_template<50>,
	&fbgui_callback_template<51>,
	&fbgui_callback_template<52>,
	&fbgui_callback_template<53>,
	&fbgui_callback_template<54>,
	&fbgui_callback_template<55>,
	&fbgui_callback_template<56>,
	&fbgui_callback_template<57>,
	&fbgui_callback_template<58>,
	&fbgui_callback_template<59>,
	&fbgui_callback_template<60>,
	&fbgui_callback_template<61>,
	&fbgui_callback_template<62>,
	&fbgui_callback_template<63>,
	&fbgui_callback_template<64>,
	&fbgui_callback_template<65>,
	&fbgui_callback_template<66>,
	&fbgui_callback_template<67>,
	&fbgui_callback_template<68>,
	&fbgui_callback_template<69>,
	&fbgui_callback_template<70>,
	&fbgui_callback_template<71>,
	&fbgui_callback_template<72>,
	&fbgui_callback_template<73>,
	&fbgui_callback_template<74>,
	&fbgui_callback_template<75>,
	&fbgui_callback_template<76>,
	&fbgui_callback_template<77>,
	&fbgui_callback_template<78>,
	&fbgui_callback_template<79>,
	&fbgui_callback_template<80>,
	&fbgui_callback_template<81>,
	&fbgui_callback_template<82>,
	&fbgui_callback_template<83>,
	&fbgui_callback_template<84>,
	&fbgui_callback_template<85>,
	&fbgui_callback_template<86>,
	&fbgui_callback_template<87>,
	&fbgui_callback_template<88>,
	&fbgui_callback_template<89>,
	&fbgui_callback_template<90>,
	&fbgui_callback_template<91>,
	&fbgui_callback_template<92>,
	&fbgui_callback_template<93>,
	&fbgui_callback_template<94>,
	&fbgui_callback_template<95>,
	&fbgui_callback_template<96>,
	&fbgui_callback_template<97>,
	&fbgui_callback_template<98>,
	&fbgui_callback_template<99>,
	&fbgui_callback_template<100>,
	&fbgui_callback_template<101>,
	&fbgui_callback_template<102>,
	&fbgui_callback_template<103>,
	&fbgui_callback_template<104>,
	&fbgui_callback_template<105>,
	&fbgui_callback_template<106>,
	&fbgui_callback_template<107>,
	&fbgui_callback_template<108>,
	&fbgui_callback_template<109>,
	&fbgui_callback_template<110>,
	&fbgui_callback_template<111>,
	&fbgui_callback_template<112>,
	&fbgui_callback_template<113>,
	&fbgui_callback_template<114>,
	&fbgui_callback_template<115>,
	&fbgui_callback_template<116>,
	&fbgui_callback_template<117>,
	&fbgui_callback_template<118>,
	&fbgui_callback_template<119>,
	&fbgui_callback_template<120>,
	&fbgui_callback_template<121>,
	&fbgui_callback_template<122>,
	&fbgui_callback_template<123>,
	&fbgui_callback_template<124>,
	&fbgui_callback_template<125>,
	&fbgui_callback_template<126>,
	&fbgui_callback_template<127>,
	&fbgui_callback_template<128>,
	&fbgui_callback_template<129>,
	&fbgui_callback_template<130>,
	&fbgui_callback_template<131>,
	&fbgui_callback_template<132>,
	&fbgui_callback_template<133>,
	&fbgui_callback_template<134>,
	&fbgui_callback_template<135>,
	&fbgui_callback_template<136>,
	&fbgui_callback_template<137>,
	&fbgui_callback_template<138>,
	&fbgui_callback_template<139>,
	&fbgui_callback_template<140>,
	&fbgui_callback_template<141>,
	&fbgui_callback_template<142>,
	&fbgui_callback_template<143>,
	&fbgui_callback_template<144>,
	&fbgui_callback_template<145>,
	&fbgui_callback_template<146>,
	&fbgui_callback_template<147>,
	&fbgui_callback_template<148>,
	&fbgui_callback_template<149>,
	&fbgui_callback_template<150>,
	&fbgui_callback_template<151>,
	&fbgui_callback_template<152>,
	&fbgui_callback_template<153>,
	&fbgui_callback_template<154>,
	&fbgui_callback_template<155>,
	&fbgui_callback_template<156>,
	&fbgui_callback_template<157>,
	&fbgui_callback_template<158>,
	&fbgui_callback_template<159>,
	&fbgui_callback_template<160>,
	&fbgui_callback_template<161>,
	&fbgui_callback_template<162>,
	&fbgui_callback_template<163>,
	&fbgui_callback_template<164>,
	&fbgui_callback_template<165>,
	&fbgui_callback_template<166>,
	&fbgui_callback_template<167>,
	&fbgui_callback_template<168>,
	&fbgui_callback_template<169>,
	&fbgui_callback_template<170>,
	&fbgui_callback_template<171>,
	&fbgui_callback_template<172>,
	&fbgui_callback_template<173>,
	&fbgui_callback_template<174>,
	&fbgui_callback_template<175>,
	&fbgui_callback_template<176>,
	&fbgui_callback_template<177>,
	&fbgui_callback_template<178>,
	&fbgui_callback_template<179>,
	&fbgui_callback_template<180>,
	&fbgui_callback_template<181>,
	&fbgui_callback_template<182>,
	&fbgui_callback_template<183>,
	&fbgui_callback_template<184>,
	&fbgui_callback_template<185>,
	&fbgui_callback_template<186>,
	&fbgui_callback_template<187>,
	&fbgui_callback_template<188>,
	&fbgui_callback_template<189>,
	&fbgui_callback_template<190>,
	&fbgui_callback_template<191>,
	&fbgui_callback_template<192>,
	&fbgui_callback_template<193>,
	&fbgui_callback_template<194>,
	&fbgui_callback_template<195>,
	&fbgui_callback_template<196>,
	&fbgui_callback_template<197>,
	&fbgui_callback_template<198>,
	&fbgui_callback_template<199>,
	&fbgui_callback_template<200>,
	&fbgui_callback_template<201>,
	&fbgui_callback_template<202>,
	&fbgui_callback_template<203>,
	&fbgui_callback_template<204>,
	&fbgui_callback_template<205>,
	&fbgui_callback_template<206>,
	&fbgui_callback_template<207>,
	&fbgui_callback_template<208>,
	&fbgui_callback_template<209>,
	&fbgui_callback_template<210>,
	&fbgui_callback_template<211>,
	&fbgui_callback_template<212>,
	&fbgui_callback_template<213>,
	&fbgui_callback_template<214>,
	&fbgui_callback_template<215>,
	&fbgui_callback_template<216>,
	&fbgui_callback_template<217>,
	&fbgui_callback_template<218>,
	&fbgui_callback_template<219>,
	&fbgui_callback_template<220>,
	&fbgui_callback_template<221>,
	&fbgui_callback_template<222>,
	&fbgui_callback_template<223>,
	&fbgui_callback_template<224>,
	&fbgui_callback_template<225>,
	&fbgui_callback_template<226>,
	&fbgui_callback_template<227>,
	&fbgui_callback_template<228>,
	&fbgui_callback_template<229>,
	&fbgui_callback_template<230>,
	&fbgui_callback_template<231>,
	&fbgui_callback_template<232>,
	&fbgui_callback_template<233>,
	&fbgui_callback_template<234>,
	&fbgui_callback_template<235>,
	&fbgui_callback_template<236>,
	&fbgui_callback_template<237>,
	&fbgui_callback_template<238>,
	&fbgui_callback_template<239>,
	&fbgui_callback_template<240>,
	&fbgui_callback_template<241>,
	&fbgui_callback_template<242>,
	&fbgui_callback_template<243>,
	&fbgui_callback_template<244>,
	&fbgui_callback_template<245>,
	&fbgui_callback_template<246>,
	&fbgui_callback_template<247>,
	&fbgui_callback_template<248>,
	&fbgui_callback_template<249>,
	&fbgui_callback_template<250>,
	&fbgui_callback_template<251>,
	&fbgui_callback_template<252>,
	&fbgui_callback_template<253>,
	&fbgui_callback_template<254>,
	&fbgui_callback_template<255>,
	&fbgui_callback_template<256>,
	&fbgui_callback_template<257>,
	&fbgui_callback_template<258>,
	&fbgui_callback_template<259>,
	&fbgui_callback_template<260>,
	&fbgui_callback_template<261>,
	&fbgui_callback_template<262>,
	&fbgui_callback_template<263>,
	&fbgui_callback_template<264>,
	&fbgui_callback_template<265>,
	&fbgui_callback_template<266>,
	&fbgui_callback_template<267>,
	&fbgui_callback_template<268>,
	&fbgui_callback_template<269>,
	&fbgui_callback_template<270>,
	&fbgui_callback_template<271>,
	&fbgui_callback_template<272>,
	&fbgui_callback_template<273>,
	&fbgui_callback_template<274>,
	&fbgui_callback_template<275>,
	&fbgui_callback_template<276>,
	&fbgui_callback_template<277>,
	&fbgui_callback_template<278>,
	&fbgui_callback_template<279>,
	&fbgui_callback_template<280>,
	&fbgui_callback_template<281>,
	&fbgui_callback_template<282>,
	&fbgui_callback_template<283>,
	&fbgui_callback_template<284>,
	&fbgui_callback_template<285>,
	&fbgui_callback_template<286>,
	&fbgui_callback_template<287>,
	&fbgui_callback_template<288>,
	&fbgui_callback_template<289>,
	&fbgui_callback_template<290>,
	&fbgui_callback_template<291>,
	&fbgui_callback_template<292>,
	&fbgui_callback_template<293>,
	&fbgui_callback_template<294>,
	&fbgui_callback_template<295>,
	&fbgui_callback_template<296>,
	&fbgui_callback_template<297>,
	&fbgui_callback_template<298>,
	&fbgui_callback_template<299>,
	&fbgui_callback_template<300>,
	&fbgui_callback_template<301>,
	&fbgui_callback_template<302>,
	&fbgui_callback_template<303>,
	&fbgui_callback_template<304>,
	&fbgui_callback_template<305>,
	&fbgui_callback_template<306>,
	&fbgui_callback_template<307>,
	&fbgui_callback_template<308>,
	&fbgui_callback_template<309>,
	&fbgui_callback_template<310>,
	&fbgui_callback_template<311>,
	&fbgui_callback_template<312>,
	&fbgui_callback_template<313>,
	&fbgui_callback_template<314>,
	&fbgui_callback_template<315>,
	&fbgui_callback_template<316>,
	&fbgui_callback_template<317>,
	&fbgui_callback_template<318>,
	&fbgui_callback_template<319>,
	&fbgui_callback_template<320>,
	&fbgui_callback_template<321>,
	&fbgui_callback_template<322>,
	&fbgui_callback_template<323>,
	&fbgui_callback_template<324>,
	&fbgui_callback_template<325>,
	&fbgui_callback_template<326>,
	&fbgui_callback_template<327>,
	&fbgui_callback_template<328>,
	&fbgui_callback_template<329>,
	&fbgui_callback_template<330>,
	&fbgui_callback_template<331>,
	&fbgui_callback_template<332>,
	&fbgui_callback_template<333>,
	&fbgui_callback_template<334>,
	&fbgui_callback_template<335>,
	&fbgui_callback_template<336>,
	&fbgui_callback_template<337>,
	&fbgui_callback_template<338>,
	&fbgui_callback_template<339>,
	&fbgui_callback_template<340>,
	&fbgui_callback_template<341>,
	&fbgui_callback_template<342>,
	&fbgui_callback_template<343>,
	&fbgui_callback_template<344>,
	&fbgui_callback_template<345>,
	&fbgui_callback_template<346>,
	&fbgui_callback_template<347>,
	&fbgui_callback_template<348>,
	&fbgui_callback_template<349>,
	&fbgui_callback_template<350>,
	&fbgui_callback_template<351>,
	&fbgui_callback_template<352>,
	&fbgui_callback_template<353>,
	&fbgui_callback_template<354>,
	&fbgui_callback_template<355>,
	&fbgui_callback_template<356>,
	&fbgui_callback_template<357>,
	&fbgui_callback_template<358>,
	&fbgui_callback_template<359>,
	&fbgui_callback_template<360>,
	&fbgui_callback_template<361>,
	&fbgui_callback_template<362>,
	&fbgui_callback_template<363>,
	&fbgui_callback_template<364>,
	&fbgui_callback_template<365>,
	&fbgui_callback_template<366>,
	&fbgui_callback_template<367>,
	&fbgui_callback_template<368>,
	&fbgui_callback_template<369>,
	&fbgui_callback_template<370>,
	&fbgui_callback_template<371>,
	&fbgui_callback_template<372>,
	&fbgui_callback_template<373>,
	&fbgui_callback_template<374>,
	&fbgui_callback_template<375>,
	&fbgui_callback_template<376>,
	&fbgui_callback_template<377>,
	&fbgui_callback_template<378>,
	&fbgui_callback_template<379>,
	&fbgui_callback_template<380>,
	&fbgui_callback_template<381>,
	&fbgui_callback_template<382>,
	&fbgui_callback_template<383>,
	&fbgui_callback_template<384>,
	&fbgui_callback_template<385>,
	&fbgui_callback_template<386>,
	&fbgui_callback_template<387>,
	&fbgui_callback_template<388>,
	&fbgui_callback_template<389>,
	&fbgui_callback_template<390>,
	&fbgui_callback_template<391>,
	&fbgui_callback_template<392>,
	&fbgui_callback_template<393>,
	&fbgui_callback_template<394>,
	&fbgui_callback_template<395>,
	&fbgui_callback_template<396>,
	&fbgui_callback_template<397>,
	&fbgui_callback_template<398>,
	&fbgui_callback_template<399>,
	&fbgui_callback_template<400>,
	&fbgui_callback_template<401>,
	&fbgui_callback_template<402>,
	&fbgui_callback_template<403>,
	&fbgui_callback_template<404>,
	&fbgui_callback_template<405>,
	&fbgui_callback_template<406>,
	&fbgui_callback_template<407>,
	&fbgui_callback_template<408>,
	&fbgui_callback_template<409>,
	&fbgui_callback_template<410>,
	&fbgui_callback_template<411>,
	&fbgui_callback_template<412>,
	&fbgui_callback_template<413>,
	&fbgui_callback_template<414>,
	&fbgui_callback_template<415>,
	&fbgui_callback_template<416>,
	&fbgui_callback_template<417>,
	&fbgui_callback_template<418>,
	&fbgui_callback_template<419>,
	&fbgui_callback_template<420>,
	&fbgui_callback_template<421>,
	&fbgui_callback_template<422>,
	&fbgui_callback_template<423>,
	&fbgui_callback_template<424>,
	&fbgui_callback_template<425>,
	&fbgui_callback_template<426>,
	&fbgui_callback_template<427>,
	&fbgui_callback_template<428>,
	&fbgui_callback_template<429>,
	&fbgui_callback_template<430>,
	&fbgui_callback_template<431>,
	&fbgui_callback_template<432>,
	&fbgui_callback_template<433>,
	&fbgui_callback_template<434>,
	&fbgui_callback_template<435>,
	&fbgui_callback_template<436>,
	&fbgui_callback_template<437>,
	&fbgui_callback_template<438>,
	&fbgui_callback_template<439>,
	&fbgui_callback_template<440>,
	&fbgui_callback_template<441>,
	&fbgui_callback_template<442>,
	&fbgui_callback_template<443>,
	&fbgui_callback_template<444>,
	&fbgui_callback_template<445>,
	&fbgui_callback_template<446>,
	&fbgui_callback_template<447>,
	&fbgui_callback_template<448>,
	&fbgui_callback_template<449>,
	&fbgui_callback_template<450>,
	&fbgui_callback_template<451>,
	&fbgui_callback_template<452>,
	&fbgui_callback_template<453>,
	&fbgui_callback_template<454>,
	&fbgui_callback_template<455>,
	&fbgui_callback_template<456>,
	&fbgui_callback_template<457>,
	&fbgui_callback_template<458>,
	&fbgui_callback_template<459>,
	&fbgui_callback_template<460>,
	&fbgui_callback_template<461>,
	&fbgui_callback_template<462>,
	&fbgui_callback_template<463>,
	&fbgui_callback_template<464>,
	&fbgui_callback_template<465>,
	&fbgui_callback_template<466>,
	&fbgui_callback_template<467>,
	&fbgui_callback_template<468>,
	&fbgui_callback_template<469>,
	&fbgui_callback_template<470>,
	&fbgui_callback_template<471>,
	&fbgui_callback_template<472>,
	&fbgui_callback_template<473>,
	&fbgui_callback_template<474>,
	&fbgui_callback_template<475>,
	&fbgui_callback_template<476>,
	&fbgui_callback_template<477>,
	&fbgui_callback_template<478>,
	&fbgui_callback_template<479>,
	&fbgui_callback_template<480>,
	&fbgui_callback_template<481>,
	&fbgui_callback_template<482>,
	&fbgui_callback_template<483>,
	&fbgui_callback_template<484>,
	&fbgui_callback_template<485>,
	&fbgui_callback_template<486>,
	&fbgui_callback_template<487>,
	&fbgui_callback_template<488>,
	&fbgui_callback_template<489>,
	&fbgui_callback_template<490>,
	&fbgui_callback_template<491>,
	&fbgui_callback_template<492>,
	&fbgui_callback_template<493>,
	&fbgui_callback_template<494>,
	&fbgui_callback_template<495>,
	&fbgui_callback_template<496>,
	&fbgui_callback_template<497>,
	&fbgui_callback_template<498>,
	&fbgui_callback_template<499>,
};

fbgui_callback_t fbgui_callback_alloc(lua_State* L, int index) {
	const void* ptr = lua_topointer(L, index);
	assert(ptr != 0);
	for (int i = 0; i < 500; ++i) {
		if (ptr == fbgui_callback_datas[i].ptr) {
			return fbgui_callback_callbacks[i];
		}
	}
	lua_pushvalue(L, index);
	int functionref = luaL_ref(L, LUA_REGISTRYINDEX);
	for (int i = 0; i < 500; ++i) {
		if (fbgui_callback_datas[i].L == 0) {
			fbgui_callback_datas[i].L = L;
			fbgui_callback_datas[i].ptr = ptr;
			fbgui_callback_datas[i].functionref = functionref;
			return fbgui_callback_callbacks[i];
		}
	}
	return 0;
}

void fbgui_callback_clear(lua_State* L) {
	for (int i = 0; i < 500; ++i) {
		if (L == fbgui_callback_datas[i].L) {
			fbgui_callback_datas[i].L = 0;
			fbgui_callback_datas[i].ptr = 0;
			fbgui_callback_datas[i].functionref = 0;
		}
	}
}

static int event_data_get_source(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for EventData:get_source");
		return 0;
	}
	fbgui_event_data_t* self = (fbgui_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:get_source");
		return 0;
	}

	fbgui_node_t* _luaresult = fbgui_event_data_get_source(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int event_data_get_target(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for EventData:get_target");
		return 0;
	}
	fbgui_event_data_t* self = (fbgui_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:get_target");
		return 0;
	}

	fbgui_node_t* _luaresult = fbgui_event_data_get_target(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int event_data_get_x(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for EventData:get_x");
		return 0;
	}
	fbgui_event_data_t* self = (fbgui_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:get_x");
		return 0;
	}

	int _luaresult = fbgui_event_data_get_x(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int event_data_get_y(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for EventData:get_y");
		return 0;
	}
	fbgui_event_data_t* self = (fbgui_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:get_y");
		return 0;
	}

	int _luaresult = fbgui_event_data_get_y(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int event_data_get_unicode(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for EventData:get_unicode");
		return 0;
	}
	fbgui_event_data_t* self = (fbgui_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:get_unicode");
		return 0;
	}

	int _luaresult = fbgui_event_data_get_unicode(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int event_data_get_scancode(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for EventData:get_scancode");
		return 0;
	}
	fbgui_event_data_t* self = (fbgui_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:get_scancode");
		return 0;
	}

	int _luaresult = fbgui_event_data_get_scancode(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg event_data_lib[] = {
	{ "get_source", event_data_get_source },
	{ "get_target", event_data_get_target },
	{ "get_x", event_data_get_x },
	{ "get_y", event_data_get_y },
	{ "get_unicode", event_data_get_unicode },
	{ "get_scancode", event_data_get_scancode },
	{ NULL, NULL }
};

static int context_create(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 1) {
		luaL_error(L, "Invalid argument count for Context:create");
		return 0;
	}
	fbgui_context_t* _luaresult = fbgui_context_create();
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int context_create_child(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Context:create_child");
		return 0;
	}
	void* wnd = lua_touserdata(L, 2);
	fbgui_context_t* _luaresult = fbgui_context_create_child(wnd);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int context_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Context:destroy");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:destroy");
		return 0;
	}

	fbgui_context_destroy(self);
	return 0;
}

static int context_register_font(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Context:register_font");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:register_font");
		return 0;
	}

	const char* fontname = luaL_tolstring(L, 3, &len);
	const char* filename = luaL_tolstring(L, 4, &len);
	int _luaresult = fbgui_context_register_font(self, fontname, filename);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int context_get_graphics(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Context:get_graphics");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:get_graphics");
		return 0;
	}

	fbgui_graphics_t* _luaresult = fbgui_context_get_graphics(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int context_get_root(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Context:get_root");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:get_root");
		return 0;
	}

	fbgui_node_t* _luaresult = fbgui_context_get_root(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

void(*luacallback_fbgui_context_bind)(lua_State* L, fbgui_context_t* context, const char* selector, const char* eventname, fbgui_callback_t handler);

static int context_bind(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Context:bind");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:bind");
		return 0;
	}

	const char* selector = luaL_tolstring(L, 3, &len);
	const char* eventname = luaL_tolstring(L, 4, &len);
	fbgui_callback_t handler = fbgui_callback_alloc(L, 5);
	if (luacallback_fbgui_context_bind) luacallback_fbgui_context_bind(L, self,  selector,  eventname,  handler);
	fbgui_context_bind(self, selector, eventname, handler);
	return 0;
}

static int context_trigger(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Context:trigger");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:trigger");
		return 0;
	}

	fbgui_node_t* target = (fbgui_node_t*)lua_touserdata(L, 3);
	const char* eventname = luaL_tolstring(L, 4, &len);
	fbgui_event_data_t* data = (fbgui_event_data_t*)lua_touserdata(L, 5);
	fbgui_context_trigger(self, target, eventname, data);
	return 0;
}

static int context_load_stylesheet(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:load_stylesheet");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:load_stylesheet");
		return 0;
	}

	const char* filename = luaL_tolstring(L, 3, &len);
	int _luaresult = fbgui_context_load_stylesheet(self, filename);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int context_load_markup(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:load_markup");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:load_markup");
		return 0;
	}

	const char* filename = luaL_tolstring(L, 3, &len);
	fbgui_node_t* _luaresult = fbgui_context_load_markup(self, filename);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int context_parse_markup(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:parse_markup");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:parse_markup");
		return 0;
	}

	const char* ml = luaL_tolstring(L, 3, &len);
	fbgui_node_t* _luaresult = fbgui_context_parse_markup(self, ml);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int context_create_text(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:create_text");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:create_text");
		return 0;
	}

	const char* text = luaL_tolstring(L, 3, &len);
	fbgui_node_t* _luaresult = fbgui_context_create_text(self, text);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int context_create_element(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:create_element");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:create_element");
		return 0;
	}

	const char* tagname = luaL_tolstring(L, 3, &len);
	fbgui_node_t* _luaresult = fbgui_context_create_element(self, tagname);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int context_run(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:run");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:run");
		return 0;
	}

	fbgui_node_t* view = (fbgui_node_t*)lua_touserdata(L, 3);
	fbgui_context_run(self, view);
	return 0;
}

static int context_set_idle_modulus(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:set_idle_modulus");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:set_idle_modulus");
		return 0;
	}

	int n = lua_tonumber(L, 3);
	fbgui_context_set_idle_modulus(self, n);
	return 0;
}

static int context_poll(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Context:poll");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:poll");
		return 0;
	}

	fbgui_context_poll(self);
	return 0;
}

static int context_quit(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Context:quit");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:quit");
		return 0;
	}

	fbgui_context_quit(self);
	return 0;
}

static int context_capture(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:capture");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:capture");
		return 0;
	}

	fbgui_node_t* n = (fbgui_node_t*)lua_touserdata(L, 3);
	fbgui_context_capture(self, n);
	return 0;
}

static int context_release_capture(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Context:release_capture");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:release_capture");
		return 0;
	}

	fbgui_context_release_capture(self);
	return 0;
}

static int context_set_modal(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:set_modal");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:set_modal");
		return 0;
	}

	fbgui_node_t* n = (fbgui_node_t*)lua_touserdata(L, 3);
	fbgui_context_set_modal(self, n);
	return 0;
}

static int context_invalidate_rectangle(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Context:invalidate_rectangle");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:invalidate_rectangle");
		return 0;
	}

	fbgui_node_t* n = (fbgui_node_t*)lua_touserdata(L, 3);
	int x = lua_tonumber(L, 4);
	int y = lua_tonumber(L, 5);
	int width = lua_tonumber(L, 6);
	int height = lua_tonumber(L, 7);
	fbgui_context_invalidate_rectangle(self, n, x, y, width, height);
	return 0;
}

static int context_invalidate(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Context:invalidate");
		return 0;
	}
	fbgui_context_t* self = (fbgui_context_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Context:invalidate");
		return 0;
	}

	fbgui_node_t* n = (fbgui_node_t*)lua_touserdata(L, 3);
	fbgui_context_invalidate(self, n);
	return 0;
}

static const luaL_Reg context_lib[] = {
	{ "create", context_create },
	{ "create_child", context_create_child },
	{ "destroy", context_destroy },
	{ "register_font", context_register_font },
	{ "get_graphics", context_get_graphics },
	{ "get_root", context_get_root },
	{ "bind", context_bind },
	{ "trigger", context_trigger },
	{ "load_stylesheet", context_load_stylesheet },
	{ "load_markup", context_load_markup },
	{ "parse_markup", context_parse_markup },
	{ "create_text", context_create_text },
	{ "create_element", context_create_element },
	{ "run", context_run },
	{ "set_idle_modulus", context_set_idle_modulus },
	{ "poll", context_poll },
	{ "quit", context_quit },
	{ "capture", context_capture },
	{ "release_capture", context_release_capture },
	{ "set_modal", context_set_modal },
	{ "invalidate_rectangle", context_invalidate_rectangle },
	{ "invalidate", context_invalidate },
	{ NULL, NULL }
};

static int graphics_set_color(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Graphics:set_color");
		return 0;
	}
	fbgui_graphics_t* self = (fbgui_graphics_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Graphics:set_color");
		return 0;
	}

	unsigned int color = lua_tonumber(L, 3);
	fbgui_graphics_set_color(self, color);
	return 0;
}

static int graphics_set_fillcolor(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Graphics:set_fillcolor");
		return 0;
	}
	fbgui_graphics_t* self = (fbgui_graphics_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Graphics:set_fillcolor");
		return 0;
	}

	unsigned int color = lua_tonumber(L, 3);
	fbgui_graphics_set_fillcolor(self, color);
	return 0;
}

static int graphics_draw_line(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Graphics:draw_line");
		return 0;
	}
	fbgui_graphics_t* self = (fbgui_graphics_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Graphics:draw_line");
		return 0;
	}

	int x0 = lua_tonumber(L, 3);
	int y0 = lua_tonumber(L, 4);
	int x1 = lua_tonumber(L, 5);
	int y1 = lua_tonumber(L, 6);
	fbgui_graphics_draw_line(self, x0, y0, x1, y1);
	return 0;
}

static int graphics_draw_string(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Graphics:draw_string");
		return 0;
	}
	fbgui_graphics_t* self = (fbgui_graphics_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Graphics:draw_string");
		return 0;
	}

	int x = lua_tonumber(L, 3);
	int y = lua_tonumber(L, 4);
	const char* text = luaL_tolstring(L, 5, &len);
	fbgui_graphics_draw_string(self, x, y, text);
	return 0;
}

static int graphics_fill_rectangle(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Graphics:fill_rectangle");
		return 0;
	}
	fbgui_graphics_t* self = (fbgui_graphics_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Graphics:fill_rectangle");
		return 0;
	}

	int x = lua_tonumber(L, 3);
	int y = lua_tonumber(L, 4);
	int width = lua_tonumber(L, 5);
	int height = lua_tonumber(L, 6);
	fbgui_graphics_fill_rectangle(self, x, y, width, height);
	return 0;
}

static int graphics_get_clip_left(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Graphics:get_clip_left");
		return 0;
	}
	fbgui_graphics_t* self = (fbgui_graphics_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Graphics:get_clip_left");
		return 0;
	}

	int _luaresult = fbgui_graphics_get_clip_left(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int graphics_get_clip_top(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Graphics:get_clip_top");
		return 0;
	}
	fbgui_graphics_t* self = (fbgui_graphics_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Graphics:get_clip_top");
		return 0;
	}

	int _luaresult = fbgui_graphics_get_clip_top(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int graphics_get_clip_width(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Graphics:get_clip_width");
		return 0;
	}
	fbgui_graphics_t* self = (fbgui_graphics_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Graphics:get_clip_width");
		return 0;
	}

	int _luaresult = fbgui_graphics_get_clip_width(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int graphics_get_clip_height(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Graphics:get_clip_height");
		return 0;
	}
	fbgui_graphics_t* self = (fbgui_graphics_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Graphics:get_clip_height");
		return 0;
	}

	int _luaresult = fbgui_graphics_get_clip_height(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg graphics_lib[] = {
	{ "set_color", graphics_set_color },
	{ "set_fillcolor", graphics_set_fillcolor },
	{ "draw_line", graphics_draw_line },
	{ "draw_string", graphics_draw_string },
	{ "fill_rectangle", graphics_fill_rectangle },
	{ "get_clip_left", graphics_get_clip_left },
	{ "get_clip_top", graphics_get_clip_top },
	{ "get_clip_width", graphics_get_clip_width },
	{ "get_clip_height", graphics_get_clip_height },
	{ NULL, NULL }
};

static int node_get_parent(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_parent");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_parent");
		return 0;
	}

	fbgui_node_t* _luaresult = fbgui_node_get_parent(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int node_get_tagname(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_tagname");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_tagname");
		return 0;
	}

	const char* _luaresult = fbgui_node_get_tagname(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int node_addref(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:addref");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:addref");
		return 0;
	}

	fbgui_node_addref(self);
	return 0;
}

static int node_unref(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:unref");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:unref");
		return 0;
	}

	fbgui_node_unref(self);
	return 0;
}

static int node_get_text(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_text");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_text");
		return 0;
	}

	const char* _luaresult = fbgui_node_get_text(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int node_set_text(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Node:set_text");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:set_text");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* text = luaL_tolstring(L, 4, &len);
	fbgui_node_set_text(self, ctx, text);
	return 0;
}

static int node_set_inner_text(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Node:set_inner_text");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:set_inner_text");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* text = luaL_tolstring(L, 4, &len);
	fbgui_node_set_inner_text(self, ctx, text);
	return 0;
}

static int node_get_attribute(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Node:get_attribute");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_attribute");
		return 0;
	}

	const char* key = luaL_tolstring(L, 3, &len);
	const char* _luaresult = fbgui_node_get_attribute(self, key);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int node_set_attribute(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Node:set_attribute");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:set_attribute");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* key = luaL_tolstring(L, 4, &len);
	const char* value = luaL_tolstring(L, 5, &len);
	fbgui_node_set_attribute(self, ctx, key, value);
	return 0;
}

static int node_append_child(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Node:append_child");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:append_child");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	fbgui_node_t* child = (fbgui_node_t*)lua_touserdata(L, 4);
	fbgui_node_append_child(self, ctx, child);
	return 0;
}

static int node_insert_child(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Node:insert_child");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:insert_child");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	int index = lua_tonumber(L, 4);
	fbgui_node_t* child = (fbgui_node_t*)lua_touserdata(L, 5);
	fbgui_node_insert_child(self, ctx, index, child);
	return 0;
}

static int node_remove_child(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Node:remove_child");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:remove_child");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	fbgui_node_t* child = (fbgui_node_t*)lua_touserdata(L, 4);
	fbgui_node_remove_child(self, ctx, child);
	return 0;
}

static int node_has_class(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Node:has_class");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:has_class");
		return 0;
	}

	const char* classname = luaL_tolstring(L, 3, &len);
	int _luaresult = fbgui_node_has_class(self, classname);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int node_add_class(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Node:add_class");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:add_class");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* classname = luaL_tolstring(L, 4, &len);
	fbgui_node_add_class(self, ctx, classname);
	return 0;
}

static int node_remove_class(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Node:remove_class");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:remove_class");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* classname = luaL_tolstring(L, 4, &len);
	fbgui_node_remove_class(self, ctx, classname);
	return 0;
}

static int node_get_child_nodes(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_child_nodes");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_child_nodes");
		return 0;
	}

	fbgui_node_list_t* _luaresult = fbgui_node_get_child_nodes(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int node_find(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Node:find");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:find");
		return 0;
	}

	const char* selector = luaL_tolstring(L, 3, &len);
	fbgui_node_list_t* _luaresult = fbgui_node_find(self, selector);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int node_match(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Node:match");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:match");
		return 0;
	}

	const char* selector = luaL_tolstring(L, 3, &len);
	int _luaresult = fbgui_node_match(self, selector);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int node_style(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Node:style");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:style");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* declarations = luaL_tolstring(L, 4, &len);
	fbgui_node_style(self, ctx, declarations);
	return 0;
}

static int node_get_screen_left(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_screen_left");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_screen_left");
		return 0;
	}

	int _luaresult = fbgui_node_get_screen_left(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int node_get_screen_top(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_screen_top");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_screen_top");
		return 0;
	}

	int _luaresult = fbgui_node_get_screen_top(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int node_get_offset_left(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_offset_left");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_offset_left");
		return 0;
	}

	int _luaresult = fbgui_node_get_offset_left(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int node_get_offset_top(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_offset_top");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_offset_top");
		return 0;
	}

	int _luaresult = fbgui_node_get_offset_top(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int node_get_width(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_width");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_width");
		return 0;
	}

	int _luaresult = fbgui_node_get_width(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int node_get_height(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_height");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_height");
		return 0;
	}

	int _luaresult = fbgui_node_get_height(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int node_get_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Node:get_index");
		return 0;
	}
	fbgui_node_t* self = (fbgui_node_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Node:get_index");
		return 0;
	}

	int _luaresult = fbgui_node_get_index(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg node_lib[] = {
	{ "get_parent", node_get_parent },
	{ "get_tagname", node_get_tagname },
	{ "addref", node_addref },
	{ "unref", node_unref },
	{ "get_text", node_get_text },
	{ "set_text", node_set_text },
	{ "set_inner_text", node_set_inner_text },
	{ "get_attribute", node_get_attribute },
	{ "set_attribute", node_set_attribute },
	{ "append_child", node_append_child },
	{ "insert_child", node_insert_child },
	{ "remove_child", node_remove_child },
	{ "has_class", node_has_class },
	{ "add_class", node_add_class },
	{ "remove_class", node_remove_class },
	{ "get_child_nodes", node_get_child_nodes },
	{ "find", node_find },
	{ "match", node_match },
	{ "style", node_style },
	{ "get_screen_left", node_get_screen_left },
	{ "get_screen_top", node_get_screen_top },
	{ "get_offset_left", node_get_offset_left },
	{ "get_offset_top", node_get_offset_top },
	{ "get_width", node_get_width },
	{ "get_height", node_get_height },
	{ "get_index", node_get_index },
	{ NULL, NULL }
};

static int node_list_get_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for NodeList:get_count");
		return 0;
	}
	fbgui_node_list_t* self = (fbgui_node_list_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for NodeList:get_count");
		return 0;
	}

	int _luaresult = fbgui_node_list_get_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int node_list_get_item(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for NodeList:get_item");
		return 0;
	}
	fbgui_node_list_t* self = (fbgui_node_list_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for NodeList:get_item");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	fbgui_node_t* _luaresult = fbgui_node_list_get_item(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int node_list_style(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for NodeList:style");
		return 0;
	}
	fbgui_node_list_t* self = (fbgui_node_list_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for NodeList:style");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* declarations = luaL_tolstring(L, 4, &len);
	fbgui_node_list_style(self, ctx, declarations);
	return 0;
}

static int node_list_add_class(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for NodeList:add_class");
		return 0;
	}
	fbgui_node_list_t* self = (fbgui_node_list_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for NodeList:add_class");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* classname = luaL_tolstring(L, 4, &len);
	fbgui_node_list_add_class(self, ctx, classname);
	return 0;
}

static int node_list_remove_class(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for NodeList:remove_class");
		return 0;
	}
	fbgui_node_list_t* self = (fbgui_node_list_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for NodeList:remove_class");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* classname = luaL_tolstring(L, 4, &len);
	fbgui_node_list_remove_class(self, ctx, classname);
	return 0;
}

static int node_list_set_inner_text(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for NodeList:set_inner_text");
		return 0;
	}
	fbgui_node_list_t* self = (fbgui_node_list_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for NodeList:set_inner_text");
		return 0;
	}

	fbgui_context_t* ctx = (fbgui_context_t*)lua_touserdata(L, 3);
	const char* text = luaL_tolstring(L, 4, &len);
	fbgui_node_list_set_inner_text(self, ctx, text);
	return 0;
}

static int node_list_find(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for NodeList:find");
		return 0;
	}
	fbgui_node_list_t* self = (fbgui_node_list_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for NodeList:find");
		return 0;
	}

	const char* selector = luaL_tolstring(L, 3, &len);
	fbgui_node_list_t* _luaresult = fbgui_node_list_find(self, selector);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int node_list_index_of(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for NodeList:index_of");
		return 0;
	}
	fbgui_node_list_t* self = (fbgui_node_list_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for NodeList:index_of");
		return 0;
	}

	fbgui_node_t* n = (fbgui_node_t*)lua_touserdata(L, 3);
	int _luaresult = fbgui_node_list_index_of(self, n);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg node_list_lib[] = {
	{ "get_count", node_list_get_count },
	{ "get_item", node_list_get_item },
	{ "style", node_list_style },
	{ "add_class", node_list_add_class },
	{ "remove_class", node_list_remove_class },
	{ "set_inner_text", node_list_set_inner_text },
	{ "find", node_list_find },
	{ "index_of", node_list_index_of },
	{ NULL, NULL }
};

int luaopen_fbgui(lua_State* L) {
	lua_newtable(L);
	lua_pushliteral(L, "version"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_setglobal(L, "fbgui");

	luaL_newlib(L, event_data_lib);
	lua_setglobal(L, "fbgui_event_data");

	luaL_newlib(L, context_lib);
	lua_setglobal(L, "fbgui_context");

	luaL_newlib(L, graphics_lib);
	lua_setglobal(L, "fbgui_graphics");

	luaL_newlib(L, node_lib);
	lua_setglobal(L, "fbgui_node");

	luaL_newlib(L, node_list_lib);
	lua_setglobal(L, "fbgui_node_list");

	return 1;
}
void luaclose_fbgui(lua_State* L) {
	fbgui_callback_clear(L);
}
