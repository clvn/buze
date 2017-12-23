#include <zzub/zzub.h>
#include <cassert>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

extern void report_errors(lua_State *L, int status);

struct zzub_callback_data {
	lua_State *L;
	int functionref;
	const void* ptr;
};

zzub_callback_data zzub_callback_datas[500];

template <int N>
int zzub_callback_template(zzub_player_t* player, zzub_plugin_t* plugin, zzub_event_data_t* data, void* tag) {
	lua_State* L = zzub_callback_datas[N].L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, zzub_callback_datas[N].functionref);
	if (player != NULL)
		lua_pushlightuserdata(L, player);
	else
		lua_pushnil(L);
	if (plugin != NULL)
		lua_pushlightuserdata(L, plugin);
	else
		lua_pushnil(L);
	if (data != NULL)
		lua_pushlightuserdata(L, data);
	else
		lua_pushnil(L);
	lua_pushlightuserdata(L, tag);
	int status = lua_pcall(L, 4, 1, 0);
	report_errors(L, status);
	return lua_tonumber(L, -1);
}

zzub_callback_t zzub_callback_callbacks[] = {
	&zzub_callback_template<0>,
	&zzub_callback_template<1>,
	&zzub_callback_template<2>,
	&zzub_callback_template<3>,
	&zzub_callback_template<4>,
	&zzub_callback_template<5>,
	&zzub_callback_template<6>,
	&zzub_callback_template<7>,
	&zzub_callback_template<8>,
	&zzub_callback_template<9>,
	&zzub_callback_template<10>,
	&zzub_callback_template<11>,
	&zzub_callback_template<12>,
	&zzub_callback_template<13>,
	&zzub_callback_template<14>,
	&zzub_callback_template<15>,
	&zzub_callback_template<16>,
	&zzub_callback_template<17>,
	&zzub_callback_template<18>,
	&zzub_callback_template<19>,
	&zzub_callback_template<20>,
	&zzub_callback_template<21>,
	&zzub_callback_template<22>,
	&zzub_callback_template<23>,
	&zzub_callback_template<24>,
	&zzub_callback_template<25>,
	&zzub_callback_template<26>,
	&zzub_callback_template<27>,
	&zzub_callback_template<28>,
	&zzub_callback_template<29>,
	&zzub_callback_template<30>,
	&zzub_callback_template<31>,
	&zzub_callback_template<32>,
	&zzub_callback_template<33>,
	&zzub_callback_template<34>,
	&zzub_callback_template<35>,
	&zzub_callback_template<36>,
	&zzub_callback_template<37>,
	&zzub_callback_template<38>,
	&zzub_callback_template<39>,
	&zzub_callback_template<40>,
	&zzub_callback_template<41>,
	&zzub_callback_template<42>,
	&zzub_callback_template<43>,
	&zzub_callback_template<44>,
	&zzub_callback_template<45>,
	&zzub_callback_template<46>,
	&zzub_callback_template<47>,
	&zzub_callback_template<48>,
	&zzub_callback_template<49>,
	&zzub_callback_template<50>,
	&zzub_callback_template<51>,
	&zzub_callback_template<52>,
	&zzub_callback_template<53>,
	&zzub_callback_template<54>,
	&zzub_callback_template<55>,
	&zzub_callback_template<56>,
	&zzub_callback_template<57>,
	&zzub_callback_template<58>,
	&zzub_callback_template<59>,
	&zzub_callback_template<60>,
	&zzub_callback_template<61>,
	&zzub_callback_template<62>,
	&zzub_callback_template<63>,
	&zzub_callback_template<64>,
	&zzub_callback_template<65>,
	&zzub_callback_template<66>,
	&zzub_callback_template<67>,
	&zzub_callback_template<68>,
	&zzub_callback_template<69>,
	&zzub_callback_template<70>,
	&zzub_callback_template<71>,
	&zzub_callback_template<72>,
	&zzub_callback_template<73>,
	&zzub_callback_template<74>,
	&zzub_callback_template<75>,
	&zzub_callback_template<76>,
	&zzub_callback_template<77>,
	&zzub_callback_template<78>,
	&zzub_callback_template<79>,
	&zzub_callback_template<80>,
	&zzub_callback_template<81>,
	&zzub_callback_template<82>,
	&zzub_callback_template<83>,
	&zzub_callback_template<84>,
	&zzub_callback_template<85>,
	&zzub_callback_template<86>,
	&zzub_callback_template<87>,
	&zzub_callback_template<88>,
	&zzub_callback_template<89>,
	&zzub_callback_template<90>,
	&zzub_callback_template<91>,
	&zzub_callback_template<92>,
	&zzub_callback_template<93>,
	&zzub_callback_template<94>,
	&zzub_callback_template<95>,
	&zzub_callback_template<96>,
	&zzub_callback_template<97>,
	&zzub_callback_template<98>,
	&zzub_callback_template<99>,
	&zzub_callback_template<100>,
	&zzub_callback_template<101>,
	&zzub_callback_template<102>,
	&zzub_callback_template<103>,
	&zzub_callback_template<104>,
	&zzub_callback_template<105>,
	&zzub_callback_template<106>,
	&zzub_callback_template<107>,
	&zzub_callback_template<108>,
	&zzub_callback_template<109>,
	&zzub_callback_template<110>,
	&zzub_callback_template<111>,
	&zzub_callback_template<112>,
	&zzub_callback_template<113>,
	&zzub_callback_template<114>,
	&zzub_callback_template<115>,
	&zzub_callback_template<116>,
	&zzub_callback_template<117>,
	&zzub_callback_template<118>,
	&zzub_callback_template<119>,
	&zzub_callback_template<120>,
	&zzub_callback_template<121>,
	&zzub_callback_template<122>,
	&zzub_callback_template<123>,
	&zzub_callback_template<124>,
	&zzub_callback_template<125>,
	&zzub_callback_template<126>,
	&zzub_callback_template<127>,
	&zzub_callback_template<128>,
	&zzub_callback_template<129>,
	&zzub_callback_template<130>,
	&zzub_callback_template<131>,
	&zzub_callback_template<132>,
	&zzub_callback_template<133>,
	&zzub_callback_template<134>,
	&zzub_callback_template<135>,
	&zzub_callback_template<136>,
	&zzub_callback_template<137>,
	&zzub_callback_template<138>,
	&zzub_callback_template<139>,
	&zzub_callback_template<140>,
	&zzub_callback_template<141>,
	&zzub_callback_template<142>,
	&zzub_callback_template<143>,
	&zzub_callback_template<144>,
	&zzub_callback_template<145>,
	&zzub_callback_template<146>,
	&zzub_callback_template<147>,
	&zzub_callback_template<148>,
	&zzub_callback_template<149>,
	&zzub_callback_template<150>,
	&zzub_callback_template<151>,
	&zzub_callback_template<152>,
	&zzub_callback_template<153>,
	&zzub_callback_template<154>,
	&zzub_callback_template<155>,
	&zzub_callback_template<156>,
	&zzub_callback_template<157>,
	&zzub_callback_template<158>,
	&zzub_callback_template<159>,
	&zzub_callback_template<160>,
	&zzub_callback_template<161>,
	&zzub_callback_template<162>,
	&zzub_callback_template<163>,
	&zzub_callback_template<164>,
	&zzub_callback_template<165>,
	&zzub_callback_template<166>,
	&zzub_callback_template<167>,
	&zzub_callback_template<168>,
	&zzub_callback_template<169>,
	&zzub_callback_template<170>,
	&zzub_callback_template<171>,
	&zzub_callback_template<172>,
	&zzub_callback_template<173>,
	&zzub_callback_template<174>,
	&zzub_callback_template<175>,
	&zzub_callback_template<176>,
	&zzub_callback_template<177>,
	&zzub_callback_template<178>,
	&zzub_callback_template<179>,
	&zzub_callback_template<180>,
	&zzub_callback_template<181>,
	&zzub_callback_template<182>,
	&zzub_callback_template<183>,
	&zzub_callback_template<184>,
	&zzub_callback_template<185>,
	&zzub_callback_template<186>,
	&zzub_callback_template<187>,
	&zzub_callback_template<188>,
	&zzub_callback_template<189>,
	&zzub_callback_template<190>,
	&zzub_callback_template<191>,
	&zzub_callback_template<192>,
	&zzub_callback_template<193>,
	&zzub_callback_template<194>,
	&zzub_callback_template<195>,
	&zzub_callback_template<196>,
	&zzub_callback_template<197>,
	&zzub_callback_template<198>,
	&zzub_callback_template<199>,
	&zzub_callback_template<200>,
	&zzub_callback_template<201>,
	&zzub_callback_template<202>,
	&zzub_callback_template<203>,
	&zzub_callback_template<204>,
	&zzub_callback_template<205>,
	&zzub_callback_template<206>,
	&zzub_callback_template<207>,
	&zzub_callback_template<208>,
	&zzub_callback_template<209>,
	&zzub_callback_template<210>,
	&zzub_callback_template<211>,
	&zzub_callback_template<212>,
	&zzub_callback_template<213>,
	&zzub_callback_template<214>,
	&zzub_callback_template<215>,
	&zzub_callback_template<216>,
	&zzub_callback_template<217>,
	&zzub_callback_template<218>,
	&zzub_callback_template<219>,
	&zzub_callback_template<220>,
	&zzub_callback_template<221>,
	&zzub_callback_template<222>,
	&zzub_callback_template<223>,
	&zzub_callback_template<224>,
	&zzub_callback_template<225>,
	&zzub_callback_template<226>,
	&zzub_callback_template<227>,
	&zzub_callback_template<228>,
	&zzub_callback_template<229>,
	&zzub_callback_template<230>,
	&zzub_callback_template<231>,
	&zzub_callback_template<232>,
	&zzub_callback_template<233>,
	&zzub_callback_template<234>,
	&zzub_callback_template<235>,
	&zzub_callback_template<236>,
	&zzub_callback_template<237>,
	&zzub_callback_template<238>,
	&zzub_callback_template<239>,
	&zzub_callback_template<240>,
	&zzub_callback_template<241>,
	&zzub_callback_template<242>,
	&zzub_callback_template<243>,
	&zzub_callback_template<244>,
	&zzub_callback_template<245>,
	&zzub_callback_template<246>,
	&zzub_callback_template<247>,
	&zzub_callback_template<248>,
	&zzub_callback_template<249>,
	&zzub_callback_template<250>,
	&zzub_callback_template<251>,
	&zzub_callback_template<252>,
	&zzub_callback_template<253>,
	&zzub_callback_template<254>,
	&zzub_callback_template<255>,
	&zzub_callback_template<256>,
	&zzub_callback_template<257>,
	&zzub_callback_template<258>,
	&zzub_callback_template<259>,
	&zzub_callback_template<260>,
	&zzub_callback_template<261>,
	&zzub_callback_template<262>,
	&zzub_callback_template<263>,
	&zzub_callback_template<264>,
	&zzub_callback_template<265>,
	&zzub_callback_template<266>,
	&zzub_callback_template<267>,
	&zzub_callback_template<268>,
	&zzub_callback_template<269>,
	&zzub_callback_template<270>,
	&zzub_callback_template<271>,
	&zzub_callback_template<272>,
	&zzub_callback_template<273>,
	&zzub_callback_template<274>,
	&zzub_callback_template<275>,
	&zzub_callback_template<276>,
	&zzub_callback_template<277>,
	&zzub_callback_template<278>,
	&zzub_callback_template<279>,
	&zzub_callback_template<280>,
	&zzub_callback_template<281>,
	&zzub_callback_template<282>,
	&zzub_callback_template<283>,
	&zzub_callback_template<284>,
	&zzub_callback_template<285>,
	&zzub_callback_template<286>,
	&zzub_callback_template<287>,
	&zzub_callback_template<288>,
	&zzub_callback_template<289>,
	&zzub_callback_template<290>,
	&zzub_callback_template<291>,
	&zzub_callback_template<292>,
	&zzub_callback_template<293>,
	&zzub_callback_template<294>,
	&zzub_callback_template<295>,
	&zzub_callback_template<296>,
	&zzub_callback_template<297>,
	&zzub_callback_template<298>,
	&zzub_callback_template<299>,
	&zzub_callback_template<300>,
	&zzub_callback_template<301>,
	&zzub_callback_template<302>,
	&zzub_callback_template<303>,
	&zzub_callback_template<304>,
	&zzub_callback_template<305>,
	&zzub_callback_template<306>,
	&zzub_callback_template<307>,
	&zzub_callback_template<308>,
	&zzub_callback_template<309>,
	&zzub_callback_template<310>,
	&zzub_callback_template<311>,
	&zzub_callback_template<312>,
	&zzub_callback_template<313>,
	&zzub_callback_template<314>,
	&zzub_callback_template<315>,
	&zzub_callback_template<316>,
	&zzub_callback_template<317>,
	&zzub_callback_template<318>,
	&zzub_callback_template<319>,
	&zzub_callback_template<320>,
	&zzub_callback_template<321>,
	&zzub_callback_template<322>,
	&zzub_callback_template<323>,
	&zzub_callback_template<324>,
	&zzub_callback_template<325>,
	&zzub_callback_template<326>,
	&zzub_callback_template<327>,
	&zzub_callback_template<328>,
	&zzub_callback_template<329>,
	&zzub_callback_template<330>,
	&zzub_callback_template<331>,
	&zzub_callback_template<332>,
	&zzub_callback_template<333>,
	&zzub_callback_template<334>,
	&zzub_callback_template<335>,
	&zzub_callback_template<336>,
	&zzub_callback_template<337>,
	&zzub_callback_template<338>,
	&zzub_callback_template<339>,
	&zzub_callback_template<340>,
	&zzub_callback_template<341>,
	&zzub_callback_template<342>,
	&zzub_callback_template<343>,
	&zzub_callback_template<344>,
	&zzub_callback_template<345>,
	&zzub_callback_template<346>,
	&zzub_callback_template<347>,
	&zzub_callback_template<348>,
	&zzub_callback_template<349>,
	&zzub_callback_template<350>,
	&zzub_callback_template<351>,
	&zzub_callback_template<352>,
	&zzub_callback_template<353>,
	&zzub_callback_template<354>,
	&zzub_callback_template<355>,
	&zzub_callback_template<356>,
	&zzub_callback_template<357>,
	&zzub_callback_template<358>,
	&zzub_callback_template<359>,
	&zzub_callback_template<360>,
	&zzub_callback_template<361>,
	&zzub_callback_template<362>,
	&zzub_callback_template<363>,
	&zzub_callback_template<364>,
	&zzub_callback_template<365>,
	&zzub_callback_template<366>,
	&zzub_callback_template<367>,
	&zzub_callback_template<368>,
	&zzub_callback_template<369>,
	&zzub_callback_template<370>,
	&zzub_callback_template<371>,
	&zzub_callback_template<372>,
	&zzub_callback_template<373>,
	&zzub_callback_template<374>,
	&zzub_callback_template<375>,
	&zzub_callback_template<376>,
	&zzub_callback_template<377>,
	&zzub_callback_template<378>,
	&zzub_callback_template<379>,
	&zzub_callback_template<380>,
	&zzub_callback_template<381>,
	&zzub_callback_template<382>,
	&zzub_callback_template<383>,
	&zzub_callback_template<384>,
	&zzub_callback_template<385>,
	&zzub_callback_template<386>,
	&zzub_callback_template<387>,
	&zzub_callback_template<388>,
	&zzub_callback_template<389>,
	&zzub_callback_template<390>,
	&zzub_callback_template<391>,
	&zzub_callback_template<392>,
	&zzub_callback_template<393>,
	&zzub_callback_template<394>,
	&zzub_callback_template<395>,
	&zzub_callback_template<396>,
	&zzub_callback_template<397>,
	&zzub_callback_template<398>,
	&zzub_callback_template<399>,
	&zzub_callback_template<400>,
	&zzub_callback_template<401>,
	&zzub_callback_template<402>,
	&zzub_callback_template<403>,
	&zzub_callback_template<404>,
	&zzub_callback_template<405>,
	&zzub_callback_template<406>,
	&zzub_callback_template<407>,
	&zzub_callback_template<408>,
	&zzub_callback_template<409>,
	&zzub_callback_template<410>,
	&zzub_callback_template<411>,
	&zzub_callback_template<412>,
	&zzub_callback_template<413>,
	&zzub_callback_template<414>,
	&zzub_callback_template<415>,
	&zzub_callback_template<416>,
	&zzub_callback_template<417>,
	&zzub_callback_template<418>,
	&zzub_callback_template<419>,
	&zzub_callback_template<420>,
	&zzub_callback_template<421>,
	&zzub_callback_template<422>,
	&zzub_callback_template<423>,
	&zzub_callback_template<424>,
	&zzub_callback_template<425>,
	&zzub_callback_template<426>,
	&zzub_callback_template<427>,
	&zzub_callback_template<428>,
	&zzub_callback_template<429>,
	&zzub_callback_template<430>,
	&zzub_callback_template<431>,
	&zzub_callback_template<432>,
	&zzub_callback_template<433>,
	&zzub_callback_template<434>,
	&zzub_callback_template<435>,
	&zzub_callback_template<436>,
	&zzub_callback_template<437>,
	&zzub_callback_template<438>,
	&zzub_callback_template<439>,
	&zzub_callback_template<440>,
	&zzub_callback_template<441>,
	&zzub_callback_template<442>,
	&zzub_callback_template<443>,
	&zzub_callback_template<444>,
	&zzub_callback_template<445>,
	&zzub_callback_template<446>,
	&zzub_callback_template<447>,
	&zzub_callback_template<448>,
	&zzub_callback_template<449>,
	&zzub_callback_template<450>,
	&zzub_callback_template<451>,
	&zzub_callback_template<452>,
	&zzub_callback_template<453>,
	&zzub_callback_template<454>,
	&zzub_callback_template<455>,
	&zzub_callback_template<456>,
	&zzub_callback_template<457>,
	&zzub_callback_template<458>,
	&zzub_callback_template<459>,
	&zzub_callback_template<460>,
	&zzub_callback_template<461>,
	&zzub_callback_template<462>,
	&zzub_callback_template<463>,
	&zzub_callback_template<464>,
	&zzub_callback_template<465>,
	&zzub_callback_template<466>,
	&zzub_callback_template<467>,
	&zzub_callback_template<468>,
	&zzub_callback_template<469>,
	&zzub_callback_template<470>,
	&zzub_callback_template<471>,
	&zzub_callback_template<472>,
	&zzub_callback_template<473>,
	&zzub_callback_template<474>,
	&zzub_callback_template<475>,
	&zzub_callback_template<476>,
	&zzub_callback_template<477>,
	&zzub_callback_template<478>,
	&zzub_callback_template<479>,
	&zzub_callback_template<480>,
	&zzub_callback_template<481>,
	&zzub_callback_template<482>,
	&zzub_callback_template<483>,
	&zzub_callback_template<484>,
	&zzub_callback_template<485>,
	&zzub_callback_template<486>,
	&zzub_callback_template<487>,
	&zzub_callback_template<488>,
	&zzub_callback_template<489>,
	&zzub_callback_template<490>,
	&zzub_callback_template<491>,
	&zzub_callback_template<492>,
	&zzub_callback_template<493>,
	&zzub_callback_template<494>,
	&zzub_callback_template<495>,
	&zzub_callback_template<496>,
	&zzub_callback_template<497>,
	&zzub_callback_template<498>,
	&zzub_callback_template<499>,
};

zzub_callback_t zzub_callback_alloc(lua_State* L, int index) {
	const void* ptr = lua_topointer(L, index);
	assert(ptr != 0);
	for (int i = 0; i < 500; ++i) {
		if (ptr == zzub_callback_datas[i].ptr) {
			return zzub_callback_callbacks[i];
		}
	}
	lua_pushvalue(L, index);
	int functionref = luaL_ref(L, LUA_REGISTRYINDEX);
	for (int i = 0; i < 500; ++i) {
		if (zzub_callback_datas[i].L == 0) {
			zzub_callback_datas[i].L = L;
			zzub_callback_datas[i].ptr = ptr;
			zzub_callback_datas[i].functionref = functionref;
			return zzub_callback_callbacks[i];
		}
	}
	return 0;
}

void zzub_callback_clear(lua_State* L) {
	for (int i = 0; i < 500; ++i) {
		if (L == zzub_callback_datas[i].L) {
			zzub_callback_datas[i].L = 0;
			zzub_callback_datas[i].ptr = 0;
			zzub_callback_datas[i].functionref = 0;
		}
	}
}

static int double_click_get_plugin(lua_State *L) {
	zzub_event_data_double_click_t* self = (zzub_event_data_double_click_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DoubleClick:plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg double_click_lib[] = {
	{ "get_plugin", double_click_get_plugin }, 
	{ NULL, NULL }
};

static int insert_plugin_get_plugin(lua_State *L) {
	zzub_event_data_insert_plugin_t* self = (zzub_event_data_insert_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertPlugin:plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg insert_plugin_lib[] = {
	{ "get_plugin", insert_plugin_get_plugin }, 
	{ NULL, NULL }
};

static int delete_plugin_get_plugin(lua_State *L) {
	zzub_event_data_delete_plugin_t* self = (zzub_event_data_delete_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeletePlugin:plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg delete_plugin_lib[] = {
	{ "get_plugin", delete_plugin_get_plugin }, 
	{ NULL, NULL }
};

static int update_plugin_get_plugin(lua_State *L) {
	zzub_event_data_update_plugin_t* self = (zzub_event_data_update_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePlugin:plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg update_plugin_lib[] = {
	{ "get_plugin", update_plugin_get_plugin }, 
	{ NULL, NULL }
};

static int insert_connection_get_from_plugin(lua_State *L) {
	zzub_event_data_insert_connection_t* self = (zzub_event_data_insert_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertConnection:from_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->from_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int insert_connection_get_to_plugin(lua_State *L) {
	zzub_event_data_insert_connection_t* self = (zzub_event_data_insert_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertConnection:to_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->to_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int insert_connection_get_connection_plugin(lua_State *L) {
	zzub_event_data_insert_connection_t* self = (zzub_event_data_insert_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertConnection:connection_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->connection_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int insert_connection_get_type(lua_State *L) {
	zzub_event_data_insert_connection_t* self = (zzub_event_data_insert_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertConnection:type");
		return 0;
	}

	int _luaresult = self->type;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg insert_connection_lib[] = {
	{ "get_from_plugin", insert_connection_get_from_plugin }, 
	{ "get_to_plugin", insert_connection_get_to_plugin }, 
	{ "get_connection_plugin", insert_connection_get_connection_plugin }, 
	{ "get_type", insert_connection_get_type }, 
	{ NULL, NULL }
};

static int update_connection_get_connection_plugin(lua_State *L) {
	zzub_event_data_update_connection_t* self = (zzub_event_data_update_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdateConnection:connection_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->connection_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int update_connection_get_from_plugin(lua_State *L) {
	zzub_event_data_update_connection_t* self = (zzub_event_data_update_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdateConnection:from_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->from_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int update_connection_get_to_plugin(lua_State *L) {
	zzub_event_data_update_connection_t* self = (zzub_event_data_update_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdateConnection:to_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->to_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int update_connection_get_type(lua_State *L) {
	zzub_event_data_update_connection_t* self = (zzub_event_data_update_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdateConnection:type");
		return 0;
	}

	int _luaresult = self->type;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg update_connection_lib[] = {
	{ "get_connection_plugin", update_connection_get_connection_plugin }, 
	{ "get_from_plugin", update_connection_get_from_plugin }, 
	{ "get_to_plugin", update_connection_get_to_plugin }, 
	{ "get_type", update_connection_get_type }, 
	{ NULL, NULL }
};

static int delete_connection_get_connection_plugin(lua_State *L) {
	zzub_event_data_delete_connection_t* self = (zzub_event_data_delete_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeleteConnection:connection_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->connection_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int delete_connection_get_from_plugin(lua_State *L) {
	zzub_event_data_delete_connection_t* self = (zzub_event_data_delete_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeleteConnection:from_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->from_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int delete_connection_get_to_plugin(lua_State *L) {
	zzub_event_data_delete_connection_t* self = (zzub_event_data_delete_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeleteConnection:to_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->to_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int delete_connection_get_type(lua_State *L) {
	zzub_event_data_delete_connection_t* self = (zzub_event_data_delete_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeleteConnection:type");
		return 0;
	}

	int _luaresult = self->type;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg delete_connection_lib[] = {
	{ "get_connection_plugin", delete_connection_get_connection_plugin }, 
	{ "get_from_plugin", delete_connection_get_from_plugin }, 
	{ "get_to_plugin", delete_connection_get_to_plugin }, 
	{ "get_type", delete_connection_get_type }, 
	{ NULL, NULL }
};

static int insert_pattern_get_pattern(lua_State *L) {
	zzub_event_data_insert_pattern_t* self = (zzub_event_data_insert_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertPattern:pattern");
		return 0;
	}

	zzub_pattern_t* _luaresult = self->pattern;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg insert_pattern_lib[] = {
	{ "get_pattern", insert_pattern_get_pattern }, 
	{ NULL, NULL }
};

static int update_pattern_get_pattern(lua_State *L) {
	zzub_event_data_update_pattern_t* self = (zzub_event_data_update_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePattern:pattern");
		return 0;
	}

	zzub_pattern_t* _luaresult = self->pattern;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg update_pattern_lib[] = {
	{ "get_pattern", update_pattern_get_pattern }, 
	{ NULL, NULL }
};

static int delete_pattern_get_pattern(lua_State *L) {
	zzub_event_data_delete_pattern_t* self = (zzub_event_data_delete_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeletePattern:pattern");
		return 0;
	}

	zzub_pattern_t* _luaresult = self->pattern;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg delete_pattern_lib[] = {
	{ "get_pattern", delete_pattern_get_pattern }, 
	{ NULL, NULL }
};

static int insert_pattern_event_get_patternevent(lua_State *L) {
	zzub_event_data_insert_pattern_event_t* self = (zzub_event_data_insert_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertPatternEvent:patternevent");
		return 0;
	}

	zzub_pattern_event_t* _luaresult = self->patternevent;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg insert_pattern_event_lib[] = {
	{ "get_patternevent", insert_pattern_event_get_patternevent }, 
	{ NULL, NULL }
};

static int update_pattern_event_get_patternevent(lua_State *L) {
	zzub_event_data_update_pattern_event_t* self = (zzub_event_data_update_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePatternEvent:patternevent");
		return 0;
	}

	zzub_pattern_event_t* _luaresult = self->patternevent;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg update_pattern_event_lib[] = {
	{ "get_patternevent", update_pattern_event_get_patternevent }, 
	{ NULL, NULL }
};

static int delete_pattern_event_get_patternevent(lua_State *L) {
	zzub_event_data_delete_pattern_event_t* self = (zzub_event_data_delete_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeletePatternEvent:patternevent");
		return 0;
	}

	zzub_pattern_event_t* _luaresult = self->patternevent;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg delete_pattern_event_lib[] = {
	{ "get_patternevent", delete_pattern_event_get_patternevent }, 
	{ NULL, NULL }
};

static int insert_pattern_format_get_patternformat(lua_State *L) {
	zzub_event_data_insert_pattern_format_t* self = (zzub_event_data_insert_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertPatternFormat:patternformat");
		return 0;
	}

	zzub_pattern_format_t* _luaresult = self->patternformat;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg insert_pattern_format_lib[] = {
	{ "get_patternformat", insert_pattern_format_get_patternformat }, 
	{ NULL, NULL }
};

static int update_pattern_format_get_patternformat(lua_State *L) {
	zzub_event_data_update_pattern_format_t* self = (zzub_event_data_update_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePatternFormat:patternformat");
		return 0;
	}

	zzub_pattern_format_t* _luaresult = self->patternformat;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg update_pattern_format_lib[] = {
	{ "get_patternformat", update_pattern_format_get_patternformat }, 
	{ NULL, NULL }
};

static int delete_pattern_format_get_patternformat(lua_State *L) {
	zzub_event_data_delete_pattern_format_t* self = (zzub_event_data_delete_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeletePatternFormat:patternformat");
		return 0;
	}

	zzub_pattern_format_t* _luaresult = self->patternformat;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg delete_pattern_format_lib[] = {
	{ "get_patternformat", delete_pattern_format_get_patternformat }, 
	{ NULL, NULL }
};

static int insert_pattern_format_column_get_patternformatcolumn(lua_State *L) {
	zzub_event_data_insert_pattern_format_column_t* self = (zzub_event_data_insert_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertPatternFormatColumn:patternformatcolumn");
		return 0;
	}

	zzub_pattern_format_column_t* _luaresult = self->patternformatcolumn;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg insert_pattern_format_column_lib[] = {
	{ "get_patternformatcolumn", insert_pattern_format_column_get_patternformatcolumn }, 
	{ NULL, NULL }
};

static int update_pattern_format_column_get_patternformatcolumn(lua_State *L) {
	zzub_event_data_update_pattern_format_column_t* self = (zzub_event_data_update_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePatternFormatColumn:patternformatcolumn");
		return 0;
	}

	zzub_pattern_format_column_t* _luaresult = self->patternformatcolumn;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg update_pattern_format_column_lib[] = {
	{ "get_patternformatcolumn", update_pattern_format_column_get_patternformatcolumn }, 
	{ NULL, NULL }
};

static int delete_pattern_format_column_get_patternformatcolumn(lua_State *L) {
	zzub_event_data_delete_pattern_format_column_t* self = (zzub_event_data_delete_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeletePatternFormatColumn:patternformatcolumn");
		return 0;
	}

	zzub_pattern_format_column_t* _luaresult = self->patternformatcolumn;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg delete_pattern_format_column_lib[] = {
	{ "get_patternformatcolumn", delete_pattern_format_column_get_patternformatcolumn }, 
	{ NULL, NULL }
};

static int midi_message_get_status(lua_State *L) {
	zzub_event_data_midi_message_t* self = (zzub_event_data_midi_message_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MidiMessage:status");
		return 0;
	}

	unsigned char _luaresult = self->status;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int midi_message_get_data1(lua_State *L) {
	zzub_event_data_midi_message_t* self = (zzub_event_data_midi_message_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MidiMessage:data1");
		return 0;
	}

	unsigned char _luaresult = self->data1;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int midi_message_get_data2(lua_State *L) {
	zzub_event_data_midi_message_t* self = (zzub_event_data_midi_message_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for MidiMessage:data2");
		return 0;
	}

	unsigned char _luaresult = self->data2;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg midi_message_lib[] = {
	{ "get_status", midi_message_get_status }, 
	{ "get_data1", midi_message_get_data1 }, 
	{ "get_data2", midi_message_get_data2 }, 
	{ NULL, NULL }
};

static int update_plugin_parameter_get_plugin(lua_State *L) {
	zzub_event_data_update_plugin_parameter_t* self = (zzub_event_data_update_plugin_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePluginParameter:plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = self->plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int update_plugin_parameter_get_group(lua_State *L) {
	zzub_event_data_update_plugin_parameter_t* self = (zzub_event_data_update_plugin_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePluginParameter:group");
		return 0;
	}

	int _luaresult = self->group;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int update_plugin_parameter_get_track(lua_State *L) {
	zzub_event_data_update_plugin_parameter_t* self = (zzub_event_data_update_plugin_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePluginParameter:track");
		return 0;
	}

	int _luaresult = self->track;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int update_plugin_parameter_get_param(lua_State *L) {
	zzub_event_data_update_plugin_parameter_t* self = (zzub_event_data_update_plugin_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePluginParameter:param");
		return 0;
	}

	int _luaresult = self->param;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int update_plugin_parameter_get_value(lua_State *L) {
	zzub_event_data_update_plugin_parameter_t* self = (zzub_event_data_update_plugin_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePluginParameter:value");
		return 0;
	}

	int _luaresult = self->value;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg update_plugin_parameter_lib[] = {
	{ "get_plugin", update_plugin_parameter_get_plugin }, 
	{ "get_group", update_plugin_parameter_get_group }, 
	{ "get_track", update_plugin_parameter_get_track }, 
	{ "get_param", update_plugin_parameter_get_param }, 
	{ "get_value", update_plugin_parameter_get_value }, 
	{ NULL, NULL }
};

static int player_state_changed_get_player_state(lua_State *L) {
	zzub_event_data_player_state_changed_t* self = (zzub_event_data_player_state_changed_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PlayerStateChanged:player_state");
		return 0;
	}

	int _luaresult = self->player_state;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg player_state_changed_lib[] = {
	{ "get_player_state", player_state_changed_get_player_state }, 
	{ NULL, NULL }
};

static int player_order_changed_get_orderindex(lua_State *L) {
	zzub_event_data_player_order_changed_t* self = (zzub_event_data_player_order_changed_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PlayerOrderChanged:orderindex");
		return 0;
	}

	int _luaresult = self->orderindex;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg player_order_changed_lib[] = {
	{ "get_orderindex", player_order_changed_get_orderindex }, 
	{ NULL, NULL }
};

static int player_load_get_userdata(lua_State *L) {
	zzub_event_data_player_load_t* self = (zzub_event_data_player_load_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PlayerLoad:userdata");
		return 0;
	}

	zzub_archive_t* _luaresult = self->userdata;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg player_load_lib[] = {
	{ "get_userdata", player_load_get_userdata }, 
	{ NULL, NULL }
};

static int player_save_get_userdata(lua_State *L) {
	zzub_event_data_player_save_t* self = (zzub_event_data_player_save_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PlayerSave:userdata");
		return 0;
	}

	zzub_archive_t* _luaresult = self->userdata;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg player_save_lib[] = {
	{ "get_userdata", player_save_get_userdata }, 
	{ NULL, NULL }
};

static int vu_get_size(lua_State *L) {
	zzub_event_data_vu_t* self = (zzub_event_data_vu_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Vu:size");
		return 0;
	}

	int _luaresult = self->size;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int vu_get_left_amp(lua_State *L) {
	zzub_event_data_vu_t* self = (zzub_event_data_vu_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Vu:left_amp");
		return 0;
	}

	float _luaresult = self->left_amp;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int vu_get_right_amp(lua_State *L) {
	zzub_event_data_vu_t* self = (zzub_event_data_vu_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Vu:right_amp");
		return 0;
	}

	float _luaresult = self->right_amp;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int vu_get_time(lua_State *L) {
	zzub_event_data_vu_t* self = (zzub_event_data_vu_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Vu:time");
		return 0;
	}

	float _luaresult = self->time;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg vu_lib[] = {
	{ "get_size", vu_get_size }, 
	{ "get_left_amp", vu_get_left_amp }, 
	{ "get_right_amp", vu_get_right_amp }, 
	{ "get_time", vu_get_time }, 
	{ NULL, NULL }
};

static int serialize_get_mode(lua_State *L) {
	zzub_event_data_serialize_t* self = (zzub_event_data_serialize_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Serialize:mode");
		return 0;
	}

	char _luaresult = self->mode;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int serialize_get_archive(lua_State *L) {
	zzub_event_data_serialize_t* self = (zzub_event_data_serialize_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Serialize:archive");
		return 0;
	}

	zzub_archive_t* _luaresult = self->archive;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg serialize_lib[] = {
	{ "get_mode", serialize_get_mode }, 
	{ "get_archive", serialize_get_archive }, 
	{ NULL, NULL }
};

static int unknown_get_param(lua_State *L) {
	zzub_event_data_unknown_t* self = (zzub_event_data_unknown_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Unknown:param");
		return 0;
	}

	void* _luaresult = self->param;
	lua_pushlightuserdata(L, _luaresult);
	return 1;
};

static const luaL_Reg unknown_lib[] = {
	{ "get_param", unknown_get_param }, 
	{ NULL, NULL }
};

static int osc_message_get_path(lua_State *L) {
	zzub_event_data_osc_message_t* self = (zzub_event_data_osc_message_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for OscMessage:path");
		return 0;
	}

	const char* _luaresult = self->path;
	lua_pushstring(L, _luaresult);
	return 1;
};

static int osc_message_get_types(lua_State *L) {
	zzub_event_data_osc_message_t* self = (zzub_event_data_osc_message_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for OscMessage:types");
		return 0;
	}

	const char* _luaresult = self->types;
	lua_pushstring(L, _luaresult);
	return 1;
};

static int osc_message_get_argv(lua_State *L) {
	zzub_event_data_osc_message_t* self = (zzub_event_data_osc_message_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for OscMessage:argv");
		return 0;
	}

	const void** _luaresult = self->argv;
	lua_pushnumber(L, 0); // TODO: not supported type: array
	return 1;
};

static int osc_message_get_argc(lua_State *L) {
	zzub_event_data_osc_message_t* self = (zzub_event_data_osc_message_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for OscMessage:argc");
		return 0;
	}

	int _luaresult = self->argc;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int osc_message_get_msg(lua_State *L) {
	zzub_event_data_osc_message_t* self = (zzub_event_data_osc_message_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for OscMessage:msg");
		return 0;
	}

	void* _luaresult = self->msg;
	lua_pushlightuserdata(L, _luaresult);
	return 1;
};

static const luaL_Reg osc_message_lib[] = {
	{ "get_path", osc_message_get_path }, 
	{ "get_types", osc_message_get_types }, 
	{ "get_argv", osc_message_get_argv }, 
	{ "get_argc", osc_message_get_argc }, 
	{ "get_msg", osc_message_get_msg }, 
	{ NULL, NULL }
};

static int insert_wave_get_wave(lua_State *L) {
	zzub_event_data_insert_wave_t* self = (zzub_event_data_insert_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertWave:wave");
		return 0;
	}

	zzub_wave_t* _luaresult = self->wave;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg insert_wave_lib[] = {
	{ "get_wave", insert_wave_get_wave }, 
	{ NULL, NULL }
};

static int update_wave_get_wave(lua_State *L) {
	zzub_event_data_update_wave_t* self = (zzub_event_data_update_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdateWave:wave");
		return 0;
	}

	zzub_wave_t* _luaresult = self->wave;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg update_wave_lib[] = {
	{ "get_wave", update_wave_get_wave }, 
	{ NULL, NULL }
};

static int delete_wave_get_wave(lua_State *L) {
	zzub_event_data_delete_wave_t* self = (zzub_event_data_delete_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeleteWave:wave");
		return 0;
	}

	zzub_wave_t* _luaresult = self->wave;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg delete_wave_lib[] = {
	{ "get_wave", delete_wave_get_wave }, 
	{ NULL, NULL }
};

static int insert_wavelevel_get_wavelevel(lua_State *L) {
	zzub_event_data_insert_wavelevel_t* self = (zzub_event_data_insert_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertWavelevel:wavelevel");
		return 0;
	}

	zzub_wavelevel_t* _luaresult = self->wavelevel;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg insert_wavelevel_lib[] = {
	{ "get_wavelevel", insert_wavelevel_get_wavelevel }, 
	{ NULL, NULL }
};

static int update_wavelevel_get_wavelevel(lua_State *L) {
	zzub_event_data_update_wavelevel_t* self = (zzub_event_data_update_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdateWavelevel:wavelevel");
		return 0;
	}

	zzub_wavelevel_t* _luaresult = self->wavelevel;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg update_wavelevel_lib[] = {
	{ "get_wavelevel", update_wavelevel_get_wavelevel }, 
	{ NULL, NULL }
};

static int update_wavelevel_samples_get_wavelevel(lua_State *L) {
	zzub_event_data_update_wavelevel_samples_t* self = (zzub_event_data_update_wavelevel_samples_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdateWavelevelSamples:wavelevel");
		return 0;
	}

	zzub_wavelevel_t* _luaresult = self->wavelevel;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg update_wavelevel_samples_lib[] = {
	{ "get_wavelevel", update_wavelevel_samples_get_wavelevel }, 
	{ NULL, NULL }
};

static int delete_wavelevel_get_wavelevel(lua_State *L) {
	zzub_event_data_delete_wavelevel_t* self = (zzub_event_data_delete_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeleteWavelevel:wavelevel");
		return 0;
	}

	zzub_wavelevel_t* _luaresult = self->wavelevel;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg delete_wavelevel_lib[] = {
	{ "get_wavelevel", delete_wavelevel_get_wavelevel }, 
	{ NULL, NULL }
};

static int insert_plugin_group_get_group(lua_State *L) {
	zzub_event_data_insert_plugin_group_t* self = (zzub_event_data_insert_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for InsertPluginGroup:group");
		return 0;
	}

	zzub_plugin_group_t* _luaresult = self->group;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg insert_plugin_group_lib[] = {
	{ "get_group", insert_plugin_group_get_group }, 
	{ NULL, NULL }
};

static int delete_plugin_group_get_group(lua_State *L) {
	zzub_event_data_delete_plugin_group_t* self = (zzub_event_data_delete_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeletePluginGroup:group");
		return 0;
	}

	zzub_plugin_group_t* _luaresult = self->group;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg delete_plugin_group_lib[] = {
	{ "get_group", delete_plugin_group_get_group }, 
	{ NULL, NULL }
};

static int update_plugin_group_get_group(lua_State *L) {
	zzub_event_data_update_plugin_group_t* self = (zzub_event_data_update_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UpdatePluginGroup:group");
		return 0;
	}

	zzub_plugin_group_t* _luaresult = self->group;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg update_plugin_group_lib[] = {
	{ "get_group", update_plugin_group_get_group }, 
	{ NULL, NULL }
};

static int custom_get_id(lua_State *L) {
	zzub_event_data_custom_t* self = (zzub_event_data_custom_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Custom:id");
		return 0;
	}

	int _luaresult = self->id;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int custom_get_data(lua_State *L) {
	zzub_event_data_custom_t* self = (zzub_event_data_custom_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Custom:data");
		return 0;
	}

	void* _luaresult = self->data;
	lua_pushlightuserdata(L, _luaresult);
	return 1;
};

static const luaL_Reg custom_lib[] = {
	{ "get_id", custom_get_id }, 
	{ "get_data", custom_get_data }, 
	{ NULL, NULL }
};

static int user_alert_get_type(lua_State *L) {
	zzub_event_data_user_alert_t* self = (zzub_event_data_user_alert_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UserAlert:type");
		return 0;
	}

	int _luaresult = self->type;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int user_alert_get_collection(lua_State *L) {
	zzub_event_data_user_alert_t* self = (zzub_event_data_user_alert_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UserAlert:collection");
		return 0;
	}

	zzub_plugincollection_t* _luaresult = self->collection;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int user_alert_get_plugin(lua_State *L) {
	zzub_event_data_user_alert_t* self = (zzub_event_data_user_alert_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UserAlert:plugin");
		return 0;
	}

	zzub_pluginloader_t* _luaresult = self->plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int user_alert_get_wave(lua_State *L) {
	zzub_event_data_user_alert_t* self = (zzub_event_data_user_alert_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UserAlert:wave");
		return 0;
	}

	zzub_wave_t* _luaresult = self->wave;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int user_alert_get_progress(lua_State *L) {
	zzub_event_data_user_alert_t* self = (zzub_event_data_user_alert_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for UserAlert:progress");
		return 0;
	}

	int _luaresult = self->progress;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static const luaL_Reg user_alert_lib[] = {
	{ "get_type", user_alert_get_type }, 
	{ "get_collection", user_alert_get_collection }, 
	{ "get_plugin", user_alert_get_plugin }, 
	{ "get_wave", user_alert_get_wave }, 
	{ "get_progress", user_alert_get_progress }, 
	{ NULL, NULL }
};

static int event_data_get_type(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:type");
		return 0;
	}

	int _luaresult = self->type;
	lua_pushnumber(L, _luaresult);
	return 1;
};

static int event_data_get_userdata(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:userdata");
		return 0;
	}

	void* _luaresult = self->userdata;
	lua_pushlightuserdata(L, _luaresult);
	return 1;
};

static int event_data_get_double_click(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:double_click");
		return 0;
	}

	zzub_event_data_double_click_t* _luaresult = &self->double_click;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_midi_message(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:midi_message");
		return 0;
	}

	zzub_event_data_midi_message_t* _luaresult = &self->midi_message;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_player_state_changed(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:player_state_changed");
		return 0;
	}

	zzub_event_data_player_state_changed_t* _luaresult = &self->player_state_changed;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_player_order_changed(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:player_order_changed");
		return 0;
	}

	zzub_event_data_player_order_changed_t* _luaresult = &self->player_order_changed;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_player_load(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:player_load");
		return 0;
	}

	zzub_event_data_player_load_t* _luaresult = &self->player_load;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_player_save(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:player_save");
		return 0;
	}

	zzub_event_data_player_save_t* _luaresult = &self->player_save;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_osc_message(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:osc_message");
		return 0;
	}

	zzub_event_data_osc_message_t* _luaresult = &self->osc_message;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_vu(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:vu");
		return 0;
	}

	zzub_event_data_vu_t* _luaresult = &self->vu;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_serialize(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:serialize");
		return 0;
	}

	zzub_event_data_serialize_t* _luaresult = &self->serialize;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_insert_plugin(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:insert_plugin");
		return 0;
	}

	zzub_event_data_insert_plugin_t* _luaresult = &self->insert_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_plugin(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_plugin");
		return 0;
	}

	zzub_event_data_update_plugin_t* _luaresult = &self->update_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_delete_plugin(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:delete_plugin");
		return 0;
	}

	zzub_event_data_delete_plugin_t* _luaresult = &self->delete_plugin;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_pluginparameter(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_pluginparameter");
		return 0;
	}

	zzub_event_data_update_plugin_parameter_t* _luaresult = &self->update_pluginparameter;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_insert_connection(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:insert_connection");
		return 0;
	}

	zzub_event_data_insert_connection_t* _luaresult = &self->insert_connection;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_connection(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_connection");
		return 0;
	}

	zzub_event_data_update_connection_t* _luaresult = &self->update_connection;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_delete_connection(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:delete_connection");
		return 0;
	}

	zzub_event_data_delete_connection_t* _luaresult = &self->delete_connection;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_insert_pattern(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:insert_pattern");
		return 0;
	}

	zzub_event_data_insert_pattern_t* _luaresult = &self->insert_pattern;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_pattern(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_pattern");
		return 0;
	}

	zzub_event_data_update_pattern_t* _luaresult = &self->update_pattern;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_delete_pattern(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:delete_pattern");
		return 0;
	}

	zzub_event_data_delete_pattern_t* _luaresult = &self->delete_pattern;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_insert_patternevent(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:insert_patternevent");
		return 0;
	}

	zzub_event_data_insert_pattern_event_t* _luaresult = &self->insert_patternevent;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_patternevent(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_patternevent");
		return 0;
	}

	zzub_event_data_update_pattern_event_t* _luaresult = &self->update_patternevent;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_delete_patternevent(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:delete_patternevent");
		return 0;
	}

	zzub_event_data_delete_pattern_event_t* _luaresult = &self->delete_patternevent;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_insert_pattern_format(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:insert_pattern_format");
		return 0;
	}

	zzub_event_data_insert_pattern_format_t* _luaresult = &self->insert_pattern_format;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_pattern_format(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_pattern_format");
		return 0;
	}

	zzub_event_data_update_pattern_format_t* _luaresult = &self->update_pattern_format;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_delete_pattern_format(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:delete_pattern_format");
		return 0;
	}

	zzub_event_data_delete_pattern_format_t* _luaresult = &self->delete_pattern_format;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_insert_pattern_format_column(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:insert_pattern_format_column");
		return 0;
	}

	zzub_event_data_insert_pattern_format_column_t* _luaresult = &self->insert_pattern_format_column;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_pattern_format_column(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_pattern_format_column");
		return 0;
	}

	zzub_event_data_update_pattern_format_column_t* _luaresult = &self->update_pattern_format_column;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_delete_pattern_format_column(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:delete_pattern_format_column");
		return 0;
	}

	zzub_event_data_delete_pattern_format_column_t* _luaresult = &self->delete_pattern_format_column;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_insert_wave(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:insert_wave");
		return 0;
	}

	zzub_event_data_insert_wave_t* _luaresult = &self->insert_wave;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_wave(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_wave");
		return 0;
	}

	zzub_event_data_update_wave_t* _luaresult = &self->update_wave;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_delete_wave(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:delete_wave");
		return 0;
	}

	zzub_event_data_delete_wave_t* _luaresult = &self->delete_wave;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_insert_wavelevel(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:insert_wavelevel");
		return 0;
	}

	zzub_event_data_insert_wavelevel_t* _luaresult = &self->insert_wavelevel;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_wavelevel(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_wavelevel");
		return 0;
	}

	zzub_event_data_update_wavelevel_t* _luaresult = &self->update_wavelevel;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_delete_wavelevel(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:delete_wavelevel");
		return 0;
	}

	zzub_event_data_delete_wavelevel_t* _luaresult = &self->delete_wavelevel;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_wavelevel_samples(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_wavelevel_samples");
		return 0;
	}

	zzub_event_data_update_wavelevel_samples_t* _luaresult = &self->update_wavelevel_samples;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_insert_plugin_group(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:insert_plugin_group");
		return 0;
	}

	zzub_event_data_insert_plugin_group_t* _luaresult = &self->insert_plugin_group;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_update_plugin_group(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:update_plugin_group");
		return 0;
	}

	zzub_event_data_update_plugin_group_t* _luaresult = &self->update_plugin_group;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_delete_plugin_group(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:delete_plugin_group");
		return 0;
	}

	zzub_event_data_delete_plugin_group_t* _luaresult = &self->delete_plugin_group;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_alert(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:alert");
		return 0;
	}

	zzub_event_data_user_alert_t* _luaresult = &self->alert;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_custom(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:custom");
		return 0;
	}

	zzub_event_data_custom_t* _luaresult = &self->custom;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static int event_data_get_unknown(lua_State *L) {
	zzub_event_data_t* self = (zzub_event_data_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for EventData:unknown");
		return 0;
	}

	zzub_event_data_unknown_t* _luaresult = &self->unknown;
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
};

static const luaL_Reg event_data_lib[] = {
	{ "get_type", event_data_get_type }, 
	{ "get_userdata", event_data_get_userdata }, 
	{ "get_double_click", event_data_get_double_click }, 
	{ "get_midi_message", event_data_get_midi_message }, 
	{ "get_player_state_changed", event_data_get_player_state_changed }, 
	{ "get_player_order_changed", event_data_get_player_order_changed }, 
	{ "get_player_load", event_data_get_player_load }, 
	{ "get_player_save", event_data_get_player_save }, 
	{ "get_osc_message", event_data_get_osc_message }, 
	{ "get_vu", event_data_get_vu }, 
	{ "get_serialize", event_data_get_serialize }, 
	{ "get_insert_plugin", event_data_get_insert_plugin }, 
	{ "get_update_plugin", event_data_get_update_plugin }, 
	{ "get_delete_plugin", event_data_get_delete_plugin }, 
	{ "get_update_pluginparameter", event_data_get_update_pluginparameter }, 
	{ "get_insert_connection", event_data_get_insert_connection }, 
	{ "get_update_connection", event_data_get_update_connection }, 
	{ "get_delete_connection", event_data_get_delete_connection }, 
	{ "get_insert_pattern", event_data_get_insert_pattern }, 
	{ "get_update_pattern", event_data_get_update_pattern }, 
	{ "get_delete_pattern", event_data_get_delete_pattern }, 
	{ "get_insert_patternevent", event_data_get_insert_patternevent }, 
	{ "get_update_patternevent", event_data_get_update_patternevent }, 
	{ "get_delete_patternevent", event_data_get_delete_patternevent }, 
	{ "get_insert_pattern_format", event_data_get_insert_pattern_format }, 
	{ "get_update_pattern_format", event_data_get_update_pattern_format }, 
	{ "get_delete_pattern_format", event_data_get_delete_pattern_format }, 
	{ "get_insert_pattern_format_column", event_data_get_insert_pattern_format_column }, 
	{ "get_update_pattern_format_column", event_data_get_update_pattern_format_column }, 
	{ "get_delete_pattern_format_column", event_data_get_delete_pattern_format_column }, 
	{ "get_insert_wave", event_data_get_insert_wave }, 
	{ "get_update_wave", event_data_get_update_wave }, 
	{ "get_delete_wave", event_data_get_delete_wave }, 
	{ "get_insert_wavelevel", event_data_get_insert_wavelevel }, 
	{ "get_update_wavelevel", event_data_get_update_wavelevel }, 
	{ "get_delete_wavelevel", event_data_get_delete_wavelevel }, 
	{ "get_update_wavelevel_samples", event_data_get_update_wavelevel_samples }, 
	{ "get_insert_plugin_group", event_data_get_insert_plugin_group }, 
	{ "get_update_plugin_group", event_data_get_update_plugin_group }, 
	{ "get_delete_plugin_group", event_data_get_delete_plugin_group }, 
	{ "get_alert", event_data_get_alert }, 
	{ "get_custom", event_data_get_custom }, 
	{ "get_unknown", event_data_get_unknown }, 
	{ NULL, NULL }
};

static int device_info_get_api(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for DeviceInfo:get_api");
		return 0;
	}
	zzub_device_info_t* self = (zzub_device_info_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfo:get_api");
		return 0;
	}

	int _luaresult = zzub_device_info_get_api(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int device_info_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for DeviceInfo:get_name");
		return 0;
	}
	zzub_device_info_t* self = (zzub_device_info_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfo:get_name");
		return 0;
	}

	const char* _luaresult = zzub_device_info_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int device_info_get_supported_buffersizes(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for DeviceInfo:get_supported_buffersizes");
		return 0;
	}
	zzub_device_info_t* self = (zzub_device_info_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfo:get_supported_buffersizes");
		return 0;
	}

	int* result;
	int maxsizes = lua_tonumber(L, 4);
	int _luaresult = zzub_device_info_get_supported_buffersizes(self, result, maxsizes);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int device_info_get_supported_samplerates(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for DeviceInfo:get_supported_samplerates");
		return 0;
	}
	zzub_device_info_t* self = (zzub_device_info_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfo:get_supported_samplerates");
		return 0;
	}

	int* result;
	int maxrates = lua_tonumber(L, 4);
	int _luaresult = zzub_device_info_get_supported_samplerates(self, result, maxrates);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int device_info_get_supported_output_channels(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for DeviceInfo:get_supported_output_channels");
		return 0;
	}
	zzub_device_info_t* self = (zzub_device_info_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfo:get_supported_output_channels");
		return 0;
	}

	int _luaresult = zzub_device_info_get_supported_output_channels(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int device_info_get_supported_input_channels(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for DeviceInfo:get_supported_input_channels");
		return 0;
	}
	zzub_device_info_t* self = (zzub_device_info_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfo:get_supported_input_channels");
		return 0;
	}

	int _luaresult = zzub_device_info_get_supported_input_channels(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg device_info_lib[] = {
	{ "get_api", device_info_get_api },
	{ "get_name", device_info_get_name },
	{ "get_supported_buffersizes", device_info_get_supported_buffersizes },
	{ "get_supported_samplerates", device_info_get_supported_samplerates },
	{ "get_supported_output_channels", device_info_get_supported_output_channels },
	{ "get_supported_input_channels", device_info_get_supported_input_channels },
	{ NULL, NULL }
};

static int device_info_iterator_next(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for DeviceInfoIterator:next");
		return 0;
	}
	zzub_device_info_iterator_t* self = (zzub_device_info_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfoIterator:next");
		return 0;
	}

	zzub_device_info_iterator_next(self);
	return 0;
}

static int device_info_iterator_valid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for DeviceInfoIterator:valid");
		return 0;
	}
	zzub_device_info_iterator_t* self = (zzub_device_info_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfoIterator:valid");
		return 0;
	}

	int _luaresult = zzub_device_info_iterator_valid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int device_info_iterator_current(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for DeviceInfoIterator:current");
		return 0;
	}
	zzub_device_info_iterator_t* self = (zzub_device_info_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfoIterator:current");
		return 0;
	}

	zzub_device_info_t* _luaresult = zzub_device_info_iterator_current(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int device_info_iterator_reset(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for DeviceInfoIterator:reset");
		return 0;
	}
	zzub_device_info_iterator_t* self = (zzub_device_info_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfoIterator:reset");
		return 0;
	}

	zzub_device_info_iterator_reset(self);
	return 0;
}

static int device_info_iterator_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for DeviceInfoIterator:destroy");
		return 0;
	}
	zzub_device_info_iterator_t* self = (zzub_device_info_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for DeviceInfoIterator:destroy");
		return 0;
	}

	zzub_device_info_iterator_destroy(self);
	return 0;
}

static const luaL_Reg device_info_iterator_lib[] = {
	{ "next", device_info_iterator_next },
	{ "valid", device_info_iterator_valid },
	{ "current", device_info_iterator_current },
	{ "reset", device_info_iterator_reset },
	{ "destroy", device_info_iterator_destroy },
	{ NULL, NULL }
};

static int audiodriver_create_silent(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Audiodriver:create_silent");
		return 0;
	}
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 2);
	const char* name = luaL_tolstring(L, 3, &len);
	int out_channels = lua_tonumber(L, 4);
	int in_channels = lua_tonumber(L, 5);
	int* supported_rates;
	int num_rates = lua_tonumber(L, 7);
	zzub_audiodriver_t* _luaresult = zzub_audiodriver_create_silent(player, name, out_channels, in_channels, supported_rates, num_rates);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int audiodriver_create(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:create");
		return 0;
	}
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 2);
	zzub_audiodriver_t* _luaresult = zzub_audiodriver_create(player);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int audiodriver_get_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_count");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_count");
		return 0;
	}

	int _luaresult = zzub_audiodriver_get_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int audiodriver_get_device_info(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_device_info");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_device_info");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_device_info_t* _luaresult = zzub_audiodriver_get_device_info(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int audiodriver_get_device_info_by_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_device_info_by_name");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_device_info_by_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	zzub_device_info_t* _luaresult = zzub_audiodriver_get_device_info_by_name(self, name);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int audiodriver_get_output_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_output_iterator");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_output_iterator");
		return 0;
	}

	zzub_device_info_iterator_t* _luaresult = zzub_audiodriver_get_output_iterator(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int audiodriver_get_input_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_input_iterator");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_input_iterator");
		return 0;
	}

	zzub_device_info_iterator_t* _luaresult = zzub_audiodriver_get_input_iterator(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int audiodriver_get_input_iterator_for_output(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_input_iterator_for_output");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_input_iterator_for_output");
		return 0;
	}

	zzub_device_info_t* info = (zzub_device_info_t*)lua_touserdata(L, 3);
	zzub_device_info_iterator_t* _luaresult = zzub_audiodriver_get_input_iterator_for_output(self, info);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int audiodriver_create_device(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Audiodriver:create_device");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:create_device");
		return 0;
	}

	const char* input_name = luaL_tolstring(L, 3, &len);
	const char* output_name = luaL_tolstring(L, 4, &len);
	int buffersize = lua_tonumber(L, 5);
	int samplerate = lua_tonumber(L, 6);
	int _luaresult = zzub_audiodriver_create_device(self, input_name, output_name, buffersize, samplerate);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int audiodriver_get_current_device(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_current_device");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_current_device");
		return 0;
	}

	int for_input = lua_toboolean(L, 3);
	zzub_device_info_t* _luaresult = zzub_audiodriver_get_current_device(self, for_input);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int audiodriver_enable(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Audiodriver:enable");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:enable");
		return 0;
	}

	int state = lua_toboolean(L, 3);
	zzub_audiodriver_enable(self, state);
	return 0;
}

static int audiodriver_get_enabled(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_enabled");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_enabled");
		return 0;
	}

	int _luaresult = zzub_audiodriver_get_enabled(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int audiodriver_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:destroy");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:destroy");
		return 0;
	}

	zzub_audiodriver_destroy(self);
	return 0;
}

static int audiodriver_destroy_device(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:destroy_device");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:destroy_device");
		return 0;
	}

	zzub_audiodriver_destroy_device(self);
	return 0;
}

static int audiodriver_get_samplerate(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_samplerate");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_samplerate");
		return 0;
	}

	unsigned int _luaresult = zzub_audiodriver_get_samplerate(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int audiodriver_get_buffersize(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_buffersize");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_buffersize");
		return 0;
	}

	unsigned int _luaresult = zzub_audiodriver_get_buffersize(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int audiodriver_get_cpu_load(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_cpu_load");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_cpu_load");
		return 0;
	}

	double _luaresult = zzub_audiodriver_get_cpu_load(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int audiodriver_get_master_channel(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:get_master_channel");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:get_master_channel");
		return 0;
	}

	int _luaresult = zzub_audiodriver_get_master_channel(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int audiodriver_set_master_channel(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Audiodriver:set_master_channel");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:set_master_channel");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_audiodriver_set_master_channel(self, index);
	return 0;
}

static int audiodriver_configure(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Audiodriver:configure");
		return 0;
	}
	zzub_audiodriver_t* self = (zzub_audiodriver_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Audiodriver:configure");
		return 0;
	}

	zzub_audiodriver_configure(self);
	return 0;
}

static const luaL_Reg audiodriver_lib[] = {
	{ "create_silent", audiodriver_create_silent },
	{ "create", audiodriver_create },
	{ "get_count", audiodriver_get_count },
	{ "get_device_info", audiodriver_get_device_info },
	{ "get_device_info_by_name", audiodriver_get_device_info_by_name },
	{ "get_output_iterator", audiodriver_get_output_iterator },
	{ "get_input_iterator", audiodriver_get_input_iterator },
	{ "get_input_iterator_for_output", audiodriver_get_input_iterator_for_output },
	{ "create_device", audiodriver_create_device },
	{ "get_current_device", audiodriver_get_current_device },
	{ "enable", audiodriver_enable },
	{ "get_enabled", audiodriver_get_enabled },
	{ "destroy", audiodriver_destroy },
	{ "destroy_device", audiodriver_destroy_device },
	{ "get_samplerate", audiodriver_get_samplerate },
	{ "get_buffersize", audiodriver_get_buffersize },
	{ "get_cpu_load", audiodriver_get_cpu_load },
	{ "get_master_channel", audiodriver_get_master_channel },
	{ "set_master_channel", audiodriver_set_master_channel },
	{ "configure", audiodriver_configure },
	{ NULL, NULL }
};

static int mididriver_get_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Mididriver:get_count");
		return 0;
	}
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 2);
	int _luaresult = zzub_mididriver_get_count(player);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int mididriver_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Mididriver:get_name");
		return 0;
	}
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 2);
	int index = lua_tonumber(L, 3);
	const char* _luaresult = zzub_mididriver_get_name(player, index);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int mididriver_is_input(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Mididriver:is_input");
		return 0;
	}
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 2);
	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_mididriver_is_input(player, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int mididriver_is_output(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Mididriver:is_output");
		return 0;
	}
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 2);
	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_mididriver_is_output(player, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int mididriver_open(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Mididriver:open");
		return 0;
	}
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 2);
	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_mididriver_open(player, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int mididriver_close_all(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Mididriver:close_all");
		return 0;
	}
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 2);
	int _luaresult = zzub_mididriver_close_all(player);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg mididriver_lib[] = {
	{ "get_count", mididriver_get_count },
	{ "get_name", mididriver_get_name },
	{ "is_input", mididriver_is_input },
	{ "is_output", mididriver_is_output },
	{ "open", mididriver_open },
	{ "close_all", mididriver_close_all },
	{ NULL, NULL }
};

static int plugincollection_get_by_uri(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugincollection:get_by_uri");
		return 0;
	}
	zzub_player_t* player = (zzub_player_t*)lua_touserdata(L, 2);
	const char* uri = luaL_tolstring(L, 3, &len);
	zzub_plugincollection_t* _luaresult = zzub_plugincollection_get_by_uri(player, uri);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugincollection_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugincollection:get_name");
		return 0;
	}
	zzub_plugincollection_t* self = (zzub_plugincollection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugincollection:get_name");
		return 0;
	}

	const char* _luaresult = zzub_plugincollection_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugincollection_configure(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Plugincollection:configure");
		return 0;
	}
	zzub_plugincollection_t* self = (zzub_plugincollection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugincollection:configure");
		return 0;
	}

	const char* key = luaL_tolstring(L, 3, &len);
	const char* value = luaL_tolstring(L, 4, &len);
	zzub_plugincollection_configure(self, key, value);
	return 0;
}

static const luaL_Reg plugincollection_lib[] = {
	{ "get_by_uri", plugincollection_get_by_uri },
	{ "get_name", plugincollection_get_name },
	{ "configure", plugincollection_configure },
	{ NULL, NULL }
};

static int input_open_file(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Input:open_file");
		return 0;
	}
	const char* filename = luaL_tolstring(L, 2, &len);
	zzub_input_t* _luaresult = zzub_input_open_file(filename);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int input_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Input:destroy");
		return 0;
	}
	zzub_input_t* self = (zzub_input_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Input:destroy");
		return 0;
	}

	zzub_input_destroy(self);
	return 0;
}

static int input_read(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Input:read");
		return 0;
	}
	zzub_input_t* self = (zzub_input_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Input:read");
		return 0;
	}

	char* buffer;
	int bytes = lua_tonumber(L, 4);
	int _luaresult = zzub_input_read(self, buffer, bytes);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int input_size(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Input:size");
		return 0;
	}
	zzub_input_t* self = (zzub_input_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Input:size");
		return 0;
	}

	int _luaresult = zzub_input_size(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int input_position(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Input:position");
		return 0;
	}
	zzub_input_t* self = (zzub_input_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Input:position");
		return 0;
	}

	int _luaresult = zzub_input_position(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int input_seek(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Input:seek");
		return 0;
	}
	zzub_input_t* self = (zzub_input_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Input:seek");
		return 0;
	}

	int pos = lua_tonumber(L, 3);
	int mode = lua_tonumber(L, 4);
	zzub_input_seek(self, pos, mode);
	return 0;
}

static const luaL_Reg input_lib[] = {
	{ "open_file", input_open_file },
	{ "destroy", input_destroy },
	{ "read", input_read },
	{ "size", input_size },
	{ "position", input_position },
	{ "seek", input_seek },
	{ NULL, NULL }
};

static int output_create_file(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Output:create_file");
		return 0;
	}
	const char* filename = luaL_tolstring(L, 2, &len);
	zzub_output_t* _luaresult = zzub_output_create_file(filename);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int output_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Output:destroy");
		return 0;
	}
	zzub_output_t* self = (zzub_output_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Output:destroy");
		return 0;
	}

	zzub_output_destroy(self);
	return 0;
}

static int output_write(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Output:write");
		return 0;
	}
	zzub_output_t* self = (zzub_output_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Output:write");
		return 0;
	}

	const char* buffer;	int bytes = lua_tonumber(L, 4);
	zzub_output_write(self, buffer, bytes);
	return 0;
}

static int output_position(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Output:position");
		return 0;
	}
	zzub_output_t* self = (zzub_output_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Output:position");
		return 0;
	}

	int _luaresult = zzub_output_position(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int output_seek(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Output:seek");
		return 0;
	}
	zzub_output_t* self = (zzub_output_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Output:seek");
		return 0;
	}

	int pos = lua_tonumber(L, 3);
	int mode = lua_tonumber(L, 4);
	zzub_output_seek(self, pos, mode);
	return 0;
}

static const luaL_Reg output_lib[] = {
	{ "create_file", output_create_file },
	{ "destroy", output_destroy },
	{ "write", output_write },
	{ "position", output_position },
	{ "seek", output_seek },
	{ NULL, NULL }
};

static int archive_create_memory(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 1) {
		luaL_error(L, "Invalid argument count for Archive:create_memory");
		return 0;
	}
	zzub_archive_t* _luaresult = zzub_archive_create_memory();
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int archive_get_output(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Archive:get_output");
		return 0;
	}
	zzub_archive_t* self = (zzub_archive_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Archive:get_output");
		return 0;
	}

	const char* path = luaL_tolstring(L, 3, &len);
	zzub_output_t* _luaresult = zzub_archive_get_output(self, path);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int archive_get_input(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Archive:get_input");
		return 0;
	}
	zzub_archive_t* self = (zzub_archive_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Archive:get_input");
		return 0;
	}

	const char* path = luaL_tolstring(L, 3, &len);
	zzub_input_t* _luaresult = zzub_archive_get_input(self, path);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int archive_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Archive:destroy");
		return 0;
	}
	zzub_archive_t* self = (zzub_archive_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Archive:destroy");
		return 0;
	}

	zzub_archive_destroy(self);
	return 0;
}

static const luaL_Reg archive_lib[] = {
	{ "create_memory", archive_create_memory },
	{ "get_output", archive_get_output },
	{ "get_input", archive_get_input },
	{ "destroy", archive_destroy },
	{ NULL, NULL }
};

static int midimapping_get_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Midimapping:get_plugin");
		return 0;
	}
	zzub_midimapping_t* self = (zzub_midimapping_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Midimapping:get_plugin");
		return 0;
	}

	int _luaresult = zzub_midimapping_get_plugin(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int midimapping_get_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Midimapping:get_group");
		return 0;
	}
	zzub_midimapping_t* self = (zzub_midimapping_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Midimapping:get_group");
		return 0;
	}

	int _luaresult = zzub_midimapping_get_group(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int midimapping_get_track(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Midimapping:get_track");
		return 0;
	}
	zzub_midimapping_t* self = (zzub_midimapping_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Midimapping:get_track");
		return 0;
	}

	int _luaresult = zzub_midimapping_get_track(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int midimapping_get_column(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Midimapping:get_column");
		return 0;
	}
	zzub_midimapping_t* self = (zzub_midimapping_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Midimapping:get_column");
		return 0;
	}

	int _luaresult = zzub_midimapping_get_column(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int midimapping_get_channel(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Midimapping:get_channel");
		return 0;
	}
	zzub_midimapping_t* self = (zzub_midimapping_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Midimapping:get_channel");
		return 0;
	}

	int _luaresult = zzub_midimapping_get_channel(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int midimapping_get_controller(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Midimapping:get_controller");
		return 0;
	}
	zzub_midimapping_t* self = (zzub_midimapping_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Midimapping:get_controller");
		return 0;
	}

	int _luaresult = zzub_midimapping_get_controller(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg midimapping_lib[] = {
	{ "get_plugin", midimapping_get_plugin },
	{ "get_group", midimapping_get_group },
	{ "get_track", midimapping_get_track },
	{ "get_column", midimapping_get_column },
	{ "get_channel", midimapping_get_channel },
	{ "get_controller", midimapping_get_controller },
	{ NULL, NULL }
};

static int pattern_event_get_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEvent:get_id");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:get_id");
		return 0;
	}

	int _luaresult = zzub_pattern_event_get_id(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_get_pluginid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEvent:get_pluginid");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:get_pluginid");
		return 0;
	}

	int _luaresult = zzub_pattern_event_get_pluginid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_get_pattern(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEvent:get_pattern");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:get_pattern");
		return 0;
	}

	zzub_pattern_t* _luaresult = zzub_pattern_event_get_pattern(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_event_get_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEvent:get_group");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:get_group");
		return 0;
	}

	int _luaresult = zzub_pattern_event_get_group(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_get_track(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEvent:get_track");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:get_track");
		return 0;
	}

	int _luaresult = zzub_pattern_event_get_track(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_get_column(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEvent:get_column");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:get_column");
		return 0;
	}

	int _luaresult = zzub_pattern_event_get_column(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_get_time(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEvent:get_time");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:get_time");
		return 0;
	}

	int _luaresult = zzub_pattern_event_get_time(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_get_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEvent:get_value");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:get_value");
		return 0;
	}

	int _luaresult = zzub_pattern_event_get_value(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_get_meta(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEvent:get_meta");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:get_meta");
		return 0;
	}

	int _luaresult = zzub_pattern_event_get_meta(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_set_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PatternEvent:set_value");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:set_value");
		return 0;
	}

	int value = lua_tonumber(L, 3);
	int _luaresult = zzub_pattern_event_set_value(self, value);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_set_meta(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PatternEvent:set_meta");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:set_meta");
		return 0;
	}

	int meta = lua_tonumber(L, 3);
	int _luaresult = zzub_pattern_event_set_meta(self, meta);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_set_time(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PatternEvent:set_time");
		return 0;
	}
	zzub_pattern_event_t* self = (zzub_pattern_event_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEvent:set_time");
		return 0;
	}

	int value = lua_tonumber(L, 3);
	int _luaresult = zzub_pattern_event_set_time(self, value);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg pattern_event_lib[] = {
	{ "get_id", pattern_event_get_id },
	{ "get_pluginid", pattern_event_get_pluginid },
	{ "get_pattern", pattern_event_get_pattern },
	{ "get_group", pattern_event_get_group },
	{ "get_track", pattern_event_get_track },
	{ "get_column", pattern_event_get_column },
	{ "get_time", pattern_event_get_time },
	{ "get_value", pattern_event_get_value },
	{ "get_meta", pattern_event_get_meta },
	{ "set_value", pattern_event_set_value },
	{ "set_meta", pattern_event_set_meta },
	{ "set_time", pattern_event_set_time },
	{ NULL, NULL }
};

static int pattern_iterator_next(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternIterator:next");
		return 0;
	}
	zzub_pattern_iterator_t* self = (zzub_pattern_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternIterator:next");
		return 0;
	}

	zzub_pattern_iterator_next(self);
	return 0;
}

static int pattern_iterator_valid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternIterator:valid");
		return 0;
	}
	zzub_pattern_iterator_t* self = (zzub_pattern_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternIterator:valid");
		return 0;
	}

	int _luaresult = zzub_pattern_iterator_valid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_iterator_current(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternIterator:current");
		return 0;
	}
	zzub_pattern_iterator_t* self = (zzub_pattern_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternIterator:current");
		return 0;
	}

	zzub_pattern_t* _luaresult = zzub_pattern_iterator_current(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_iterator_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternIterator:destroy");
		return 0;
	}
	zzub_pattern_iterator_t* self = (zzub_pattern_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternIterator:destroy");
		return 0;
	}

	zzub_pattern_iterator_destroy(self);
	return 0;
}

static const luaL_Reg pattern_iterator_lib[] = {
	{ "next", pattern_iterator_next },
	{ "valid", pattern_iterator_valid },
	{ "current", pattern_iterator_current },
	{ "destroy", pattern_iterator_destroy },
	{ NULL, NULL }
};

static int pattern_event_iterator_next(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEventIterator:next");
		return 0;
	}
	zzub_pattern_event_iterator_t* self = (zzub_pattern_event_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEventIterator:next");
		return 0;
	}

	zzub_pattern_event_iterator_next(self);
	return 0;
}

static int pattern_event_iterator_valid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEventIterator:valid");
		return 0;
	}
	zzub_pattern_event_iterator_t* self = (zzub_pattern_event_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEventIterator:valid");
		return 0;
	}

	int _luaresult = zzub_pattern_event_iterator_valid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_event_iterator_current(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEventIterator:current");
		return 0;
	}
	zzub_pattern_event_iterator_t* self = (zzub_pattern_event_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEventIterator:current");
		return 0;
	}

	zzub_pattern_event_t* _luaresult = zzub_pattern_event_iterator_current(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_event_iterator_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternEventIterator:destroy");
		return 0;
	}
	zzub_pattern_event_iterator_t* self = (zzub_pattern_event_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternEventIterator:destroy");
		return 0;
	}

	zzub_pattern_event_iterator_destroy(self);
	return 0;
}

static const luaL_Reg pattern_event_iterator_lib[] = {
	{ "next", pattern_event_iterator_next },
	{ "valid", pattern_event_iterator_valid },
	{ "current", pattern_event_iterator_current },
	{ "destroy", pattern_event_iterator_destroy },
	{ NULL, NULL }
};

static int pattern_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:destroy");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:destroy");
		return 0;
	}

	zzub_pattern_destroy(self);
	return 0;
}

static int pattern_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_name");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_name");
		return 0;
	}

	const char* _luaresult = zzub_pattern_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int pattern_set_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:set_name");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	zzub_pattern_set_name(self, name);
	return 0;
}

static int pattern_get_row_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_row_count");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_row_count");
		return 0;
	}

	int _luaresult = zzub_pattern_get_row_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_set_row_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:set_row_count");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_row_count");
		return 0;
	}

	int length = lua_tonumber(L, 3);
	zzub_pattern_set_row_count(self, length);
	return 0;
}

static int pattern_get_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_id");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_id");
		return 0;
	}

	int _luaresult = zzub_pattern_get_id(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_get_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_format");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_format");
		return 0;
	}

	zzub_pattern_format_t* _luaresult = zzub_pattern_get_format(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_set_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:set_format");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_format");
		return 0;
	}

	zzub_pattern_format_t* format = (zzub_pattern_format_t*)lua_touserdata(L, 3);
	zzub_pattern_set_format(self, format);
	return 0;
}

static int pattern_get_resolution(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_resolution");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_resolution");
		return 0;
	}

	int _luaresult = zzub_pattern_get_resolution(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_set_resolution(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:set_resolution");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_resolution");
		return 0;
	}

	int resolution = lua_tonumber(L, 3);
	zzub_pattern_set_resolution(self, resolution);
	return 0;
}

static int pattern_get_display_resolution(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_display_resolution");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_display_resolution");
		return 0;
	}

	int _luaresult = zzub_pattern_get_display_resolution(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_set_display_resolution(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:set_display_resolution");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_display_resolution");
		return 0;
	}

	int resolution = lua_tonumber(L, 3);
	zzub_pattern_set_display_resolution(self, resolution);
	return 0;
}

static int pattern_get_display_beat_rows(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Pattern:get_display_beat_rows");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_display_beat_rows");
		return 0;
	}

	int verydarkrow;
	int darkrow;
	zzub_pattern_get_display_beat_rows(self, &verydarkrow, &darkrow);
	return 0;
}

static int pattern_set_display_beat_rows(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Pattern:set_display_beat_rows");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_display_beat_rows");
		return 0;
	}

	int verydarkrow = lua_tonumber(L, 3);
	int darkrow = lua_tonumber(L, 4);
	zzub_pattern_set_display_beat_rows(self, verydarkrow, darkrow);
	return 0;
}

static int pattern_get_loop_start(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_loop_start");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_loop_start");
		return 0;
	}

	int _luaresult = zzub_pattern_get_loop_start(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_set_loop_start(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:set_loop_start");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_loop_start");
		return 0;
	}

	int pos = lua_tonumber(L, 3);
	zzub_pattern_set_loop_start(self, pos);
	return 0;
}

static int pattern_get_loop_end(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_loop_end");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_loop_end");
		return 0;
	}

	int _luaresult = zzub_pattern_get_loop_end(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_set_loop_end(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:set_loop_end");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_loop_end");
		return 0;
	}

	int pos = lua_tonumber(L, 3);
	zzub_pattern_set_loop_end(self, pos);
	return 0;
}

static int pattern_get_loop_enabled(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_loop_enabled");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_loop_enabled");
		return 0;
	}

	int _luaresult = zzub_pattern_get_loop_enabled(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_set_loop_enabled(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:set_loop_enabled");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_loop_enabled");
		return 0;
	}

	int enable = lua_tonumber(L, 3);
	zzub_pattern_set_loop_enabled(self, enable);
	return 0;
}

static int pattern_get_replay_row(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_replay_row");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_replay_row");
		return 0;
	}

	int _luaresult = zzub_pattern_get_replay_row(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_set_replay_row(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:set_replay_row");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_replay_row");
		return 0;
	}

	int row = lua_tonumber(L, 3);
	zzub_pattern_set_replay_row(self, row);
	return 0;
}

static int pattern_get_currently_playing_row(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pattern:get_currently_playing_row");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_currently_playing_row");
		return 0;
	}

	int _luaresult = zzub_pattern_get_currently_playing_row(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_set_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:set_value");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_value");
		return 0;
	}

	int row = lua_tonumber(L, 3);
	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 4);
	int group = lua_tonumber(L, 5);
	int track = lua_tonumber(L, 6);
	int column = lua_tonumber(L, 7);
	int value = lua_tonumber(L, 8);
	int meta = lua_tonumber(L, 9);
	zzub_pattern_set_value(self, row, plugin, group, track, column, value, meta);
	return 0;
}

static int pattern_get_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:get_value");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_value");
		return 0;
	}

	int row = lua_tonumber(L, 3);
	int pluginid = lua_tonumber(L, 4);
	int group = lua_tonumber(L, 5);
	int track = lua_tonumber(L, 6);
	int column = lua_tonumber(L, 7);
	int value;
	int meta;
	int _luaresult = zzub_pattern_get_value(self, row, pluginid, group, track, column, &value, &meta);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_get_event_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Pattern:get_event_iterator");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_event_iterator");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	zzub_pattern_event_iterator_t* _luaresult = zzub_pattern_get_event_iterator(self, plugin, group, track, column);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_get_event_unsorted_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Pattern:get_event_unsorted_iterator");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:get_event_unsorted_iterator");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	zzub_pattern_event_iterator_t* _luaresult = zzub_pattern_get_event_unsorted_iterator(self, plugin, group, track, column);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_insert_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:insert_value");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:insert_value");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int time = lua_tonumber(L, 7);
	int value = lua_tonumber(L, 8);
	int meta = lua_tonumber(L, 9);
	zzub_pattern_insert_value(self, pluginid, group, track, column, time, value, meta);
	return 0;
}

static int pattern_delete_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:delete_value");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:delete_value");
		return 0;
	}

	int id = lua_tonumber(L, 3);
	zzub_pattern_delete_value(self, id);
	return 0;
}

static int pattern_update_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Pattern:update_value");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:update_value");
		return 0;
	}

	int id = lua_tonumber(L, 3);
	int time = lua_tonumber(L, 4);
	int value = lua_tonumber(L, 5);
	int meta = lua_tonumber(L, 6);
	zzub_pattern_update_value(self, id, time, value, meta);
	return 0;
}

static int pattern_update_value_full(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 10) {
		luaL_error(L, "Invalid argument count for Pattern:update_value_full");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:update_value_full");
		return 0;
	}

	int id = lua_tonumber(L, 3);
	int pluginid = lua_tonumber(L, 4);
	int group = lua_tonumber(L, 5);
	int track = lua_tonumber(L, 6);
	int column = lua_tonumber(L, 7);
	int time = lua_tonumber(L, 8);
	int value = lua_tonumber(L, 9);
	int meta = lua_tonumber(L, 10);
	zzub_pattern_update_value_full(self, id, pluginid, group, track, column, time, value, meta);
	return 0;
}

static int pattern_compact_pattern(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:compact_pattern");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:compact_pattern");
		return 0;
	}

	int factor = lua_tonumber(L, 3);
	zzub_pattern_compact_pattern(self, factor);
	return 0;
}

static int pattern_expand_pattern(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pattern:expand_pattern");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:expand_pattern");
		return 0;
	}

	int factor = lua_tonumber(L, 3);
	zzub_pattern_expand_pattern(self, factor);
	return 0;
}

static int pattern_timeshift_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Pattern:timeshift_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:timeshift_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int timeshift = lua_tonumber(L, 8);
	zzub_pattern_timeshift_events(self, pluginid, group, track, column, fromtime, timeshift);
	return 0;
}

static int pattern_delete_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Pattern:delete_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:delete_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	zzub_pattern_delete_events(self, pluginid, group, track, column, fromtime, length);
	return 0;
}

static int pattern_move_scale_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 10) {
		luaL_error(L, "Invalid argument count for Pattern:move_scale_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:move_scale_events");
		return 0;
	}

	int src_idx = lua_tonumber(L, 3);
	int src_time = lua_tonumber(L, 4);
	int dst_idx = lua_tonumber(L, 5);
	int dst_time = lua_tonumber(L, 6);
	int width = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int mode = lua_tonumber(L, 9);
	int makecopy = lua_tonumber(L, 10);
	zzub_pattern_move_scale_events(self, src_idx, src_time, dst_idx, dst_time, width, length, mode, makecopy);
	return 0;
}

static int pattern_paste_stream_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Pattern:paste_stream_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:paste_stream_events");
		return 0;
	}

	int fromidx = lua_tonumber(L, 3);
	int fromtime = lua_tonumber(L, 4);
	int mode = lua_tonumber(L, 5);
	const char* charbuf = luaL_tolstring(L, 6, &len);
	zzub_pattern_paste_stream_events(self, fromidx, fromtime, mode, charbuf);
	return 0;
}

static int pattern_transpose_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 14) {
		luaL_error(L, "Invalid argument count for Pattern:transpose_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:transpose_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int delta = lua_tonumber(L, 9);
	int* holes;
	int holecount = lua_tonumber(L, 11);
	int* metas;
	int metacount = lua_tonumber(L, 13);
	int chromatic = lua_tonumber(L, 14);
	zzub_pattern_transpose_events(self, pluginid, group, track, column, fromtime, length, delta, holes, holecount, metas, metacount, chromatic);
	return 0;
}

static int pattern_randomize_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:randomize_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:randomize_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int skip = lua_tonumber(L, 9);
	zzub_pattern_randomize_events(self, pluginid, group, track, column, fromtime, length, skip);
	return 0;
}

static int pattern_randomize_range_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 11) {
		luaL_error(L, "Invalid argument count for Pattern:randomize_range_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:randomize_range_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int from_val = lua_tonumber(L, 9);
	int to_val = lua_tonumber(L, 10);
	int skip = lua_tonumber(L, 11);
	zzub_pattern_randomize_range_events(self, pluginid, group, track, column, fromtime, length, from_val, to_val, skip);
	return 0;
}

static int pattern_randomize_from_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:randomize_from_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:randomize_from_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int skip = lua_tonumber(L, 9);
	zzub_pattern_randomize_from_events(self, pluginid, group, track, column, fromtime, length, skip);
	return 0;
}

static int pattern_humanize_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:humanize_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:humanize_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int deviation = lua_tonumber(L, 9);
	zzub_pattern_humanize_events(self, pluginid, group, track, column, fromtime, length, deviation);
	return 0;
}

static int pattern_shuffle_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Pattern:shuffle_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:shuffle_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	zzub_pattern_shuffle_events(self, pluginid, group, track, column, fromtime, length);
	return 0;
}

static int pattern_interpolate_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:interpolate_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:interpolate_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int skip = lua_tonumber(L, 9);
	zzub_pattern_interpolate_events(self, pluginid, group, track, column, fromtime, length, skip);
	return 0;
}

static int pattern_gradiate_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:gradiate_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:gradiate_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int skip = lua_tonumber(L, 9);
	zzub_pattern_gradiate_events(self, pluginid, group, track, column, fromtime, length, skip);
	return 0;
}

static int pattern_smooth_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:smooth_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:smooth_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int strength = lua_tonumber(L, 9);
	zzub_pattern_smooth_events(self, pluginid, group, track, column, fromtime, length, strength);
	return 0;
}

static int pattern_reverse_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Pattern:reverse_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:reverse_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	zzub_pattern_reverse_events(self, pluginid, group, track, column, fromtime, length);
	return 0;
}

static int pattern_compact_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:compact_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:compact_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int factor = lua_tonumber(L, 9);
	zzub_pattern_compact_events(self, pluginid, group, track, column, fromtime, length, factor);
	return 0;
}

static int pattern_expand_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:expand_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:expand_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int factor = lua_tonumber(L, 9);
	zzub_pattern_expand_events(self, pluginid, group, track, column, fromtime, length, factor);
	return 0;
}

static int pattern_thin_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:thin_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:thin_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int major = lua_tonumber(L, 9);
	zzub_pattern_thin_events(self, pluginid, group, track, column, fromtime, length, major);
	return 0;
}

static int pattern_repeat_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:repeat_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:repeat_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int major = lua_tonumber(L, 9);
	zzub_pattern_repeat_events(self, pluginid, group, track, column, fromtime, length, major);
	return 0;
}

static int pattern_echo_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:echo_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:echo_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int major = lua_tonumber(L, 9);
	zzub_pattern_echo_events(self, pluginid, group, track, column, fromtime, length, major);
	return 0;
}

static int pattern_unique_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Pattern:unique_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:unique_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	zzub_pattern_unique_events(self, pluginid, group, track, column, fromtime, length);
	return 0;
}

static int pattern_scale_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 12) {
		luaL_error(L, "Invalid argument count for Pattern:scale_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:scale_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	double min1 = lua_tonumber(L, 9);
	double max1 = lua_tonumber(L, 10);
	double min2 = lua_tonumber(L, 11);
	double max2 = lua_tonumber(L, 12);
	zzub_pattern_scale_events(self, pluginid, group, track, column, fromtime, length, min1, max1, min2, max2);
	return 0;
}

static int pattern_fade_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 10) {
		luaL_error(L, "Invalid argument count for Pattern:fade_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:fade_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	double fromvalue = lua_tonumber(L, 9);
	double tovalue = lua_tonumber(L, 10);
	zzub_pattern_fade_events(self, pluginid, group, track, column, fromtime, length, fromvalue, tovalue);
	return 0;
}

static int pattern_curvemap_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:curvemap_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:curvemap_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int mode = lua_tonumber(L, 9);
	zzub_pattern_curvemap_events(self, pluginid, group, track, column, fromtime, length, mode);
	return 0;
}

static int pattern_invert_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Pattern:invert_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:invert_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	zzub_pattern_invert_events(self, pluginid, group, track, column, fromtime, length);
	return 0;
}

static int pattern_rotate_rows_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:rotate_rows_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:rotate_rows_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int offset = lua_tonumber(L, 9);
	zzub_pattern_rotate_rows_events(self, pluginid, group, track, column, fromtime, length, offset);
	return 0;
}

static int pattern_rotate_vals_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:rotate_vals_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:rotate_vals_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int offset = lua_tonumber(L, 9);
	zzub_pattern_rotate_vals_events(self, pluginid, group, track, column, fromtime, length, offset);
	return 0;
}

static int pattern_rotate_dist_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Pattern:rotate_dist_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:rotate_dist_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int offset = lua_tonumber(L, 9);
	zzub_pattern_rotate_dist_events(self, pluginid, group, track, column, fromtime, length, offset);
	return 0;
}

static int pattern_set_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 10) {
		luaL_error(L, "Invalid argument count for Pattern:set_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:set_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int value = lua_tonumber(L, 9);
	int meta = lua_tonumber(L, 10);
	zzub_pattern_set_events(self, pluginid, group, track, column, fromtime, length, value, meta);
	return 0;
}

static int pattern_replace_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 12) {
		luaL_error(L, "Invalid argument count for Pattern:replace_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:replace_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int from_value = lua_tonumber(L, 9);
	int from_meta = lua_tonumber(L, 10);
	int to_value = lua_tonumber(L, 11);
	int to_meta = lua_tonumber(L, 12);
	zzub_pattern_replace_events(self, pluginid, group, track, column, fromtime, length, from_value, from_meta, to_value, to_meta);
	return 0;
}

static int pattern_remove_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 10) {
		luaL_error(L, "Invalid argument count for Pattern:remove_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:remove_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int value = lua_tonumber(L, 9);
	int meta = lua_tonumber(L, 10);
	zzub_pattern_remove_events(self, pluginid, group, track, column, fromtime, length, value, meta);
	return 0;
}

static int pattern_notelength_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 11) {
		luaL_error(L, "Invalid argument count for Pattern:notelength_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:notelength_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int fromtime = lua_tonumber(L, 7);
	int length = lua_tonumber(L, 8);
	int desired_len = lua_tonumber(L, 9);
	int mode = lua_tonumber(L, 10);
	int off_value = lua_tonumber(L, 11);
	zzub_pattern_notelength_events(self, pluginid, group, track, column, fromtime, length, desired_len, mode, off_value);
	return 0;
}

static int pattern_volumes_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 10) {
		luaL_error(L, "Invalid argument count for Pattern:volumes_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:volumes_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int note_column = lua_tonumber(L, 6);
	int vol_column = lua_tonumber(L, 7);
	int fromtime = lua_tonumber(L, 8);
	int length = lua_tonumber(L, 9);
	int mode = lua_tonumber(L, 10);
	zzub_pattern_volumes_events(self, pluginid, group, track, note_column, vol_column, fromtime, length, mode);
	return 0;
}

static int pattern_swap_track_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Pattern:swap_track_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:swap_track_events");
		return 0;
	}

	int left_idx = lua_tonumber(L, 3);
	int right_idx = lua_tonumber(L, 4);
	int fromtime = lua_tonumber(L, 5);
	int length = lua_tonumber(L, 6);
	zzub_pattern_swap_track_events(self, left_idx, right_idx, fromtime, length);
	return 0;
}

static int pattern_swap_rows_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Pattern:swap_rows_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:swap_rows_events");
		return 0;
	}

	int pluginid = lua_tonumber(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int top_row = lua_tonumber(L, 7);
	int bottom_row = lua_tonumber(L, 8);
	zzub_pattern_swap_rows_events(self, pluginid, group, track, column, top_row, bottom_row);
	return 0;
}

static int pattern_invert_chord_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 8) {
		luaL_error(L, "Invalid argument count for Pattern:invert_chord_events");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:invert_chord_events");
		return 0;
	}

	int left_idx = lua_tonumber(L, 3);
	int right_idx = lua_tonumber(L, 4);
	int fromtime = lua_tonumber(L, 5);
	int length = lua_tonumber(L, 6);
	int direction = lua_tonumber(L, 7);
	int mode = lua_tonumber(L, 8);
	zzub_pattern_invert_chord_events(self, left_idx, right_idx, fromtime, length, direction, mode);
	return 0;
}

static int pattern_move_and_transpose_notes(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Pattern:move_and_transpose_notes");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:move_and_transpose_notes");
		return 0;
	}

	const zzub_pattern_event_t** events;	int numevents = lua_tonumber(L, 4);
	int timeshift = lua_tonumber(L, 5);
	int pitchshift = lua_tonumber(L, 6);
	int mode = lua_tonumber(L, 7);
	zzub_pattern_move_and_transpose_notes(self, events, numevents, timeshift, pitchshift, mode);
	return 0;
}

static int pattern_insert_note(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Pattern:insert_note");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:insert_note");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int time = lua_tonumber(L, 4);
	int note = lua_tonumber(L, 5);
	int length = lua_tonumber(L, 6);
	zzub_pattern_insert_note(self, plugin, time, note, length);
	return 0;
}

static int pattern_update_note(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Pattern:update_note");
		return 0;
	}
	zzub_pattern_t* self = (zzub_pattern_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pattern:update_note");
		return 0;
	}

	zzub_pattern_event_t* patternevent = (zzub_pattern_event_t*)lua_touserdata(L, 3);
	int time = lua_tonumber(L, 4);
	int note = lua_tonumber(L, 5);
	int length = lua_tonumber(L, 6);
	zzub_pattern_update_note(self, patternevent, time, note, length);
	return 0;
}

static const luaL_Reg pattern_lib[] = {
	{ "destroy", pattern_destroy },
	{ "get_name", pattern_get_name },
	{ "set_name", pattern_set_name },
	{ "get_row_count", pattern_get_row_count },
	{ "set_row_count", pattern_set_row_count },
	{ "get_id", pattern_get_id },
	{ "get_format", pattern_get_format },
	{ "set_format", pattern_set_format },
	{ "get_resolution", pattern_get_resolution },
	{ "set_resolution", pattern_set_resolution },
	{ "get_display_resolution", pattern_get_display_resolution },
	{ "set_display_resolution", pattern_set_display_resolution },
	{ "get_display_beat_rows", pattern_get_display_beat_rows },
	{ "set_display_beat_rows", pattern_set_display_beat_rows },
	{ "get_loop_start", pattern_get_loop_start },
	{ "set_loop_start", pattern_set_loop_start },
	{ "get_loop_end", pattern_get_loop_end },
	{ "set_loop_end", pattern_set_loop_end },
	{ "get_loop_enabled", pattern_get_loop_enabled },
	{ "set_loop_enabled", pattern_set_loop_enabled },
	{ "get_replay_row", pattern_get_replay_row },
	{ "set_replay_row", pattern_set_replay_row },
	{ "get_currently_playing_row", pattern_get_currently_playing_row },
	{ "set_value", pattern_set_value },
	{ "get_value", pattern_get_value },
	{ "get_event_iterator", pattern_get_event_iterator },
	{ "get_event_unsorted_iterator", pattern_get_event_unsorted_iterator },
	{ "insert_value", pattern_insert_value },
	{ "delete_value", pattern_delete_value },
	{ "update_value", pattern_update_value },
	{ "update_value_full", pattern_update_value_full },
	{ "compact_pattern", pattern_compact_pattern },
	{ "expand_pattern", pattern_expand_pattern },
	{ "timeshift_events", pattern_timeshift_events },
	{ "delete_events", pattern_delete_events },
	{ "move_scale_events", pattern_move_scale_events },
	{ "paste_stream_events", pattern_paste_stream_events },
	{ "transpose_events", pattern_transpose_events },
	{ "randomize_events", pattern_randomize_events },
	{ "randomize_range_events", pattern_randomize_range_events },
	{ "randomize_from_events", pattern_randomize_from_events },
	{ "humanize_events", pattern_humanize_events },
	{ "shuffle_events", pattern_shuffle_events },
	{ "interpolate_events", pattern_interpolate_events },
	{ "gradiate_events", pattern_gradiate_events },
	{ "smooth_events", pattern_smooth_events },
	{ "reverse_events", pattern_reverse_events },
	{ "compact_events", pattern_compact_events },
	{ "expand_events", pattern_expand_events },
	{ "thin_events", pattern_thin_events },
	{ "repeat_events", pattern_repeat_events },
	{ "echo_events", pattern_echo_events },
	{ "unique_events", pattern_unique_events },
	{ "scale_events", pattern_scale_events },
	{ "fade_events", pattern_fade_events },
	{ "curvemap_events", pattern_curvemap_events },
	{ "invert_events", pattern_invert_events },
	{ "rotate_rows_events", pattern_rotate_rows_events },
	{ "rotate_vals_events", pattern_rotate_vals_events },
	{ "rotate_dist_events", pattern_rotate_dist_events },
	{ "set_events", pattern_set_events },
	{ "replace_events", pattern_replace_events },
	{ "remove_events", pattern_remove_events },
	{ "notelength_events", pattern_notelength_events },
	{ "volumes_events", pattern_volumes_events },
	{ "swap_track_events", pattern_swap_track_events },
	{ "swap_rows_events", pattern_swap_rows_events },
	{ "invert_chord_events", pattern_invert_chord_events },
	{ "move_and_transpose_notes", pattern_move_and_transpose_notes },
	{ "insert_note", pattern_insert_note },
	{ "update_note", pattern_update_note },
	{ NULL, NULL }
};

static int pattern_format_add_column(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for PatternFormat:add_column");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:add_column");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	int idx = lua_tonumber(L, 7);
	zzub_pattern_format_column_t* _luaresult = zzub_pattern_format_add_column(self, plugin, group, track, column, idx);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_format_delete_column(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for PatternFormat:delete_column");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:delete_column");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	zzub_pattern_format_delete_column(self, plugin, group, track, column);
	return 0;
}

static int pattern_format_get_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormat:get_iterator");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:get_iterator");
		return 0;
	}

	zzub_pattern_format_column_iterator_t* _luaresult = zzub_pattern_format_get_iterator(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_format_get_column(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for PatternFormat:get_column");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:get_column");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	zzub_pattern_format_column_t* _luaresult = zzub_pattern_format_get_column(self, plugin, group, track, column);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_format_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormat:get_name");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:get_name");
		return 0;
	}

	const char* _luaresult = zzub_pattern_format_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int pattern_format_set_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PatternFormat:set_name");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:set_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	zzub_pattern_format_set_name(self, name);
	return 0;
}

static int pattern_format_get_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormat:get_id");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:get_id");
		return 0;
	}

	int _luaresult = zzub_pattern_format_get_id(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_set_track_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for PatternFormat:set_track_name");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:set_track_name");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	const char* name = luaL_tolstring(L, 6, &len);
	zzub_pattern_format_set_track_name(self, plugin, group, track, name);
	return 0;
}

static int pattern_format_get_track_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for PatternFormat:get_track_name");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:get_track_name");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	const char* _luaresult = zzub_pattern_format_get_track_name(self, plugin, group, track);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int pattern_format_set_track_mute(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for PatternFormat:set_track_mute");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:set_track_mute");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int state = lua_tonumber(L, 6);
	zzub_pattern_format_set_track_mute(self, plugin, group, track, state);
	return 0;
}

static int pattern_format_get_track_mute(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for PatternFormat:get_track_mute");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:get_track_mute");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int _luaresult = zzub_pattern_format_get_track_mute(self, plugin, group, track);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_add_column_filter(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for PatternFormat:add_column_filter");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:add_column_filter");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	zzub_pattern_format_t* filterformat = (zzub_pattern_format_t*)lua_touserdata(L, 7);
	zzub_pattern_format_add_column_filter(self, plugin, group, track, column, filterformat);
	return 0;
}

static int pattern_format_remove_column_filter(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for PatternFormat:remove_column_filter");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:remove_column_filter");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	zzub_pattern_format_t* filterformat = (zzub_pattern_format_t*)lua_touserdata(L, 7);
	zzub_pattern_format_remove_column_filter(self, plugin, group, track, column, filterformat);
	return 0;
}

static int pattern_format_get_column_filters(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for PatternFormat:get_column_filters");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:get_column_filters");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	int column = lua_tonumber(L, 6);
	zzub_pattern_format_iterator_t* _luaresult = zzub_pattern_format_get_column_filters(self, plugin, group, track, column);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_format_get_scroller_width(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormat:get_scroller_width");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:get_scroller_width");
		return 0;
	}

	int _luaresult = zzub_pattern_format_get_scroller_width(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_set_scroller_width(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PatternFormat:set_scroller_width");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:set_scroller_width");
		return 0;
	}

	int width = lua_tonumber(L, 3);
	zzub_pattern_format_set_scroller_width(self, width);
	return 0;
}

static int pattern_format_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormat:destroy");
		return 0;
	}
	zzub_pattern_format_t* self = (zzub_pattern_format_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormat:destroy");
		return 0;
	}

	zzub_pattern_format_destroy(self);
	return 0;
}

static const luaL_Reg pattern_format_lib[] = {
	{ "add_column", pattern_format_add_column },
	{ "delete_column", pattern_format_delete_column },
	{ "get_iterator", pattern_format_get_iterator },
	{ "get_column", pattern_format_get_column },
	{ "get_name", pattern_format_get_name },
	{ "set_name", pattern_format_set_name },
	{ "get_id", pattern_format_get_id },
	{ "set_track_name", pattern_format_set_track_name },
	{ "get_track_name", pattern_format_get_track_name },
	{ "set_track_mute", pattern_format_set_track_mute },
	{ "get_track_mute", pattern_format_get_track_mute },
	{ "add_column_filter", pattern_format_add_column_filter },
	{ "remove_column_filter", pattern_format_remove_column_filter },
	{ "get_column_filters", pattern_format_get_column_filters },
	{ "get_scroller_width", pattern_format_get_scroller_width },
	{ "set_scroller_width", pattern_format_set_scroller_width },
	{ "destroy", pattern_format_destroy },
	{ NULL, NULL }
};

static int pattern_format_iterator_next(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatIterator:next");
		return 0;
	}
	zzub_pattern_format_iterator_t* self = (zzub_pattern_format_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatIterator:next");
		return 0;
	}

	zzub_pattern_format_iterator_next(self);
	return 0;
}

static int pattern_format_iterator_valid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatIterator:valid");
		return 0;
	}
	zzub_pattern_format_iterator_t* self = (zzub_pattern_format_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatIterator:valid");
		return 0;
	}

	int _luaresult = zzub_pattern_format_iterator_valid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_iterator_current(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatIterator:current");
		return 0;
	}
	zzub_pattern_format_iterator_t* self = (zzub_pattern_format_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatIterator:current");
		return 0;
	}

	zzub_pattern_format_t* _luaresult = zzub_pattern_format_iterator_current(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_format_iterator_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatIterator:destroy");
		return 0;
	}
	zzub_pattern_format_iterator_t* self = (zzub_pattern_format_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatIterator:destroy");
		return 0;
	}

	zzub_pattern_format_iterator_destroy(self);
	return 0;
}

static const luaL_Reg pattern_format_iterator_lib[] = {
	{ "next", pattern_format_iterator_next },
	{ "valid", pattern_format_iterator_valid },
	{ "current", pattern_format_iterator_current },
	{ "destroy", pattern_format_iterator_destroy },
	{ NULL, NULL }
};

static int pattern_format_column_get_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:get_plugin");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:get_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = zzub_pattern_format_column_get_plugin(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_format_column_get_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:get_group");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:get_group");
		return 0;
	}

	int _luaresult = zzub_pattern_format_column_get_group(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_column_get_track(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:get_track");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:get_track");
		return 0;
	}

	int _luaresult = zzub_pattern_format_column_get_track(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_column_get_column(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:get_column");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:get_column");
		return 0;
	}

	int _luaresult = zzub_pattern_format_column_get_column(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_column_get_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:get_format");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:get_format");
		return 0;
	}

	zzub_pattern_format_t* _luaresult = zzub_pattern_format_column_get_format(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_format_column_get_mode(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:get_mode");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:get_mode");
		return 0;
	}

	int _luaresult = zzub_pattern_format_column_get_mode(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_column_set_mode(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:set_mode");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:set_mode");
		return 0;
	}

	int mode = lua_tonumber(L, 3);
	zzub_pattern_format_column_set_mode(self, mode);
	return 0;
}

static int pattern_format_column_get_collapsed(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:get_collapsed");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:get_collapsed");
		return 0;
	}

	int _luaresult = zzub_pattern_format_column_get_collapsed(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_column_set_collapsed(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:set_collapsed");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:set_collapsed");
		return 0;
	}

	int is_collapsed = lua_tonumber(L, 3);
	zzub_pattern_format_column_set_collapsed(self, is_collapsed);
	return 0;
}

static int pattern_format_column_get_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:get_index");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:get_index");
		return 0;
	}

	int _luaresult = zzub_pattern_format_column_get_index(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_column_set_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PatternFormatColumn:set_index");
		return 0;
	}
	zzub_pattern_format_column_t* self = (zzub_pattern_format_column_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumn:set_index");
		return 0;
	}

	int idx = lua_tonumber(L, 3);
	zzub_pattern_format_column_set_index(self, idx);
	return 0;
}

static const luaL_Reg pattern_format_column_lib[] = {
	{ "get_plugin", pattern_format_column_get_plugin },
	{ "get_group", pattern_format_column_get_group },
	{ "get_track", pattern_format_column_get_track },
	{ "get_column", pattern_format_column_get_column },
	{ "get_format", pattern_format_column_get_format },
	{ "get_mode", pattern_format_column_get_mode },
	{ "set_mode", pattern_format_column_set_mode },
	{ "get_collapsed", pattern_format_column_get_collapsed },
	{ "set_collapsed", pattern_format_column_set_collapsed },
	{ "get_index", pattern_format_column_get_index },
	{ "set_index", pattern_format_column_set_index },
	{ NULL, NULL }
};

static int pattern_format_column_iterator_next(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumnIterator:next");
		return 0;
	}
	zzub_pattern_format_column_iterator_t* self = (zzub_pattern_format_column_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumnIterator:next");
		return 0;
	}

	zzub_pattern_format_column_iterator_next(self);
	return 0;
}

static int pattern_format_column_iterator_valid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumnIterator:valid");
		return 0;
	}
	zzub_pattern_format_column_iterator_t* self = (zzub_pattern_format_column_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumnIterator:valid");
		return 0;
	}

	int _luaresult = zzub_pattern_format_column_iterator_valid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pattern_format_column_iterator_current(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumnIterator:current");
		return 0;
	}
	zzub_pattern_format_column_iterator_t* self = (zzub_pattern_format_column_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumnIterator:current");
		return 0;
	}

	zzub_pattern_format_column_t* _luaresult = zzub_pattern_format_column_iterator_current(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pattern_format_column_iterator_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PatternFormatColumnIterator:destroy");
		return 0;
	}
	zzub_pattern_format_column_iterator_t* self = (zzub_pattern_format_column_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PatternFormatColumnIterator:destroy");
		return 0;
	}

	zzub_pattern_format_column_iterator_destroy(self);
	return 0;
}

static const luaL_Reg pattern_format_column_iterator_lib[] = {
	{ "next", pattern_format_column_iterator_next },
	{ "valid", pattern_format_column_iterator_valid },
	{ "current", pattern_format_column_iterator_current },
	{ "destroy", pattern_format_column_iterator_destroy },
	{ NULL, NULL }
};

static int parameter_get_type(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Parameter:get_type");
		return 0;
	}
	zzub_parameter_t* self = (zzub_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Parameter:get_type");
		return 0;
	}

	int _luaresult = zzub_parameter_get_type(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int parameter_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Parameter:get_name");
		return 0;
	}
	zzub_parameter_t* self = (zzub_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Parameter:get_name");
		return 0;
	}

	const char* _luaresult = zzub_parameter_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int parameter_get_description(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Parameter:get_description");
		return 0;
	}
	zzub_parameter_t* self = (zzub_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Parameter:get_description");
		return 0;
	}

	const char* _luaresult = zzub_parameter_get_description(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int parameter_get_value_min(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Parameter:get_value_min");
		return 0;
	}
	zzub_parameter_t* self = (zzub_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Parameter:get_value_min");
		return 0;
	}

	int _luaresult = zzub_parameter_get_value_min(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int parameter_get_value_max(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Parameter:get_value_max");
		return 0;
	}
	zzub_parameter_t* self = (zzub_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Parameter:get_value_max");
		return 0;
	}

	int _luaresult = zzub_parameter_get_value_max(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int parameter_get_value_none(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Parameter:get_value_none");
		return 0;
	}
	zzub_parameter_t* self = (zzub_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Parameter:get_value_none");
		return 0;
	}

	int _luaresult = zzub_parameter_get_value_none(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int parameter_get_value_default(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Parameter:get_value_default");
		return 0;
	}
	zzub_parameter_t* self = (zzub_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Parameter:get_value_default");
		return 0;
	}

	int _luaresult = zzub_parameter_get_value_default(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int parameter_get_flags(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Parameter:get_flags");
		return 0;
	}
	zzub_parameter_t* self = (zzub_parameter_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Parameter:get_flags");
		return 0;
	}

	int _luaresult = zzub_parameter_get_flags(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg parameter_lib[] = {
	{ "get_type", parameter_get_type },
	{ "get_name", parameter_get_name },
	{ "get_description", parameter_get_description },
	{ "get_value_min", parameter_get_value_min },
	{ "get_value_max", parameter_get_value_max },
	{ "get_value_none", parameter_get_value_none },
	{ "get_value_default", parameter_get_value_default },
	{ "get_flags", parameter_get_flags },
	{ NULL, NULL }
};

static int attribute_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Attribute:get_name");
		return 0;
	}
	zzub_attribute_t* self = (zzub_attribute_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Attribute:get_name");
		return 0;
	}

	const char* _luaresult = zzub_attribute_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int attribute_get_value_min(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Attribute:get_value_min");
		return 0;
	}
	zzub_attribute_t* self = (zzub_attribute_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Attribute:get_value_min");
		return 0;
	}

	int _luaresult = zzub_attribute_get_value_min(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int attribute_get_value_max(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Attribute:get_value_max");
		return 0;
	}
	zzub_attribute_t* self = (zzub_attribute_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Attribute:get_value_max");
		return 0;
	}

	int _luaresult = zzub_attribute_get_value_max(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int attribute_get_value_default(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Attribute:get_value_default");
		return 0;
	}
	zzub_attribute_t* self = (zzub_attribute_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Attribute:get_value_default");
		return 0;
	}

	int _luaresult = zzub_attribute_get_value_default(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg attribute_lib[] = {
	{ "get_name", attribute_get_name },
	{ "get_value_min", attribute_get_value_min },
	{ "get_value_max", attribute_get_value_max },
	{ "get_value_default", attribute_get_value_default },
	{ NULL, NULL }
};

static int pluginloader_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_name");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_name");
		return 0;
	}

	const char* _luaresult = zzub_pluginloader_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int pluginloader_get_short_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_short_name");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_short_name");
		return 0;
	}

	const char* _luaresult = zzub_pluginloader_get_short_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int pluginloader_get_parameter_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_parameter_count");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_parameter_count");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int _luaresult = zzub_pluginloader_get_parameter_count(self, group);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pluginloader_get_parameter(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_parameter");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_parameter");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int index = lua_tonumber(L, 4);
	zzub_parameter_t* _luaresult = zzub_pluginloader_get_parameter(self, group, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pluginloader_get_attribute_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_attribute_count");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_attribute_count");
		return 0;
	}

	int _luaresult = zzub_pluginloader_get_attribute_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pluginloader_get_attribute(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_attribute");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_attribute");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_attribute_t* _luaresult = zzub_pluginloader_get_attribute(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int pluginloader_get_flags(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_flags");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_flags");
		return 0;
	}

	int _luaresult = zzub_pluginloader_get_flags(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pluginloader_get_uri(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_uri");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_uri");
		return 0;
	}

	const char* _luaresult = zzub_pluginloader_get_uri(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int pluginloader_get_author(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_author");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_author");
		return 0;
	}

	const char* _luaresult = zzub_pluginloader_get_author(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int pluginloader_get_instrument_list(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_instrument_list");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_instrument_list");
		return 0;
	}

	char* result;
	int maxbytes = lua_tonumber(L, 4);
	int _luaresult = zzub_pluginloader_get_instrument_list(self, result, maxbytes);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pluginloader_get_tracks_min(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_tracks_min");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_tracks_min");
		return 0;
	}

	int _luaresult = zzub_pluginloader_get_tracks_min(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pluginloader_get_tracks_max(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_tracks_max");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_tracks_max");
		return 0;
	}

	int _luaresult = zzub_pluginloader_get_tracks_max(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pluginloader_get_stream_format_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_stream_format_count");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_stream_format_count");
		return 0;
	}

	int _luaresult = zzub_pluginloader_get_stream_format_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pluginloader_get_stream_format_ext(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_stream_format_ext");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_stream_format_ext");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	const char* _luaresult = zzub_pluginloader_get_stream_format_ext(self, index);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int pluginloader_get_output_channel_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_output_channel_count");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_output_channel_count");
		return 0;
	}

	int _luaresult = zzub_pluginloader_get_output_channel_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pluginloader_get_input_channel_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_input_channel_count");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_input_channel_count");
		return 0;
	}

	int _luaresult = zzub_pluginloader_get_input_channel_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int pluginloader_get_plugin_file(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_plugin_file");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_plugin_file");
		return 0;
	}

	const char* _luaresult = zzub_pluginloader_get_plugin_file(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int pluginloader_get_plugincollection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Pluginloader:get_plugincollection");
		return 0;
	}
	zzub_pluginloader_t* self = (zzub_pluginloader_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Pluginloader:get_plugincollection");
		return 0;
	}

	zzub_plugincollection_t* _luaresult = zzub_pluginloader_get_plugincollection(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static const luaL_Reg pluginloader_lib[] = {
	{ "get_name", pluginloader_get_name },
	{ "get_short_name", pluginloader_get_short_name },
	{ "get_parameter_count", pluginloader_get_parameter_count },
	{ "get_parameter", pluginloader_get_parameter },
	{ "get_attribute_count", pluginloader_get_attribute_count },
	{ "get_attribute", pluginloader_get_attribute },
	{ "get_flags", pluginloader_get_flags },
	{ "get_uri", pluginloader_get_uri },
	{ "get_author", pluginloader_get_author },
	{ "get_instrument_list", pluginloader_get_instrument_list },
	{ "get_tracks_min", pluginloader_get_tracks_min },
	{ "get_tracks_max", pluginloader_get_tracks_max },
	{ "get_stream_format_count", pluginloader_get_stream_format_count },
	{ "get_stream_format_ext", pluginloader_get_stream_format_ext },
	{ "get_output_channel_count", pluginloader_get_output_channel_count },
	{ "get_input_channel_count", pluginloader_get_input_channel_count },
	{ "get_plugin_file", pluginloader_get_plugin_file },
	{ "get_plugincollection", pluginloader_get_plugincollection },
	{ NULL, NULL }
};

static int plugin_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:destroy");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:destroy");
		return 0;
	}

	int _luaresult = zzub_plugin_destroy(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_load(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:load");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:load");
		return 0;
	}

	zzub_input_t* input = (zzub_input_t*)lua_touserdata(L, 3);
	int _luaresult = zzub_plugin_load(self, input);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_save(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:save");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:save");
		return 0;
	}

	zzub_output_t* ouput = (zzub_output_t*)lua_touserdata(L, 3);
	int _luaresult = zzub_plugin_save(self, ouput);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_set_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:set_name");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	int _luaresult = zzub_plugin_set_name(self, name);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_name");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_name");
		return 0;
	}

	const char* _luaresult = zzub_plugin_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_get_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_id");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_id");
		return 0;
	}

	int _luaresult = zzub_plugin_get_id(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_position_x(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_position_x");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_position_x");
		return 0;
	}

	float _luaresult = zzub_plugin_get_position_x(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_position_y(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_position_y");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_position_y");
		return 0;
	}

	float _luaresult = zzub_plugin_get_position_y(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_set_position(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Plugin:set_position");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_position");
		return 0;
	}

	float x = lua_tonumber(L, 3);
	float y = lua_tonumber(L, 4);
	zzub_plugin_set_position(self, x, y);
	return 0;
}

static int plugin_set_position_direct(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Plugin:set_position_direct");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_position_direct");
		return 0;
	}

	float x = lua_tonumber(L, 3);
	float y = lua_tonumber(L, 4);
	zzub_plugin_set_position_direct(self, x, y);
	return 0;
}

static int plugin_get_flags(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_flags");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_flags");
		return 0;
	}

	int _luaresult = zzub_plugin_get_flags(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_track_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_track_count");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_track_count");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int _luaresult = zzub_plugin_get_track_count(self, group);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_set_track_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:set_track_count");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_track_count");
		return 0;
	}

	int count = lua_tonumber(L, 3);
	zzub_plugin_set_track_count(self, count);
	return 0;
}

static int plugin_get_mute(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_mute");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_mute");
		return 0;
	}

	int _luaresult = zzub_plugin_get_mute(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_set_mute(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:set_mute");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_mute");
		return 0;
	}

	int muted = lua_tonumber(L, 3);
	zzub_plugin_set_mute(self, muted);
	return 0;
}

static int plugin_get_bypass(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_bypass");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_bypass");
		return 0;
	}

	int _luaresult = zzub_plugin_get_bypass(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_set_bypass(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:set_bypass");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_bypass");
		return 0;
	}

	int muted = lua_tonumber(L, 3);
	zzub_plugin_set_bypass(self, muted);
	return 0;
}

static int plugin_get_minimize(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_minimize");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_minimize");
		return 0;
	}

	int _luaresult = zzub_plugin_get_minimize(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_set_minimize(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:set_minimize");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_minimize");
		return 0;
	}

	int minimized = lua_tonumber(L, 3);
	zzub_plugin_set_minimize(self, minimized);
	return 0;
}

static int plugin_configure(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Plugin:configure");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:configure");
		return 0;
	}

	const char* key = luaL_tolstring(L, 3, &len);
	const char* value = luaL_tolstring(L, 4, &len);
	zzub_plugin_configure(self, key, value);
	return 0;
}

static int plugin_command(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:command");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:command");
		return 0;
	}

	int i = lua_tonumber(L, 3);
	zzub_plugin_command(self, i);
	return 0;
}

static int plugin_get_pluginloader(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_pluginloader");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_pluginloader");
		return 0;
	}

	zzub_pluginloader_t* _luaresult = zzub_plugin_get_pluginloader(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_get_midi_output_device_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_midi_output_device_count");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_midi_output_device_count");
		return 0;
	}

	int _luaresult = zzub_plugin_get_midi_output_device_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_midi_output_device(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_midi_output_device");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_midi_output_device");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	const char* _luaresult = zzub_plugin_get_midi_output_device(self, index);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_get_envelope_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_envelope_count");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_envelope_count");
		return 0;
	}

	int _luaresult = zzub_plugin_get_envelope_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_envelope_flags(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_envelope_flags");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_envelope_flags");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_plugin_get_envelope_flags(self, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_envelope_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_envelope_name");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_envelope_name");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	const char* _luaresult = zzub_plugin_get_envelope_name(self, index);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_set_stream_source(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:set_stream_source");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_stream_source");
		return 0;
	}

	const char* resource = luaL_tolstring(L, 3, &len);
	zzub_plugin_set_stream_source(self, resource);
	return 0;
}

static int plugin_get_stream_source(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_stream_source");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_stream_source");
		return 0;
	}

	const char* _luaresult = zzub_plugin_get_stream_source(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_set_instrument(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:set_instrument");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_instrument");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	int _luaresult = zzub_plugin_set_instrument(self, name);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_describe_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Plugin:describe_value");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:describe_value");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int column = lua_tonumber(L, 4);
	int value = lua_tonumber(L, 5);
	const char* _luaresult = zzub_plugin_describe_value(self, group, column, value);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_get_parameter_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Plugin:get_parameter_value");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_parameter_value");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int track = lua_tonumber(L, 4);
	int column = lua_tonumber(L, 5);
	int _luaresult = zzub_plugin_get_parameter_value(self, group, track, column);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_set_parameter_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Plugin:set_parameter_value");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_parameter_value");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int track = lua_tonumber(L, 4);
	int column = lua_tonumber(L, 5);
	int value = lua_tonumber(L, 6);
	int record = lua_tonumber(L, 7);
	zzub_plugin_set_parameter_value(self, group, track, column, value, record);
	return 0;
}

static int plugin_set_parameter_value_direct(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Plugin:set_parameter_value_direct");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_parameter_value_direct");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int track = lua_tonumber(L, 4);
	int column = lua_tonumber(L, 5);
	int value = lua_tonumber(L, 6);
	int record = lua_tonumber(L, 7);
	zzub_plugin_set_parameter_value_direct(self, group, track, column, value, record);
	return 0;
}

static int plugin_get_parameter_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Plugin:get_parameter_count");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_parameter_count");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int track = lua_tonumber(L, 4);
	int _luaresult = zzub_plugin_get_parameter_count(self, group, track);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_parameter(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Plugin:get_parameter");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_parameter");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int track = lua_tonumber(L, 4);
	int column = lua_tonumber(L, 5);
	zzub_parameter_t* _luaresult = zzub_plugin_get_parameter(self, group, track, column);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_get_parameter_interpolator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Plugin:get_parameter_interpolator");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_parameter_interpolator");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int track = lua_tonumber(L, 4);
	int column = lua_tonumber(L, 5);
	int _luaresult = zzub_plugin_get_parameter_interpolator(self, group, track, column);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_set_parameter_interpolator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Plugin:set_parameter_interpolator");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_parameter_interpolator");
		return 0;
	}

	int group = lua_tonumber(L, 3);
	int track = lua_tonumber(L, 4);
	int column = lua_tonumber(L, 5);
	int mode = lua_tonumber(L, 6);
	zzub_plugin_set_parameter_interpolator(self, group, track, column, mode);
	return 0;
}

static int plugin_get_input_connection_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_input_connection_count");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_input_connection_count");
		return 0;
	}

	int _luaresult = zzub_plugin_get_input_connection_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_input_connection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_input_connection");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_input_connection");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_connection_t* _luaresult = zzub_plugin_get_input_connection(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_get_input_connection_by_type(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Plugin:get_input_connection_by_type");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_input_connection_by_type");
		return 0;
	}

	zzub_plugin_t* from_plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int type = lua_tonumber(L, 4);
	zzub_connection_t* _luaresult = zzub_plugin_get_input_connection_by_type(self, from_plugin, type);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_get_output_connection_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_output_connection_count");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_output_connection_count");
		return 0;
	}

	int _luaresult = zzub_plugin_get_output_connection_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_output_connection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_output_connection");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_output_connection");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_connection_t* _luaresult = zzub_plugin_get_output_connection(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_get_output_connection_by_type(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Plugin:get_output_connection_by_type");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_output_connection_by_type");
		return 0;
	}

	zzub_plugin_t* from_plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int type = lua_tonumber(L, 4);
	zzub_connection_t* _luaresult = zzub_plugin_get_output_connection_by_type(self, from_plugin, type);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_create_audio_connection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Plugin:create_audio_connection");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:create_audio_connection");
		return 0;
	}

	zzub_plugin_t* from_plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	int first_input = lua_tonumber(L, 4);
	int input_count = lua_tonumber(L, 5);
	int first_output = lua_tonumber(L, 6);
	int output_count = lua_tonumber(L, 7);
	zzub_connection_t* _luaresult = zzub_plugin_create_audio_connection(self, from_plugin, first_input, input_count, first_output, output_count);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_create_midi_connection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Plugin:create_midi_connection");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:create_midi_connection");
		return 0;
	}

	zzub_plugin_t* from_plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	const char* midi_device = luaL_tolstring(L, 4, &len);
	zzub_connection_t* _luaresult = zzub_plugin_create_midi_connection(self, from_plugin, midi_device);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_create_event_connection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:create_event_connection");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:create_event_connection");
		return 0;
	}

	zzub_plugin_t* from_plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	zzub_connection_t* _luaresult = zzub_plugin_create_event_connection(self, from_plugin);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_create_note_connection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:create_note_connection");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:create_note_connection");
		return 0;
	}

	zzub_plugin_t* from_plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	zzub_connection_t* _luaresult = zzub_plugin_create_note_connection(self, from_plugin);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_get_last_peak(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_last_peak");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_last_peak");
		return 0;
	}

	int channel = lua_tonumber(L, 3);
	float _luaresult = zzub_plugin_get_last_peak(self, channel);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_last_cpu_load(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_last_cpu_load");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_last_cpu_load");
		return 0;
	}

	double _luaresult = zzub_plugin_get_last_cpu_load(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_last_midi_result(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_last_midi_result");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_last_midi_result");
		return 0;
	}

	int _luaresult = zzub_plugin_get_last_midi_result(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_last_audio_result(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_last_audio_result");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_last_audio_result");
		return 0;
	}

	int _luaresult = zzub_plugin_get_last_audio_result(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_tick(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:tick");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:tick");
		return 0;
	}

	int immediate = lua_toboolean(L, 3);
	zzub_plugin_tick(self, immediate);
	return 0;
}

static int plugin_get_attribute_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_attribute_value");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_attribute_value");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_plugin_get_attribute_value(self, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_set_attribute_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Plugin:set_attribute_value");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_attribute_value");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int value = lua_tonumber(L, 4);
	zzub_plugin_set_attribute_value(self, index, value);
	return 0;
}

static int plugin_play_midi_note(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Plugin:play_midi_note");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:play_midi_note");
		return 0;
	}

	int note = lua_tonumber(L, 3);
	int prevNote = lua_tonumber(L, 4);
	int velocity = lua_tonumber(L, 5);
	zzub_plugin_play_midi_note(self, note, prevNote, velocity);
	return 0;
}

static int plugin_set_timesource(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Plugin:set_timesource");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_timesource");
		return 0;
	}

	zzub_plugin_t* timesource = (zzub_plugin_t*)lua_touserdata(L, 3);
	int group = lua_tonumber(L, 4);
	int track = lua_tonumber(L, 5);
	zzub_plugin_set_timesource(self, timesource, group, track);
	return 0;
}

static int plugin_get_timesource_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_timesource_plugin");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_timesource_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = zzub_plugin_get_timesource_plugin(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_get_timesource_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_timesource_group");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_timesource_group");
		return 0;
	}

	int _luaresult = zzub_plugin_get_timesource_group(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_timesource_track(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_timesource_track");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_timesource_track");
		return 0;
	}

	int _luaresult = zzub_plugin_get_timesource_track(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_output_channel_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_output_channel_count");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_output_channel_count");
		return 0;
	}

	int _luaresult = zzub_plugin_get_output_channel_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_input_channel_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_input_channel_count");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_input_channel_count");
		return 0;
	}

	int _luaresult = zzub_plugin_get_input_channel_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_output_channel_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_output_channel_name");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_output_channel_name");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	const char* _luaresult = zzub_plugin_get_output_channel_name(self, index);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_get_input_channel_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:get_input_channel_name");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_input_channel_name");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	const char* _luaresult = zzub_plugin_get_input_channel_name(self, index);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_get_encoder_digest(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Plugin:get_encoder_digest");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_encoder_digest");
		return 0;
	}

	int type = lua_tonumber(L, 3);
	float** buffers;
	int numsamples = lua_tonumber(L, 5);
	int _luaresult = zzub_plugin_get_encoder_digest(self, type, buffers, numsamples);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_connection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_connection");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_connection");
		return 0;
	}

	zzub_connection_t* _luaresult = zzub_plugin_get_connection(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_has_embedded_gui(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:has_embedded_gui");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:has_embedded_gui");
		return 0;
	}

	int _luaresult = zzub_plugin_has_embedded_gui(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_create_embedded_gui(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:create_embedded_gui");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:create_embedded_gui");
		return 0;
	}

	void* hwnd = lua_touserdata(L, 3);
	int _luaresult = zzub_plugin_create_embedded_gui(self, hwnd);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_resize_embedded_gui(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Plugin:resize_embedded_gui");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:resize_embedded_gui");
		return 0;
	}

	void* hwnd = lua_touserdata(L, 3);
	int width;
	int height;
	zzub_plugin_resize_embedded_gui(self, hwnd, &width, &height);
	return 0;
}

static int plugin_set_latency(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:set_latency");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_latency");
		return 0;
	}

	int samplecount = lua_tonumber(L, 3);
	zzub_plugin_set_latency(self, samplecount);
	return 0;
}

static int plugin_get_latency(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_latency");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_latency");
		return 0;
	}

	int _luaresult = zzub_plugin_get_latency(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_latency_actual(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_latency_actual");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_latency_actual");
		return 0;
	}

	int _luaresult = zzub_plugin_get_latency_actual(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_get_plugin_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Plugin:get_plugin_group");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:get_plugin_group");
		return 0;
	}

	zzub_plugin_group_t* _luaresult = zzub_plugin_get_plugin_group(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_set_plugin_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Plugin:set_plugin_group");
		return 0;
	}
	zzub_plugin_t* self = (zzub_plugin_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Plugin:set_plugin_group");
		return 0;
	}

	zzub_plugin_group_t* group = (zzub_plugin_group_t*)lua_touserdata(L, 3);
	zzub_plugin_set_plugin_group(self, group);
	return 0;
}

static const luaL_Reg plugin_lib[] = {
	{ "destroy", plugin_destroy },
	{ "load", plugin_load },
	{ "save", plugin_save },
	{ "set_name", plugin_set_name },
	{ "get_name", plugin_get_name },
	{ "get_id", plugin_get_id },
	{ "get_position_x", plugin_get_position_x },
	{ "get_position_y", plugin_get_position_y },
	{ "set_position", plugin_set_position },
	{ "set_position_direct", plugin_set_position_direct },
	{ "get_flags", plugin_get_flags },
	{ "get_track_count", plugin_get_track_count },
	{ "set_track_count", plugin_set_track_count },
	{ "get_mute", plugin_get_mute },
	{ "set_mute", plugin_set_mute },
	{ "get_bypass", plugin_get_bypass },
	{ "set_bypass", plugin_set_bypass },
	{ "get_minimize", plugin_get_minimize },
	{ "set_minimize", plugin_set_minimize },
	{ "configure", plugin_configure },
	{ "command", plugin_command },
	{ "get_pluginloader", plugin_get_pluginloader },
	{ "get_midi_output_device_count", plugin_get_midi_output_device_count },
	{ "get_midi_output_device", plugin_get_midi_output_device },
	{ "get_envelope_count", plugin_get_envelope_count },
	{ "get_envelope_flags", plugin_get_envelope_flags },
	{ "get_envelope_name", plugin_get_envelope_name },
	{ "set_stream_source", plugin_set_stream_source },
	{ "get_stream_source", plugin_get_stream_source },
	{ "set_instrument", plugin_set_instrument },
	{ "describe_value", plugin_describe_value },
	{ "get_parameter_value", plugin_get_parameter_value },
	{ "set_parameter_value", plugin_set_parameter_value },
	{ "set_parameter_value_direct", plugin_set_parameter_value_direct },
	{ "get_parameter_count", plugin_get_parameter_count },
	{ "get_parameter", plugin_get_parameter },
	{ "get_parameter_interpolator", plugin_get_parameter_interpolator },
	{ "set_parameter_interpolator", plugin_set_parameter_interpolator },
	{ "get_input_connection_count", plugin_get_input_connection_count },
	{ "get_input_connection", plugin_get_input_connection },
	{ "get_input_connection_by_type", plugin_get_input_connection_by_type },
	{ "get_output_connection_count", plugin_get_output_connection_count },
	{ "get_output_connection", plugin_get_output_connection },
	{ "get_output_connection_by_type", plugin_get_output_connection_by_type },
	{ "create_audio_connection", plugin_create_audio_connection },
	{ "create_midi_connection", plugin_create_midi_connection },
	{ "create_event_connection", plugin_create_event_connection },
	{ "create_note_connection", plugin_create_note_connection },
	{ "get_last_peak", plugin_get_last_peak },
	{ "get_last_cpu_load", plugin_get_last_cpu_load },
	{ "get_last_midi_result", plugin_get_last_midi_result },
	{ "get_last_audio_result", plugin_get_last_audio_result },
	{ "tick", plugin_tick },
	{ "get_attribute_value", plugin_get_attribute_value },
	{ "set_attribute_value", plugin_set_attribute_value },
	{ "play_midi_note", plugin_play_midi_note },
	{ "set_timesource", plugin_set_timesource },
	{ "get_timesource_plugin", plugin_get_timesource_plugin },
	{ "get_timesource_group", plugin_get_timesource_group },
	{ "get_timesource_track", plugin_get_timesource_track },
	{ "get_output_channel_count", plugin_get_output_channel_count },
	{ "get_input_channel_count", plugin_get_input_channel_count },
	{ "get_output_channel_name", plugin_get_output_channel_name },
	{ "get_input_channel_name", plugin_get_input_channel_name },
	{ "get_encoder_digest", plugin_get_encoder_digest },
	{ "get_connection", plugin_get_connection },
	{ "has_embedded_gui", plugin_has_embedded_gui },
	{ "create_embedded_gui", plugin_create_embedded_gui },
	{ "resize_embedded_gui", plugin_resize_embedded_gui },
	{ "set_latency", plugin_set_latency },
	{ "get_latency", plugin_get_latency },
	{ "get_latency_actual", plugin_get_latency_actual },
	{ "get_plugin_group", plugin_get_plugin_group },
	{ "set_plugin_group", plugin_set_plugin_group },
	{ NULL, NULL }
};

static int plugin_iterator_next(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIterator:next");
		return 0;
	}
	zzub_plugin_iterator_t* self = (zzub_plugin_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIterator:next");
		return 0;
	}

	zzub_plugin_iterator_next(self);
	return 0;
}

static int plugin_iterator_valid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIterator:valid");
		return 0;
	}
	zzub_plugin_iterator_t* self = (zzub_plugin_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIterator:valid");
		return 0;
	}

	int _luaresult = zzub_plugin_iterator_valid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_iterator_current(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIterator:current");
		return 0;
	}
	zzub_plugin_iterator_t* self = (zzub_plugin_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIterator:current");
		return 0;
	}

	zzub_plugin_t* _luaresult = zzub_plugin_iterator_current(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_iterator_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginIterator:destroy");
		return 0;
	}
	zzub_plugin_iterator_t* self = (zzub_plugin_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginIterator:destroy");
		return 0;
	}

	zzub_plugin_iterator_destroy(self);
	return 0;
}

static const luaL_Reg plugin_iterator_lib[] = {
	{ "next", plugin_iterator_next },
	{ "valid", plugin_iterator_valid },
	{ "current", plugin_iterator_current },
	{ "destroy", plugin_iterator_destroy },
	{ NULL, NULL }
};

static int connection_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:destroy");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:destroy");
		return 0;
	}

	zzub_connection_destroy(self);
	return 0;
}

static int connection_get_type(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_type");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_type");
		return 0;
	}

	int _luaresult = zzub_connection_get_type(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_get_from_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_from_plugin");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_from_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = zzub_connection_get_from_plugin(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int connection_get_to_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_to_plugin");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_to_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = zzub_connection_get_to_plugin(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int connection_get_connection_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_connection_plugin");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_connection_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = zzub_connection_get_connection_plugin(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int connection_get_first_input(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_first_input");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_first_input");
		return 0;
	}

	int _luaresult = zzub_connection_get_first_input(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_set_first_input(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Connection:set_first_input");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:set_first_input");
		return 0;
	}

	int value = lua_tonumber(L, 3);
	zzub_connection_set_first_input(self, value);
	return 0;
}

static int connection_get_input_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_input_count");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_input_count");
		return 0;
	}

	int _luaresult = zzub_connection_get_input_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_set_input_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Connection:set_input_count");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:set_input_count");
		return 0;
	}

	int value = lua_tonumber(L, 3);
	zzub_connection_set_input_count(self, value);
	return 0;
}

static int connection_get_first_output(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_first_output");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_first_output");
		return 0;
	}

	int _luaresult = zzub_connection_get_first_output(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_set_first_output(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Connection:set_first_output");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:set_first_output");
		return 0;
	}

	int value = lua_tonumber(L, 3);
	zzub_connection_set_first_output(self, value);
	return 0;
}

static int connection_get_output_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_output_count");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_output_count");
		return 0;
	}

	int _luaresult = zzub_connection_get_output_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_set_output_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Connection:set_output_count");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:set_output_count");
		return 0;
	}

	int value = lua_tonumber(L, 3);
	zzub_connection_set_output_count(self, value);
	return 0;
}

static int connection_set_midi_device(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Connection:set_midi_device");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:set_midi_device");
		return 0;
	}

	const char* midi_device = luaL_tolstring(L, 3, &len);
	zzub_connection_set_midi_device(self, midi_device);
	return 0;
}

static int connection_get_midi_device(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_midi_device");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_midi_device");
		return 0;
	}

	const char* _luaresult = zzub_connection_get_midi_device(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int connection_get_event_binding_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_event_binding_count");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_event_binding_count");
		return 0;
	}

	int _luaresult = zzub_connection_get_event_binding_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_add_event_connection_binding(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Connection:add_event_connection_binding");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:add_event_connection_binding");
		return 0;
	}

	int sourceparam = lua_tonumber(L, 3);
	int targetgroup = lua_tonumber(L, 4);
	int targettrack = lua_tonumber(L, 5);
	int targetparam = lua_tonumber(L, 6);
	zzub_connection_add_event_connection_binding(self, sourceparam, targetgroup, targettrack, targetparam);
	return 0;
}

static int connection_remove_event_connection_binding(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Connection:remove_event_connection_binding");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:remove_event_connection_binding");
		return 0;
	}

	int sourceparam = lua_tonumber(L, 3);
	int targetgroup = lua_tonumber(L, 4);
	int targettrack = lua_tonumber(L, 5);
	int targetparam = lua_tonumber(L, 6);
	zzub_connection_remove_event_connection_binding(self, sourceparam, targetgroup, targettrack, targetparam);
	return 0;
}

static int connection_get_event_binding_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Connection:get_event_binding_iterator");
		return 0;
	}
	zzub_connection_t* self = (zzub_connection_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Connection:get_event_binding_iterator");
		return 0;
	}

	zzub_connection_binding_iterator_t* _luaresult = zzub_connection_get_event_binding_iterator(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static const luaL_Reg connection_lib[] = {
	{ "destroy", connection_destroy },
	{ "get_type", connection_get_type },
	{ "get_from_plugin", connection_get_from_plugin },
	{ "get_to_plugin", connection_get_to_plugin },
	{ "get_connection_plugin", connection_get_connection_plugin },
	{ "get_first_input", connection_get_first_input },
	{ "set_first_input", connection_set_first_input },
	{ "get_input_count", connection_get_input_count },
	{ "set_input_count", connection_set_input_count },
	{ "get_first_output", connection_get_first_output },
	{ "set_first_output", connection_set_first_output },
	{ "get_output_count", connection_get_output_count },
	{ "set_output_count", connection_set_output_count },
	{ "set_midi_device", connection_set_midi_device },
	{ "get_midi_device", connection_get_midi_device },
	{ "get_event_binding_count", connection_get_event_binding_count },
	{ "add_event_connection_binding", connection_add_event_connection_binding },
	{ "remove_event_connection_binding", connection_remove_event_connection_binding },
	{ "get_event_binding_iterator", connection_get_event_binding_iterator },
	{ NULL, NULL }
};

static int connection_binding_get_connection(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBinding:get_connection");
		return 0;
	}
	zzub_connection_binding_t* self = (zzub_connection_binding_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBinding:get_connection");
		return 0;
	}

	zzub_connection_t* _luaresult = zzub_connection_binding_get_connection(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int connection_binding_get_source_column(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBinding:get_source_column");
		return 0;
	}
	zzub_connection_binding_t* self = (zzub_connection_binding_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBinding:get_source_column");
		return 0;
	}

	int _luaresult = zzub_connection_binding_get_source_column(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_binding_get_target_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBinding:get_target_group");
		return 0;
	}
	zzub_connection_binding_t* self = (zzub_connection_binding_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBinding:get_target_group");
		return 0;
	}

	int _luaresult = zzub_connection_binding_get_target_group(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_binding_get_target_track(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBinding:get_target_track");
		return 0;
	}
	zzub_connection_binding_t* self = (zzub_connection_binding_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBinding:get_target_track");
		return 0;
	}

	int _luaresult = zzub_connection_binding_get_target_track(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_binding_get_target_column(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBinding:get_target_column");
		return 0;
	}
	zzub_connection_binding_t* self = (zzub_connection_binding_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBinding:get_target_column");
		return 0;
	}

	int _luaresult = zzub_connection_binding_get_target_column(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg connection_binding_lib[] = {
	{ "get_connection", connection_binding_get_connection },
	{ "get_source_column", connection_binding_get_source_column },
	{ "get_target_group", connection_binding_get_target_group },
	{ "get_target_track", connection_binding_get_target_track },
	{ "get_target_column", connection_binding_get_target_column },
	{ NULL, NULL }
};

static int connection_binding_iterator_next(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBindingIterator:next");
		return 0;
	}
	zzub_connection_binding_iterator_t* self = (zzub_connection_binding_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBindingIterator:next");
		return 0;
	}

	zzub_connection_binding_iterator_next(self);
	return 0;
}

static int connection_binding_iterator_valid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBindingIterator:valid");
		return 0;
	}
	zzub_connection_binding_iterator_t* self = (zzub_connection_binding_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBindingIterator:valid");
		return 0;
	}

	int _luaresult = zzub_connection_binding_iterator_valid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int connection_binding_iterator_current(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBindingIterator:current");
		return 0;
	}
	zzub_connection_binding_iterator_t* self = (zzub_connection_binding_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBindingIterator:current");
		return 0;
	}

	zzub_connection_binding_t* _luaresult = zzub_connection_binding_iterator_current(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int connection_binding_iterator_reset(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBindingIterator:reset");
		return 0;
	}
	zzub_connection_binding_iterator_t* self = (zzub_connection_binding_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBindingIterator:reset");
		return 0;
	}

	zzub_connection_binding_iterator_reset(self);
	return 0;
}

static int connection_binding_iterator_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ConnectionBindingIterator:destroy");
		return 0;
	}
	zzub_connection_binding_iterator_t* self = (zzub_connection_binding_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ConnectionBindingIterator:destroy");
		return 0;
	}

	zzub_connection_binding_iterator_destroy(self);
	return 0;
}

static const luaL_Reg connection_binding_iterator_lib[] = {
	{ "next", connection_binding_iterator_next },
	{ "valid", connection_binding_iterator_valid },
	{ "current", connection_binding_iterator_current },
	{ "reset", connection_binding_iterator_reset },
	{ "destroy", connection_binding_iterator_destroy },
	{ NULL, NULL }
};

static int wave_get_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:get_id");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_id");
		return 0;
	}

	int _luaresult = zzub_wave_get_id(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_get_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:get_index");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_index");
		return 0;
	}

	int _luaresult = zzub_wave_get_index(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_clear(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:clear");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:clear");
		return 0;
	}

	int _luaresult = zzub_wave_clear(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:get_name");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_name");
		return 0;
	}

	const char* _luaresult = zzub_wave_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int wave_set_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wave:set_name");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:set_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	zzub_wave_set_name(self, name);
	return 0;
}

static int wave_get_path(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:get_path");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_path");
		return 0;
	}

	const char* _luaresult = zzub_wave_get_path(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int wave_set_path(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wave:set_path");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:set_path");
		return 0;
	}

	const char* path = luaL_tolstring(L, 3, &len);
	zzub_wave_set_path(self, path);
	return 0;
}

static int wave_get_flags(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:get_flags");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_flags");
		return 0;
	}

	int _luaresult = zzub_wave_get_flags(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_set_flags(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wave:set_flags");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:set_flags");
		return 0;
	}

	int flags = lua_tonumber(L, 3);
	zzub_wave_set_flags(self, flags);
	return 0;
}

static int wave_get_volume(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:get_volume");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_volume");
		return 0;
	}

	float _luaresult = zzub_wave_get_volume(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_set_volume(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wave:set_volume");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:set_volume");
		return 0;
	}

	float volume = lua_tonumber(L, 3);
	zzub_wave_set_volume(self, volume);
	return 0;
}

static int wave_get_envelope_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:get_envelope_count");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_envelope_count");
		return 0;
	}

	int _luaresult = zzub_wave_get_envelope_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_set_envelope_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wave:set_envelope_count");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:set_envelope_count");
		return 0;
	}

	int count = lua_tonumber(L, 3);
	zzub_wave_set_envelope_count(self, count);
	return 0;
}

static int wave_get_envelope(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wave:get_envelope");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_envelope");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_envelope_t* _luaresult = zzub_wave_get_envelope(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int wave_set_envelope(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Wave:set_envelope");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:set_envelope");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_envelope_t* env = (zzub_envelope_t*)lua_touserdata(L, 4);
	zzub_wave_set_envelope(self, index, env);
	return 0;
}

static int wave_get_level_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:get_level_count");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_level_count");
		return 0;
	}

	int _luaresult = zzub_wave_get_level_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_get_level(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wave:get_level");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:get_level");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_wavelevel_t* _luaresult = zzub_wave_get_level(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int wave_add_level(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wave:add_level");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:add_level");
		return 0;
	}

	zzub_wavelevel_t* _luaresult = zzub_wave_add_level(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int wave_remove_level(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wave:remove_level");
		return 0;
	}
	zzub_wave_t* self = (zzub_wave_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wave:remove_level");
		return 0;
	}

	int level = lua_tonumber(L, 3);
	zzub_wave_remove_level(self, level);
	return 0;
}

static const luaL_Reg wave_lib[] = {
	{ "get_id", wave_get_id },
	{ "get_index", wave_get_index },
	{ "clear", wave_clear },
	{ "get_name", wave_get_name },
	{ "set_name", wave_set_name },
	{ "get_path", wave_get_path },
	{ "set_path", wave_set_path },
	{ "get_flags", wave_get_flags },
	{ "set_flags", wave_set_flags },
	{ "get_volume", wave_get_volume },
	{ "set_volume", wave_set_volume },
	{ "get_envelope_count", wave_get_envelope_count },
	{ "set_envelope_count", wave_set_envelope_count },
	{ "get_envelope", wave_get_envelope },
	{ "set_envelope", wave_set_envelope },
	{ "get_level_count", wave_get_level_count },
	{ "get_level", wave_get_level },
	{ "add_level", wave_add_level },
	{ "remove_level", wave_remove_level },
	{ NULL, NULL }
};

static int wavelevel_get_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_id");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_id");
		return 0;
	}

	int _luaresult = zzub_wavelevel_get_id(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_get_wave(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_wave");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_wave");
		return 0;
	}

	zzub_wave_t* _luaresult = zzub_wavelevel_get_wave(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int wavelevel_clear(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wavelevel:clear");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:clear");
		return 0;
	}

	int _luaresult = zzub_wavelevel_clear(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_get_sample_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_sample_count");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_sample_count");
		return 0;
	}

	int _luaresult = zzub_wavelevel_get_sample_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_set_sample_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wavelevel:set_sample_count");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:set_sample_count");
		return 0;
	}

	int count = lua_tonumber(L, 3);
	zzub_wavelevel_set_sample_count(self, count);
	return 0;
}

static int wavelevel_get_root_note(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_root_note");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_root_note");
		return 0;
	}

	int _luaresult = zzub_wavelevel_get_root_note(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_set_root_note(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wavelevel:set_root_note");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:set_root_note");
		return 0;
	}

	int note = lua_tonumber(L, 3);
	zzub_wavelevel_set_root_note(self, note);
	return 0;
}

static int wavelevel_get_samples_per_second(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_samples_per_second");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_samples_per_second");
		return 0;
	}

	int _luaresult = zzub_wavelevel_get_samples_per_second(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_set_samples_per_second(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wavelevel:set_samples_per_second");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:set_samples_per_second");
		return 0;
	}

	int sps = lua_tonumber(L, 3);
	zzub_wavelevel_set_samples_per_second(self, sps);
	return 0;
}

static int wavelevel_get_loop_start(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_loop_start");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_loop_start");
		return 0;
	}

	int _luaresult = zzub_wavelevel_get_loop_start(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_set_loop_start(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wavelevel:set_loop_start");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:set_loop_start");
		return 0;
	}

	int pos = lua_tonumber(L, 3);
	zzub_wavelevel_set_loop_start(self, pos);
	return 0;
}

static int wavelevel_get_loop_end(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_loop_end");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_loop_end");
		return 0;
	}

	int _luaresult = zzub_wavelevel_get_loop_end(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_set_loop_end(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wavelevel:set_loop_end");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:set_loop_end");
		return 0;
	}

	int pos = lua_tonumber(L, 3);
	zzub_wavelevel_set_loop_end(self, pos);
	return 0;
}

static int wavelevel_get_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_format");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_format");
		return 0;
	}

	int _luaresult = zzub_wavelevel_get_format(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_set_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wavelevel:set_format");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:set_format");
		return 0;
	}

	int format = lua_tonumber(L, 3);
	zzub_wavelevel_set_format(self, format);
	return 0;
}

static int wavelevel_load_wav(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Wavelevel:load_wav");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:load_wav");
		return 0;
	}

	int offset = lua_tonumber(L, 3);
	int clear = lua_tonumber(L, 4);
	zzub_input_t* datastream = (zzub_input_t*)lua_touserdata(L, 5);
	int _luaresult = zzub_wavelevel_load_wav(self, offset, clear, datastream);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_save_wav(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Wavelevel:save_wav");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:save_wav");
		return 0;
	}

	zzub_output_t* datastream = (zzub_output_t*)lua_touserdata(L, 3);
	int _luaresult = zzub_wavelevel_save_wav(self, datastream);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_save_wav_range(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Wavelevel:save_wav_range");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:save_wav_range");
		return 0;
	}

	zzub_output_t* datastream = (zzub_output_t*)lua_touserdata(L, 3);
	int start = lua_tonumber(L, 4);
	int numsamples = lua_tonumber(L, 5);
	int _luaresult = zzub_wavelevel_save_wav_range(self, datastream, start, numsamples);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wavelevel_insert_sample_range(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Wavelevel:insert_sample_range");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:insert_sample_range");
		return 0;
	}

	int start = lua_tonumber(L, 3);
	void* buffer = lua_touserdata(L, 4);
	int channels = lua_tonumber(L, 5);
	int format = lua_tonumber(L, 6);
	int numsamples = lua_tonumber(L, 7);
	zzub_wavelevel_insert_sample_range(self, start, buffer, channels, format, numsamples);
	return 0;
}

static int wavelevel_remove_sample_range(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Wavelevel:remove_sample_range");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:remove_sample_range");
		return 0;
	}

	int start = lua_tonumber(L, 3);
	int numsamples = lua_tonumber(L, 4);
	zzub_wavelevel_remove_sample_range(self, start, numsamples);
	return 0;
}

static int wavelevel_replace_sample_range(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Wavelevel:replace_sample_range");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:replace_sample_range");
		return 0;
	}

	int start = lua_tonumber(L, 3);
	void* buffer = lua_touserdata(L, 4);
	int channels = lua_tonumber(L, 5);
	int format = lua_tonumber(L, 6);
	int numsamples = lua_tonumber(L, 7);
	zzub_wavelevel_replace_sample_range(self, start, buffer, channels, format, numsamples);
	return 0;
}

static int wavelevel_get_samples_digest(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 9) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_samples_digest");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_samples_digest");
		return 0;
	}

	int channel = lua_tonumber(L, 3);
	int start = lua_tonumber(L, 4);
	int end = lua_tonumber(L, 5);
	float* mindigest;
	float* maxdigest;
	float* ampdigest;
	int digestsize = lua_tonumber(L, 9);
	zzub_wavelevel_get_samples_digest(self, channel, start, end, mindigest, maxdigest, ampdigest, digestsize);
	return 0;
}

static int wavelevel_get_slices(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Wavelevel:get_slices");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:get_slices");
		return 0;
	}

	int slicecount;
	int* slices;
	zzub_wavelevel_get_slices(self, &slicecount, slices);
	return 0;
}

static int wavelevel_set_slices(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Wavelevel:set_slices");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:set_slices");
		return 0;
	}

	int slicecount = lua_tonumber(L, 3);
	const int* slices;	zzub_wavelevel_set_slices(self, slicecount, slices);
	return 0;
}

static int wavelevel_process_sample_range_offline(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Wavelevel:process_sample_range_offline");
		return 0;
	}
	zzub_wavelevel_t* self = (zzub_wavelevel_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Wavelevel:process_sample_range_offline");
		return 0;
	}

	int start = lua_tonumber(L, 3);
	int numsamples = lua_tonumber(L, 4);
	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 5);
	zzub_wavelevel_process_sample_range_offline(self, start, numsamples, plugin);
	return 0;
}

static const luaL_Reg wavelevel_lib[] = {
	{ "get_id", wavelevel_get_id },
	{ "get_wave", wavelevel_get_wave },
	{ "clear", wavelevel_clear },
	{ "get_sample_count", wavelevel_get_sample_count },
	{ "set_sample_count", wavelevel_set_sample_count },
	{ "get_root_note", wavelevel_get_root_note },
	{ "set_root_note", wavelevel_set_root_note },
	{ "get_samples_per_second", wavelevel_get_samples_per_second },
	{ "set_samples_per_second", wavelevel_set_samples_per_second },
	{ "get_loop_start", wavelevel_get_loop_start },
	{ "set_loop_start", wavelevel_set_loop_start },
	{ "get_loop_end", wavelevel_get_loop_end },
	{ "set_loop_end", wavelevel_set_loop_end },
	{ "get_format", wavelevel_get_format },
	{ "set_format", wavelevel_set_format },
	{ "load_wav", wavelevel_load_wav },
	{ "save_wav", wavelevel_save_wav },
	{ "save_wav_range", wavelevel_save_wav_range },
	{ "insert_sample_range", wavelevel_insert_sample_range },
	{ "remove_sample_range", wavelevel_remove_sample_range },
	{ "replace_sample_range", wavelevel_replace_sample_range },
	{ "get_samples_digest", wavelevel_get_samples_digest },
	{ "get_slices", wavelevel_get_slices },
	{ "set_slices", wavelevel_set_slices },
	{ "process_sample_range_offline", wavelevel_process_sample_range_offline },
	{ NULL, NULL }
};

static int envelope_get_attack(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Envelope:get_attack");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:get_attack");
		return 0;
	}

	unsigned short _luaresult = zzub_envelope_get_attack(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int envelope_get_decay(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Envelope:get_decay");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:get_decay");
		return 0;
	}

	unsigned short _luaresult = zzub_envelope_get_decay(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int envelope_get_sustain(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Envelope:get_sustain");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:get_sustain");
		return 0;
	}

	unsigned short _luaresult = zzub_envelope_get_sustain(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int envelope_get_release(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Envelope:get_release");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:get_release");
		return 0;
	}

	unsigned short _luaresult = zzub_envelope_get_release(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int envelope_set_attack(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Envelope:set_attack");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:set_attack");
		return 0;
	}

	unsigned short attack = lua_tonumber(L, 3);
	zzub_envelope_set_attack(self, attack);
	return 0;
}

static int envelope_set_decay(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Envelope:set_decay");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:set_decay");
		return 0;
	}

	unsigned short decay = lua_tonumber(L, 3);
	zzub_envelope_set_decay(self, decay);
	return 0;
}

static int envelope_set_sustain(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Envelope:set_sustain");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:set_sustain");
		return 0;
	}

	unsigned short sustain = lua_tonumber(L, 3);
	zzub_envelope_set_sustain(self, sustain);
	return 0;
}

static int envelope_set_release(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Envelope:set_release");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:set_release");
		return 0;
	}

	unsigned short release = lua_tonumber(L, 3);
	zzub_envelope_set_release(self, release);
	return 0;
}

static int envelope_get_subdivision(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Envelope:get_subdivision");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:get_subdivision");
		return 0;
	}

	char _luaresult = zzub_envelope_get_subdivision(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int envelope_set_subdivision(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Envelope:set_subdivision");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:set_subdivision");
		return 0;
	}

	char subdiv = lua_tonumber(L, 3);
	zzub_envelope_set_subdivision(self, subdiv);
	return 0;
}

static int envelope_get_flags(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Envelope:get_flags");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:get_flags");
		return 0;
	}

	char _luaresult = zzub_envelope_get_flags(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int envelope_set_flags(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Envelope:set_flags");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:set_flags");
		return 0;
	}

	char flags = lua_tonumber(L, 3);
	zzub_envelope_set_flags(self, flags);
	return 0;
}

static int envelope_is_enabled(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Envelope:is_enabled");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:is_enabled");
		return 0;
	}

	int _luaresult = zzub_envelope_is_enabled(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int envelope_enable(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Envelope:enable");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:enable");
		return 0;
	}

	int enable = lua_tonumber(L, 3);
	zzub_envelope_enable(self, enable);
	return 0;
}

static int envelope_get_point_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Envelope:get_point_count");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:get_point_count");
		return 0;
	}

	int _luaresult = zzub_envelope_get_point_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int envelope_get_point(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Envelope:get_point");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:get_point");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	unsigned short x;
	unsigned short y;
	char flags;
	zzub_envelope_get_point(self, index, &x, &y, &flags);
	return 0;
}

static int envelope_set_point(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Envelope:set_point");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:set_point");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	unsigned short x = lua_tonumber(L, 4);
	unsigned short y = lua_tonumber(L, 5);
	char flags = lua_tonumber(L, 6);
	zzub_envelope_set_point(self, index, x, y, flags);
	return 0;
}

static int envelope_insert_point(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Envelope:insert_point");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:insert_point");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_envelope_insert_point(self, index);
	return 0;
}

static int envelope_delete_point(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Envelope:delete_point");
		return 0;
	}
	zzub_envelope_t* self = (zzub_envelope_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Envelope:delete_point");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_envelope_delete_point(self, index);
	return 0;
}

static const luaL_Reg envelope_lib[] = {
	{ "get_attack", envelope_get_attack },
	{ "get_decay", envelope_get_decay },
	{ "get_sustain", envelope_get_sustain },
	{ "get_release", envelope_get_release },
	{ "set_attack", envelope_set_attack },
	{ "set_decay", envelope_set_decay },
	{ "set_sustain", envelope_set_sustain },
	{ "set_release", envelope_set_release },
	{ "get_subdivision", envelope_get_subdivision },
	{ "set_subdivision", envelope_set_subdivision },
	{ "get_flags", envelope_get_flags },
	{ "set_flags", envelope_set_flags },
	{ "is_enabled", envelope_is_enabled },
	{ "enable", envelope_enable },
	{ "get_point_count", envelope_get_point_count },
	{ "get_point", envelope_get_point },
	{ "set_point", envelope_set_point },
	{ "insert_point", envelope_insert_point },
	{ "delete_point", envelope_delete_point },
	{ NULL, NULL }
};

static int validation_error_iterator_next(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationErrorIterator:next");
		return 0;
	}
	zzub_validation_error_iterator_t* self = (zzub_validation_error_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationErrorIterator:next");
		return 0;
	}

	zzub_validation_error_iterator_next(self);
	return 0;
}

static int validation_error_iterator_valid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationErrorIterator:valid");
		return 0;
	}
	zzub_validation_error_iterator_t* self = (zzub_validation_error_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationErrorIterator:valid");
		return 0;
	}

	int _luaresult = zzub_validation_error_iterator_valid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int validation_error_iterator_current(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationErrorIterator:current");
		return 0;
	}
	zzub_validation_error_iterator_t* self = (zzub_validation_error_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationErrorIterator:current");
		return 0;
	}

	zzub_validation_error_t* _luaresult = zzub_validation_error_iterator_current(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int validation_error_iterator_reset(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationErrorIterator:reset");
		return 0;
	}
	zzub_validation_error_iterator_t* self = (zzub_validation_error_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationErrorIterator:reset");
		return 0;
	}

	zzub_validation_error_iterator_reset(self);
	return 0;
}

static int validation_error_iterator_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationErrorIterator:destroy");
		return 0;
	}
	zzub_validation_error_iterator_t* self = (zzub_validation_error_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationErrorIterator:destroy");
		return 0;
	}

	zzub_validation_error_iterator_destroy(self);
	return 0;
}

static const luaL_Reg validation_error_iterator_lib[] = {
	{ "next", validation_error_iterator_next },
	{ "valid", validation_error_iterator_valid },
	{ "current", validation_error_iterator_current },
	{ "reset", validation_error_iterator_reset },
	{ "destroy", validation_error_iterator_destroy },
	{ NULL, NULL }
};

static int validation_error_get_type(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationError:get_type");
		return 0;
	}
	zzub_validation_error_t* self = (zzub_validation_error_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationError:get_type");
		return 0;
	}

	int _luaresult = zzub_validation_error_get_type(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int validation_error_get_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationError:get_group");
		return 0;
	}
	zzub_validation_error_t* self = (zzub_validation_error_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationError:get_group");
		return 0;
	}

	int _luaresult = zzub_validation_error_get_group(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int validation_error_get_column(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationError:get_column");
		return 0;
	}
	zzub_validation_error_t* self = (zzub_validation_error_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationError:get_column");
		return 0;
	}

	int _luaresult = zzub_validation_error_get_column(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int validation_error_get_found_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationError:get_found_value");
		return 0;
	}
	zzub_validation_error_t* self = (zzub_validation_error_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationError:get_found_value");
		return 0;
	}

	int _luaresult = zzub_validation_error_get_found_value(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int validation_error_get_expected_value(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationError:get_expected_value");
		return 0;
	}
	zzub_validation_error_t* self = (zzub_validation_error_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationError:get_expected_value");
		return 0;
	}

	int _luaresult = zzub_validation_error_get_expected_value(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int validation_error_get_parameter_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationError:get_parameter_name");
		return 0;
	}
	zzub_validation_error_t* self = (zzub_validation_error_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationError:get_parameter_name");
		return 0;
	}

	const char* _luaresult = zzub_validation_error_get_parameter_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int validation_error_get_plugin_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationError:get_plugin_name");
		return 0;
	}
	zzub_validation_error_t* self = (zzub_validation_error_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationError:get_plugin_name");
		return 0;
	}

	const char* _luaresult = zzub_validation_error_get_plugin_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int validation_error_get_pluginloader(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for ValidationError:get_pluginloader");
		return 0;
	}
	zzub_validation_error_t* self = (zzub_validation_error_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for ValidationError:get_pluginloader");
		return 0;
	}

	zzub_pluginloader_t* _luaresult = zzub_validation_error_get_pluginloader(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static const luaL_Reg validation_error_lib[] = {
	{ "get_type", validation_error_get_type },
	{ "get_group", validation_error_get_group },
	{ "get_column", validation_error_get_column },
	{ "get_found_value", validation_error_get_found_value },
	{ "get_expected_value", validation_error_get_expected_value },
	{ "get_parameter_name", validation_error_get_parameter_name },
	{ "get_plugin_name", validation_error_get_plugin_name },
	{ "get_pluginloader", validation_error_get_pluginloader },
	{ NULL, NULL }
};

static int player_create(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:create");
		return 0;
	}
	const char* hostpath = luaL_tolstring(L, 2, &len);
	const char* userpath = luaL_tolstring(L, 3, &len);
	const char* temppath = luaL_tolstring(L, 4, &len);
	zzub_player_t* _luaresult = zzub_player_create(hostpath, userpath, temppath);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:destroy");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:destroy");
		return 0;
	}

	zzub_player_destroy(self);
	return 0;
}

static int player_initialize(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:initialize");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:initialize");
		return 0;
	}

	int samplesPerSecond = lua_tonumber(L, 3);
	int _luaresult = zzub_player_initialize(self, samplesPerSecond);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_remote_connect(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:remote_connect");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:remote_connect");
		return 0;
	}

	const char* host = luaL_tolstring(L, 3, &len);
	const char* port = luaL_tolstring(L, 4, &len);
	int _luaresult = zzub_player_remote_connect(self, host, port);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_remote_disconnect(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:remote_disconnect");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:remote_disconnect");
		return 0;
	}

	zzub_player_remote_disconnect(self);
	return 0;
}

static int player_remote_open(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:remote_open");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:remote_open");
		return 0;
	}

	const char* project = luaL_tolstring(L, 3, &len);
	const char* password = luaL_tolstring(L, 4, &len);
	int _luaresult = zzub_player_remote_open(self, project, password);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_remote_create(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:remote_create");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:remote_create");
		return 0;
	}

	const char* project = luaL_tolstring(L, 3, &len);
	const char* password = luaL_tolstring(L, 4, &len);
	int _luaresult = zzub_player_remote_create(self, project, password);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_remote_delete(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:remote_delete");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:remote_delete");
		return 0;
	}

	const char* project = luaL_tolstring(L, 3, &len);
	const char* password = luaL_tolstring(L, 4, &len);
	int _luaresult = zzub_player_remote_delete(self, project, password);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_remote_client_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_remote_client_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_remote_client_count");
		return 0;
	}

	int _luaresult = zzub_player_get_remote_client_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_is_remote_connected(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:is_remote_connected");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:is_remote_connected");
		return 0;
	}

	int _luaresult = zzub_player_is_remote_connected(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_load_armz(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Player:load_armz");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:load_armz");
		return 0;
	}

	const char* fileName = luaL_tolstring(L, 3, &len);
	int mode = lua_tonumber(L, 4);
	zzub_plugin_group_t* plugingroup = (zzub_plugin_group_t*)lua_touserdata(L, 5);
	int _luaresult = zzub_player_load_armz(self, fileName, mode, plugingroup);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_save_armz(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Player:save_armz");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:save_armz");
		return 0;
	}

	const char* fileName = luaL_tolstring(L, 3, &len);
	const zzub_plugin_t** plugins;	int plugincount = lua_tonumber(L, 5);
	zzub_plugin_group_t* plugingroup = (zzub_plugin_group_t*)lua_touserdata(L, 6);
	int _luaresult = zzub_player_save_armz(self, fileName, plugins, plugincount, plugingroup);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_load_bmx(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 6) {
		luaL_error(L, "Invalid argument count for Player:load_bmx");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:load_bmx");
		return 0;
	}

	zzub_input_t* datastream = (zzub_input_t*)lua_touserdata(L, 3);
	int flags = lua_tonumber(L, 4);
	float x = lua_tonumber(L, 5);
	float y = lua_tonumber(L, 6);
	int _luaresult = zzub_player_load_bmx(self, datastream, flags, x, y);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_load_module(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:load_module");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:load_module");
		return 0;
	}

	const char* fileName = luaL_tolstring(L, 3, &len);
	int _luaresult = zzub_player_load_module(self, fileName);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_validation_errors(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_validation_errors");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_validation_errors");
		return 0;
	}

	zzub_validation_error_iterator_t* _luaresult = zzub_player_get_validation_errors(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_state(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_state");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_state");
		return 0;
	}

	int _luaresult = zzub_player_get_state(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_state(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:set_state");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_state");
		return 0;
	}

	int state = lua_tonumber(L, 3);
	int stoprow = lua_tonumber(L, 4);
	zzub_player_set_state(self, state, stoprow);
	return 0;
}

static int player_get_pluginloader_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_pluginloader_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pluginloader_count");
		return 0;
	}

	int _luaresult = zzub_player_get_pluginloader_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_pluginloader(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_pluginloader");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pluginloader");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_pluginloader_t* _luaresult = zzub_player_get_pluginloader(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_pluginloader_by_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_pluginloader_by_name");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pluginloader_by_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	zzub_pluginloader_t* _luaresult = zzub_player_get_pluginloader_by_name(self, name);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_plugin_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_plugin_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_plugin_count");
		return 0;
	}

	int _luaresult = zzub_player_get_plugin_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_add_midimapping(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Player:add_midimapping");
		return 0;
	}
	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 2);
	int group = lua_tonumber(L, 3);
	int track = lua_tonumber(L, 4);
	int param = lua_tonumber(L, 5);
	int channel = lua_tonumber(L, 6);
	int controller = lua_tonumber(L, 7);
	zzub_midimapping_t* _luaresult = zzub_player_add_midimapping(plugin, group, track, param, channel, controller);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_remove_midimapping(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Player:remove_midimapping");
		return 0;
	}
	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 2);
	int group = lua_tonumber(L, 3);
	int track = lua_tonumber(L, 4);
	int param = lua_tonumber(L, 5);
	int _luaresult = zzub_player_remove_midimapping(plugin, group, track, param);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_plugin_by_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_plugin_by_name");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_plugin_by_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	zzub_plugin_t* _luaresult = zzub_player_get_plugin_by_name(self, name);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_plugin_by_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_plugin_by_id");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_plugin_by_id");
		return 0;
	}

	int id = lua_tonumber(L, 3);
	zzub_plugin_t* _luaresult = zzub_player_get_plugin_by_id(self, id);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_plugin");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_plugin");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_plugin_t* _luaresult = zzub_player_get_plugin(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_plugin_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_plugin_iterator");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_plugin_iterator");
		return 0;
	}

	zzub_plugin_iterator_t* _luaresult = zzub_player_get_plugin_iterator(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_pattern_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_iterator");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_iterator");
		return 0;
	}

	zzub_pattern_iterator_t* _luaresult = zzub_player_get_pattern_iterator(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_pattern_by_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_by_id");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_by_id");
		return 0;
	}

	int id = lua_tonumber(L, 3);
	zzub_pattern_t* _luaresult = zzub_player_get_pattern_by_id(self, id);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_pattern_by_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_by_index");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_by_index");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_pattern_t* _luaresult = zzub_player_get_pattern_by_index(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_pattern_event_by_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_event_by_id");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_event_by_id");
		return 0;
	}

	int id = lua_tonumber(L, 3);
	zzub_pattern_event_t* _luaresult = zzub_player_get_pattern_event_by_id(self, id);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_new_pattern_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:get_new_pattern_name");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_new_pattern_name");
		return 0;
	}

	zzub_pattern_format_t* format = (zzub_pattern_format_t*)lua_touserdata(L, 3);
	const char* description = luaL_tolstring(L, 4, &len);
	const char* _luaresult = zzub_player_get_new_pattern_name(self, format, description);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int player_get_pattern_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_count");
		return 0;
	}

	int _luaresult = zzub_player_get_pattern_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_pattern_by_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_by_name");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_by_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	zzub_pattern_t* _luaresult = zzub_player_get_pattern_by_name(self, name);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_pattern_format_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_format_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_format_count");
		return 0;
	}

	int _luaresult = zzub_player_get_pattern_format_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_new_pattern_format_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_new_pattern_format_name");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_new_pattern_format_name");
		return 0;
	}

	const char* description = luaL_tolstring(L, 3, &len);
	const char* _luaresult = zzub_player_get_new_pattern_format_name(self, description);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int player_get_pattern_format_by_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_format_by_name");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_format_by_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	zzub_pattern_format_t* _luaresult = zzub_player_get_pattern_format_by_name(self, name);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_pattern_format_by_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_format_by_index");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_format_by_index");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_pattern_format_t* _luaresult = zzub_player_get_pattern_format_by_index(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_pattern_format_by_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_format_by_id");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_format_by_id");
		return 0;
	}

	int id = lua_tonumber(L, 3);
	zzub_pattern_format_t* _luaresult = zzub_player_get_pattern_format_by_id(self, id);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_pattern_format_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_pattern_format_iterator");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_pattern_format_iterator");
		return 0;
	}

	zzub_pattern_format_iterator_t* _luaresult = zzub_player_get_pattern_format_iterator(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_work_stereo(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Player:work_stereo");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:work_stereo");
		return 0;
	}

	const float** inbuffers;	float** outbuffers;
	int inchannels = lua_tonumber(L, 5);
	int outchannels = lua_tonumber(L, 6);
	int numsamples = lua_tonumber(L, 7);
	zzub_player_work_stereo(self, inbuffers, outbuffers, inchannels, outchannels, numsamples);
	return 0;
}

static int player_clear(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:clear");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:clear");
		return 0;
	}

	zzub_player_clear(self);
	return 0;
}

static int player_get_wave_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_wave_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_wave_count");
		return 0;
	}

	int _luaresult = zzub_player_get_wave_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_wave(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_wave");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_wave");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_wave_t* _luaresult = zzub_player_get_wave(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

void(*luacallback_zzub_player_add_callback)(lua_State* L, zzub_player_t* player, zzub_callback_t callback, void* tag);

static int player_add_callback(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:add_callback");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:add_callback");
		return 0;
	}

	zzub_callback_t callback = zzub_callback_alloc(L, 3);
	void* tag = lua_touserdata(L, 4);
	if (luacallback_zzub_player_add_callback) luacallback_zzub_player_add_callback(L, self,  callback,  tag);
	zzub_player_add_callback(self, callback, tag);
	return 0;
}

void(*luacallback_zzub_player_remove_callback)(lua_State* L, zzub_player_t* player, zzub_callback_t callback, void* tag);

static int player_remove_callback(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:remove_callback");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:remove_callback");
		return 0;
	}

	zzub_callback_t callback = zzub_callback_alloc(L, 3);
	void* tag = lua_touserdata(L, 4);
	if (luacallback_zzub_player_remove_callback) luacallback_zzub_player_remove_callback(L, self,  callback,  tag);
	zzub_player_remove_callback(self, callback, tag);
	return 0;
}

static int player_handle_events(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:handle_events");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:handle_events");
		return 0;
	}

	zzub_player_handle_events(self);
	return 0;
}

static int player_get_midimapping(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_midimapping");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_midimapping");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_midimapping_t* _luaresult = zzub_player_get_midimapping(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_midimapping_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_midimapping_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_midimapping_count");
		return 0;
	}

	int _luaresult = zzub_player_get_midimapping_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_automation(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_automation");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_automation");
		return 0;
	}

	int _luaresult = zzub_player_get_automation(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_automation(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_automation");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_automation");
		return 0;
	}

	int enable = lua_toboolean(L, 3);
	zzub_player_set_automation(self, enable);
	return 0;
}

static int player_get_midi_transport(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_midi_transport");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_midi_transport");
		return 0;
	}

	int _luaresult = zzub_player_get_midi_transport(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_midi_transport(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_midi_transport");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_midi_transport");
		return 0;
	}

	int enable = lua_toboolean(L, 3);
	zzub_player_set_midi_transport(self, enable);
	return 0;
}

static int player_get_infotext(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_infotext");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_infotext");
		return 0;
	}

	const char* _luaresult = zzub_player_get_infotext(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int player_set_infotext(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_infotext");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_infotext");
		return 0;
	}

	const char* text = luaL_tolstring(L, 3, &len);
	zzub_player_set_infotext(self, text);
	return 0;
}

static int player_set_midi_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_midi_plugin");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_midi_plugin");
		return 0;
	}

	zzub_plugin_t* plugin = (zzub_plugin_t*)lua_touserdata(L, 3);
	zzub_player_set_midi_plugin(self, plugin);
	return 0;
}

static int player_get_midi_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_midi_plugin");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_midi_plugin");
		return 0;
	}

	zzub_plugin_t* _luaresult = zzub_player_get_midi_plugin(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_midi_lock(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_midi_lock");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_midi_lock");
		return 0;
	}

	int _luaresult = zzub_player_get_midi_lock(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_midi_lock(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_midi_lock");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_midi_lock");
		return 0;
	}

	int state = lua_toboolean(L, 3);
	zzub_player_set_midi_lock(self, state);
	return 0;
}

static int player_get_new_plugin_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_new_plugin_name");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_new_plugin_name");
		return 0;
	}

	const char* uri = luaL_tolstring(L, 3, &len);
	const char* _luaresult = zzub_player_get_new_plugin_name(self, uri);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int player_reset_keyjazz(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:reset_keyjazz");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:reset_keyjazz");
		return 0;
	}

	zzub_player_reset_keyjazz(self);
	return 0;
}

static int player_create_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 7) {
		luaL_error(L, "Invalid argument count for Player:create_plugin");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:create_plugin");
		return 0;
	}

	zzub_input_t* input = (zzub_input_t*)lua_touserdata(L, 3);
	int dataSize = lua_tonumber(L, 4);
	const char* instanceName = luaL_tolstring(L, 5, &len);
	zzub_pluginloader_t* loader = (zzub_pluginloader_t*)lua_touserdata(L, 6);
	zzub_plugin_group_t* group = (zzub_plugin_group_t*)lua_touserdata(L, 7);
	zzub_plugin_t* _luaresult = zzub_player_create_plugin(self, input, dataSize, instanceName, loader, group);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_create_pattern(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Player:create_pattern");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:create_pattern");
		return 0;
	}

	zzub_pattern_format_t* format = (zzub_pattern_format_t*)lua_touserdata(L, 3);
	const char* description = luaL_tolstring(L, 4, &len);
	int rows = lua_tonumber(L, 5);
	zzub_pattern_t* _luaresult = zzub_player_create_pattern(self, format, description, rows);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_clone_pattern(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:clone_pattern");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:clone_pattern");
		return 0;
	}

	zzub_pattern_t* pattern = (zzub_pattern_t*)lua_touserdata(L, 3);
	const char* description = luaL_tolstring(L, 4, &len);
	zzub_pattern_t* _luaresult = zzub_player_clone_pattern(self, pattern, description);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_create_pattern_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:create_pattern_format");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:create_pattern_format");
		return 0;
	}

	const char* description = luaL_tolstring(L, 3, &len);
	zzub_pattern_format_t* _luaresult = zzub_player_create_pattern_format(self, description);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_clone_pattern_format(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:clone_pattern_format");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:clone_pattern_format");
		return 0;
	}

	zzub_pattern_format_t* format = (zzub_pattern_format_t*)lua_touserdata(L, 3);
	const char* description = luaL_tolstring(L, 4, &len);
	zzub_pattern_format_t* _luaresult = zzub_player_clone_pattern_format(self, format, description);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_undo(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:undo");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:undo");
		return 0;
	}

	zzub_player_undo(self);
	return 0;
}

static int player_redo(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:redo");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:redo");
		return 0;
	}

	zzub_player_redo(self);
	return 0;
}

static int player_history_enable(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:history_enable");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:history_enable");
		return 0;
	}

	int state = lua_toboolean(L, 3);
	int _luaresult = zzub_player_history_enable(self, state);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_history_begin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:history_begin");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:history_begin");
		return 0;
	}

	void* userdata = lua_touserdata(L, 3);
	zzub_player_history_begin(self, userdata);
	return 0;
}

static int player_history_commit(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Player:history_commit");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:history_commit");
		return 0;
	}

	int redo_id = lua_tonumber(L, 3);
	int undo_id = lua_tonumber(L, 4);
	const char* description = luaL_tolstring(L, 5, &len);
	zzub_player_history_commit(self, redo_id, undo_id, description);
	return 0;
}

static int player_history_get_uncomitted_operations(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:history_get_uncomitted_operations");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:history_get_uncomitted_operations");
		return 0;
	}

	int _luaresult = zzub_player_history_get_uncomitted_operations(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_history_reset(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:history_reset");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:history_reset");
		return 0;
	}

	zzub_player_history_reset(self);
	return 0;
}

static int player_history_get_size(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:history_get_size");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:history_get_size");
		return 0;
	}

	int _luaresult = zzub_player_history_get_size(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_history_get_position(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:history_get_position");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:history_get_position");
		return 0;
	}

	int _luaresult = zzub_player_history_get_position(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_history_get_description(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:history_get_description");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:history_get_description");
		return 0;
	}

	int position = lua_tonumber(L, 3);
	const char* _luaresult = zzub_player_history_get_description(self, position);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int player_set_host_info(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Player:set_host_info");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_host_info");
		return 0;
	}

	int id = lua_tonumber(L, 3);
	int version = lua_tonumber(L, 4);
	void* host_ptr = lua_touserdata(L, 5);
	zzub_player_set_host_info(self, id, version, host_ptr);
	return 0;
}

static int player_invoke_event(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:invoke_event");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:invoke_event");
		return 0;
	}

	zzub_event_data_t* data = (zzub_event_data_t*)lua_touserdata(L, 3);
	int immediate = lua_tonumber(L, 4);
	int _luaresult = zzub_player_invoke_event(self, data, immediate);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_order_length(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_order_length");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_order_length");
		return 0;
	}

	int length = lua_tonumber(L, 3);
	zzub_player_set_order_length(self, length);
	return 0;
}

static int player_get_order_length(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_order_length");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_order_length");
		return 0;
	}

	int _luaresult = zzub_player_get_order_length(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_order_pattern(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:set_order_pattern");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_order_pattern");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_pattern_t* pattern = (zzub_pattern_t*)lua_touserdata(L, 4);
	zzub_player_set_order_pattern(self, index, pattern);
	return 0;
}

static int player_get_order_pattern(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_order_pattern");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_order_pattern");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_pattern_t* _luaresult = zzub_player_get_order_pattern(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_order_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_order_iterator");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_order_iterator");
		return 0;
	}

	zzub_pattern_iterator_t* _luaresult = zzub_player_get_order_iterator(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_order_loop_start(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_order_loop_start");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_order_loop_start");
		return 0;
	}

	int _luaresult = zzub_player_get_order_loop_start(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_order_loop_start(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_order_loop_start");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_order_loop_start");
		return 0;
	}

	int pos = lua_tonumber(L, 3);
	zzub_player_set_order_loop_start(self, pos);
	return 0;
}

static int player_get_order_loop_end(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_order_loop_end");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_order_loop_end");
		return 0;
	}

	int _luaresult = zzub_player_get_order_loop_end(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_order_loop_end(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_order_loop_end");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_order_loop_end");
		return 0;
	}

	int pos = lua_tonumber(L, 3);
	zzub_player_set_order_loop_end(self, pos);
	return 0;
}

static int player_get_order_loop_enabled(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_order_loop_enabled");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_order_loop_enabled");
		return 0;
	}

	int _luaresult = zzub_player_get_order_loop_enabled(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_order_loop_enabled(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_order_loop_enabled");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_order_loop_enabled");
		return 0;
	}

	int enable = lua_tonumber(L, 3);
	zzub_player_set_order_loop_enabled(self, enable);
	return 0;
}

static int player_set_queue_order_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_queue_order_index");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_queue_order_index");
		return 0;
	}

	int pos = lua_tonumber(L, 3);
	zzub_player_set_queue_order_index(self, pos);
	return 0;
}

static int player_get_queue_order_index(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_queue_order_index");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_queue_order_index");
		return 0;
	}

	int _luaresult = zzub_player_get_queue_order_index(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_timeshift_order(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:timeshift_order");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:timeshift_order");
		return 0;
	}

	int fromindex = lua_tonumber(L, 3);
	int timeshift = lua_tonumber(L, 4);
	zzub_player_timeshift_order(self, fromindex, timeshift);
	return 0;
}

static int player_get_position_order(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_position_order");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_position_order");
		return 0;
	}

	int _luaresult = zzub_player_get_position_order(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_position_row(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_position_row");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_position_row");
		return 0;
	}

	int _luaresult = zzub_player_get_position_row(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_position_samples(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_position_samples");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_position_samples");
		return 0;
	}

	int _luaresult = zzub_player_get_position_samples(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_position(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:set_position");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_position");
		return 0;
	}

	int orderindex = lua_tonumber(L, 3);
	int tick = lua_tonumber(L, 4);
	zzub_player_set_position(self, orderindex, tick);
	return 0;
}

static int player_adjust_position_order(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:adjust_position_order");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:adjust_position_order");
		return 0;
	}

	int orderindex = lua_tonumber(L, 3);
	zzub_player_adjust_position_order(self, orderindex);
	return 0;
}

static int player_get_bpm(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_bpm");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_bpm");
		return 0;
	}

	float _luaresult = zzub_player_get_bpm(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_tpb(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_tpb");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_tpb");
		return 0;
	}

	int _luaresult = zzub_player_get_tpb(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_swing(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_swing");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_swing");
		return 0;
	}

	float _luaresult = zzub_player_get_swing(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_bpm(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_bpm");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_bpm");
		return 0;
	}

	float bpm = lua_tonumber(L, 3);
	zzub_player_set_bpm(self, bpm);
	return 0;
}

static int player_set_tpb(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_tpb");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_tpb");
		return 0;
	}

	int tpb = lua_tonumber(L, 3);
	zzub_player_set_tpb(self, tpb);
	return 0;
}

static int player_set_swing(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_swing");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_swing");
		return 0;
	}

	float swing = lua_tonumber(L, 3);
	zzub_player_set_swing(self, swing);
	return 0;
}

static int player_set_swing_ticks(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_swing_ticks");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_swing_ticks");
		return 0;
	}

	int swing_ticks = lua_tonumber(L, 3);
	zzub_player_set_swing_ticks(self, swing_ticks);
	return 0;
}

static int player_get_timesource_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_timesource_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_timesource_count");
		return 0;
	}

	int _luaresult = zzub_player_get_timesource_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_timesource_plugin(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_timesource_plugin");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_timesource_plugin");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_plugin_t* _luaresult = zzub_player_get_timesource_plugin(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_timesource_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_timesource_group");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_timesource_group");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_player_get_timesource_group(self, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_timesource_track(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_timesource_track");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_timesource_track");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_player_get_timesource_track(self, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_play_pattern(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for Player:play_pattern");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:play_pattern");
		return 0;
	}

	zzub_pattern_t* pat = (zzub_pattern_t*)lua_touserdata(L, 3);
	int row = lua_tonumber(L, 4);
	int stoprow = lua_tonumber(L, 5);
	zzub_player_play_pattern(self, pat, row, stoprow);
	return 0;
}

static int player_get_machineview_offset_x(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_machineview_offset_x");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_machineview_offset_x");
		return 0;
	}

	double _luaresult = zzub_player_get_machineview_offset_x(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_machineview_offset_y(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_machineview_offset_y");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_machineview_offset_y");
		return 0;
	}

	double _luaresult = zzub_player_get_machineview_offset_y(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_machineview_offset(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:set_machineview_offset");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_machineview_offset");
		return 0;
	}

	double x = lua_tonumber(L, 3);
	double y = lua_tonumber(L, 4);
	zzub_player_set_machineview_offset(self, x, y);
	return 0;
}

static int player_get_thread_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_thread_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_thread_count");
		return 0;
	}

	int _luaresult = zzub_player_get_thread_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_set_thread_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:set_thread_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:set_thread_count");
		return 0;
	}

	int threads = lua_tonumber(L, 3);
	zzub_player_set_thread_count(self, threads);
	return 0;
}

static int player_get_peaks(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:get_peaks");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_peaks");
		return 0;
	}

	float* peaks;
	int peakcount;
	zzub_player_get_peaks(self, peaks, &peakcount);
	return 0;
}

static int player_get_waveimporter_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for Player:get_waveimporter_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_waveimporter_count");
		return 0;
	}

	int _luaresult = zzub_player_get_waveimporter_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_waveimporter_format_ext_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_waveimporter_format_ext_count");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_waveimporter_format_ext_count");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_player_get_waveimporter_format_ext_count(self, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_waveimporter_format_ext(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:get_waveimporter_format_ext");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_waveimporter_format_ext");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int extindex = lua_tonumber(L, 4);
	const char* _luaresult = zzub_player_get_waveimporter_format_ext(self, index, extindex);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int player_get_waveimporter_format_is_container(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_waveimporter_format_is_container");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_waveimporter_format_is_container");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_player_get_waveimporter_format_is_container(self, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_get_waveimporter_format_type(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_waveimporter_format_type");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_waveimporter_format_type");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_player_get_waveimporter_format_type(self, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int player_create_waveimporter(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:create_waveimporter");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:create_waveimporter");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_wave_importer_t* _luaresult = zzub_player_create_waveimporter(self, index);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_create_waveimporter_by_file(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:create_waveimporter_by_file");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:create_waveimporter_by_file");
		return 0;
	}

	const char* filename = luaL_tolstring(L, 3, &len);
	zzub_wave_importer_t* _luaresult = zzub_player_create_waveimporter_by_file(self, filename);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_create_plugin_group(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for Player:create_plugin_group");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:create_plugin_group");
		return 0;
	}

	zzub_plugin_group_t* parent = (zzub_plugin_group_t*)lua_touserdata(L, 3);
	const char* name = luaL_tolstring(L, 4, &len);
	zzub_plugin_group_t* _luaresult = zzub_player_create_plugin_group(self, parent, name);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_plugin_group_by_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_plugin_group_by_id");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_plugin_group_by_id");
		return 0;
	}

	int id = lua_tonumber(L, 3);
	zzub_plugin_group_t* _luaresult = zzub_player_get_plugin_group_by_id(self, id);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int player_get_plugin_group_iterator(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for Player:get_plugin_group_iterator");
		return 0;
	}
	zzub_player_t* self = (zzub_player_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for Player:get_plugin_group_iterator");
		return 0;
	}

	zzub_plugin_group_t* parent = (zzub_plugin_group_t*)lua_touserdata(L, 3);
	zzub_plugin_group_iterator_t* _luaresult = zzub_player_get_plugin_group_iterator(self, parent);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static const luaL_Reg player_lib[] = {
	{ "create", player_create },
	{ "destroy", player_destroy },
	{ "initialize", player_initialize },
	{ "remote_connect", player_remote_connect },
	{ "remote_disconnect", player_remote_disconnect },
	{ "remote_open", player_remote_open },
	{ "remote_create", player_remote_create },
	{ "remote_delete", player_remote_delete },
	{ "get_remote_client_count", player_get_remote_client_count },
	{ "is_remote_connected", player_is_remote_connected },
	{ "load_armz", player_load_armz },
	{ "save_armz", player_save_armz },
	{ "load_bmx", player_load_bmx },
	{ "load_module", player_load_module },
	{ "get_validation_errors", player_get_validation_errors },
	{ "get_state", player_get_state },
	{ "set_state", player_set_state },
	{ "get_pluginloader_count", player_get_pluginloader_count },
	{ "get_pluginloader", player_get_pluginloader },
	{ "get_pluginloader_by_name", player_get_pluginloader_by_name },
	{ "get_plugin_count", player_get_plugin_count },
	{ "add_midimapping", player_add_midimapping },
	{ "remove_midimapping", player_remove_midimapping },
	{ "get_plugin_by_name", player_get_plugin_by_name },
	{ "get_plugin_by_id", player_get_plugin_by_id },
	{ "get_plugin", player_get_plugin },
	{ "get_plugin_iterator", player_get_plugin_iterator },
	{ "get_pattern_iterator", player_get_pattern_iterator },
	{ "get_pattern_by_id", player_get_pattern_by_id },
	{ "get_pattern_by_index", player_get_pattern_by_index },
	{ "get_pattern_event_by_id", player_get_pattern_event_by_id },
	{ "get_new_pattern_name", player_get_new_pattern_name },
	{ "get_pattern_count", player_get_pattern_count },
	{ "get_pattern_by_name", player_get_pattern_by_name },
	{ "get_pattern_format_count", player_get_pattern_format_count },
	{ "get_new_pattern_format_name", player_get_new_pattern_format_name },
	{ "get_pattern_format_by_name", player_get_pattern_format_by_name },
	{ "get_pattern_format_by_index", player_get_pattern_format_by_index },
	{ "get_pattern_format_by_id", player_get_pattern_format_by_id },
	{ "get_pattern_format_iterator", player_get_pattern_format_iterator },
	{ "work_stereo", player_work_stereo },
	{ "clear", player_clear },
	{ "get_wave_count", player_get_wave_count },
	{ "get_wave", player_get_wave },
	{ "add_callback", player_add_callback },
	{ "remove_callback", player_remove_callback },
	{ "handle_events", player_handle_events },
	{ "get_midimapping", player_get_midimapping },
	{ "get_midimapping_count", player_get_midimapping_count },
	{ "get_automation", player_get_automation },
	{ "set_automation", player_set_automation },
	{ "get_midi_transport", player_get_midi_transport },
	{ "set_midi_transport", player_set_midi_transport },
	{ "get_infotext", player_get_infotext },
	{ "set_infotext", player_set_infotext },
	{ "set_midi_plugin", player_set_midi_plugin },
	{ "get_midi_plugin", player_get_midi_plugin },
	{ "get_midi_lock", player_get_midi_lock },
	{ "set_midi_lock", player_set_midi_lock },
	{ "get_new_plugin_name", player_get_new_plugin_name },
	{ "reset_keyjazz", player_reset_keyjazz },
	{ "create_plugin", player_create_plugin },
	{ "create_pattern", player_create_pattern },
	{ "clone_pattern", player_clone_pattern },
	{ "create_pattern_format", player_create_pattern_format },
	{ "clone_pattern_format", player_clone_pattern_format },
	{ "undo", player_undo },
	{ "redo", player_redo },
	{ "history_enable", player_history_enable },
	{ "history_begin", player_history_begin },
	{ "history_commit", player_history_commit },
	{ "history_get_uncomitted_operations", player_history_get_uncomitted_operations },
	{ "history_reset", player_history_reset },
	{ "history_get_size", player_history_get_size },
	{ "history_get_position", player_history_get_position },
	{ "history_get_description", player_history_get_description },
	{ "set_host_info", player_set_host_info },
	{ "invoke_event", player_invoke_event },
	{ "set_order_length", player_set_order_length },
	{ "get_order_length", player_get_order_length },
	{ "set_order_pattern", player_set_order_pattern },
	{ "get_order_pattern", player_get_order_pattern },
	{ "get_order_iterator", player_get_order_iterator },
	{ "get_order_loop_start", player_get_order_loop_start },
	{ "set_order_loop_start", player_set_order_loop_start },
	{ "get_order_loop_end", player_get_order_loop_end },
	{ "set_order_loop_end", player_set_order_loop_end },
	{ "get_order_loop_enabled", player_get_order_loop_enabled },
	{ "set_order_loop_enabled", player_set_order_loop_enabled },
	{ "set_queue_order_index", player_set_queue_order_index },
	{ "get_queue_order_index", player_get_queue_order_index },
	{ "timeshift_order", player_timeshift_order },
	{ "get_position_order", player_get_position_order },
	{ "get_position_row", player_get_position_row },
	{ "get_position_samples", player_get_position_samples },
	{ "set_position", player_set_position },
	{ "adjust_position_order", player_adjust_position_order },
	{ "get_bpm", player_get_bpm },
	{ "get_tpb", player_get_tpb },
	{ "get_swing", player_get_swing },
	{ "set_bpm", player_set_bpm },
	{ "set_tpb", player_set_tpb },
	{ "set_swing", player_set_swing },
	{ "set_swing_ticks", player_set_swing_ticks },
	{ "get_timesource_count", player_get_timesource_count },
	{ "get_timesource_plugin", player_get_timesource_plugin },
	{ "get_timesource_group", player_get_timesource_group },
	{ "get_timesource_track", player_get_timesource_track },
	{ "play_pattern", player_play_pattern },
	{ "get_machineview_offset_x", player_get_machineview_offset_x },
	{ "get_machineview_offset_y", player_get_machineview_offset_y },
	{ "set_machineview_offset", player_set_machineview_offset },
	{ "get_thread_count", player_get_thread_count },
	{ "set_thread_count", player_set_thread_count },
	{ "get_peaks", player_get_peaks },
	{ "get_waveimporter_count", player_get_waveimporter_count },
	{ "get_waveimporter_format_ext_count", player_get_waveimporter_format_ext_count },
	{ "get_waveimporter_format_ext", player_get_waveimporter_format_ext },
	{ "get_waveimporter_format_is_container", player_get_waveimporter_format_is_container },
	{ "get_waveimporter_format_type", player_get_waveimporter_format_type },
	{ "create_waveimporter", player_create_waveimporter },
	{ "create_waveimporter_by_file", player_create_waveimporter_by_file },
	{ "create_plugin_group", player_create_plugin_group },
	{ "get_plugin_group_by_id", player_get_plugin_group_by_id },
	{ "get_plugin_group_iterator", player_get_plugin_group_iterator },
	{ NULL, NULL }
};

static int plugin_group_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroup:destroy");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:destroy");
		return 0;
	}

	zzub_plugin_group_destroy(self);
	return 0;
}

static int plugin_group_get_id(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroup:get_id");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:get_id");
		return 0;
	}

	int _luaresult = zzub_plugin_group_get_id(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_group_get_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroup:get_name");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:get_name");
		return 0;
	}

	const char* _luaresult = zzub_plugin_group_get_name(self);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int plugin_group_set_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PluginGroup:set_name");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:set_name");
		return 0;
	}

	const char* name = luaL_tolstring(L, 3, &len);
	zzub_plugin_group_set_name(self, name);
	return 0;
}

static int plugin_group_get_parent(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroup:get_parent");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:get_parent");
		return 0;
	}

	zzub_plugin_group_t* _luaresult = zzub_plugin_group_get_parent(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_group_set_parent(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for PluginGroup:set_parent");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:set_parent");
		return 0;
	}

	zzub_plugin_group_t* newparent = (zzub_plugin_group_t*)lua_touserdata(L, 3);
	zzub_plugin_group_set_parent(self, newparent);
	return 0;
}

static int plugin_group_get_position_x(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroup:get_position_x");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:get_position_x");
		return 0;
	}

	float _luaresult = zzub_plugin_group_get_position_x(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_group_get_position_y(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroup:get_position_y");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:get_position_y");
		return 0;
	}

	float _luaresult = zzub_plugin_group_get_position_y(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_group_set_position(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for PluginGroup:set_position");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:set_position");
		return 0;
	}

	float x = lua_tonumber(L, 3);
	float y = lua_tonumber(L, 4);
	zzub_plugin_group_set_position(self, x, y);
	return 0;
}

static int plugin_group_get_plugins(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroup:get_plugins");
		return 0;
	}
	zzub_plugin_group_t* self = (zzub_plugin_group_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroup:get_plugins");
		return 0;
	}

	zzub_plugin_iterator_t* _luaresult = zzub_plugin_group_get_plugins(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static const luaL_Reg plugin_group_lib[] = {
	{ "destroy", plugin_group_destroy },
	{ "get_id", plugin_group_get_id },
	{ "get_name", plugin_group_get_name },
	{ "set_name", plugin_group_set_name },
	{ "get_parent", plugin_group_get_parent },
	{ "set_parent", plugin_group_set_parent },
	{ "get_position_x", plugin_group_get_position_x },
	{ "get_position_y", plugin_group_get_position_y },
	{ "set_position", plugin_group_set_position },
	{ "get_plugins", plugin_group_get_plugins },
	{ NULL, NULL }
};

static int plugin_group_iterator_next(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroupIterator:next");
		return 0;
	}
	zzub_plugin_group_iterator_t* self = (zzub_plugin_group_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroupIterator:next");
		return 0;
	}

	zzub_plugin_group_iterator_next(self);
	return 0;
}

static int plugin_group_iterator_valid(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroupIterator:valid");
		return 0;
	}
	zzub_plugin_group_iterator_t* self = (zzub_plugin_group_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroupIterator:valid");
		return 0;
	}

	int _luaresult = zzub_plugin_group_iterator_valid(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int plugin_group_iterator_current(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroupIterator:current");
		return 0;
	}
	zzub_plugin_group_iterator_t* self = (zzub_plugin_group_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroupIterator:current");
		return 0;
	}

	zzub_plugin_group_t* _luaresult = zzub_plugin_group_iterator_current(self);
	if (_luaresult != NULL)
		lua_pushlightuserdata(L, _luaresult);
	else
		lua_pushnil(L);
	return 1;
}

static int plugin_group_iterator_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for PluginGroupIterator:destroy");
		return 0;
	}
	zzub_plugin_group_iterator_t* self = (zzub_plugin_group_iterator_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for PluginGroupIterator:destroy");
		return 0;
	}

	zzub_plugin_group_iterator_destroy(self);
	return 0;
}

static const luaL_Reg plugin_group_iterator_lib[] = {
	{ "next", plugin_group_iterator_next },
	{ "valid", plugin_group_iterator_valid },
	{ "current", plugin_group_iterator_current },
	{ "destroy", plugin_group_iterator_destroy },
	{ NULL, NULL }
};

static int wave_importer_open(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for WaveImporter:open");
		return 0;
	}
	zzub_wave_importer_t* self = (zzub_wave_importer_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for WaveImporter:open");
		return 0;
	}

	const char* filename = luaL_tolstring(L, 3, &len);
	zzub_input_t* strm = (zzub_input_t*)lua_touserdata(L, 4);
	int _luaresult = zzub_wave_importer_open(self, filename, strm);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_importer_destroy(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for WaveImporter:destroy");
		return 0;
	}
	zzub_wave_importer_t* self = (zzub_wave_importer_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for WaveImporter:destroy");
		return 0;
	}

	zzub_wave_importer_destroy(self);
	return 0;
}

static int wave_importer_get_instrument_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 2) {
		luaL_error(L, "Invalid argument count for WaveImporter:get_instrument_count");
		return 0;
	}
	zzub_wave_importer_t* self = (zzub_wave_importer_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for WaveImporter:get_instrument_count");
		return 0;
	}

	int _luaresult = zzub_wave_importer_get_instrument_count(self);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_importer_get_instrument_name(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for WaveImporter:get_instrument_name");
		return 0;
	}
	zzub_wave_importer_t* self = (zzub_wave_importer_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for WaveImporter:get_instrument_name");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	const char* _luaresult = zzub_wave_importer_get_instrument_name(self, index);
	lua_pushstring(L, _luaresult);
	return 1;
}

static int wave_importer_get_instrument_sample_count(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 3) {
		luaL_error(L, "Invalid argument count for WaveImporter:get_instrument_sample_count");
		return 0;
	}
	zzub_wave_importer_t* self = (zzub_wave_importer_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for WaveImporter:get_instrument_sample_count");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int _luaresult = zzub_wave_importer_get_instrument_sample_count(self, index);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_importer_get_instrument_sample_info(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 10) {
		luaL_error(L, "Invalid argument count for WaveImporter:get_instrument_sample_info");
		return 0;
	}
	zzub_wave_importer_t* self = (zzub_wave_importer_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for WaveImporter:get_instrument_sample_info");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int sample = lua_tonumber(L, 4);
	char* name;
	int namelen = lua_tonumber(L, 6);
	int samplecount;
	int channels;
	int format;
	int samplerate;
	zzub_wave_importer_get_instrument_sample_info(self, index, sample, name, namelen, &samplecount, &channels, &format, &samplerate);
	return 0;
}

static int wave_importer_load_instrument(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 4) {
		luaL_error(L, "Invalid argument count for WaveImporter:load_instrument");
		return 0;
	}
	zzub_wave_importer_t* self = (zzub_wave_importer_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for WaveImporter:load_instrument");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	zzub_wave_t* dest = (zzub_wave_t*)lua_touserdata(L, 4);
	int _luaresult = zzub_wave_importer_load_instrument(self, index, dest);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static int wave_importer_load_instrument_sample(lua_State *L) {
	size_t len;
	int argc = lua_gettop(L);
	if (argc != 5) {
		luaL_error(L, "Invalid argument count for WaveImporter:load_instrument_sample");
		return 0;
	}
	zzub_wave_importer_t* self = (zzub_wave_importer_t*)lua_touserdata(L, 2);
	if (self == 0) {
		luaL_error(L, "Null self argument for WaveImporter:load_instrument_sample");
		return 0;
	}

	int index = lua_tonumber(L, 3);
	int sample = lua_tonumber(L, 4);
	zzub_wavelevel_t* dest = (zzub_wavelevel_t*)lua_touserdata(L, 5);
	int _luaresult = zzub_wave_importer_load_instrument_sample(self, index, sample, dest);
	lua_pushnumber(L, _luaresult);
	return 1;
}

static const luaL_Reg wave_importer_lib[] = {
	{ "open", wave_importer_open },
	{ "destroy", wave_importer_destroy },
	{ "get_instrument_count", wave_importer_get_instrument_count },
	{ "get_instrument_name", wave_importer_get_instrument_name },
	{ "get_instrument_sample_count", wave_importer_get_instrument_sample_count },
	{ "get_instrument_sample_info", wave_importer_get_instrument_sample_info },
	{ "load_instrument", wave_importer_load_instrument },
	{ "load_instrument_sample", wave_importer_load_instrument_sample },
	{ NULL, NULL }
};

int luaopen_zzub(lua_State* L) {
	lua_newtable(L);
	lua_pushliteral(L, "double_click"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "update_song"); lua_pushnumber(L, 60); lua_settable(L, -3);
	lua_pushliteral(L, "insert_plugin"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "before_delete_plugin"); lua_pushnumber(L, 5); lua_settable(L, -3);
	lua_pushliteral(L, "delete_plugin"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "update_plugin"); lua_pushnumber(L, 30); lua_settable(L, -3);
	lua_pushliteral(L, "insert_connection"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_pushliteral(L, "delete_connection"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_pushliteral(L, "update_connection"); lua_pushnumber(L, 68); lua_settable(L, -3);
	lua_pushliteral(L, "update_pluginparameter"); lua_pushnumber(L, 7); lua_settable(L, -3);
	lua_pushliteral(L, "insert_pattern"); lua_pushnumber(L, 25); lua_settable(L, -3);
	lua_pushliteral(L, "update_pattern"); lua_pushnumber(L, 31); lua_settable(L, -3);
	lua_pushliteral(L, "delete_pattern"); lua_pushnumber(L, 26); lua_settable(L, -3);
	lua_pushliteral(L, "insert_patternevent"); lua_pushnumber(L, 27); lua_settable(L, -3);
	lua_pushliteral(L, "update_patternevent"); lua_pushnumber(L, 48); lua_settable(L, -3);
	lua_pushliteral(L, "delete_patternevent"); lua_pushnumber(L, 49); lua_settable(L, -3);
	lua_pushliteral(L, "insert_orderlist"); lua_pushnumber(L, 32); lua_settable(L, -3);
	lua_pushliteral(L, "delete_orderlist"); lua_pushnumber(L, 33); lua_settable(L, -3);
	lua_pushliteral(L, "update_orderlist"); lua_pushnumber(L, 41); lua_settable(L, -3);
	lua_pushliteral(L, "insert_patternformat"); lua_pushnumber(L, 52); lua_settable(L, -3);
	lua_pushliteral(L, "update_patternformat"); lua_pushnumber(L, 61); lua_settable(L, -3);
	lua_pushliteral(L, "delete_patternformat"); lua_pushnumber(L, 53); lua_settable(L, -3);
	lua_pushliteral(L, "insert_patternformatcolumn"); lua_pushnumber(L, 54); lua_settable(L, -3);
	lua_pushliteral(L, "update_patternformatcolumn"); lua_pushnumber(L, 62); lua_settable(L, -3);
	lua_pushliteral(L, "delete_patternformatcolumn"); lua_pushnumber(L, 55); lua_settable(L, -3);
	lua_pushliteral(L, "insert_patternformattrack"); lua_pushnumber(L, 63); lua_settable(L, -3);
	lua_pushliteral(L, "update_patternformattrack"); lua_pushnumber(L, 64); lua_settable(L, -3);
	lua_pushliteral(L, "delete_patternformattrack"); lua_pushnumber(L, 65); lua_settable(L, -3);
	lua_pushliteral(L, "insert_plugin_group"); lua_pushnumber(L, 71); lua_settable(L, -3);
	lua_pushliteral(L, "update_plugin_group"); lua_pushnumber(L, 72); lua_settable(L, -3);
	lua_pushliteral(L, "delete_plugin_group"); lua_pushnumber(L, 73); lua_settable(L, -3);
	lua_pushliteral(L, "envelope_changed"); lua_pushnumber(L, 37); lua_settable(L, -3);
	lua_pushliteral(L, "slices_changed"); lua_pushnumber(L, 38); lua_settable(L, -3);
	lua_pushliteral(L, "insert_wave"); lua_pushnumber(L, 56); lua_settable(L, -3);
	lua_pushliteral(L, "update_wave"); lua_pushnumber(L, 39); lua_settable(L, -3);
	lua_pushliteral(L, "delete_wave"); lua_pushnumber(L, 40); lua_settable(L, -3);
	lua_pushliteral(L, "insert_wavelevel"); lua_pushnumber(L, 12); lua_settable(L, -3);
	lua_pushliteral(L, "update_wavelevel"); lua_pushnumber(L, 57); lua_settable(L, -3);
	lua_pushliteral(L, "delete_wavelevel"); lua_pushnumber(L, 58); lua_settable(L, -3);
	lua_pushliteral(L, "update_wavelevel_samples"); lua_pushnumber(L, 59); lua_settable(L, -3);
	lua_pushliteral(L, "user_alert"); lua_pushnumber(L, 8); lua_settable(L, -3);
	lua_pushliteral(L, "midi_control"); lua_pushnumber(L, 11); lua_settable(L, -3);
	lua_pushliteral(L, "player_state_changed"); lua_pushnumber(L, 20); lua_settable(L, -3);
	lua_pushliteral(L, "osc_message"); lua_pushnumber(L, 21); lua_settable(L, -3);
	lua_pushliteral(L, "vu"); lua_pushnumber(L, 22); lua_settable(L, -3);
	lua_pushliteral(L, "player_order_changed"); lua_pushnumber(L, 69); lua_settable(L, -3);
	lua_pushliteral(L, "player_order_queue_changed"); lua_pushnumber(L, 70); lua_settable(L, -3);
	lua_pushliteral(L, "custom"); lua_pushnumber(L, 44); lua_settable(L, -3);
	lua_pushliteral(L, "samplerate_changed"); lua_pushnumber(L, 50); lua_settable(L, -3);
	lua_pushliteral(L, "latency_changed"); lua_pushnumber(L, 76); lua_settable(L, -3);
	lua_pushliteral(L, "device_reset"); lua_pushnumber(L, 77); lua_settable(L, -3);
	lua_pushliteral(L, "barrier"); lua_pushnumber(L, 51); lua_settable(L, -3);
	lua_pushliteral(L, "player_save"); lua_pushnumber(L, 74); lua_settable(L, -3);
	lua_pushliteral(L, "player_load"); lua_pushnumber(L, 75); lua_settable(L, -3);
	lua_pushliteral(L, "all"); lua_pushnumber(L, 255); lua_settable(L, -3);
	lua_setglobal(L, "zzub_event_type");

	lua_newtable(L);
	lua_pushliteral(L, "EnumeratingPlugins"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "EnumeratingPluginsDone"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "MixdownProgress"); lua_pushnumber(L, 20); lua_settable(L, -3);
	lua_pushliteral(L, "LoadingPlugins"); lua_pushnumber(L, 100); lua_settable(L, -3);
	lua_pushliteral(L, "LoadingPatterns"); lua_pushnumber(L, 101); lua_settable(L, -3);
	lua_pushliteral(L, "LoadingWaves"); lua_pushnumber(L, 102); lua_settable(L, -3);
	lua_pushliteral(L, "PatternRecursion"); lua_pushnumber(L, 400); lua_settable(L, -3);
	lua_setglobal(L, "zzub_alert_type");

	lua_newtable(L);
	lua_pushliteral(L, "parameter_count_mismatch"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "parameter_type_mismatch"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "parameter_flags_mismatch"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_pushliteral(L, "parameter_value_min_mismatch"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_pushliteral(L, "parameter_value_max_mismatch"); lua_pushnumber(L, 5); lua_settable(L, -3);
	lua_pushliteral(L, "parameter_value_none_mismatch"); lua_pushnumber(L, 6); lua_settable(L, -3);
	lua_pushliteral(L, "parameter_value_default_mismatch"); lua_pushnumber(L, 7); lua_settable(L, -3);
	lua_pushliteral(L, "plugin_not_found_using_dummy"); lua_pushnumber(L, 8); lua_settable(L, -3);
	lua_pushliteral(L, "plugin_validation_failed_using_dummy"); lua_pushnumber(L, 9); lua_settable(L, -3);
	lua_pushliteral(L, "plugin_not_found"); lua_pushnumber(L, 10); lua_settable(L, -3);
	lua_pushliteral(L, "plugin_inputs_mismatch"); lua_pushnumber(L, 11); lua_settable(L, -3);
	lua_pushliteral(L, "plugin_outputs_mismatch"); lua_pushnumber(L, 12); lua_settable(L, -3);
	lua_setglobal(L, "zzub_validation_error_type");

	lua_newtable(L);
	lua_pushliteral(L, "wave_file"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "wave_archive"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "instrument_file"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "instrument_archive"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_setglobal(L, "zzub_wave_importer_type");

	lua_newtable(L);
	lua_pushliteral(L, "version"); lua_pushnumber(L, 15); lua_settable(L, -3);
	lua_pushliteral(L, "buffer_size"); lua_pushnumber(L, 256); lua_settable(L, -3);
	lua_setglobal(L, "zzub");

	lua_newtable(L);
	lua_pushliteral(L, "playing"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "stopped"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "muted"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "released"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_setglobal(L, "zzub_player_state");

	lua_newtable(L);
	lua_pushliteral(L, "note"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "switch"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "byte"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "word"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_pushliteral(L, "meta"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_setglobal(L, "zzub_parameter_type");

	lua_newtable(L);
	lua_pushliteral(L, "si16"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "f32"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "si32"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "si24"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_setglobal(L, "zzub_wave_buffer_type");

	lua_newtable(L);
	lua_pushliteral(L, "sine"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "sawtooth"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "pulse"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "triangle"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_pushliteral(L, "noise"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_pushliteral(L, "sawtooth_303"); lua_pushnumber(L, 5); lua_settable(L, -3);
	lua_setglobal(L, "zzub_oscillator_type");

	lua_newtable(L);
	lua_pushliteral(L, "none"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "off"); lua_pushnumber(L, 255); lua_settable(L, -3);
	lua_pushliteral(L, "cut"); lua_pushnumber(L, 254); lua_settable(L, -3);
	lua_pushliteral(L, "min"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "max"); lua_pushnumber(L, 156); lua_settable(L, -3);
	lua_pushliteral(L, "c4"); lua_pushnumber(L, 65); lua_settable(L, -3);
	lua_setglobal(L, "zzub_note_value");

	lua_newtable(L);
	lua_pushliteral(L, "none"); lua_pushnumber(L, 255); lua_settable(L, -3);
	lua_pushliteral(L, "off"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "on"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_setglobal(L, "zzub_switch_value");

	lua_newtable(L);
	lua_pushliteral(L, "none"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "min"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "max"); lua_pushnumber(L, 200); lua_settable(L, -3);
	lua_setglobal(L, "zzub_wavetable_index_value");

	lua_newtable(L);
	lua_pushliteral(L, "wavetable_index"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "state"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "event_on_edit"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_pushliteral(L, "pattern_index"); lua_pushnumber(L, 8); lua_settable(L, -3);
	lua_pushliteral(L, "velocity_index"); lua_pushnumber(L, 16); lua_settable(L, -3);
	lua_pushliteral(L, "delay_index"); lua_pushnumber(L, 32); lua_settable(L, -3);
	lua_pushliteral(L, "compound"); lua_pushnumber(L, 64); lua_settable(L, -3);
	lua_pushliteral(L, "char_index"); lua_pushnumber(L, 128); lua_settable(L, -3);
	lua_pushliteral(L, "harmony_index"); lua_pushnumber(L, 256); lua_settable(L, -3);
	lua_pushliteral(L, "meta_note"); lua_pushnumber(L, 512); lua_settable(L, -3);
	lua_pushliteral(L, "meta_wave"); lua_pushnumber(L, 1024); lua_settable(L, -3);
	lua_setglobal(L, "zzub_parameter_flag");

	lua_newtable(L);
	lua_pushliteral(L, "plays_waves"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "uses_lib_interface"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_pushliteral(L, "does_input_mixing"); lua_pushnumber(L, 16); lua_settable(L, -3);
	lua_pushliteral(L, "is_singleton"); lua_pushnumber(L, 32768); lua_settable(L, -3);
	lua_pushliteral(L, "is_root"); lua_pushnumber(L, 65536); lua_settable(L, -3);
	lua_pushliteral(L, "has_audio_input"); lua_pushnumber(L, 131072); lua_settable(L, -3);
	lua_pushliteral(L, "has_audio_output"); lua_pushnumber(L, 262144); lua_settable(L, -3);
	lua_pushliteral(L, "is_offline"); lua_pushnumber(L, 524288); lua_settable(L, -3);
	lua_pushliteral(L, "has_event_output"); lua_pushnumber(L, 1048576); lua_settable(L, -3);
	lua_pushliteral(L, "stream"); lua_pushnumber(L, 4194304); lua_settable(L, -3);
	lua_pushliteral(L, "has_midi_input"); lua_pushnumber(L, 8388608); lua_settable(L, -3);
	lua_pushliteral(L, "has_midi_output"); lua_pushnumber(L, 16777216); lua_settable(L, -3);
	lua_pushliteral(L, "has_group_input"); lua_pushnumber(L, 33554432); lua_settable(L, -3);
	lua_pushliteral(L, "has_group_output"); lua_pushnumber(L, 67108864); lua_settable(L, -3);
	lua_pushliteral(L, "is_sequence"); lua_pushnumber(L, 134217728); lua_settable(L, -3);
	lua_pushliteral(L, "is_connection"); lua_pushnumber(L, 268435456); lua_settable(L, -3);
	lua_pushliteral(L, "is_interval"); lua_pushnumber(L, 536870912); lua_settable(L, -3);
	lua_pushliteral(L, "is_encoder"); lua_pushnumber(L, 1073741824); lua_settable(L, -3);
	lua_pushliteral(L, "has_note_output"); lua_pushnumber(L, -2147483648); lua_settable(L, -3);
	lua_setglobal(L, "zzub_plugin_flag");

	lua_newtable(L);
	lua_pushliteral(L, "loop"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "extended"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_pushliteral(L, "stereo"); lua_pushnumber(L, 8); lua_settable(L, -3);
	lua_pushliteral(L, "pingpong"); lua_pushnumber(L, 16); lua_settable(L, -3);
	lua_pushliteral(L, "envelope"); lua_pushnumber(L, 128); lua_settable(L, -3);
	lua_setglobal(L, "zzub_wave_flag");

	lua_newtable(L);
	lua_pushliteral(L, "sustain"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "loop"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_setglobal(L, "zzub_envelope_flag");

	lua_newtable(L);
	lua_pushliteral(L, "no_io"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "read"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "write"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "read_write"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_setglobal(L, "zzub_process_mode");

	lua_newtable(L);
	lua_pushliteral(L, "playing"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "stopped"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "deleted"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_pushliteral(L, "seeking"); lua_pushnumber(L, 8); lua_settable(L, -3);
	lua_pushliteral(L, "created"); lua_pushnumber(L, 16); lua_settable(L, -3);
	lua_setglobal(L, "zzub_encoder_state");

	lua_newtable(L);
	lua_pushliteral(L, "audio"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "event"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "midi"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "note"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_setglobal(L, "zzub_connection_type");

	lua_newtable(L);
	lua_pushliteral(L, "internal"); lua_pushnumber(L, 0); lua_settable(L, -3);
	lua_pushliteral(L, "global"); lua_pushnumber(L, 1); lua_settable(L, -3);
	lua_pushliteral(L, "track"); lua_pushnumber(L, 2); lua_settable(L, -3);
	lua_pushliteral(L, "controller"); lua_pushnumber(L, 3); lua_settable(L, -3);
	lua_pushliteral(L, "virtual"); lua_pushnumber(L, 4); lua_settable(L, -3);
	lua_setglobal(L, "zzub_parameter_group");

	luaL_newlib(L, double_click_lib);
	lua_setglobal(L, "zzub_event_data_double_click");

	luaL_newlib(L, insert_plugin_lib);
	lua_setglobal(L, "zzub_event_data_insert_plugin");

	luaL_newlib(L, delete_plugin_lib);
	lua_setglobal(L, "zzub_event_data_delete_plugin");

	luaL_newlib(L, update_plugin_lib);
	lua_setglobal(L, "zzub_event_data_update_plugin");

	luaL_newlib(L, insert_connection_lib);
	lua_setglobal(L, "zzub_event_data_insert_connection");

	luaL_newlib(L, update_connection_lib);
	lua_setglobal(L, "zzub_event_data_update_connection");

	luaL_newlib(L, delete_connection_lib);
	lua_setglobal(L, "zzub_event_data_delete_connection");

	luaL_newlib(L, insert_pattern_lib);
	lua_setglobal(L, "zzub_event_data_insert_pattern");

	luaL_newlib(L, update_pattern_lib);
	lua_setglobal(L, "zzub_event_data_update_pattern");

	luaL_newlib(L, delete_pattern_lib);
	lua_setglobal(L, "zzub_event_data_delete_pattern");

	luaL_newlib(L, insert_pattern_event_lib);
	lua_setglobal(L, "zzub_event_data_insert_pattern_event");

	luaL_newlib(L, update_pattern_event_lib);
	lua_setglobal(L, "zzub_event_data_update_pattern_event");

	luaL_newlib(L, delete_pattern_event_lib);
	lua_setglobal(L, "zzub_event_data_delete_pattern_event");

	luaL_newlib(L, insert_pattern_format_lib);
	lua_setglobal(L, "zzub_event_data_insert_pattern_format");

	luaL_newlib(L, update_pattern_format_lib);
	lua_setglobal(L, "zzub_event_data_update_pattern_format");

	luaL_newlib(L, delete_pattern_format_lib);
	lua_setglobal(L, "zzub_event_data_delete_pattern_format");

	luaL_newlib(L, insert_pattern_format_column_lib);
	lua_setglobal(L, "zzub_event_data_insert_pattern_format_column");

	luaL_newlib(L, update_pattern_format_column_lib);
	lua_setglobal(L, "zzub_event_data_update_pattern_format_column");

	luaL_newlib(L, delete_pattern_format_column_lib);
	lua_setglobal(L, "zzub_event_data_delete_pattern_format_column");

	luaL_newlib(L, midi_message_lib);
	lua_setglobal(L, "zzub_event_data_midi_message");

	luaL_newlib(L, update_plugin_parameter_lib);
	lua_setglobal(L, "zzub_event_data_update_plugin_parameter");

	luaL_newlib(L, player_state_changed_lib);
	lua_setglobal(L, "zzub_event_data_player_state_changed");

	luaL_newlib(L, player_order_changed_lib);
	lua_setglobal(L, "zzub_event_data_player_order_changed");

	luaL_newlib(L, player_load_lib);
	lua_setglobal(L, "zzub_event_data_player_load");

	luaL_newlib(L, player_save_lib);
	lua_setglobal(L, "zzub_event_data_player_save");

	luaL_newlib(L, vu_lib);
	lua_setglobal(L, "zzub_event_data_vu");

	luaL_newlib(L, serialize_lib);
	lua_setglobal(L, "zzub_event_data_serialize");

	luaL_newlib(L, unknown_lib);
	lua_setglobal(L, "zzub_event_data_unknown");

	luaL_newlib(L, osc_message_lib);
	lua_setglobal(L, "zzub_event_data_osc_message");

	luaL_newlib(L, insert_wave_lib);
	lua_setglobal(L, "zzub_event_data_insert_wave");

	luaL_newlib(L, update_wave_lib);
	lua_setglobal(L, "zzub_event_data_update_wave");

	luaL_newlib(L, delete_wave_lib);
	lua_setglobal(L, "zzub_event_data_delete_wave");

	luaL_newlib(L, insert_wavelevel_lib);
	lua_setglobal(L, "zzub_event_data_insert_wavelevel");

	luaL_newlib(L, update_wavelevel_lib);
	lua_setglobal(L, "zzub_event_data_update_wavelevel");

	luaL_newlib(L, update_wavelevel_samples_lib);
	lua_setglobal(L, "zzub_event_data_update_wavelevel_samples");

	luaL_newlib(L, delete_wavelevel_lib);
	lua_setglobal(L, "zzub_event_data_delete_wavelevel");

	luaL_newlib(L, insert_plugin_group_lib);
	lua_setglobal(L, "zzub_event_data_insert_plugin_group");

	luaL_newlib(L, delete_plugin_group_lib);
	lua_setglobal(L, "zzub_event_data_delete_plugin_group");

	luaL_newlib(L, update_plugin_group_lib);
	lua_setglobal(L, "zzub_event_data_update_plugin_group");

	luaL_newlib(L, custom_lib);
	lua_setglobal(L, "zzub_event_data_custom");

	luaL_newlib(L, user_alert_lib);
	lua_setglobal(L, "zzub_event_data_user_alert");

	luaL_newlib(L, event_data_lib);
	lua_setglobal(L, "zzub_event_data");

	luaL_newlib(L, device_info_lib);
	lua_setglobal(L, "zzub_device_info");

	luaL_newlib(L, device_info_iterator_lib);
	lua_setglobal(L, "zzub_device_info_iterator");

	luaL_newlib(L, audiodriver_lib);
	lua_setglobal(L, "zzub_audiodriver");

	luaL_newlib(L, mididriver_lib);
	lua_setglobal(L, "zzub_mididriver");

	luaL_newlib(L, plugincollection_lib);
	lua_setglobal(L, "zzub_plugincollection");

	luaL_newlib(L, input_lib);
	lua_setglobal(L, "zzub_input");

	luaL_newlib(L, output_lib);
	lua_setglobal(L, "zzub_output");

	luaL_newlib(L, archive_lib);
	lua_setglobal(L, "zzub_archive");

	luaL_newlib(L, midimapping_lib);
	lua_setglobal(L, "zzub_midimapping");

	luaL_newlib(L, pattern_event_lib);
	lua_setglobal(L, "zzub_pattern_event");

	luaL_newlib(L, pattern_iterator_lib);
	lua_setglobal(L, "zzub_pattern_iterator");

	luaL_newlib(L, pattern_event_iterator_lib);
	lua_setglobal(L, "zzub_pattern_event_iterator");

	luaL_newlib(L, pattern_lib);
	lua_setglobal(L, "zzub_pattern");

	luaL_newlib(L, pattern_format_lib);
	lua_setglobal(L, "zzub_pattern_format");

	luaL_newlib(L, pattern_format_iterator_lib);
	lua_setglobal(L, "zzub_pattern_format_iterator");

	luaL_newlib(L, pattern_format_column_lib);
	lua_setglobal(L, "zzub_pattern_format_column");

	luaL_newlib(L, pattern_format_column_iterator_lib);
	lua_setglobal(L, "zzub_pattern_format_column_iterator");

	luaL_newlib(L, parameter_lib);
	lua_setglobal(L, "zzub_parameter");

	luaL_newlib(L, attribute_lib);
	lua_setglobal(L, "zzub_attribute");

	luaL_newlib(L, pluginloader_lib);
	lua_setglobal(L, "zzub_pluginloader");

	luaL_newlib(L, plugin_lib);
	lua_setglobal(L, "zzub_plugin");

	luaL_newlib(L, plugin_iterator_lib);
	lua_setglobal(L, "zzub_plugin_iterator");

	luaL_newlib(L, connection_lib);
	lua_setglobal(L, "zzub_connection");

	luaL_newlib(L, connection_binding_lib);
	lua_setglobal(L, "zzub_connection_binding");

	luaL_newlib(L, connection_binding_iterator_lib);
	lua_setglobal(L, "zzub_connection_binding_iterator");

	luaL_newlib(L, wave_lib);
	lua_setglobal(L, "zzub_wave");

	luaL_newlib(L, wavelevel_lib);
	lua_setglobal(L, "zzub_wavelevel");

	luaL_newlib(L, envelope_lib);
	lua_setglobal(L, "zzub_envelope");

	luaL_newlib(L, validation_error_iterator_lib);
	lua_setglobal(L, "zzub_validation_error_iterator");

	luaL_newlib(L, validation_error_lib);
	lua_setglobal(L, "zzub_validation_error");

	luaL_newlib(L, player_lib);
	lua_setglobal(L, "zzub_player");

	luaL_newlib(L, plugin_group_lib);
	lua_setglobal(L, "zzub_plugin_group");

	luaL_newlib(L, plugin_group_iterator_lib);
	lua_setglobal(L, "zzub_plugin_group_iterator");

	luaL_newlib(L, wave_importer_lib);
	lua_setglobal(L, "zzub_wave_importer");

	return 1;
}
void luaclose_zzub(lua_State* L) {
	zzub_callback_clear(L);
}
