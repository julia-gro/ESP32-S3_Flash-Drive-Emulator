#include <Arduino.h>

#include <USB.h>
#include <USBMSC.h>
#include <FirmwareMSC.h>
#include <esp_partition.h>

#define BLOCK_SIZE 512
#define BLOCK_COUNT 128

USBMSC MSC;
const esp_partition_t* store;

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  // Write bufsize bytes from buffer to your storage at block lba
  Serial.printf("WRITE box %u size %u\n", lba, bufsize);
  const uint32_t SECTOR = 4096;
  uint32_t addr = lba * BLOCK_SIZE + offset;          //where in flash im writing
  uint32_t done = 0;

  while (done < bufsize) {
    uint32_t here           = addr + done;
    uint32_t chunkStart     = (here / SECTOR) * SECTOR;
    uint32_t within         = here - chunkStart;
    uint32_t canDo          = SECTOR - within;
    if (canDo > bufsize - done) canDo = bufsize - done;

    static uint8_t chunk[4096];               // a scratch copy of one 4096 chunk

    esp_partition_read(store, chunkStart, chunk, SECTOR);   //1. read old chunk
    memcpy(chunk + within, buffer + done, canDo);                //2. patch in new data
    esp_partition_erase_range(store, chunkStart, SECTOR);    //3. wipe whole chunk
    esp_partition_write(store, chunkStart, chunk, SECTOR);   //4. write all back

    done += canDo;
  }
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  // Read bufsize bytes into buffer from storage at block lba
  Serial.printf("WRITE box %u size %u\n", lba, bufsize);

  esp_partition_read(store, lba * 512 + offset, buffer, bufsize);
  return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  return true;
}


void setup() {
    delay(10000);
    Serial.begin(9600);
    store = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, "storage");
    
    if (store == nullptr) {
        Serial.println("Could not find 'storage' partition! Check partitions.csv");
        return;   // stop here instead of crashing
    }
    MSC.vendorID("ESP32");
    MSC.productID("USB_MSC");
    MSC.productRevision("1.0");

    MSC.onRead(onRead);
    MSC.onWrite(onWrite);

    MSC.onStartStop(onStartStop);
    MSC.mediaPresent(true);
    MSC.begin(store->size / BLOCK_SIZE, BLOCK_SIZE);

    USB.begin();

    Serial.println("Setup phase completed");

}

void loop() {
    delay(1000);
}
