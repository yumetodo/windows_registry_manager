#include "RegistryRead.h"
#include <cstring>
#include <codecvt>

RegistryRead::RegistryRead(HKEY parent_key_handle, const TCHAR* sub_key_root)
	: key(key), dwType(REG_SZ), dwByte(32) {
	if (ERROR_SUCCESS != RegOpenKeyEx(parent_key_handle, sub_key_root, 0, KEY_READ | KEY_WOW64_64KEY, &this->key))
		throw std::runtime_error(this->get_last_error());
}

RegistryRead::~RegistryRead() {
	RegCloseKey(this->key);
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
	if (ERROR_SUCCESS != RegQueryValueEx(this->key, key_name, 0, &this->dwType, nullptr, &this->dwByte))
		throw std::runtime_error(this->get_last_error());
	this->buf.resize(this->dwByte);
	RegQueryValueEx(this->key, key_name, 0, nullptr, (LPBYTE)&this->buf[0], &this->dwByte);
	this->buf.resize(
#ifdef UNICODE
		std::wcslen(this->buf.c_str())
#else
		std::strlen(this->buf.c_str())
#endif
	);
}

#ifdef UNICODE
std::wstring& 
#else
std::string&
#endif
RegistryRead::get_data() {
	return this->buf;
}
