/*  Copyright (C) 2024 Brian Walton

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#include <optional>
#include <cstring>
#undef max
#undef min

int MidiCallback(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);

namespace Pinetime {
  namespace Controllers {
    class NimbleController;

    class MidiService {
    public:
      enum MIDI_CMD {
        MIDI_NOTEOFF      = 0x80,
        MIDI_NOTEON       = 0x90,
        MIDI_POLYPRES     = 0xa0,
        MIDI_CC           = 0xb0,
        MIDI_PROGCHANGE   = 0xc0,
        MIDI_CHANPRES     = 0xd0,
        MIDI_PITCHBEND    = 0xe0,
        MIDI_SYSEX_START  = 0xf0,
        MIDI_TIMECODE     = 0xf1,
        MIDI_SONGPOS      = 0xf2,
        MIDI_SONGSEL      = 0xf3,
        MIDI_TUNEREQ      = 0xf6,
        MIDI_CLOCK        = 0xf8,
        MIDI_START        = 0xfa,
        MIDI_CONTINUE     = 0xfb,
        MIDI_STOP         = 0xfc,
        MIDI_ACTIVESENS   = 0xfd,
        MIDI_RESET        = 0xff
      };

      struct MidiEvent {
        uint8_t header = 0x80;
        uint8_t timestamp = 0x80;
        uint8_t cmd;
        uint8_t val1 = 0;
        uint8_t val2 = 0;
      };

      explicit MidiService(NimbleController& nimble);

      void Init();
      void SetEventCallback(void* target, void (*fptr)(void*, const MidiEvent&)) { eventTarget = target; eventCallback = fptr; };

      int OnCommand(struct ble_gatt_access_ctxt* ctxt);

      bool SendCommand(const MidiEvent& event);

    private:
      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t eventHandle {}; //!@todo Why does this have {} ?

      // 03B80E5A-EDE8-4B33-A751-6CE34EC4C700
      ble_uuid128_t midiServiceCharUuid = {.u = {.type = BLE_UUID_TYPE_128},
                          .value = {0x00, 0xc7, 0xc4, 0x4e, 0xe3, 0x6c, 0x51, 0xa7, 0x33, 0x4b, 0xe8, 0xed, 0x5a, 0x0e, 0xb8, 0x03}};
      // 7772E5DB-3868-4112-A1A9-F2669D106BF3
      ble_uuid128_t midiDataCharUuid = {.u = {.type = BLE_UUID_TYPE_128},
                          .value = {0xf3, 0x6b, 0x10, 0x9d, 0x66, 0xf2, 0xa9, 0xa1, 0x12, 0x41, 0x68, 0x38, 0xdb, 0xe5, 0x72, 0x77}};

      uint8_t runningStatus = 0;
      NimbleController& nimble;
      void (*eventCallback)(void*, const MidiEvent&) = nullptr;
      void* eventTarget = nullptr;
    };
  }
}
