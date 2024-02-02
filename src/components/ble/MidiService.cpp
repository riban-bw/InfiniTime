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

#include "components/ble/MidiService.h"
#include "components/ble/NimbleController.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <nrf_log.h>

using namespace Pinetime::Controllers;

MidiService::MidiService(NimbleController& nimble) : nimble(nimble) {
  characteristicDefinition[0] = {.uuid = &midiDataCharUuid.u,
                                 .access_cb = MidiCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_WRITE_NO_RSP,
                                 .val_handle = &eventHandle};
  characteristicDefinition[1] = {0};
  serviceDefinition[0] = {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &midiServiceCharUuid.u, .characteristics = characteristicDefinition};
  serviceDefinition[1] = {0};
};

int MidiCallback(uint16_t /*connHandle*/, uint16_t /*attrHandle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  return static_cast<Pinetime::Controllers::MidiService*>(arg)->OnCommand(ctxt);
}

void MidiService::Init() {
  ble_gatts_count_cfg(serviceDefinition);
  ble_gatts_add_svcs(serviceDefinition);
}

int MidiService::OnCommand(struct ble_gatt_access_ctxt* ctxt) {
  //static bool toggle = false; //!@todo Lose this debug
  if (!eventTarget)
    return 0; // No app registered to handle incoming MIDI messages
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    const auto* buffer = ctxt->om;
    const auto* dataBuffer = buffer->om_data;
    const auto dataLen = buffer->om_len;
    /* BLE MIDI data stream:
      Header Byte: 10xxxxxx x: 6 most significant timestamp bits
      Timestamp byte: 1xxxxxxx x: 7 least significant timestamp bits
      Data bytes: Raw MIDI data. May use running status. May interleave with Timestamp bytes.
    */

    if (dataLen < 3)
      return 0;
    uint8_t i = 0;
    uint16_t timestamp = 0;
    uint16_t timestampHigh = dataBuffer[i++] << 7;
    uint8_t timestampLow = 0; //!@todo Use timestamp to improve jitter
    MidiEvent event;
    while (i < dataLen) {
      if (dataBuffer[i] & 0x80) {
        if (timestampLow >= dataBuffer[i])
          timestampHigh += 0b10000000;
        timestampLow = dataBuffer[i++];
        timestamp = timestampHigh + timestampLow;
        if (dataBuffer[i] & 0x80)
          runningStatus = dataBuffer[i++];
        //!@todo Handle sysex within running status
      }
      if (runningStatus) {
        event.cmd = runningStatus;
        event.timestamp = timestamp;
        switch (runningStatus) {
          case MIDI_NOTEOFF:
          case MIDI_NOTEON:
          case MIDI_POLYPRES:
          case MIDI_CC:
          case MIDI_PITCHBEND:
          case MIDI_SONGPOS:
            if (i + 1 < dataLen) {
              event.val1 =  dataBuffer[i++];
              event.val2 = dataBuffer[i++];
              eventCallback(eventTarget, event);
            }
            break;
          case MIDI_PROGCHANGE:
          case MIDI_CHANPRES:
          case MIDI_TIMECODE:
          case MIDI_SONGSEL:
            if (i < dataLen) {
              event.val1 = dataBuffer[i++];
              eventCallback(eventTarget, event);
            }
            break;
          case MIDI_TUNEREQ:
          case MIDI_CLOCK:
          case MIDI_START:
          case MIDI_CONTINUE:
          case MIDI_STOP:
          case MIDI_ACTIVESENS:
          case MIDI_RESET:
            eventCallback(eventTarget, event);
            break;
          //!@todo: Handle Sysex
          default:
            ++i;
        }
      } else {
        // If running status has not been set then we assume a bad packet - this may not be true if unhandled messages are received
        break;
      }
    }
    runningStatus = 0;
  }
  return 0;
}

bool MidiService::SendCommand(const MidiEvent& event) {
  uint16_t connectionHandle = nimble.connHandle();
  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return false;
  }
  auto* om = ble_hs_mbuf_from_flat(&event, 5);
  ble_gattc_notify_custom(connectionHandle, eventHandle, om);
  return true;
}
