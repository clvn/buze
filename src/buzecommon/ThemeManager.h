#pragma once

struct default_theme_type {
	const char* name;
	COLORREF color;
};

class ThemeManager {
public:
	static default_theme_type defaultTheme[];
	static const size_t defaultThemeCount;

	std::string currentTheme;
	std::map<std::string, COLORREF> theme;

	std::vector<std::string> themes;

	void initialize();
	size_t getThemes();
	std::string getThemeName(size_t index);
	bool loadTheme(std::string const& name);
	bool saveTheme(std::string const& name);
	COLORREF getThemeColor(std::string const& name);
	bool setThemeColor(std::string const& name, COLORREF value);
	void setDefaultOverrideColor(std::string const& dest, std::string const& src);
};
