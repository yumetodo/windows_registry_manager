#include "win32_reg/registry_key.hpp"
#include <iostream>
#include <locale>
#include <tchar.h>
#include <memory>
using microsoft::win32::registry_key;
using microsoft::win32::registry_value_kind;
int get_file_version(const TCHAR* file)
{
	ULONG reserved = 0;
	UINT size = GetFileVersionInfoSize(file, &reserved);
	auto vbuf = std::make_unique<TCHAR[]>(size);
	if (GetFileVersionInfo(file, 0, size, vbuf.get()))
	{
		TCHAR *buf = nullptr;
		VerQueryValue(vbuf.get(), _T("\\VarFileInfo\\Translation"), reinterpret_cast<void**>(&buf), &size);
		int Locale = 0;
		CopyMemory(&Locale, buf, sizeof(int));
		TCHAR str[256];
		wsprintf(str, _T("\\StringFileInfo\\%04X%04X\\%s"), LOWORD(Locale), HIWORD(Locale), _T("FileVersion"));
		VerQueryValue(vbuf.get(), str, reinterpret_cast<void**>(&buf), &size);
		if(std::char_traits<TCHAR>::length(buf)) return int(std::stof(buf) * 100);
	}
	return 0;
}
int get_ie_version()
{
	switch (get_file_version(_T("Shdocvw.dll")))
	{
		case 470: return 300;
		case 471: return 400;
		case 472: return 401;
		case 500: return 500;
		case 550: return 550;
		case 600:
		default: {
			int ieVersion = 600;
			registry_key reg(
				microsoft::win32::registry_hive::local_machine,
				_T("SOFTWARE\\Microsoft\\Internet Explorer"), 
				system::security::registry_rights::read_key
			);
			ieVersion = std::stoi(reg.get_value<registry_value_kind::string>(_T("Version"))) * 100;
			if (ieVersion == 900) {
				ieVersion = std::stoi(reg.get_value<registry_value_kind::string>(_T("svcVersion"))) * 100;
			}
			return ieVersion;
		}
	}
}

bool is_dot_net2()
{
	registry_key reg;
	try {
		reg.open(
			microsoft::win32::registry_hive::local_machine,
			_T("Software\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727"),
			system::security::registry_rights::read_key
		);
	}
	catch (...) {
		reg.open(
			microsoft::win32::registry_hive::local_machine,
			_T("Software\\Wow6432Node\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727"),
			system::security::registry_rights::read_key
		);
	}
	return 1 == reg.get_value<registry_value_kind::dword>(_T("Install"));
}

BOOL is_dot_net4()
{
	registry_key reg;
	try {
		reg.open(
			microsoft::win32::registry_hive::local_machine,
			_T("Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Client"),
			system::security::registry_rights::read_key
		);
	}
	catch (...) {
		reg.open(
			microsoft::win32::registry_hive::local_machine,
			_T("Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full"),
			system::security::registry_rights::read_key
		);
	}
	return 1 == reg.get_value<registry_value_kind::dword>(_T("Install"));
}
#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

int main() {
	try {
		registry_key reg(
			microsoft::win32::registry_hive::current_user,
			_T(R"(Software\Microsoft\Windows\CurrentVersion\Explorer)"),
			system::security::registry_rights::read_key
		);
#ifdef UNICODE
		std::wcout.imbue(std::locale(""));
#endif
		for (auto&& s : reg.get_sub_key_names()) {
			tcout << s << std::endl;
		}
		registry_key reg2(reg, _T("User Shell Folders"), system::security::registry_rights::read_key);
		tcout << reg2.get_value<registry_value_kind::expand_string>(_T("Personal")) << std::endl << std::endl;
		for (auto&& s : reg.get_value_names()) {
			tcout << s << std::endl;
		}
		reg.close();
		tcout 
			<< std::endl
			<< _T("ie version:") << get_ie_version() << std::endl
			<< _T("is_dot_net2:") << is_dot_net2() << std::endl
			<< _T("is_dot_net4:") << is_dot_net4() << std::endl;
		return 0;
	}
	catch (std::exception &er) {
		std::cout << er.what() << std::endl;
	}
}
