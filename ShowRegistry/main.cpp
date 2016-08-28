#include <Windows.h>
#include <iostream>
#include <string>
#include <cstring>
#include <codecvt>
#include <tchar.h>

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
std::wstring shift_jis_to_utf_16(const std::string& str)
{
	static_assert(sizeof(wchar_t) == 2, "this function is windows only");
	const int len = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	std::wstring re(len * 2 + 2, L'\0');
	if (!::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &re[0], len)) {
		const auto ec = ::GetLastError();
		switch (ec)
		{
			case ERROR_INSUFFICIENT_BUFFER:
				throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_INSUFFICIENT_BUFFER"); break;
			case ERROR_INVALID_FLAGS:
				throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_INVALID_FLAGS"); break;
			case ERROR_INVALID_PARAMETER:
				throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_INVALID_PARAMETER"); break;
			case ERROR_NO_UNICODE_TRANSLATION:
				throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_NO_UNICODE_TRANSLATION"); break;
			default:
				throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: unknown(" + std::to_string(ec) + ')'); break;
		}
	}
	const std::size_t real_len = std::wcslen(re.c_str());
	re.resize(real_len);
	re.shrink_to_fit();
	return re;
}
std::string SJIStoUTF8(const std::string& str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
	return cvt.to_bytes(shift_jis_to_utf_16(str));
}

std::string RegistryRead::get_data() {

	return SJIStoUTF8(this->buf.c_str());
}

int main() {
	try {
		RegistryRead reg(HKEY_CURRENT_USER, _T("Control Panel\\Mouse"));
		reg.read(TEXT("DoubleClickSpeed"));
		if (reg.dwType == REG_SZ) std::cout << reg.get_data() << std::endl;
		return 0;
	}
	catch (std::exception &er) {
		std::cout << er.what() << std::endl;
	}
}
