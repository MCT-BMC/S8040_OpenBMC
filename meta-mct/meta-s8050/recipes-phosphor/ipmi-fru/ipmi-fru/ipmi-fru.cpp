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

namespace ipmi {

namespace storage {
std::map<uint8_t, std::string> __attribute__((init_priority(101)))
fruIDMap = {{0, "/sys/bus/i2c/devices/i2c-1/1-0050/eeprom"}};

std::map<uint8_t, std::vector<uint8_t>> fruCache
    __attribute__((init_priority(101)));

constexpr uint16_t FRUMAXSIZE = 0x8000;

void register_netfn_storage_functions() __attribute__((constructor));

void cacheFruData() {
  for (auto fruInfo : fruIDMap) {
    std::ifstream fruFile(fruInfo.second, std::ios::binary);
    if (!fruFile) {
      continue;
    }
    fruFile.seekg(0, std::ios::end);
    std::streampos fileSize = fruFile.tellg();
    fruFile.seekg(0, std::ios::beg);

    fruCache[fruInfo.first].resize(fileSize);
    fruFile.read(reinterpret_cast<char *>(fruCache[fruInfo.first].data()),
                 fileSize);
    fruCache[fruInfo.first].resize(FRUMAXSIZE);
  }
}

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

  if (fruIDMap.find(fruDeviceId) == fruIDMap.end() || fruDeviceId == 0xFF ||
      fruCache.find(fruDeviceId) == fruCache.end()) {
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

  if ((fruInventoryOffset + dataToWrite.size()) > FRUMAXSIZE) {
    return ipmi::responseParmOutOfRange();
  }

  std::copy(dataToWrite.begin(), dataToWrite.end(),
            fruCache[fruDeviceId].begin() + fruInventoryOffset);

  std::ofstream fruFile(fruIDMap[fruDeviceId],
                        std::ios::binary | std::ios::out);
  if (!fruFile) {
    return ipmi::responseUnspecifiedError();
  }

  fruFile.seekp(fruInventoryOffset);

  std::copy(dataToWrite.begin(), dataToWrite.end(),
            std::ostreambuf_iterator<char>(fruFile));

  countWritten = std::min(dataToWrite.size(), static_cast<size_t>(0xFF));

  return ipmi::responseSuccess(countWritten);
}

/**@brief implements the Read FRU Data command
 * @param fruDeviceId - FRU device ID. FFh = reserved
 * @param offset      - FRU inventory offset to read
 * @param readCount   - count to read
 *
 * @return IPMI completion code plus response data
 * - returnCount - response data count.
 * - data        -  response data
 */
ipmi::RspType<uint8_t,              // count returned
              std::vector<uint8_t>> // FRU data
ipmiStorageReadFruData(uint8_t fruDeviceId, uint16_t offset,
                       uint8_t readCount) {
  if (fruDeviceId == 0xFF) {
    return ipmi::responseInvalidFieldRequest();
  }

  auto iter = fruIDMap.find(fruDeviceId);
  if (iter == fruIDMap.end()) {
    return ipmi::responseSensorInvalid();
  }

  if (fruCache.find(fruDeviceId) == fruCache.end()) {
    return ipmi::responseSensorInvalid();
  }

  uint8_t returnCount;
  if ((offset + readCount) <= FRUMAXSIZE) {
    returnCount = readCount;
  } else {
    returnCount = FRUMAXSIZE - offset;
  }

  std::vector<uint8_t> fruData(
      (fruCache[fruDeviceId].begin() + offset),
      (fruCache[fruDeviceId].begin() + offset + returnCount));
  return ipmi::responseSuccess(returnCount, fruData);
}

/** @brief implements the get FRU Inventory Area Info command
 *
 *  @returns IPMI completion code plus response data
 *   - FRU Inventory area size in bytes,
 *   - access bit
 **/
ipmi::RspType<uint16_t, // FRU Inventory area size in bytes,
              uint8_t   // access size (bytes / words)
              >
ipmiStorageGetFruInvAreaInfo(uint8_t fruID) {

  auto iter = fruIDMap.find(fruID);
  if (iter == fruIDMap.end()) {
    return ipmi::responseSensorInvalid();
  }

  if (!std::filesystem::exists(iter->second)) {
    return ipmi::responseSensorInvalid();
  }

  return ipmi::responseSuccess(static_cast<uint16_t>(FRUMAXSIZE),
                               static_cast<uint8_t>(0));
}

void register_netfn_storage_functions() {
  cacheFruData();
  // <WRITE FRU Data>
  ipmi::registerHandler(ipmi::prioMax, ipmi::netFnStorage,
                        ipmi::storage::cmdWriteFruData,
                        ipmi::Privilege::Operator, ipmiStorageWriteFruData);
  // <READ FRU Data>
  ipmi::registerHandler(ipmi::prioMax, ipmi::netFnStorage,
                        ipmi::storage::cmdReadFruData,
                        ipmi::Privilege::Operator, ipmiStorageReadFruData);
  // <Get FRU Inventory Area Info>
  ipmi::registerHandler(ipmi::prioMax, ipmi::netFnStorage,
                        ipmi::storage::cmdGetFruInventoryAreaInfo,
                        ipmi::Privilege::User, ipmiStorageGetFruInvAreaInfo);
}
} // namespace storage
} // namespace ipmi