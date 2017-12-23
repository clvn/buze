#pragma once

#include <boost/unordered_map.hpp>

/// Todo: this needs to be moved
#include "../armstrong/src/plugins/hw2zzub/jsoncpp/include/json/json.h"

// ---------------------------------------------------------------------------------------------------------------

struct id_desc_t {
	char const* name;
	WORD id;
};

struct vkey_desc_t {
	char const* name;
	WORD vkey;
};

class CHotkeys
{
  public:

	HACCEL mainframe_hAccel;
	HACCEL patternview_hAccel;
	HACCEL machineview_hAccel;
	HACCEL wavetable_hAccel;
	HACCEL parameter_hAccel;
	HACCEL orderlist_hAccel;
	HACCEL filebrowser_hAccel;
	HACCEL properties_hAccel;
	HACCEL patternlistview_hAccel;
	HACCEL patternformatview_hAccel;

	typedef boost::unordered_map<WORD, ACCEL*> accel_reverse_map_t;
	typedef boost::unordered_map<std::string, accel_reverse_map_t> view_accel_reverse_maps_t;
	view_accel_reverse_maps_t view_accel_reverse_maps;

	typedef std::vector<ACCEL> accel_vec_t;
	typedef boost::unordered_map<std::string, accel_vec_t> view_accel_maps_t;
	view_accel_maps_t view_accel_maps;

	typedef boost::unordered_map<std::string, WORD> id_map_t;
	typedef boost::unordered_map<std::string, id_map_t> view_id_maps_t;
	view_id_maps_t view_id_maps;

	typedef boost::unordered_map<std::string, int> keyjazz_map_t;
	keyjazz_map_t keyjazz_map;
	typedef boost::unordered_map<WORD, int> keyjazz_key_map_t;
	keyjazz_key_map_t keyjazz_key_map;

	typedef boost::unordered_map<std::string, int> vkey_map_t;
	vkey_map_t vkey_map;

	enum CreateAccelResult {
		create_ok,
		create_skip,
		create_error,
	};

	CHotkeys();
	~CHotkeys();

	// init
	void InitMaps();
	std::string GetPath() const;
	bool ReadJson();
	void ShowHotkeyError(std::string const& view, std::string const& name);

	// accels
	template <size_t N> void BuildIdMap(std::string const& view, id_desc_t const (&id_descs)[N]);
	bool CreateViewAccels(Json::Value& root, std::string const& view);
	CreateAccelResult CreateAccel(std::string const& name, std::string const& hotkey, ACCEL& acc, id_map_t const& id_map);
	int CreateStringAccel(std::string const& view, std::string const& name, std::string const& hotkey, accel_vec_t& accel_vec, accel_reverse_map_t& accel_reverse_map, id_map_t const& id_map);
	void CreateAccelTables();
	void CreateAccelTable(std::string const& view, HACCEL& hAccel);
	void DestroyAccelTables();
	void Reload();

	// keyjazz
	bool CreateKeyjazzKeyMap(Json::Value& root);
	bool CreateKeyjazzKey(std::string const& name, std::string const& key);

	// menu updates
	void UpdateMenuKeys(std::string const& view, HMENU hMenu, bool show_accels);
	void UpdateMenuKeys(accel_reverse_map_t const& accel_reverse_map, HMENU hMenu, bool show_accels);
	void AddMenuKeys(accel_reverse_map_t const& accel_reverse_map, HMENU hMenu);
	void AddMenuKeys(std::string const& view, HMENU hMenu);
};
