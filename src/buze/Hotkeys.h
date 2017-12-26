#pragma once

#include <boost/unordered_map.hpp>

/// Todo: this needs to be moved
#include <json/json.h>

// ---------------------------------------------------------------------------------------------------------------

struct vkey_desc_t {
	char const* name;
	WORD vkey;
};

struct accel_default_t {
	std::vector<std::string> hotkeys;
	std::string action;
};

class CHotkeys
{
  public:

	std::map<std::string, std::vector<accel_default_t> > accel_default_map;
	std::map<std::string, HACCEL> accel_map;

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
	void Initialize();
	void RegisterAcceleratorId(const std::string& view, const std::string& name, WORD id, const std::string& hotkey);

	void InitMaps();
	std::string GetPath() const;
	bool ParseJson();
	bool ParseViewAccels(Json::Value& root, std::string const& view);
	void ShowHotkeyError(std::string const& view, std::string const& name);

	// accels
	CreateAccelResult CreateAccel(std::string const& name, std::string const& hotkey, ACCEL& acc, const id_map_t& id_map);
	int CreateStringAccel(std::string const& view, std::string const& name, std::string const& hotkey, accel_vec_t& accel_vec, accel_reverse_map_t& accel_reverse_map, id_map_t const& id_map);
	void CreateAccelTables();
	void CreateAccelTable(std::string const& view, HACCEL& hAccel);
	void DestroyAccelTables();
	void Reload();
	HACCEL GetAccelerators(const std::string& view);

	// keyjazz
	bool CreateKeyjazzKeyMap(Json::Value& root);
	bool CreateKeyjazzKey(std::string const& name, std::string const& key);

	// menu updates
	void UpdateMenuKeys(std::string const& view, HMENU hMenu, bool show_accels);
	void UpdateMenuKeys(accel_reverse_map_t const& accel_reverse_map, HMENU hMenu, bool show_accels);
	void AddMenuKeys(accel_reverse_map_t const& accel_reverse_map, HMENU hMenu);
	void AddMenuKeys(std::string const& view, HMENU hMenu);
};
