#pragma once
#if !defined(CINTERFACE) && defined(__c2__) &&  __clang_major__ == 3 && __clang_minor__ == 8
//To avoid compile error
//C:\Program Files (x86)\Windows Kits\8.1\Include\um\combaseapi.h(229,21): error : unknown type name 'IUnknown'
//          static_cast<IUnknown*>(*pp);    // make sure everyone derives from IUnknown
#define CINTERFACE
#endif
#include "config/suffix.hpp"
#include <Windows.h>
#include <string>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <climits>
#include <array>
namespace w_system {
	namespace security {
		template<bool con> using concept = typename std::enable_if<con, std::nullptr_t>::type;
		enum class registry_rights : std::uint32_t {
			//from C# System.Security.AccessControl.RegistryRights
			delete_permissions = DELETE,//C# original name is Delete however, we rename this to avoid name conflict with reserved keyword.
			read_permissions = READ_CONTROL,
			change_permissions = WRITE_DAC,
			take_ownership = WRITE_OWNER,

			//from winnt.h
			synchronize = SYNCHRONIZE,
			standard_rights_required = delete_permissions | read_permissions | change_permissions | take_ownership,
			standard_rights_read = read_permissions,//STANDARD_RIGHTS_READ
			standard_rights_write = read_permissions,//STANDARD_RIGHTS_WRITE
			standard_rights_all = standard_rights_required | synchronize,

			//from C# System.Security.AccessControl.RegistryRights
			create_link = KEY_CREATE_LINK,
			create_sub_key = KEY_CREATE_SUB_KEY,
			enumerate_sub_keys = KEY_ENUMERATE_SUB_KEYS,
			notify = KEY_NOTIFY,
			query_values = KEY_QUERY_VALUE,
			set_value = KEY_SET_VALUE,
			read_key = (standard_rights_read | query_values | enumerate_sub_keys | notify) & (~synchronize),
			write_key = (standard_rights_write | set_value | create_sub_key) & (~synchronize),
			execute_key = read_key & (~synchronize),
			full_control = standard_rights_required | query_values | set_value | create_sub_key | enumerate_sub_keys | notify | create_link,
		};
		static_assert(static_cast<std::uint32_t>(registry_rights::standard_rights_read) == STANDARD_RIGHTS_READ, "registry_rights::standard_rights_read");
		static_assert(static_cast<std::uint32_t>(registry_rights::standard_rights_write) == STANDARD_RIGHTS_WRITE, "registry_rights::standard_rights_write");
		inline WIN32_REG_CONSTEXPR registry_rights operator| (registry_rights l, registry_rights r) {
			return static_cast<registry_rights>(static_cast<std::uint64_t>(l) | static_cast<std::uint64_t>(r));
		}
		namespace principal {
			class security_identifier {
			private:
				std::vector<std::uint8_t> buffer;

			public:
				WIN32_REG_STATIC_CONSTEXPR int MaxBinaryLength = 68;
				WIN32_REG_STATIC_CONSTEXPR int MinBinaryLength = 8;
			};
		}
		namespace access_control {
			enum class ace_flags : std::uint8_t {
				None = 0,
				ObjectInherit = 0x01,
				ContainerInherit = 0x02,
				NoPropagateInherit = 0x04,
				InheritOnly = 0x08,
				InheritanceFlags = ObjectInherit | ContainerInherit | NoPropagateInherit | InheritOnly,
				Inherited = 0x10,
				SuccessfulAccess = 0x40,
				FailedAccess = 0x80,
				AuditFlags = SuccessfulAccess | FailedAccess,
			};
			inline WIN32_REG_CONSTEXPR ace_flags operator& (ace_flags l, ace_flags r) { 
				return static_cast<ace_flags>(static_cast<std::uint8_t>(l) & static_cast<std::uint8_t>(r));
			}
			inline WIN32_REG_CONSTEXPR ace_flags operator| (ace_flags l, ace_flags r) {
				return static_cast<ace_flags>(static_cast<std::uint8_t>(l) | static_cast<std::uint8_t>(r));
			}
			inline WIN32_REG_CONSTEXPR bool operator==(ace_flags l, std::nullptr_t) { return 0 == static_cast<std::uint8_t>(l); }
			inline WIN32_REG_CONSTEXPR bool operator==(std::nullptr_t, ace_flags r) { return r == 0; }
			inline WIN32_REG_CONSTEXPR bool operator!=(ace_flags l, std::nullptr_t) { return !(0 == l); }
			inline WIN32_REG_CONSTEXPR bool operator!=(std::nullptr_t, ace_flags r) { return !(0 == r); }
			enum class ace_type : std::uint8_t {
				AccessAllowed = 0,
				AccessDenied = 1,
				SystemAudit = 2,
				SystemAlarm = 3,
				AccessAllowedCompound = 4,
				AccessAllowedObject = 5,
				AccessDeniedObject = 6,
				SystemAuditObject = 7,
				SystemAlarmObject = 8,
				AccessAllowedCallback = 9,
				AccessDeniedCallback = 10,
				AccessAllowedCallbackObject = 11,
				AccessDeniedCallbackObject = 12,
				SystemAuditCallback = 13,
				SystemAlarmCallback = 14,
				SystemAuditCallbackObject = 15,
				SystemAlarmCallbackObject = 16,
				MaxDefinedAceType = 16,
			};
			enum class audit_flags : std::uint8_t {
				None = 0,
				Success = 1,
				Failure = 2,
			};
			enum class inheritance_flags : std::uint8_t {
				None = 0,
				ContainerInherit = 1,
				ObjectInherit = 2,
			};
			enum class propagation_flags : std::uint8_t {
				None = 0,
				NoPropagateInherit = 1,
				InheritOnly = 2,
			};
			enum class object_ace_flags : std::uint8_t {
				None = 0,
				ObjectAceTypePresent = 1,
				InheritedObjectAceTypePresent = 2,
			};
			enum class ace_qualifier : std::uint8_t {
				AccessAllowed = 0,
				AccessDenied = 1,
				SystemAudit = 2,
				SystemAlarm = 3,
			};
			class generic_ace {
			private:
				ace_type ace_type_;
				ace_flags ace_flags_;
			public:
				//observer
				WIN32_REG_CONSTEXPR_OR_CONST ace_type type() const WIN32_REG_NOEXCEPT_OR_NOTHROW { return this->ace_type_; }
				ace_flags& flags() { return this->ace_flags_; }
				WIN32_REG_CONSTEXPR_OR_CONST ace_flags flags() const WIN32_REG_NOEXCEPT_OR_NOTHROW { return this->ace_flags_; }
				//constructor
				WIN32_REG_CONSTEXPR_OR_CONST generic_ace(const generic_ace& o) WIN32_REG_NOEXCEPT_OR_NOTHROW
					: ace_type_(o.ace_type_), ace_flags_(o.ace_flags_)
				{}
			protected:
				WIN32_REG_CONSTEXPR generic_ace(ace_type type, ace_flags flags) WIN32_REG_NOEXCEPT_OR_NOTHROW
					: ace_type_(type), ace_flags_(flags)
				{}
				template<std::size_t N, concept<(2 <= N)> = nullptr>
				WIN32_REG_CONSTEXPR generic_ace(const std::array<std::uint8_t, N>& binary_form, std::size_t offset) WIN32_REG_NOEXCEPT_OR_NOTHROW :
					ace_type_(static_cast<ace_type>(binary_form[offset])),
					ace_flags_(static_cast<ace_flags>(binary_form[offset + 1]))
				{}
				generic_ace(const std::vector<std::uint8_t>& binary_form, std::size_t offset) : 
					ace_type_(static_cast<ace_type>(binary_form.at(offset))),
					ace_flags_(static_cast<ace_flags>(binary_form.at(offset + 1)))
				{}
				WIN32_REG_CONSTEXPR audit_flags audit_flags() const WIN32_REG_NOEXCEPT_OR_NOTHROW {
					return
						(0 != (this->ace_flags_ & ace_flags::SuccessfulAccess)) ? audit_flags::Success :
						(0 != (this->ace_flags_ & ace_flags::FailedAccess)) ? audit_flags::Failure :
						audit_flags::None;
				}
				virtual std::size_t binary_length() = delete;
				WIN32_REG_CONSTEXPR inheritance_flags inheritance_flags() const WIN32_REG_NOEXCEPT_OR_NOTHROW {
					return
						(0 != (this->ace_flags_ & ace_flags::ObjectInherit)) ? inheritance_flags::ObjectInherit :
						(0 != (this->ace_flags_ & ace_flags::ContainerInherit)) ? inheritance_flags::ContainerInherit :
						inheritance_flags::None;
				}
				WIN32_REG_CONSTEXPR bool is_inherited() const WIN32_REG_NOEXCEPT_OR_NOTHROW { 
					return ace_flags::None != (this->ace_flags_ & ace_flags::Inherited); 
				}
				WIN32_REG_CONSTEXPR propagation_flags propagation_flags() const WIN32_REG_NOEXCEPT_OR_NOTHROW {
					return
						(0 != (this->ace_flags_ & ace_flags::InheritOnly)) ? propagation_flags::InheritOnly :
						(0 != (this->ace_flags_ & ace_flags::NoPropagateInherit)) ? propagation_flags::NoPropagateInherit :
						propagation_flags::None;
				}
				/*
				public static GenericAce CreateFromBinaryForm (byte[] binaryForm, int offset)
				{
					if (binaryForm == null)
						throw new ArgumentNullException("binaryForm");
			
					if (offset < 0 || offset > binaryForm.Length - 1)
						throw new ArgumentOutOfRangeException("offset", offset, "Offset out of range");
			
					AceType type = (AceType)binaryForm[offset];
					if (IsObjectType(type))
						return new ObjectAce(binaryForm, offset);
					else
						return new CommonAce(binaryForm, offset);
				}
				*/
				WIN32_REG_CONSTEXPR bool operator==(const generic_ace& r) const WIN32_REG_NOEXCEPT_OR_NOTHROW {
					return this->ace_type_ == r.ace_type_ && this->ace_flags_ == r.ace_flags_;
				}
				WIN32_REG_CONSTEXPR bool operator!=(const generic_ace& r) const WIN32_REG_NOEXCEPT_OR_NOTHROW {
					return !(r == *this);
				}
				virtual void get_binary_form(const std::vector<std::uint8_t>& binary_form, std::size_t offset) = delete;
				template<std::size_t N, concept<(2 <= N)> = nullptr>
				virtual void get_binary_form(const std::array<std::uint8_t, N>& binary_form, std::size_t offset) = delete;
			};
			class known_ace : public generic_ace {
			public:
				using security_identifier = ::w_system::security::principal::security_identifier;
			private:
				int access_mask;
				security_identifier identifier;
			};
			class qualified_ace : public known_ace {};
			class object_ace : public qualified_ace {
			private:
				GUID object_ace_type;
				GUID inherited_object_type;
				object_ace_flags object_ace_flags;
				static WIN32_REG_CONSTEXPR ace_type convert_type(ace_qualifier qualifier, bool isCallback)
				{
					return
						(qualifier == ace_qualifier::AccessAllowed)
						? (isCallback) ? ace_type::AccessAllowedCallbackObject : ace_type::AccessAllowedObject :
						(qualifier == ace_qualifier::AccessDenied)
						? (isCallback) ? ace_type::AccessDeniedCallbackObject : ace_type::AccessDeniedObject :
						(qualifier == ace_qualifier::SystemAlarm)
						? (isCallback) ? ace_type::SystemAlarmCallbackObject : ace_type::SystemAlarmObject :
						(isCallback) ? ace_type::SystemAuditCallbackObject : ace_type::SystemAuditObject;
				}
			public:
				/*object_ace(ace_flags aceFlags, ace_qualifier qualifier,
					int accessMask, security_identifier sid,
					ObjectAceFlags flags, GUID type,
					GUID inheritedType, bool isCallback,
					const std::vector<std::uint8_t>& opaque)
					: generic_ace(convert_type(qualifier, isCallback), aceFlags, opaque)
				{
					AccessMask = accessMask;
					SecurityIdentifier = sid;
					ObjectAceFlags = flags;
					ObjectAceType = type;
					InheritedObjectAceType = inheritedType;
				}*/

			};
			class object_security {};
			class registry_security {};
		}
	}
}
namespace std {
	template <>
	class hash<w_system::security::access_control::generic_ace> {
	public:
		WIN32_REG_CONSTEXPR size_t operator()(const w_system::security::access_control::generic_ace& x) const {
			return 0x9e377901 ^ ((static_cast<size_t>(x.flags()) << (sizeof(size_t) + sizeof(uint8_t) * CHAR_BIT)) & static_cast<size_t>(x.type()));
		}
	};
}
namespace microsoft {
	namespace win32 {
		using tstring = std::basic_string<TCHAR>;
		template<bool con> using concept = typename std::enable_if<con, std::nullptr_t>::type;
		enum class registry_hive : ULONG_PTR {
#if (WINVER >= 0x0400)
			current_config = 0x80000005,//HKEY_CURRENT_CONFIG,
			dyn_data = 0x80000006,//HKEY_DYN_DATA,
#endif
			classes_root = 0x80000000,//HKEY_CLASSES_ROOT,
			current_user = 0x80000001,//HKEY_CURRENT_USER,
			local_machine = 0x80000002,//HKEY_LOCAL_MACHINE,
			performance_data = 0x80000004,//HKEY_PERFORMANCE_DATA,
			users = 0x80000003,//HKEY_USERS
		};
		enum class registry_view : std::uint32_t {
			v_default,
			v_32 = KEY_WOW64_32KEY,
			v_64 = KEY_WOW64_64KEY
		};
		enum class registry_value_kind : DWORD {
			//from C# Microsoft.Win32.RegistryValueKind
			none = REG_NONE,//-1 in C#, however, we use REG_NONE(0)
			string = REG_SZ,
			expand_string = REG_EXPAND_SZ,
			binary = REG_BINARY,
			dword = REG_DWORD,
			multi_string = REG_MULTI_SZ,
			qword = REG_QWORD,
			//we will never support resource list(REG_RESOURCE_LIST, REG_FULL_RESOURCE_DESCRIPTOR, REG_RESOURCE_REQUIREMENTS_LIST)
			unknown = (~static_cast<DWORD>(0u)),//0 in C#, however, we use 4294967295.
			//from winnt.h
			dword_little_endian = REG_DWORD_LITTLE_ENDIAN,
			dword_big_endian = REG_DWORD_BIG_ENDIAN,
			qword_little_endian = REG_QWORD_LITTLE_ENDIAN,
			link = REG_LINK,
		};
		static_assert(registry_value_kind::dword == registry_value_kind::dword_little_endian, "DWord is big endian.");
		static_assert(registry_value_kind::qword == registry_value_kind::qword_little_endian, "QWord endian is unknown.");
		class registry_key {
		private:
			HKEY key;
			HKEY parent_key_handle_;
			bool is_open_;
			std::vector<std::uint8_t> get_value_as_binary(const TCHAR* key_name);
			tstring get_value_as_string(DWORD dwType, const TCHAR* key_name);
			std::vector<tstring> get_value_as_string_arr(const TCHAR* key_name);
			std::uint32_t get_value_as_dword(DWORD dwType, const TCHAR* key_name);
			std::uint64_t get_value_as_qword(const TCHAR * key_name);
			void check_open() const;
			std::pair<registry_value_kind, DWORD> get_value_kind_and_size(const TCHAR* key_name) const;
		public:
			registry_key() = default;
			registry_key(registry_hive parent_key_handle, const TCHAR* sub_key_root, w_system::security::registry_rights rights, registry_view view = registry_view::v_default);
			registry_key(HKEY parent_key_handle, const TCHAR* sub_key_root, w_system::security::registry_rights rights, registry_view view = registry_view::v_default);
			registry_key(const registry_key& parent_key_handle, const TCHAR* sub_key_root, w_system::security::registry_rights rights, registry_view view = registry_view::v_default);
			registry_key(const registry_key&) = delete;
			registry_key(registry_key&&) = delete;
			registry_key& operator=(const registry_key&) = delete;
			registry_key& operator=(registry_key&&) = delete;
			~registry_key() WIN32_REG_NOEXCEPT_OR_NOTHROW;
			void open(registry_hive parent_key_handle, const TCHAR* sub_key_root, w_system::security::registry_rights rights, registry_view view = registry_view::v_default);
			void open(HKEY parent_key_handle, const TCHAR* sub_key_root, w_system::security::registry_rights rights, registry_view view = registry_view::v_default);
			void open(const registry_key& parent_key_handle, const TCHAR* sub_key_root, w_system::security::registry_rights rights, registry_view view = registry_view::v_default);
			void close() WIN32_REG_NOEXCEPT_OR_NOTHROW;
			bool is_open() const WIN32_REG_NOEXCEPT_OR_NOTHROW;
			registry_value_kind get_value_kind(const TCHAR* key_name) const;

			template<registry_value_kind type, concept<
				type == registry_value_kind::expand_string || type == registry_value_kind::string || type == registry_value_kind::link
			> = nullptr>
				tstring get_value(const TCHAR* key_name) {
				return get_value_as_string(static_cast<DWORD>(type), key_name);
			}
			template<
				registry_value_kind type,
				concept<type == registry_value_kind::dword_little_endian || type == registry_value_kind::dword_big_endian> = nullptr
			>
				std::uint32_t get_value(const TCHAR* key_name) {
				return get_value_as_dword(static_cast<DWORD>(type), key_name);
			}
			template<registry_value_kind type, concept<type == registry_value_kind::qword> = nullptr>
			std::uint64_t get_value(const TCHAR* key_name) {
				return get_value_as_qword(key_name);
			}
			template<registry_value_kind type, concept<type == registry_value_kind::multi_string> = nullptr>
			std::vector<tstring> get_value(const TCHAR* key_name) {
				return get_value_as_string_arr(key_name);
			}
			template<registry_value_kind type, concept<type == registry_value_kind::binary> = nullptr>
			std::vector<std::uint8_t> get_value(const TCHAR* key_name) {
				return get_value_as_binary(key_name);
			}
			std::unordered_map<tstring, std::chrono::system_clock::time_point> get_sub_key_last_written_times();
			std::vector<tstring> get_sub_key_names();
			std::vector<tstring> get_value_names();
		};
	}
}