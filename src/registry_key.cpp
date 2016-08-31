#if !defined(CINTERFACE) && defined(__c2__) &&  __clang_major__ == 3 && __clang_minor__ == 8
//To avoid compile error
//C:\Program Files (x86)\Windows Kits\8.1\Include\um\combaseapi.h(229,21): error : unknown type name 'IUnknown'
//          static_cast<IUnknown*>(*pp);    // make sure everyone derives from IUnknown
#define CINTERFACE
#endif
//for ntohl
#include <Winsock2.h>//need to link Ws2_32.lib
#ifdef _MSC_VER
#	pragma comment(lib, "Ws2_32.lib")
#endif
#include "registry_key.hpp"
#include <cstring>
#include <codecvt>

namespace win32 {
#if defined(_MSC_VER) && _MSC_VER < 1900
	std::string format_message(DWORD lasterr) {
		char* buf = nullptr;
		const auto len = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			nullptr,
			lasterr,
			LANG_NEUTRAL,
			(LPSTR)&buf,
			0,
			nullptr
		);
		auto i = (len < 3) ? 0 : len - 3;
		for (; '\r' != buf[i] && '\n' != buf[i] && '\0' != buf[i]; i++);//改行文字削除
		buf[i] = '\0';
		std::string ret = buf + ("(" + std::to_string(lasterr)) + ")";//エラーメッセージ作成
		LocalFree(buf);//FormatMessageAでFORMAT_MESSAGE_ALLOCATE_BUFFERを指定したので必ず開放
		return ret;
	}
	using std::error_category;
	class system_error_category_c : public error_category {	// categorize an operating system error
	public:
		system_error_category_c() : error_category() {}
		virtual const char *name() const { return "system"; }
		virtual std::string message(int ec) const
		{
			try {
				return format_message(ec);
			}
			catch (...) {
				return "unknown error";
			}
		}
	};
	namespace detail {
		template<typename T> T& put_on_static_storage() {
			static T storage;
			return storage;
		}
	}
	error_category& system_category() {
		return detail::put_on_static_storage<system_error_category_c>();
	}
	using std::runtime_error;
	class system_error : public runtime_error
	{
	public:
		system_error(std::error_code ec, const std::string& m)
			: ec_(ec), runtime_error((m.empty()) ? ec.message() : m + ": " + ec.message())
		{}
		system_error(std::error_code ec) : system_error(ec, "") {}
		system_error(std::error_code ec, const char *m) : system_error(ec, std::string(m)) {}
		system_error(int e, const error_category& erct) : system_error(e, erct, "") {}
		system_error(int e, const error_category& erct, const std::string& m) : system_error(std::error_code(e, erct), m) {}
		system_error(int e, const error_category& erct, const char *m) : system_error(e, erct, std::string(m)) {}
		const std::error_code& code() const { return ec_; }
	private:
		std::error_code ec_;
	};
#else
	using std::system_category;
	using std::system_error;
#endif
	registry_key::registry_key(registry_hive parent_key_handle, const TCHAR * sub_key_root, registry_rights rights, registry_view view)
		: registry_key(reinterpret_cast<HKEY>(parent_key_handle), sub_key_root, rights, view)
	{}
	registry_key::registry_key(HKEY parent_key_handle, const TCHAR* sub_key_root, registry_rights rights, registry_view view)
		: key() 
	{
		this->open(parent_key_handle, sub_key_root, rights, view);
	}

	registry_key::~registry_key() WIN32_REG_NOEXCEPT_OR_NOTHROW {
		this->close();
	}

	void registry_key::open(registry_hive parent_key_handle, const TCHAR * sub_key_root, registry_rights rights, registry_view view)
	{
		this->open(reinterpret_cast<HKEY>(parent_key_handle), sub_key_root, rights, view);
	}

	void registry_key::open(HKEY parent_key_handle, const TCHAR * sub_key_root, registry_rights rights, registry_view view)
	{
		//http://stackoverflow.com/questions/12619372/what-is-the-difference-between-registry-localmachine-and-registrykey-openbasekey
#ifdef _WIN64
		if (view == registry_view::v_default) view = registry_view::v_64;
#else
		if (view == registry_view::v_default) view = registry_view::v_32;
#endif
		const auto er = RegOpenKeyEx(parent_key_handle, sub_key_root, 0, static_cast<std::uint32_t>(rights) | static_cast<std::uint32_t>(view), &this->key);
		if (ERROR_SUCCESS != er) {
			const auto ec = GetLastError();
			throw system_error(std::error_code(ec, system_category()), "RegOpenKeyEx:(" + std::to_string(er) + ") GetLastError:(" + std::to_string(ec) + ')');
		}
		this->is_open_ = true;
	}

	void registry_key::close() WIN32_REG_NOEXCEPT_OR_NOTHROW
	{
		if (is_open_) {
			RegCloseKey(this->key);
			this->is_open_ = false;
		}
	}

	bool registry_key::is_open() const WIN32_REG_NOEXCEPT_OR_NOTHROW { return this->is_open_; }

	std::pair<registry_value_kind, DWORD> registry_key::get_value_kind_and_size(const TCHAR * key_name) const
	{
		this->check_open();
		std::pair<registry_value_kind, DWORD> re;
		if (ERROR_SUCCESS != RegQueryValueEx(this->key, key_name, 0, reinterpret_cast<DWORD*>(&re.first), nullptr, &re.second)) {
			const auto ec = GetLastError();
			throw system_error(std::error_code(ec, system_category()), '(' + std::to_string(ec) + ')');
		}
		return re;
	}
	registry_value_kind registry_key::get_value_kind(const TCHAR* key_name) const
	{
		return get_value_kind_and_size(key_name).first;
	}

	std::vector<std::uint8_t> registry_key::get_value_as_binary(const TCHAR * key_name)
	{
		const auto kind_and_size = get_value_kind_and_size(key_name);
		if (registry_value_kind::binary != kind_and_size.first) throw std::runtime_error("specified key type is not corrent.");
		std::vector<std::uint8_t> re;
		DWORD old_size, new_size = kind_and_size.second;
		do {
			old_size = new_size;
			re.resize(old_size);
			const auto er = RegQueryValueEx(this->key, key_name, 0, nullptr, reinterpret_cast<LPBYTE>(&re[0]), &new_size);
			if (ERROR_SUCCESS != er && ERROR_MORE_DATA != er) {
				const auto ec = GetLastError();
				throw system_error(std::error_code(ec, system_category()), '(' + std::to_string(ec) + ')');
			}
		} while (old_size != new_size);
		return re;
	}

	tstring registry_key::get_value_as_string(DWORD dwType, const TCHAR* key_name) {
		const auto kind_and_size = get_value_kind_and_size(key_name);
		if (dwType != static_cast<DWORD>(kind_and_size.first)) throw std::runtime_error("specified key type is not corrent.");
		tstring re;
		DWORD old_size, new_size = kind_and_size.second;
		do {
			old_size = new_size;
			re.resize(old_size);
			const auto er = RegQueryValueEx(this->key, key_name, 0, nullptr, reinterpret_cast<LPBYTE>(&re[0]), &new_size);
			if (ERROR_SUCCESS != er && ERROR_MORE_DATA != er) {
				const auto ec = GetLastError();
				throw system_error(std::error_code(ec, system_category()), '(' + std::to_string(ec) + ')');
			}
		} while (old_size != new_size);
		return re;
	}
	std::vector<tstring> registry_key::get_value_as_string_arr(const TCHAR * key_name)
	{
		const auto kind_and_size = get_value_kind_and_size(key_name);
		if (registry_value_kind::multi_string != kind_and_size.first) throw std::runtime_error("specified key type is not corrent.");
		std::vector<tstring> re;
		re.resize(1);
		auto& buf = re[0];
		const auto buf_p = &buf[0];
		DWORD old_size, new_size = kind_and_size.second;
		do {
			old_size = new_size;
			buf.resize(old_size);
			//得られるのはNULL文字区切りのbyte列
			const auto er = RegQueryValueEx(this->key, key_name, 0, nullptr, reinterpret_cast<LPBYTE>(buf_p), &new_size);
			if (ERROR_SUCCESS != er && ERROR_MORE_DATA != er) {
				const auto ec = GetLastError();
				throw system_error(std::error_code(ec, system_category()), '(' + std::to_string(ec) + ')');
			}
		} while (old_size != new_size);
		using string_traits_type = std::basic_string<TCHAR>::traits_type;
		for (
			std::size_t i = string_traits_type::length(buf_p) + 1, l = string_traits_type::length(buf_p + i);//一つ目はすっ飛ばす
			0 < l; 
			i += l + 1, l = string_traits_type::length(buf_p + i)
		) {
			re.emplace_back(buf_p + i);
		}
		buf.resize(string_traits_type::length(buf_p));//最後に一つ目をresize
		return re;
	}
	std::uint32_t registry_key::get_value_as_dword(DWORD dwType, const TCHAR * key_name)
	{
		/*const*/ auto kind_and_size = get_value_kind_and_size(key_name);
		if (dwType != static_cast<DWORD>(kind_and_size.first)) throw std::runtime_error("specified key type is not corrent.");
		std::uint32_t re;
		if (ERROR_SUCCESS != RegQueryValueEx(this->key, key_name, 0, nullptr, reinterpret_cast<LPBYTE>(&re), &kind_and_size.second)) {
			const auto ec = GetLastError();
			throw system_error(std::error_code(ec, system_category()), '(' + std::to_string(ec) + ')');
		}
		if (dwType == REG_DWORD_BIG_ENDIAN) re = ntohl(re);//convert byte order
		return re;
	}
	std::uint64_t registry_key::get_value_as_qword(const TCHAR * key_name)
	{
		/*const*/ auto kind_and_size = get_value_kind_and_size(key_name);
		if (registry_value_kind::qword != kind_and_size.first) throw std::runtime_error("specified key type is not corrent.");
		std::uint64_t re;
		if (ERROR_SUCCESS != RegQueryValueEx(this->key, key_name, 0, nullptr, reinterpret_cast<LPBYTE>(&re), &kind_and_size.second)) {
			const auto ec = GetLastError();
			throw system_error(std::error_code(ec, system_category()), '(' + std::to_string(ec) + ')');
		}
		return re;
	}
	void registry_key::check_open() const{
		if (!this->is_open()) throw std::runtime_error("Registry key is closed.");
	}
}
