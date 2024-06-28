#include <boost/algorithm/string.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/process.hpp>
#include <filesystem>
#include <functional>
#include <iostream>
#include <ipmid/api.hpp>
#include <ipmid/message.hpp>
#include <ipmid/types.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/message/types.hpp>
#include <sdbusplus/timer.hpp>
#include <stdexcept>
#include <string_view>

#include "ipmi-fru.hpp"

namespace ipmi {

namespace storage {
static std::map<uint8_t, std::string> fruIDMap = {
    {0, "/sys/bus/i2c/devices/i2c-1/1-0050/eeprom"}};
std::unique_ptr<phosphor::Timer> writeTimer = nullptr;
constexpr auto syncTimerTimeOut = 8;
void register_netfn_storage_functions() __attribute__((constructor));

void syncFruIfRunning() {

  writeTimer->start(std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::seconds(syncTimerTimeOut)));
}

bool syncFru() {
  try {
    std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
    auto method = dbus->new_method_call(
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", "StartUnit");
    method.append("obmc-read-eeprom@system-chassis-motherboard.service",
                  "replace");
    dbus->call_noreply(method);
  } catch (const std::exception &e) {
    return false;
  }
  return true;
}

void createTimer() { writeTimer = std::make_unique<phosphor::Timer>(syncFru); }

/** @brief implements the write FRU data command
 *  @param fruDeviceId        - FRU Device ID
 *  @param fruInventoryOffset - FRU Inventory Offset to write
 *  @param dataToWrite        - Data to write
 *
 *  @returns ipmi completion code plus response data
 *   - countWritten  - Count written
 */
ipmi::RspType<uint8_t>
ipmiStorageWriteFruData(ipmi::Context::ptr ctx, uint8_t fruDeviceId,
                        uint16_t fruInventoryOffset,
                        std::vector<uint8_t> &dataToWrite) {
  constexpr uint8_t fruHeaderMutiRecOffset = 5;
  constexpr uint8_t fruHeaderCheckSumOffset = 7;
  uint8_t countWritten = 0;

  if (fruIDMap.find(fruDeviceId) == fruIDMap.end() || fruDeviceId == 0xFF) {
    return ipmi::responseInvalidFieldRequest();
  }

  // Not Support MutiRecord, set to disabled
  if (fruInventoryOffset == 0) {

    dataToWrite[fruHeaderMutiRecOffset] = 0;
    int zsum = 0;
    for (uint8_t index = 0; index < 5; index++)
      zsum += dataToWrite[index];
    dataToWrite[fruHeaderCheckSumOffset] = -(zsum % 256);
  }

  std::ofstream fruFile(fruIDMap[fruDeviceId],
                        std::ios::binary | std::ios::out);
  if (!fruFile) {
    return ipmi::responseUnspecifiedError();
  }

  fruFile.seekp(fruInventoryOffset);

  std::copy(dataToWrite.begin(), dataToWrite.end(),
            std::ostreambuf_iterator<char>(fruFile));

  countWritten = std::min(dataToWrite.size(), static_cast<size_t>(0xFF));

  if (fruDeviceId == 0) {
    syncFruIfRunning();
  }

  return ipmi::responseSuccess(countWritten);
}

void register_netfn_storage_functions() {

  createTimer();
  // <WRITE FRU Data>
  ipmi::registerHandler(ipmi::prioMax, ipmi::netFnStorage,
                        ipmi::storage::cmdWriteFruData,
                        ipmi::Privilege::Operator, ipmiStorageWriteFruData);
}
} // namespace storage
} // namespace ipmi
