#ifndef FI_W1_WPA_SUPPLICANT_H_
#define FI_W1_WPA_SUPPLICANT_H_

#include <core/dbus/types/object_path.h>
#include <core/dbus/types/variant.h>
#include <core/dbus/service.h>
#include <chrono>
#include <string>
#include <vector>
#include <tuple>

/* From
 * https://w1.fi/wpa_supplicant/devel/dbus.html#dbus_interface
 */

namespace core {

struct WPASupplicant {
	struct Properties {
		struct Interfaces {
			inline static std::string name() {
				return "Interfaces";
			}
			typedef WPASupplicant Interface;
			typedef std::vector<core::dbus::types::ObjectPath> ValueType;
			static const bool readable = true;
			static const bool writable = false;
		};
	};

	struct CreateInterface {
		typedef WPASupplicant Interface;
		static const std::string& name() {
			static const std::string s {"CreateInterface"};
			return s;
		}
		inline static const std::chrono::milliseconds default_timeout() {
			return std::chrono::seconds{1};
		}
	};

	struct GetInterface {
		typedef WPASupplicant Interface;
		static const std::string& name() {
			static const std::string s {"GetInterface"};
			return s;
		}
		inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
	};

	struct Interface {
		struct Properties {
			struct BSSs {
				inline static std::string name() {
					return "BSSs";
				}
				typedef core::WPASupplicant::Interface Interface;
				typedef std::vector<core::dbus::types::ObjectPath> ValueType;
				static const bool readable = true;
				static const bool writable = false;
			};

			struct Networks {
				inline static std::string name() {
					return "Networks";
				}
				typedef core::WPASupplicant::Interface Interface;
				typedef std::vector<core::dbus::types::ObjectPath> ValueType;
				static const bool readable = true;
				static const bool writable = false;
			};
		};

		struct Scan {
			static const std::string &name() {
				static const std::string s{"Scan"};
				return s;
			}
			inline static const std::chrono::milliseconds default_timeout() {
				return std::chrono::seconds{1};
			}
			typedef core::WPASupplicant::Interface Interface;
		};

		struct AddNetwork {
			static const std::string &name() {
				static const std::string s{"AddNetwork"};
				return s;
			}
			inline static const std::chrono::milliseconds default_timeout() {
				return std::chrono::seconds{1};
			}
			typedef core::WPASupplicant::Interface Interface;
		};

		struct Disconnect {
			static const std::string &name() {
				static const std::string s{"Disconnect"};
				return s;
			}
			inline static const std::chrono::milliseconds default_timeout() {
				return std::chrono::seconds{1};
			}
			typedef core::WPASupplicant::Interface Interface;
		};

		struct SelectNetwork {
			static const std::string &name() {
				static const std::string s{"SelectNetwork"};
				return s;
			}
			inline static const std::chrono::milliseconds default_timeout() {
				return std::chrono::seconds{1};
			}
			typedef core::WPASupplicant::Interface Interface;
		};

		struct Reconnect {	// Not being recognized on the dbus - need to investigate
			static const std::string &name() {
				static const std::string s{"Reconnect"};
				return s;
			}
			inline static const std::chrono::milliseconds default_timeout() {
				return std::chrono::seconds{1};
			}
			typedef core::WPASupplicant::Interface Interface;
		};

		struct Reassociate {
			static const std::string &name() {
				static const std::string s{"Reassociate"};
				return s;
			}
			inline static const std::chrono::milliseconds default_timeout() {
				return std::chrono::seconds{1};
			}
			typedef core::WPASupplicant::Interface Interface;
		};

		struct Signals {
			struct BSSAdded {
				inline static std::string name() {
					return "BSSAdded";
				}
				typedef core::WPASupplicant::Interface Interface;
				using ArgumentType = std::tuple<core::dbus::types::ObjectPath, std::map<std::string, core::dbus::types::Variant>>;
			};
			struct ScanDone {
				inline static std::string name() {
					return "ScanDone";
				}
				typedef core::WPASupplicant::Interface Interface;
				typedef bool ArgumentType;
			};
		};
	};

	struct BSS {
		struct Properties {
			struct SSID {
				inline static std::string name() {
					return "SSID";
				}
				typedef core::WPASupplicant::BSS Interface;
				typedef std::vector<std::int8_t> ValueType;
				static const bool readable = true;
				static const bool writable = false;
			};
		};
	};

	struct Network {
		struct Properties {
			struct NetworkProperties {
				inline static std::string name() {
					return "Properties";
				}
				typedef core::WPASupplicant::Network Interface;
				using ValueType = std::map<std::string, core::dbus::types::Variant>;
				static const bool readable = true;
				static const bool writable = false;
			};

			struct NetworkEnabled {
				inline static std::string name() {
					return "Enabled";
				}
				typedef core::WPASupplicant::Network Interface;
				typedef bool ValueType;
				static const bool readable = true;
				static const bool writable = false;
			};
		};
	};
};
}

namespace core {
namespace dbus {
namespace traits {

template<>
struct Service<core::WPASupplicant>
{
	inline static const std::string& interface_name() {
		static const std::string s{"fi.w1.wpa_supplicant1"};
		return s;
	}
};

template<>
struct Service<core::WPASupplicant::Interface>
{
	inline static const std::string& interface_name() {
		static const std::string s{"fi.w1.wpa_supplicant1.Interface"};
		return s;
	}
};

template<>
struct Service<core::WPASupplicant::BSS>
{
	inline static const std::string& interface_name() {
		static const std::string s{"fi.w1.wpa_supplicant1.BSS"};
		return s;
	}
};

template<>
struct Service<core::WPASupplicant::Network>
{
	inline static const std::string& interface_name() {
		static const std::string s{"fi.w1.wpa_supplicant1.Network"};
		return s;
	}
};


}  // namespace traits
}  // namespace dbus
}  // namespace core

#endif // FI_W1_WPA_SUPPLICANT_H_
