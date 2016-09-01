#include "win32_reg/registry_key.hpp"
#include <iostream>
#include <locale>
#include <tchar.h>
#include <memory>
int GetFileVersion(const TCHAR* file)
{
	ULONG reserved = 0;
	VS_FIXEDFILEINFO vffi;
	TCHAR *buf = NULL;
	int  Locale = 0;
	TCHAR str[256];
	str[0] = '\0';

	UINT size = GetFileVersionInfoSize((TCHAR*)file, &reserved);
	auto vbuf = std::make_unique<TCHAR[]>(size);
	if (GetFileVersionInfo((TCHAR*)file, 0, size, vbuf.get()))
	{
		VerQueryValue(vbuf.get(), _T("\\"), (void**)&buf, &size);
		CopyMemory(&vffi, buf, sizeof(VS_FIXEDFILEINFO));

		VerQueryValue(vbuf.get(), _T("\\VarFileInfo\\Translation"), (void**)&buf, &size);
		CopyMemory(&Locale, buf, sizeof(int));
		wsprintf(str, _T("\\StringFileInfo\\%04X%04X\\%s"), LOWORD(Locale), HIWORD(Locale), _T("FileVersion"));
		VerQueryValue(vbuf.get(), str, (void**)&buf, &size);

		_tcscpy_s(str, 256, buf);
	}
	if (_tcscmp(str, _T("")) != 0){
		return int(std::stof(str) * 100);
	}
	else{
		return 0;
	}
}
int GetIeVersion()
{
	switch (GetFileVersion(_T("Shdocvw.dll")))
	{
		case 470: return 300;
		case 471: return 400;
		case 472: return 401;
		case 500: return 500;
		case 550: return 550;
		case 600:
		default: {
			int ieVersion = 600;
			win32::registry_key reg(
				win32::registry_hive::local_machine, 
				_T("SOFTWARE\\Microsoft\\Internet Explorer"), 
				win32::registry_rights::read_key
			);
			ieVersion = std::stoi(reg.get_value<win32::registry_value_kind::string>(_T("Version"))) * 100;
			if (ieVersion == 900) {
				ieVersion = std::stoi(reg.get_value<win32::registry_value_kind::string>(_T("svcVersion"))) * 100;
			}
			return ieVersion;
		}
	}
}

bool IsDotNet2()
{
	win32::registry_key reg;
	try {
		reg.open(
			win32::registry_hive::local_machine,
			_T("Software\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727"),
			win32::registry_rights::read_key
		);
	}
	catch (...) {
		reg.open(
			win32::registry_hive::local_machine,
			_T("Software\\Wow6432Node\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727"),
			win32::registry_rights::read_key
		);
	}
	return 1 == reg.get_value<win32::registry_value_kind::dword>(_T("Install"));
}

BOOL IsDotNet4()
{
	win32::registry_key reg;
	try {
		reg.open(
			win32::registry_hive::local_machine,
			_T("Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Client"),
			win32::registry_rights::read_key
		);
	}
	catch (...) {
		reg.open(
			win32::registry_hive::local_machine,
			_T("Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full"),
			win32::registry_rights::read_key
		);
	}
	return 1 == reg.get_value<win32::registry_value_kind::dword>(_T("Install"));
}
#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

int main() {
	try {
		win32::registry_key reg(
			win32::registry_hive::current_user, 
			_T(R"(Software\Microsoft\Windows\CurrentVersion\Explorer)"),
			win32::registry_rights::read_key
		);
#ifdef UNICODE
		std::wcout.imbue(std::locale(""));
#endif
		for (auto&& s : reg.get_sub_key_names()) {
			tcout << s << std::endl;
		}
		win32::registry_key reg2(reg, _T("User Shell Folders"), win32::registry_rights::read_key);
		tcout << reg2.get_value<win32::registry_value_kind::expand_string>(_T("Personal")) << std::endl << std::endl;
		for (auto&& s : reg.get_value_names()) {
			tcout << s << std::endl;
		}
		reg.close();
		tcout 
			<< std::endl
			<< _T("ie version:") << GetIeVersion() << std::endl
			<< _T("IsDotNet2:") << IsDotNet2() << std::endl
			<< _T("IsDotNet4:") << IsDotNet4() << std::endl;
		return 0;
	}
	catch (std::exception &er) {
		std::cout << er.what() << std::endl;
	}
}
