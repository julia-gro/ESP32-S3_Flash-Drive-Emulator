#include <Arduino.h>

#include <USB.h>
#include <USBMSC.h>

USBMSC MSC;

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  // Write bufsize bytes from buffer to your storage at block lba
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  // Read bufsize bytes into buffer from storage at block lba
  char array[bufsize] = { 0x41 };
  array[bufsize-1] = 0;
  buffer = array;
  return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  return true;
}


void setup() {
    delay(10000);
    Serial.begin(9600);
    
    USB.begin();
    MSC.vendorID("ESP32");
    MSC.productID("USB_MSC");
    MSC.productRevision("1.0");
    MSC.onRead(onRead);
    MSC.onWrite(onWrite);
    MSC.onStartStop(onStartStop);
    MSC.mediaPresent(true);
    MSC.begin(100, 4096);

    Serial.println("Setup phase completed");
}

void loop() {
    delay(1000);
}
