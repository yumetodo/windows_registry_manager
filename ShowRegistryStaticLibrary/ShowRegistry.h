#pragma once
#include <Windows.h>
#include <string>

class RegistryRead {
private:
	HKEY key;
	DWORD dwByte;
	std::string get_last_error();
#ifdef UNICODE
	std::wstring buf;
#else
	std::string buf;
#endif
public:
	RegistryRead(HKEY parent_key_handle, const TCHAR* sub_key_root);
	~RegistryRead();
	DWORD dwType;
	void read(const TCHAR* key_name);
#ifdef UNICODE
	std::wstring& get_data();
#else
	std::string& get_data();
#endif
};
