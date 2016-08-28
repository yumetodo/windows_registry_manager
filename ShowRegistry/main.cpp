#include <Windows.h>
#include <iostream>
#include <string>
#include <codecvt>

class RegistryRead {
private:
	HKEY key;
	DWORD dwByte;
	std::string get_last_error();
	std::string buf;
public:
	RegistryRead(HKEY parent_key_handle, const TCHAR* sub_key_root);
	DWORD dwType;
	void read(const TCHAR* key_name);
	std::string get_data();
};

RegistryRead::RegistryRead(HKEY parent_key_handle, const TCHAR* sub_key_root)
	: key(key), dwType(REG_SZ), dwByte(32) {
	auto p = RegOpenKeyEx(parent_key_handle, sub_key_root, 0, KEY_READ | KEY_WOW64_64KEY, &this->key);
	if (p != ERROR_SUCCESS)
		throw std::runtime_error(this->get_last_error());
}

std::string RegistryRead::get_last_error() {
	char* buf = nullptr;
	const auto lasterr = GetLastError();
	const auto len = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		nullptr,
		lasterr,
		LANG_NEUTRAL,
		(LPSTR)&buf,
		0,
		nullptr
	);
	auto i = len - 3;
	for (; '\r' != buf[i] && '\n' != buf[i] && '\0' != buf[i]; i++);//改行文字削除
	buf[i] = '\0';
	std::string ret = buf + ("(" + std::to_string(lasterr)) + ")";//エラーメッセージ作成
	LocalFree(buf);//FormatMessageAでFORMAT_MESSAGE_ALLOCATE_BUFFERを指定したので必ず開放
	return ret;
}

void RegistryRead::read(const TCHAR* key_name) {
	auto p = RegQueryValueEx(this->key, key_name, 0, &this->dwType, NULL, &this->dwByte);
	if (p != ERROR_SUCCESS)
		throw std::runtime_error(this->get_last_error());
	buf.resize(this->dwByte);
	RegQueryValueEx(this->key, key_name, 0, &this->dwType, (LPBYTE)&this->buf[0], &this->dwByte);
}

std::string SJIStoUTF8(const char* szShiftJis) {
	wchar_t bufUnicode[MAX_PATH];
	char bufUTF8[MAX_PATH];
	int iLenUnicode = MultiByteToWideChar(CP_ACP, 0, szShiftJis, strlen(szShiftJis) + 1,
		NULL, 0);
	if (iLenUnicode <= sizeof(bufUnicode) / sizeof(bufUnicode[0])) {
		MultiByteToWideChar(CP_ACP, 0, szShiftJis, strlen(szShiftJis) + 1, bufUnicode, MAX_PATH);
		// 次に、UniocdeからUTF8に変換する
		// サイズを計算する
		int iLenUtf8 = WideCharToMultiByte(CP_UTF8, 0, bufUnicode, iLenUnicode, NULL, 0,
			NULL, NULL);
		if (iLenUtf8 <= sizeof(bufUTF8)) {
			WideCharToMultiByte(CP_UTF8, 0, bufUnicode, iLenUnicode, bufUTF8, sizeof(bufUTF8),
				NULL, NULL);
		}
	}
	return bufUTF8;
}

std::string RegistryRead::get_data() {

	return SJIStoUTF8(this->buf.c_str());
}

int main() {
	try {
		RegistryRead reg(HKEY_CURRENT_USER, TEXT("\"Control Panel\\Mouse\""));
		reg.read(TEXT("DoubleClickSpeed"));
		if (reg.dwType == REG_SZ) std::cout << reg.get_data() << std::endl;
		return 0;
	}
	catch (std::exception &er) {
		std::cout << er.what() << std::endl;
	}
}
