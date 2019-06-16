#include "cgss.hpp"

using namespace std;

#if defined(WIN32) || defined(_WIN32)

void MakeDirectories(const string &s) {
	CreateDirectory(s.c_str(), nullptr);
}

#else

void MakeDirectories(const string &s) {
	char str[512] = {0};

	strncpy(str, s.c_str(), 512);
	auto len = strlen(str);

	for (auto i = 0; i < len; i++) {
		if (str[i] == '/') {
			str[i] = '\0';
			if (access(str, 0) != 0) {
				mkdir(str, 0777);
			}
			str[i] = '/';
		}
	}

	if (len > 0 && access(str, 0) != 0) {
		mkdir(str, 0777);
	}
}

#endif

string GetFilePath(const string &s) {
	const char *filePath = s.c_str();
	char *realPath;

	realPath = realpath(filePath, NULL);
	const auto pos = string(realPath).rfind("/");
	if(pos == string::npos){
		return realPath;
	}

	string path = string(realPath).substr(0, pos + 1);

	return path;
}

string GetFileName(const string &s) {
	const auto dpos = s.rfind("/") + 1;
	const auto pos = s.rfind(".");
	string filename = s.substr(dpos, pos - dpos);

	return filename;
}