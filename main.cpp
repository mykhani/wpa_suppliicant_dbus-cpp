#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/result.h>
#include <core/dbus/asio/executor.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/variant.h>

#include "wpa_supplicant.h"

#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <tuple>
#include <exception>
#include <algorithm>
#include <unordered_set>
using namespace std;

namespace dbus = core::dbus;

dbus::Object::Ptr getInterface(dbus::Object::Ptr parent, string interface)
{
	dbus::Result<dbus::types::ObjectPath> result;

	try {
		result = parent->invoke_method_synchronously<core::WPASupplicant::GetInterface, dbus::types::ObjectPath, string>(interface);
	} catch (std::exception &e) {
		cout << "Error in getting interface: " << interface << ", error: " << e.what() << endl;
		return nullptr;
	}

	dbus::Object::Ptr device = parent->add_object_for_path(result.value());

	return device;
}

dbus::Object::Ptr createInterface(dbus::Object::Ptr parent, string interface)
{
	dbus::Result<dbus::types::ObjectPath> result;

	std::map<string, dbus::types::Variant> args = {
		{"Ifname", dbus::types::Variant::encode<string>(interface)}
	};

	try {
		result = parent->invoke_method_synchronously<core::WPASupplicant::CreateInterface,
							     dbus::types::ObjectPath,
							     std::map<string, dbus::types::Variant>
							    >(args);
	} catch (std::exception &e) {
		cout << "Error in getting interface: " << interface << ", error: " << e.what() << endl;
		return nullptr;
	}

	dbus::Object::Ptr device = parent->add_object_for_path(result.value());

	return device;
}

void startScan(dbus::Object::Ptr interface, string scanType)
{
	cout << "Starting scan" << endl;

	std::map<string, dbus::types::Variant> args = {
		{"Type", dbus::types::Variant::encode<string>(scanType)}
	};

	interface->invoke_method_synchronously<core::WPASupplicant::Interface::Scan,
					       void,
					       std::map<string, dbus::types::Variant>
					      >(args);
}

dbus::types::ObjectPath createNetworkConfig(dbus::Object::Ptr interface, std::string ssid)
{
	dbus::types::ObjectPath network;
	std::string password;

	cout << "(" << ssid << "): enter password :" << endl;
	cin >> password;

	std::map<std::string, dbus::types::Variant> args = {
		{"ssid", dbus::types::Variant::encode<string>(ssid)},
		{"key_mgmt", dbus::types::Variant::encode<string>(string{"WPA-PSK"})},
		{"psk", dbus::types::Variant::encode<string>(password)},
	};

	try {
		dbus::Result<dbus::types::ObjectPath> result = interface->invoke_method_synchronously<core::WPASupplicant::Interface::AddNetwork,
						  		 			 dbus::types::ObjectPath,
											 std::map<std::string, dbus::types::Variant>
											>(args);
		network = result.value();
	} catch (std::exception &e) {
		cout << "Error in creating network config, error: " << e.what() << endl;
	}

	return network;
}

std::map<std::string, dbus::types::Variant> getNetworkProperties(dbus::Object::Ptr interface, dbus::types::ObjectPath networkObjPath)
{
	dbus::Object::Ptr network = interface->add_object_for_path(networkObjPath);
	bool enabled = network->get_property<core::WPASupplicant::Network::Properties::NetworkEnabled>()->get();

	/* workaround - use org.freedesktop.DBus.Properties interface
	 * to get_property with embedded Variant types - see
	 * https://answers.launchpad.net/ubuntu/+source/dbus-cpp/+question/593271 &
	 * https://stackoverflow.com/questions/44003627/list-wpa-supplicant-network-properties-using-dbus-cpp
	 */
	std::map<std::string, dbus::types::Variant> allProperties = network->get_all_properties<core::WPASupplicant::Network>();

	auto networkProperties = allProperties["Properties"].as<std::map<std::string, dbus::types::Variant>>();

	return networkProperties;
}

dbus::types::ObjectPath findNetwork(dbus::Object::Ptr interface, std::string ssid)
{
	vector<dbus::types::ObjectPath> networkObjPaths = interface->get_property<core::WPASupplicant::Interface::Properties::Networks>()->get();
	dbus::types::ObjectPath network;

	for (auto &networkObjPath : networkObjPaths) {
		cout << "Network: " << networkObjPath.as_string() << endl;
		std::map<std::string, dbus::types::Variant> props = getNetworkProperties(interface, networkObjPath);
		std::string network_ssid = props["ssid"].as<string>();
		// remove double quotes from the network ssid read from properties
		network_ssid.erase(
			std::remove(network_ssid.begin(), network_ssid.end(), '\"'),
			network_ssid.end());

		cout << "network ssid: " << network_ssid << ", desired ssid: " << ssid << endl;

		if (network_ssid == ssid) {
			cout << "Found Network config for ssid: " << ssid << endl;
			network = networkObjPath;
		}
	}

	return network;
}

bool connectNetwork(dbus::Object::Ptr interface, std::string ssid)
{
	dbus::types::ObjectPath network;

	network = findNetwork(interface, ssid);
	if (network.as_string() == "/") {
		cout << "Enable to find Network config for SSID: " << ssid << endl;
		network = createNetworkConfig(interface, ssid);
	}

	if (network.as_string() != "/") {
		cout << "Found network config for SSID: " << ssid << endl;
	} else {
		cout << "Unable to find/create a valid network config for SSID: " << ssid << endl;
		return false;
	}

	try {
		// disconnect interface
		interface->invoke_method_synchronously<core::WPASupplicant::Interface::Disconnect, void>();
	} catch (std::exception &e) {
		cout << "Failed to disconnect network :" << ssid << ", error: " << e.what() << endl;
		//return false; //TODO: Handle if interface is already disconnected
	}

	try {
		// SelectNetwork
		interface->invoke_method_synchronously<core::WPASupplicant::Interface::SelectNetwork, void, dbus::types::ObjectPath>(network);
	} catch (std::exception &e) {
		cout << "Failed to select network :" << ssid << ", error: " << e.what() << endl;
		return false;
	}

	try {
		// Reassociate interface
		interface->invoke_method_synchronously<core::WPASupplicant::Interface::Reassociate, void>();
	} catch (std::exception &e) {
		cout << "Failed to Reassociate to network :" << ssid << ", error: " << e.what() << endl;
		return false;
	}

	return true;
}

vector<string> getSSIDList(dbus::Object::Ptr interface)
{
	vector<dbus::types::ObjectPath> bssObjPaths = interface->get_property<core::WPASupplicant::Interface::Properties::BSSs>()->get();
	vector<string> ssids;
	std::unordered_set<string> detected_ssids; // to avoid duplicates

	for (auto &bssObjPath : bssObjPaths) {
		auto bss = interface->add_object_for_path(bssObjPath);
		vector<std::int8_t> ssid_raw = bss->get_property<core::WPASupplicant::BSS::Properties::SSID>()->get();
		std::string ssid{ssid_raw.begin(), ssid_raw.end()};
		if (ssid != "" && detected_ssids.find(ssid) == detected_ssids.end()) {
			ssids.push_back(std::string{ssid_raw.begin(), ssid_raw.end()});
			detected_ssids.insert(ssid);
		}
	}

	return ssids;
}

void showBSSList(dbus::Object::Ptr interface)
{
	vector<string> ssids = getSSIDList(interface);
	int ssid_selected = -1;
	std::string ssid;

	cout << "List of SSIDs detected: " << endl;
	for (int i = 0; i < ssids.size(); i++) {
		cout << i + 1 << ": " << ssids[i] << endl;
	}

	cout << "Select an SSID (enter number): " << endl;
	cin >> ssid_selected;

	ssid = ssids.at(ssid_selected - 1);

	connectNetwork(interface, ssid);
} 

int main(int argc, char *argv[])
{
	auto system_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::system);

	system_bus->install_executor(dbus::asio::make_executor(system_bus));
	std::thread t(&dbus::Bus::run, system_bus);

	auto service = dbus::Service::use_service(system_bus, dbus::traits::Service<core::WPASupplicant>::interface_name());
	auto root = service->object_for_path(dbus::types::ObjectPath("/fi/w1/wpa_supplicant1"));

	string interface_name = "wlp1s0";

	cout << "Using network interface: " << interface_name << endl;

	dbus::Object::Ptr interface = getInterface(root, interface_name);
	if (!interface) {
		interface = createInterface(root, interface_name);
		if (!interface) {
			cout << "Failed to create the interface: " << interface_name << endl;
			exit(1);
		}
	}

	auto bss_added_signal = interface->get_signal<core::WPASupplicant::Interface::Signals::BSSAdded>();
	bss_added_signal->connect([](const core::WPASupplicant::Interface::Signals::BSSAdded::ArgumentType &arg) {
		std::map<string, dbus::types::Variant> props = std::get<1>(arg);

		auto ssid_raw = props["SSID"].as<std::vector<std::int8_t>>();
		string ssid{ssid_raw.begin(), ssid_raw.end()};
		//std::cout << std::get<0>(arg).as_string() << endl;
		//std::cout << "BSSAdded: " << ssid << std::endl;
	});

	auto scan_done_signal = interface->get_signal<core::WPASupplicant::Interface::Signals::ScanDone>();
	scan_done_signal->connect([&interface](const bool &success) {
		std::cout << "ScanDone: " << success << std::endl;
		if (success) {
			showBSSList(interface);
		}
	});

	startScan(interface, "active");

	if (t.joinable()) {
		t.join();
	}
}
