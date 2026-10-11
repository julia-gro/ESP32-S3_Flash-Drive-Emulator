#include <Arduino.h>

#include <USB.h>
#include <USBMSC.h>
#include <FirmwareMSC.h>
#include <esp_partition.h>
#include <esp_vfs_fat.h>
#include <wear_levelling.h>


#define BLOCK_SIZE 512
#define DEBUG 0

USBMSC MSC;
const esp_partition_t* PART;
const esp_partition_t* FS;
wl_handle_t FS_wl_handle = WL_INVALID_HANDLE;

#define MBR_PARTITION_OFFSET 0
char MBR_RECORD[] = 
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x52\x97\x81\xc2\x00\x00\x00\x00"
  "\x02\x00\x83\x03\x0b\x00\x01\x00\x00\x00\xc7\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa";


esp_vfs_fat_mount_config_t mount_config = {
    .format_if_mount_failed = true,
    .allocation_unit_size = 4096,
  };



void write_MBR(const esp_partition_t* PART) {
  esp_partition_erase_range(PART, 0, PART->size);

  esp_partition_write(PART, MBR_PARTITION_OFFSET, MBR_RECORD, 512);
}



static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  Serial.printf("READ: lba=%i offset=%i blocksize=%i\n", lba, offset, bufsize);

  #if DEBUG
  char* debug = (char*) malloc(bufsize);
  esp_partition_read(PART, (512 * lba) + offset, debug, bufsize);
  for (int i = 0; i < bufsize; i += 8) {
    Serial.printf("%02X %02X %02X %02X %02X %02X %02X %02X", debug[i], debug[i+1], debug[i+2], debug[i+3], debug[i+4], debug[i+5], debug[i+6], debug[i+7]);

    if (i % 16 != 0) {
      Serial.print("    ");
    } else {
      Serial.println("");
    }
  }
  #endif

  esp_partition_read(PART, (512 * lba) + MBR_PARTITION_OFFSET + offset, buffer, bufsize);
  return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  return true;
}


void setup() {
  delay(1000);
  Serial.begin(BAUD_RATE);
  Serial.println("Setup initialized ...");

  Serial.println("Searching partitions");
  PART = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, "FS");

  esp_partition_t* modified_partition = (esp_partition_t*) malloc(sizeof(esp_partition_t));
  *modified_partition = *PART;
  modified_partition->address += MBR_PARTITION_OFFSET + 512;
  modified_partition->size -= (MBR_PARTITION_OFFSET + 512);

  FS = (const esp_partition_t*) modified_partition;
  
  if (PART == nullptr) {
    Serial.println("Could not find 'storage' partition! Check partitions.csv");
    return;
  }

  if (FS == nullptr) {
    Serial.println("Could not find 'FS' parition! Check paritions.csv");
    return;
  }

  
  write_MBR(PART);

  wl_mount(FS, &FS_wl_handle);

  esp_vfs_fat_spiflash_mount("/FS", "FS", &mount_config, &FS_wl_handle);

  char trash[4096];
  memset(trash, 0x41, 4096);
  esp_partition_write(FS, 0, trash, 4096);


  MSC.vendorID("ESP32");
  MSC.productID("USB_MSC");
  MSC.productRevision("1.0");

  MSC.onRead(onRead);
  MSC.onWrite(onWrite);

  MSC.onStartStop(onStartStop);
  MSC.mediaPresent(true);
  MSC.begin((FS->size / BLOCK_SIZE) + 1, BLOCK_SIZE);

  USB.begin();

  Serial.println("Setup phase completed");
}

void loop() {
    delay(1000);
}
