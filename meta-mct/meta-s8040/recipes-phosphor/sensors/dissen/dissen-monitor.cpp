#include <algorithm>
#include <iostream>
#include <fstream>
#include <linux/i2c.h>
#include <gpiod.hpp>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/server.hpp>
#include <nlohmann/json.hpp>
#include <boost/bind/bind.hpp>
#include <sys/inotify.h>
#include <unistd.h>

#include <filesystem>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>

#define BIT(bit) ((uint16_t)(1 << bit))
#define SCU_SOC_RESET_REGISTER 0x1E6E2074
namespace fs = std::filesystem;
struct SensorInfo {
    std::string objpath;
    std::vector<std::string> sinterfaces;
    std::vector<std::string> sintivalue;
    std::shared_ptr<sdbusplus::asio::dbus_interface> vinterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> ainterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> finterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sinterface;

    std::string chassis_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> chassis_vinterface;

    std::string acpi_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> acpi_vinterface;

    std::string btn_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> btn_vinterface;

    std::string nmi_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> nmi_vinterface;

    std::string cpu_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> cpu_vinterface;

    std::string pwr_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> pwr_vinterface;

    std::string wdt_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> wdt_vinterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> wdt_ainterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> wdt_finterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> wdt_sinterface;

    std::string sel_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sel_vinterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sel_ainterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sel_finterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> sel_sinterface;

    std::string fwver_objpath;
    std::shared_ptr<sdbusplus::asio::dbus_interface> fwver_vinterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> fwver_ainterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> fwver_finterface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> fwver_sinterface;

};

SensorInfo sensor;
int inotify_fd;
char buffer[1024];
using DbusVariantType =
std::variant<std::vector<std::string>, std::vector<int>, std::string,
    int64_t, uint64_t, double, int32_t, uint32_t, int16_t,
    uint16_t, uint8_t, bool>;

static boost::asio::io_context ioCtx;
static boost::asio::steady_timer waitTimer(ioCtx);
static gpiod::line gpioline;
static boost::asio::posix::stream_descriptor gpiolinedes(ioCtx);
static boost::asio::posix::stream_descriptor stream_descriptor(ioCtx);
std::shared_ptr<sdbusplus::asio::connection> conn =
std::make_shared<sdbusplus::asio::connection>(ioCtx);
sdbusplus::asio::object_server hostServer = sdbusplus::asio::object_server(conn);

constexpr auto busName = "xyz.openbmc_project.dissen";
constexpr auto value = "xyz.openbmc_project.Sensor.Value";
constexpr auto availability = "xyz.openbmc_project.State.Decorator.Availability";
constexpr auto func = "xyz.openbmc_project.State.Decorator.OperationalStatus";

void addSelLog(std::string path, std::vector<uint8_t> event_data)
{
	auto method = conn->new_method_call("xyz.openbmc_project.Logging.IPMI", "/xyz/openbmc_project/Logging/IPMI", "xyz.openbmc_project.Logging.IPMI", "IpmiSelAdd");
	method.append(std::string{"SEL Entry"});
	method.append(path);
	method.append(event_data, true, uint16_t(0x0020));
	try
	{
		conn->call_noreply(method);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

auto startHostStateMonitor(void)
{
	return std::make_shared<sdbusplus::bus::match::match>(
			*conn,
			"type='signal',interface='org.freedesktop.DBus.Properties',"
			"member='PropertiesChanged',arg0='xyz.openbmc_project.State.Chassis'",
			[](sdbusplus::message::message &msg)
			{
			std::string interfaceName;

			boost::container::flat_map<std::string, DbusVariantType>
			propertiesChanged; // key:value

			try
			{
			msg.read(interfaceName, propertiesChanged);
			}
			catch (std::exception &e)
			{
			    std::cerr << "Get Runtime Error :" << e.what() << std::endl;
			    return;
			}
			auto pwrstatus = propertiesChanged.find("CurrentPowerState");
			if (pwrstatus == propertiesChanged.end())
			{
				return;
			}

			if (auto v = std::get_if<std::string>(&pwrstatus->second); v)
			{
				if (*v == "xyz.openbmc_project.State.Chassis.PowerState.Off")
				{
					sensor.acpi_vinterface->set_property("Value", double(BIT(5)));
					addSelLog(std::string(sensor.acpi_objpath), std::vector<uint8_t>({0x05, 0xff, 0xff}));
				}
				else
				{
					sensor.acpi_vinterface->set_property("Value", double(BIT(0)));
					addSelLog(std::string(sensor.acpi_objpath), std::vector<uint8_t>({0x00, 0xff, 0xff}));
				}
			}
			});
}

auto startButtonStateMonitor(void)
{
	return std::make_shared<sdbusplus::bus::match::match>(
			*conn,
			"type='signal',interface='org.freedesktop.DBus.Properties',"
			"member='PropertiesChanged',arg0='xyz.openbmc_project.Chassis.Buttons'",
			[](sdbusplus::message::message &msg)
			{
			std::string interfaceName;

			boost::container::flat_map<std::string, DbusVariantType>
			propertiesChanged; // key:value

			try
			{
			msg.read(interfaceName, propertiesChanged);
			}
			catch (std::exception &e)
			{

			std::cerr << "Get Runtime Error :" << e.what() << std::endl;
			return;
			}

        	std::string path = msg.get_path();
			//auto button= propertiesChanged.find("ButtonPressed");
			auto button= propertiesChanged.find("ButtonPressed");
        	if (button == propertiesChanged.end())
        	{
        	  return;
        	}
        	try
        	{
        	  bool state = std::get<bool>(button->second);
        	  if (state)
        	  {
        	    //if (path == "/xyz/openbmc_project/chassis/buttons/nmi")
				if (path.find("nmi") != std::string::npos)
        	    {
					sensor.nmi_vinterface->set_property("Value", double(BIT(1)));
					addSelLog(std::string(sensor.nmi_objpath), std::vector<uint8_t>({0x01, 0xff, 0xff}));
        	    }
        	    else if (path.find("power") != std::string::npos)
        	    {
					sensor.btn_vinterface->set_property("Value", double(BIT(0)));
					addSelLog(std::string(sensor.btn_objpath), std::vector<uint8_t>({0x00, 0xff, 0xff}));
        	    }
        	    else if (path.find("reset") != std::string::npos)
        	    {
					sensor.btn_vinterface->set_property("Value", double(BIT(2)));
					addSelLog(std::string(sensor.btn_objpath), std::vector<uint8_t>({0x02, 0xff, 0xff}));
        	    }else{
					sensor.btn_vinterface->set_property("Value", double(0));
					sensor.nmi_vinterface->set_property("Value", double(0));
				}
        	  }
        	}
        	catch (...)
        	{
        	  return;
        	}



			});
}

void CPU0_Thertrip_Handler(bool assert) // RISING_EDGE :true else false
{
    //assert low
	if (!assert)
	{
		sensor.cpu_vinterface->set_property("Value", double(BIT(1)));
		addSelLog(std::string(sensor.cpu_objpath), std::vector<uint8_t>({0x01, 0xff, 0xff}));
	}
}

void waitForGPIOEvent(const std::string &name,
		const std::function<void(bool)> &eventHandler,
		gpiod::line &line,
		boost::asio::posix::stream_descriptor &event)
{
	event.async_wait(boost::asio::posix::stream_descriptor::wait_read,
			[&name, eventHandler, &line,
			&event](const boost::system::error_code ec)
			{
			if (ec)
			{
			return;
			}
			gpiod::line_event line_event = line.event_read();
			eventHandler(line_event.event_type == gpiod::line_event::RISING_EDGE);
			waitForGPIOEvent(name, eventHandler, line, event);
			});
}

int loadConfigValues(void)
{
    const std::string configFilePath = "/usr/share/dissen/sensors.json";
    std::ifstream configFile(configFilePath.c_str());
    if (!configFile.is_open())
    {
        std::cerr << "loadConfigValues: Cannot open config path: " << configFilePath << std::endl;
        return -1;
    }
    auto jsonData = nlohmann::json::parse(configFile, nullptr, true, true);

    if (jsonData.is_discarded())
    {
        std::cerr << "Dissen-monitor readings JSON parser failure" << std::endl;
        return -1;
    }

    auto sensors = jsonData["sensors"];
    for (nlohmann::json &senConfig : sensors)
    {
        if (!senConfig.contains("objpath"))
        {
            std::cerr << "The 'objpath' field must be defined in Json file" << std::endl;
            return -1;
        }

        std::string objpath = senConfig["objpath"];        

        if (senConfig.contains("sinterfaces"))
        {
            std::string sinterfaces = senConfig["sinterfaces"];
            std::string sinitvalue = senConfig["sinitvalue"];

            if(std::string(objpath) == "/xyz/openbmc_project/sensors/status/IPMI_Watchdog")
            {
                sensor.wdt_objpath = std::string(objpath);
                sensor.wdt_vinterface = hostServer.add_interface(std::string(objpath), value);
                sensor.wdt_ainterface = hostServer.add_interface(std::string(objpath), availability);
                sensor.wdt_finterface = hostServer.add_interface(std::string(objpath), func);
                sensor.wdt_sinterface = hostServer.add_interface(std::string(objpath), std::string(sinterfaces));
                
                sensor.wdt_vinterface->register_property("Value", double(0));
                sensor.wdt_ainterface->register_property("Available", true);
                sensor.wdt_finterface->register_property("Functional", true);
                sensor.wdt_sinterface->register_property("Status", std::string(sinitvalue), [](const std::string &req, std::string &res)
                {
				    if(req == "Expired"){
				        sensor.wdt_vinterface->set_property("Value", double(BIT(0)));
                        //addSelLog(std::string(sensor.wdt_objpath), std::vector<uint8_t>({0x00, 0xff, 0xff}));
				    }
				    else if(req == "Reset"){
				        sensor.wdt_vinterface->set_property("Value", double(BIT(1)));
                        //addSelLog(std::string(sensor.wdt_objpath), std::vector<uint8_t>({0x01, 0xff, 0xff}));
				    }
				    else if (req == "Down")
				    { 
				        sensor.wdt_vinterface->set_property("Value", double(BIT(2)));
                        //addSelLog(std::string(sensor.wdt_objpath), std::vector<uint8_t>({0x02, 0xff, 0xff}));
				    }
				    else if(req == "Cycle"){
				        sensor.wdt_vinterface->set_property("Value", double(BIT(3)));
                        //addSelLog(std::string(sensor.wdt_objpath), std::vector<uint8_t>({0x03, 0xff, 0xff}));
				    }else if(req == "Interrupt"){
                        sensor.wdt_vinterface->set_property("Value", double(BIT(4)));
                        //addSelLog(std::string(sensor.wdt_objpath), std::vector<uint8_t>({0x04, 0xff, 0xff}));
                    }
                    return true; 
                });

                sensor.wdt_vinterface->initialize();
                sensor.wdt_ainterface->initialize();
                sensor.wdt_finterface->initialize();
                sensor.wdt_sinterface->initialize();
            }
            else if(std::string(objpath) == "/xyz/openbmc_project/sensors/status/SEL_Status")
            {
                sensor.sel_objpath = std::string(objpath);
                sensor.sel_vinterface = hostServer.add_interface(std::string(sensor.sel_objpath), value);
                sensor.sel_ainterface = hostServer.add_interface(std::string(sensor.sel_objpath), availability);
                sensor.sel_finterface = hostServer.add_interface(std::string(sensor.sel_objpath), func);
                sensor.sel_sinterface = hostServer.add_interface(std::string(sensor.sel_objpath), std::string(sinterfaces));

                sensor.sel_vinterface->register_property("Value", double(0));
                sensor.sel_ainterface->register_property("Available", true);
                sensor.sel_finterface->register_property("Functional", true);
                sensor.sel_sinterface->register_property("Status", std::string(sinitvalue), [](const std::string &req, std::string &res)
                {
				    if (req == "Clear")
				    {
				        sensor.sel_vinterface->set_property("Value", double(BIT(2)));
				        addSelLog(std::string(sensor.sel_objpath), std::vector<uint8_t>({0x02, 0xff, 0xff}));
				    }
                    return true; 
                });

                sensor.sel_vinterface->initialize();
                sensor.sel_ainterface->initialize();
                sensor.sel_finterface->initialize();
                sensor.sel_sinterface->initialize();
            }else if (std::string(objpath) == "/xyz/openbmc_project/sensors/status/Firmware_Change")
            {
                sensor.fwver_objpath = std::string(objpath);
                sensor.fwver_vinterface = hostServer.add_interface(std::string(sensor.fwver_objpath), value);
                sensor.fwver_ainterface = hostServer.add_interface(std::string(sensor.fwver_objpath), availability);
                sensor.fwver_finterface = hostServer.add_interface(std::string(sensor.fwver_objpath), func);
                sensor.fwver_sinterface = hostServer.add_interface(std::string(sensor.fwver_objpath), std::string(sinterfaces));

                sensor.fwver_vinterface->register_property("Value", double(0));
		        sensor.fwver_ainterface->register_property("Available", true);
		        sensor.fwver_finterface->register_property("Functional", true);
		        sensor.fwver_sinterface->register_property("Status", std::string(sinitvalue), [](const std::string &req, std::string &res)
				{
				    if (req == "Changed")
				    {
				        sensor.fwver_vinterface->set_property("Value", double(BIT(1)));
				        addSelLog(std::string(sensor.fwver_objpath), std::vector<uint8_t>({0x01, 0xff, 0xff}));
				    }
				    return true; 
                });
		        sensor.fwver_vinterface->initialize();
		        sensor.fwver_ainterface->initialize();
		        sensor.fwver_finterface->initialize();
		        sensor.fwver_sinterface->initialize();
            }
        }else{

            if(std::string(objpath) == "/xyz/openbmc_project/sensors/status/ACPI_Pwr_Status")
            {
                sensor.acpi_objpath = std::string(objpath);
                sensor.acpi_vinterface = hostServer.add_interface(std::string(sensor.acpi_objpath), value);
                sensor.ainterface = hostServer.add_interface(std::string(sensor.acpi_objpath), availability);
                sensor.finterface = hostServer.add_interface(std::string(sensor.acpi_objpath), func);
                
                sensor.acpi_vinterface->register_property("Value", double(0));
                sensor.ainterface->register_property("Available", true);
                sensor.finterface->register_property("Functional", true);

                sensor.acpi_vinterface->initialize();
                sensor.ainterface->initialize();
                sensor.finterface->initialize();
            }else if (std::string(objpath) == "/xyz/openbmc_project/sensors/status/Button_Status")
            {
                sensor.btn_objpath = std::string(objpath);
                sensor.btn_vinterface = hostServer.add_interface(std::string(sensor.btn_objpath), value);
                sensor.ainterface = hostServer.add_interface(std::string(sensor.btn_objpath), availability);
                sensor.finterface = hostServer.add_interface(std::string(sensor.btn_objpath), func);
                
                sensor.btn_vinterface->register_property("Value", double(0));
                sensor.ainterface->register_property("Available", true);
                sensor.finterface->register_property("Functional", true);

                sensor.btn_vinterface->initialize();
                sensor.ainterface->initialize();
                sensor.finterface->initialize();
            }else if (std::string(objpath) == "/xyz/openbmc_project/sensors/status/NMI_BTN_Status")
            {
                sensor.nmi_objpath = std::string(objpath);
                sensor.nmi_vinterface = hostServer.add_interface(std::string(sensor.nmi_objpath), value);
                sensor.ainterface = hostServer.add_interface(std::string(sensor.nmi_objpath), availability);
                sensor.finterface = hostServer.add_interface(std::string(sensor.nmi_objpath), func);
                
                sensor.nmi_vinterface->register_property("Value", double(0));
                sensor.ainterface->register_property("Available", true);
                sensor.finterface->register_property("Functional", true);

                sensor.nmi_vinterface->initialize();
                sensor.ainterface->initialize();
                sensor.finterface->initialize();
            }else if (std::string(objpath) == "/xyz/openbmc_project/sensors/status/CPU0_Status")
            {
                sensor.cpu_objpath = std::string(objpath);
                sensor.cpu_vinterface = hostServer.add_interface(std::string(sensor.cpu_objpath), value);
                sensor.ainterface = hostServer.add_interface(std::string(sensor.cpu_objpath), availability);
                sensor.finterface = hostServer.add_interface(std::string(sensor.cpu_objpath), func);
                
                sensor.cpu_vinterface->register_property("Value", double(0));
                sensor.ainterface->register_property("Available", true);
                sensor.finterface->register_property("Functional", true);

                sensor.cpu_vinterface->initialize();
                sensor.ainterface->initialize();
                sensor.finterface->initialize();
            }else if (std::string(objpath) == "/xyz/openbmc_project/sensors/status/Chassis_Status")
            {
                sensor.chassis_objpath = std::string(objpath);
                sensor.chassis_vinterface = hostServer.add_interface(std::string(sensor.chassis_objpath), value);
                sensor.ainterface = hostServer.add_interface(std::string(sensor.chassis_objpath), availability);
                sensor.finterface = hostServer.add_interface(std::string(sensor.chassis_objpath), func);

                sensor.chassis_vinterface->register_property("Value", double(0));
                sensor.ainterface->register_property("Available", true);
                sensor.finterface->register_property("Functional", true);

                sensor.chassis_vinterface->initialize();
                sensor.ainterface->initialize();
                sensor.finterface->initialize();
            }else if (std::string(objpath) == "/xyz/openbmc_project/sensors/status/PWR_Unit_Status")
            {
                sensor.pwr_objpath = std::string(objpath);
                sensor.pwr_vinterface = hostServer.add_interface(std::string(sensor.pwr_objpath), value);
                sensor.ainterface = hostServer.add_interface(std::string(sensor.pwr_objpath), availability);
                sensor.finterface = hostServer.add_interface(std::string(sensor.pwr_objpath), func);

                sensor.pwr_vinterface->register_property("Value", double(0));
                sensor.ainterface->register_property("Available", true);
                sensor.finterface->register_property("Functional", true);

                sensor.pwr_vinterface->initialize();
                sensor.ainterface->initialize();
                sensor.finterface->initialize();
            }
            else{
                sensor.vinterface = hostServer.add_interface(std::string(objpath), value);
                sensor.ainterface = hostServer.add_interface(std::string(objpath), availability);
                sensor.finterface = hostServer.add_interface(std::string(objpath), func);
                
                sensor.vinterface->register_property("Value", double(0),sdbusplus::asio::PropertyPermission::readWrite);
                sensor.ainterface->register_property("Available", true);
                sensor.finterface->register_property("Functional", true);

                sensor.vinterface->initialize();
                sensor.ainterface->initialize();
                sensor.finterface->initialize();
            }
        }
    }
    
    auto gpios =  jsonData["gpio_configs"];
    for (nlohmann::json &gpioConfig : gpios)
    {
        if (!gpioConfig.contains("Name"))
        {
            std::cerr << "The 'Name' field must be defined in Json file" << std::endl;
            return -1;
        }

        std::string gpioname = gpioConfig["Name"];
        std::string gpionumber = gpioConfig["LineName"];
        if (std::string(gpioname) == "CPU0_Thermtrip")
        {
            gpioline = gpiod::find_line(std::string(gpionumber));
	        if (!gpioline)
	        {
                std::cerr << "The gpioline is null" << std::endl;
	        	return -1;
	        }

            gpioline.request({"dissen", gpiod::line_request::EVENT_BOTH_EDGES, {}});
	        auto fd = gpioline.event_get_fd();
	        gpiolinedes.assign(fd);
	        waitForGPIOEvent(std::string(gpionumber), CPU0_Thertrip_Handler, gpioline, gpiolinedes);
        }else{
            std::cerr<<"NONE gpio init" <<std::endl;
        }
    }
    return true;
}


void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred,
                 boost::asio::posix::stream_descriptor& stream_descriptor, char* buffer, const std::string& file_path)
{
    if (!ec)
    {
        int i = 0;
        while (i < bytes_transferred)
        {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            if (event->mask & IN_MODIFY)
            {
		        sensor.chassis_vinterface->set_property("Value", double(BIT(0)));
		        addSelLog(std::string(sensor.chassis_objpath), std::vector<uint8_t>({0x00, 0xff, 0xff}));
            }

            i += sizeof(struct inotify_event) + event->len;
        }

        stream_descriptor.async_read_some(boost::asio::buffer(buffer, 1024),
                                          boost::bind(&handle_read, boost::asio::placeholders::error,
                                                      boost::asio::placeholders::bytes_transferred,
                                                      boost::ref(stream_descriptor), buffer, file_path));
    }
    else
    {
        std::cerr << "Error: " << ec.message() << std::endl;
    }
}

void startRegisterStateMonitor()
{
        inotify_fd = inotify_init();
        if (inotify_fd < 0)
        {
            std::cerr << "Failed to initialize inotify" << std::endl;
            return;
        }

        std::string sysfs_file = "/sys/devices/platform/ahb/ahb:apb/1e6ef010.chassis/hwmon/";
    
        for (const auto& entry : fs::directory_iterator(sysfs_file)) {
            if (fs::is_directory(entry.path())) {
                std::string hwmon_directory = entry.path();
                for (const auto& hwmon_entry : fs::directory_iterator(hwmon_directory)) {
                    std::string file_name = hwmon_entry.path().filename();
                    if (file_name == "intrusion0_alarm") {
                        sysfs_file = hwmon_entry.path();
                        std::cout << "File path: " << hwmon_entry.path() << std::endl;
                        break;
                    }
                }
            }
        }

        int watch_descriptor = inotify_add_watch(inotify_fd, sysfs_file.c_str(), IN_MODIFY);
        if (watch_descriptor < 0)
        {
            std::cerr << "Failed to add inotify watch for file: " << sysfs_file << std::endl;
            close(inotify_fd);
            return;
        }
        stream_descriptor.assign(inotify_fd);
        stream_descriptor.async_read_some(boost::asio::buffer(buffer),
                                          boost::bind(&handle_read, boost::asio::placeholders::error,
                                                      boost::asio::placeholders::bytes_transferred,
                                                      boost::ref(stream_descriptor), buffer, sysfs_file));
}

void mmapSCUAddr()
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        std::cerr << "Error opening /dev/mem"<< std::endl;
        return;
    }

    off_t target = SCU_SOC_RESET_REGISTER;
    size_t length = 4;
    off_t page_base = target & ~(sysconf(_SC_PAGE_SIZE) - 1);
    off_t page_offset = target - page_base;

    void *map_base = mmap(NULL, length + page_offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_base);
    if (map_base == MAP_FAILED) {
        std::cerr << "Error mapping memory" << std::endl;
        close(fd);
        return;
    }

    void *virt_addr = (char *)map_base + page_offset;
    uint32_t value = *((uint32_t *)virt_addr);

    if (!(value >> 16)&1)
    {
        sensor.pwr_vinterface->set_property("Value", double(BIT(4)));
        addSelLog(std::string(sensor.pwr_objpath), std::vector<uint8_t>({0x04, 0xff, 0xff}));
    }

    if (munmap(map_base, length + page_offset) == -1) {
        std::cerr << "Error unmapping memory"<< std::endl;
        close(fd);
        return;
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    conn->request_name(busName);

    //load sensors config from json,and init interfaces
    auto q = loadConfigValues();
    //start monitor chassis power status
    auto p = startHostStateMonitor();
    //start monitor button status
    auto w = startButtonStateMonitor();
    //start monitor register driver node
    startRegisterStateMonitor();
    //start check SCU register value
    mmapSCUAddr();

    ioCtx.run();
    close(inotify_fd);
    return 0;
}