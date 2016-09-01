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
#include "win32_reg/registry_key.hpp"
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
		std::string ret = buf;//エラーメッセージ作成
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
		: key(), parent_key_handle_(parent_key_handle)
	{
		this->open(parent_key_handle, sub_key_root, rights, view);
	}

	registry_key::registry_key(const registry_key & parent_key_handle, const TCHAR * sub_key_root, registry_rights rights, registry_view view)
		: parent_key_handle_(parent_key_handle.key)
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
			throw system_error(std::error_code(er, system_category()), "RegOpenKeyEx:(" + std::to_string(er) + ')');
		}
		this->is_open_ = true;
	}

	void win32::registry_key::open(const registry_key & parent_key_handle, const TCHAR * sub_key_root, registry_rights rights, registry_view view)
	{
		this->open(reinterpret_cast<HKEY>(parent_key_handle.key), sub_key_root, rights, view);
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
		const auto er = RegQueryValueEx(this->key, key_name, 0, reinterpret_cast<DWORD*>(&re.first), nullptr, &re.second);
		if (ERROR_SUCCESS != er) {
			throw system_error(std::error_code(er, system_category()), '(' + std::to_string(er) + ')');
		}
		return re;
	}
	registry_value_kind registry_key::get_value_kind(const TCHAR* key_name) const
	{
		return get_value_kind_and_size(key_name).first;
	}
	namespace detail {
		std::chrono::system_clock::time_point make_chrono_time_point(const FILETIME& ft) {
			return std::chrono::system_clock::from_time_t(
				static_cast<time_t>(((static_cast<std::uint64_t>(ft.dwHighDateTime) << 32) + ft.dwLowDateTime - 116444736000000000) / 10000000)
			);
		}
		enum class info_type { sub_key, key_value };
		template<info_type i> struct reg_key_info { DWORD num; DWORD max_len; };
		template<info_type i>
		WIN32_REG_CONSTEXPR bool operator!=(const reg_key_info<i>& l, const reg_key_info<i>& r) { return l.max_len != r.max_len || r.num != l.num; }
		template<info_type i> LONG reg_query_info_key(HKEY key, reg_key_info<i>& sub_key);
		template<> LONG reg_query_info_key<info_type::key_value>(HKEY key, reg_key_info<info_type::key_value>& info) {
			return RegQueryInfoKey(
				key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &info.num, &info.max_len, nullptr, nullptr, nullptr
			);
		}
		template<> LONG reg_query_info_key<info_type::sub_key>(HKEY key, reg_key_info<info_type::sub_key>& info) {
			return RegQueryInfoKey(
				key, nullptr, nullptr, nullptr, &info.num, &info.max_len, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
			);
		}
		template<info_type i> reg_key_info<i> get_sub_key_info(HKEY key) {
			reg_key_info<i> re;
			const auto er = reg_query_info_key(key, re);
			if (ERROR_SUCCESS != er) throw system_error(std::error_code(er, system_category()), '(' + std::to_string(er) + ')');
			++re.max_len;//終端のNULLは含まれないことがある
			return re;
		}
		LONG reg_enum_key_ex(HKEY key, DWORD max_len, DWORD i, tstring& buf, FILETIME* ft = nullptr) {
			buf.resize(max_len);
			DWORD len = max_len;
			const auto er = RegEnumKeyEx(key, i, &buf[0], &len, nullptr, nullptr, nullptr, ft);
			switch (er) {
				case ERROR_SUCCESS: 
					buf.resize(len);
					buf.shrink_to_fit();
				case ERROR_MORE_DATA: 
				case ERROR_NO_MORE_ITEMS: 
					return er;
				default:
					throw system_error(std::error_code(er, system_category()), '(' + std::to_string(er) + ')');
			}
		}
	}
	std::unordered_map<tstring, std::chrono::system_clock::time_point> win32::registry_key::get_sub_key_last_written_times()
	{
		this->check_open();
		std::unordered_map<tstring, std::chrono::system_clock::time_point> re;
		bool force_retry = false;
		for (
			detail::reg_key_info<detail::info_type::sub_key> info = detail::get_sub_key_info<detail::info_type::sub_key>(this->key), info_old = {};
			info_old != info || force_retry;
			info_old = info, info = detail::get_sub_key_info<detail::info_type::sub_key>(this->key)
		) {
			if (!info.num) return{};
			force_retry = false;
			re.reserve(info.num);
			LONG er = ERROR_SUCCESS;
			for (DWORD i = 0; er != ERROR_NO_MORE_ITEMS; ++i) {
				tstring buf;
				FILETIME ft;
				er = detail::reg_enum_key_ex(this->key, info.max_len, i, buf, &ft);
				if (ERROR_MORE_DATA == er) {
					re.clear();
					force_retry = true;
					break;
				}
				else if(ERROR_SUCCESS == er) re.insert(std::make_pair(std::move(buf), detail::make_chrono_time_point(ft)));
			}
		}
		return re;
	}
	std::vector<tstring> win32::registry_key::get_sub_key_names()
	{
		this->check_open();
		std::vector<tstring> re;
		bool force_retry = false;
		for (
			detail::reg_key_info<detail::info_type::sub_key> info = detail::get_sub_key_info<detail::info_type::sub_key>(this->key), info_old = {};
			info_old != info || force_retry;
			info_old = info, info = detail::get_sub_key_info<detail::info_type::sub_key>(this->key)
		) {
			if (!info.num) return{};
			force_retry = false;
			re.reserve(info.num);
			LONG er = ERROR_SUCCESS;
			for (DWORD i = 0; er != ERROR_NO_MORE_ITEMS; ++i) {
				tstring buf;
				er = detail::reg_enum_key_ex(this->key, info.max_len, i, buf);
				if (ERROR_MORE_DATA == er) {
					re.clear();
					force_retry = true;
					break;
				}
				else if (ERROR_SUCCESS == er) re.push_back(std::move(buf));
			}
		}
		return re;
	}
	std::vector<tstring> win32::registry_key::get_value_names()
	{
		this->check_open();
		std::vector<tstring> re;
		bool force_retry = false;
		for (
			detail::reg_key_info<detail::info_type::key_value> info = detail::get_sub_key_info<detail::info_type::key_value>(this->key), info_old = {};
			info_old != info || force_retry;
			info_old = info, info = detail::get_sub_key_info<detail::info_type::key_value>(this->key)
		) {
			if (!info.num) return{};
			force_retry = false;
			re.reserve(info.num);
			LONG er = ERROR_SUCCESS;
			for (DWORD i = 0; er != ERROR_NO_MORE_ITEMS; ++i) {
				tstring buf;
				buf.resize(info.max_len);
				DWORD len = info.max_len;
				er = RegEnumValue(this->key, i, &buf[0], &len, nullptr, nullptr, nullptr, nullptr);
				if (ERROR_MORE_DATA == er) {
					re.clear();
					force_retry = true;
					break;
				}
				else if (ERROR_SUCCESS == er) {
					buf.resize(len);
					buf.shrink_to_fit();
					re.push_back(std::move(buf));
				}
				else if (ERROR_NO_MORE_ITEMS != er) {
					throw system_error(std::error_code(er, system_category()), '(' + std::to_string(er) + ')');
				}
			}
		}
		return re;
	}

	std::vector<std::uint8_t> registry_key::get_value_as_binary(const TCHAR * key_name)
	{
		const auto kind_and_size = get_value_kind_and_size(key_name);
		if (registry_value_kind::binary != kind_and_size.first) throw std::runtime_error("specified key type is not corrent.");
		std::vector<std::uint8_t> re;
		DWORD old_size = kind_and_size.second, new_size = kind_and_size.second;
		bool first_flg = true;
		LSTATUS er;
		do {
			if (old_size < new_size) old_size = new_size;
			if (HKEY_PERFORMANCE_DATA == this->parent_key_handle_) {
				if (!first_flg) {
					++old_size;
					new_size = old_size;
				}
			}
			first_flg = false;
			re.resize(old_size);
			er = RegQueryValueEx(this->key, key_name, 0, nullptr, reinterpret_cast<LPBYTE>(&re[0]), &new_size);
			if (ERROR_SUCCESS != er && ERROR_MORE_DATA != er) {
				throw system_error(std::error_code(er, system_category()), '(' + std::to_string(er) + ')');
			}
		} while (ERROR_MORE_DATA == er);
		return re;
	}
	namespace detail {
		void get_value_as_string_impl(HKEY key, HKEY parent_key_handle, DWORD size, const TCHAR* key_name, tstring& re) {
			DWORD old_size = size, new_size = size;
			LSTATUS er;
			do {
				if (old_size < new_size) old_size = new_size;
				if (HKEY_PERFORMANCE_DATA == parent_key_handle) {
					++old_size;
					new_size = old_size;
				}
				re.resize(old_size / sizeof(TCHAR));
				er = RegQueryValueEx(key, key_name, 0, nullptr, reinterpret_cast<LPBYTE>(&re[0]), &new_size);
				if (ERROR_SUCCESS != er && ERROR_MORE_DATA != er) {
					throw system_error(std::error_code(er, system_category()), '(' + std::to_string(er) + ')');
				}
			} while (ERROR_MORE_DATA == er);
		}
	}
	tstring registry_key::get_value_as_string(DWORD dwType, const TCHAR* key_name) {
		const auto kind_and_size = get_value_kind_and_size(key_name);
		if (dwType != static_cast<DWORD>(kind_and_size.first)) throw std::runtime_error("specified key type is not corrent.");
		tstring re;
		detail::get_value_as_string_impl(this->key, this->parent_key_handle_, kind_and_size.second, key_name, re);
		re.resize(std::char_traits<TCHAR>::length(re.c_str()));
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
		//得られるのはNULL文字区切りのbyte列
		detail::get_value_as_string_impl(this->key, this->parent_key_handle_, kind_and_size.second, key_name, buf);
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
		const auto er = RegQueryValueEx(this->key, key_name, 0, nullptr, reinterpret_cast<LPBYTE>(&re), &kind_and_size.second);
		if (ERROR_SUCCESS != er) {
			throw system_error(std::error_code(er, system_category()), '(' + std::to_string(er) + ')');
		}
		if (dwType == REG_DWORD_BIG_ENDIAN) re = ntohl(re);//convert byte order
		return re;
	}
	std::uint64_t registry_key::get_value_as_qword(const TCHAR * key_name)
	{
		/*const*/ auto kind_and_size = get_value_kind_and_size(key_name);
		if (registry_value_kind::qword != kind_and_size.first) throw std::runtime_error("specified key type is not corrent.");
		std::uint64_t re;
		const auto er = RegQueryValueEx(this->key, key_name, 0, nullptr, reinterpret_cast<LPBYTE>(&re), &kind_and_size.second);
		if (ERROR_SUCCESS != er) {
			throw system_error(std::error_code(er, system_category()), '(' + std::to_string(er) + ')');
		}
		return re;
	}
	void registry_key::check_open() const{
		if (!this->is_open()) throw std::runtime_error("Registry key is closed.");
	}
}
