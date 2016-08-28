#include "RegistryRead.h"
#include <iostream>
#include <locale>
#include <tchar.h>

int main() {
	try {
		RegistryRead reg(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"));
		reg.read(TEXT("Personal"));
#ifdef UNICODE
		std::wcout.imbue(std::locale(""));
		std::wcout
#else
		std::cout
#endif
			<< reg.get_data() << std::endl;
		return 0;
	}
	catch (std::exception &er) {
		std::cout << er.what() << std::endl;
	}
}
