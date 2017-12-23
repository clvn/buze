#pragma once

// taken from comment on
// http://www.codeproject.com/string/stringsplit.asp?df=100&forumid=2167&exp=0&select=1062827#xx1062827xx
template<typename _Cont>
inline void split(const std::string& str, _Cont& _container, const std::string& delim = ",") {
    std::string::size_type lpos = 0;
    std::string::size_type pos = str.find_first_of(delim, lpos);
    while(lpos != std::string::npos) {
        _container.insert(_container.end(), str.substr(lpos, pos - lpos));

        lpos = (pos == std::string::npos) ? std::string::npos : pos + 1;
        pos = str.find_first_of(delim, lpos);
    }
}
/*	split example:
		vector<string> subcommands;
		split<vector<string> > (commandString, subcommands, "\n");
*/
// ---

// found the trims in one of the comments at
// http://www.codeproject.com/vcpp/stl/stdstringtrim.asp

inline std::string& trimleft(std::string& s) {
	std::string::iterator it;

	for (it = s.begin(); it != s.end(); ++it)
		if (!isspace((unsigned char)*it))
			break;

	s.erase(s.begin(), it);
	return s;
}

inline std::string& trimright(std::string& s) {
	std::string::difference_type dt;
	std::string::reverse_iterator it;

	for (it = s.rbegin(); it != s.rend(); ++it)
		if (!isspace((unsigned char)*it))
			break;

	dt = s.rend() - it;

	s.erase(s.begin() + dt, s.end());
	return s;
}

inline std::string& trim(std::string& s) {
	trimleft(s);
	trimright(s);
	return s;
}

inline std::string trim(const std::string& s) {
	std::string t = s;
	return trim(t);
}
