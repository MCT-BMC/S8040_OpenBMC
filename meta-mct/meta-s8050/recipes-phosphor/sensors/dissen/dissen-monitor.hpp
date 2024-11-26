#ifndef SENSOR_INFO_HPP
#define SENSOR_INFO_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <variant>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <nlohmann/json.hpp>
#include <linux/i2c.h>
#include <gpiod.hpp>

// Macro for bit manipulation
#define BIT(bit) ((uint16_t)(1 << bit))

// Using variant for dbus properties
using DbusVariantType = std::variant<std::vector<std::string>, std::vector<int>, std::string,
    int64_t, uint64_t, double, int32_t, uint32_t, int16_t,
    uint16_t, uint8_t, bool>;

// Structure to hold sensor information and interfaces
struct SensorInfo {
    std::string objPath;
    std::vector<std::string> sinterfaces;
    std::vector<std::string> sintivalue;
    std::shared_ptr<sdbusplus::asio::dbus_interface> vinterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> ainterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> finterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sinterface;

    std::string acpi_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> acpi_vinterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> acpi_ainterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> acpi_finterface;

    std::shared_ptr<sdbusplus::asio::dbus_interface> wdt_vinterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> wdt_ainterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> wdt_finterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> wdt_sinterface;
    std::string sel_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sel_vinterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sel_ainterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sel_finterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sel_sinterface;
};

// Class to manage sensor operations
class SensorManager {
private:
    boost::asio::io_context& ioCtx;
    SensorInfo sensor;
    std::shared_ptr<sdbusplus::asio::connection> conn;
    sdbusplus::asio::object_server hostServer;

public:
    SensorManager(boost::asio::io_context& ctx, SensorInfo& sensorRef);
    void addSelLog(std::string path, std::vector<uint8_t> event_data);
    std::shared_ptr<sdbusplus::bus::match::match> startHostStateMonitor();
    int loadConfigValues();
};

#endif // SENSOR_INFO_HPP
