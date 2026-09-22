# ESP32-S3 USB Mass Storage Device

Turn an ESP32-S3 into a USB Mass Storage Device (a USB stick) using its
native USB peripheral. Files are stored in an internal flash partition, so no
SD card or extra hardware is required. Built with PlatformIO and the Arduino
framework.

This started as a prototype toward a larger goal: a device you can load photos
onto over USB, and which later runs on its own (e.g. over Wi-Fi) to display
things like live weather.

## Features

- Enumerates as a standard USB Mass Storage Device — no drivers needed on the host
- Storage backed by an internal flash partition (FAT), no SD card required
- Correct flash handling (read–modify–erase–write) so the host can format and use the drive reliably
- Serial logging of every read/write for easy debugging


 
## Known limitations

- **Single writer at a time.** While mounted over USB, the host owns the
  filesystem; the firmware must not read/write the same data concurrently.
  A future "USB mode vs. standalone mode" split avoids this.
- **Small capacity.** Internal flash gives only ~1 MB here — fine for a
  prototype, but an SD card is the path to real capacity.
- **No flash wear-leveling.** Fine for occasional prototype use; not intended
  for heavy write workloads.

## Roadmap

- [ ] Wi-Fi connectivity
- [ ] OTA image API


