#include "displayapp/screens/Midi.h"
#include "systemtask/SystemTask.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"

using namespace Pinetime::Applications::Screens;

static constexpr lv_color_t MIDI_PAD_COLORS[] = {
  LV_COLOR_MAKE(0x66, 0x66, 0x66), // Disabled
  LV_COLOR_MAKE(0x66, 0x66, 0x00), // Starting
  LV_COLOR_MAKE(0x00, 0x80, 0x00), // Playing
  LV_COLOR_MAKE(0x80, 0x00, 0x00), // Stopping
  // Groups
  LV_COLOR_MAKE(0x66, 0x24, 0x26),
  LV_COLOR_MAKE(0x3c, 0x69, 0x64),
  LV_COLOR_MAKE(0x4d, 0x68, 0x17),
  LV_COLOR_MAKE(0x66, 0x49, 0x80),
  LV_COLOR_MAKE(0x4c, 0x70, 0x9a),
  LV_COLOR_MAKE(0x4c, 0x94, 0xcc),
  LV_COLOR_MAKE(0x00, 0x60, 0x00),
  LV_COLOR_MAKE(0xb7, 0xaa, 0x5e),
  LV_COLOR_MAKE(0x99, 0x66, 0x33),
  LV_COLOR_MAKE(0x74, 0x63, 0x60),
  LV_COLOR_MAKE(0xd0, 0x72, 0x72),
  LV_COLOR_MAKE(0x00, 0x00, 0x60),
  LV_COLOR_MAKE(0x04, 0x8c, 0x8c),
  LV_COLOR_MAKE(0xf4, 0x68, 0x15),
  LV_COLOR_MAKE(0xbf, 0x9c, 0x7c),
  LV_COLOR_MAKE(0x56, 0xa5, 0x56),
  LV_COLOR_MAKE(0xfc, 0x6c, 0xb4),
  LV_COLOR_MAKE(0xcc, 0x84, 0x64),
  LV_COLOR_MAKE(0x4c, 0x94, 0xcc),
  LV_COLOR_MAKE(0xb4, 0x54, 0xcc),
  LV_COLOR_MAKE(0xb0, 0x80, 0x80),
  LV_COLOR_MAKE(0x04, 0x04, 0xfc),
  LV_COLOR_MAKE(0x9e, 0xbd, 0xac),
  LV_COLOR_MAKE(0xff, 0x13, 0xfc),
  LV_COLOR_MAKE(0x30, 0x80, 0xc0),
  LV_COLOR_MAKE(0x9c, 0x7c, 0xec)
};

Midi::Midi(uint8_t mode,
      Pinetime::Controllers::MidiService& midiService,
      System::SystemTask& systemTask,
      const Controllers::Battery& batteryController,
      const Controllers::Ble& bleController
    ) :
        mode(mode),
        midiService(midiService),
        systemTask {systemTask},
        statusIcons(batteryController, bleController) {
  
  statusIcons.Create();
  title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "BLE MIDI");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  icon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(icon, Screens::Symbols::drum);
  lv_obj_align(icon, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_hidden(icon, true);

  switch (mode) {
    case MODE_XY:
      lineX = lv_line_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_line_width(lineX, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
      lv_obj_set_style_local_line_color(lineX, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
      lv_line_set_points(lineX, lineXPoints, 2);
      lineY = lv_line_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_line_width(lineY, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
      lv_obj_set_style_local_line_color(lineY, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
      lv_line_set_points(lineY, lineYPoints, 2);
      break;
    case MODE_PADS:
      /* button matrix - can't change individual button colours in lvgl 7.11.
      static const char * btnm_map[] = {"1", "5", "9", "13", "\n",
                                        "2", "6", "10", "14", "\n",
                                        "3", "7", "11", "15", "\n",
                                        "4", "8", "12", "16", ""};

      padMatrix = lv_btnmatrix_create(lv_scr_act(), nullptr);
      lv_btnmatrix_set_map(padMatrix, btnm_map);
      lv_btnmatrix_set_btn_ctrl_all(padMatrix, LV_BTNMATRIX_CTRL_NO_REPEAT);
      lv_obj_set_width(padMatrix, LV_HOR_RES_MAX);
      lv_obj_set_height(padMatrix, LV_VER_RES_MAX - 24);

      static lv_style_t style_bg;
      lv_style_init(&style_bg);
      lv_style_set_pad_all(&style_bg, LV_STATE_DEFAULT, 0);
      lv_style_set_border_width(&style_bg, LV_STATE_DEFAULT, 1);
      lv_style_set_radius(&style_bg, LV_STATE_DEFAULT, 0);
      lv_style_set_pad_inner(&style_bg, LV_STATE_DEFAULT, 1);
      lv_obj_add_style(padMatrix, LV_BTNMATRIX_PART_BG, &style_bg);

      static lv_style_t style_btn;
      lv_style_init(&style_btn);
      lv_style_set_pad_all(&style_btn, LV_STATE_DEFAULT, 0);
      lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, 0);
      lv_style_set_border_side(&style_btn, LV_STATE_DEFAULT, LV_BORDER_SIDE_INTERNAL);
      lv_obj_add_style(padMatrix, LV_BTNMATRIX_PART_BTN, &style_btn);

      lv_obj_align(padMatrix, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 24);
      lv_obj_set_user_data(padMatrix, this);
      lv_obj_set_event_cb(padMatrix, onPadEvent);
      lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, 0);
      */

      {
        int i = 0;
        char txt[3];
        for (int col = 0; col < 4; ++col) {
          for (int row = 0; row < 4; ++row) {
            lv_obj_t* pad = lv_obj_create(lv_scr_act(), NULL);
            pads[i] = pad;
            lv_obj_set_width(pad, 60);
            lv_obj_set_height(pad, 54);
            lv_obj_align(pad, NULL, LV_ALIGN_IN_TOP_LEFT, col * 60, 24 + row * 54);
            lv_obj_t* label = lv_label_create(pad, NULL);
            sprintf(txt, "%d", i + 1);
            lv_label_set_text(label, txt);
            lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

            lv_style_t* style_pad = &padStyles[i];
            lv_style_init(style_pad);
            lv_style_set_pad_all(style_pad, LV_STATE_DEFAULT, 1);
            lv_style_set_radius(style_pad, LV_STATE_DEFAULT, 0);
            lv_style_set_border_width(style_pad, LV_STATE_DEFAULT, 2);
            lv_style_set_border_color(style_pad, LV_STATE_DEFAULT, MIDI_PAD_COLORS[0]);
            lv_style_set_bg_color(style_pad, LV_STATE_DEFAULT, MIDI_PAD_COLORS[i]);
            lv_obj_add_style(pad, LV_OBJ_PART_MAIN, style_pad);
            ++i;
          }
        }
        // Create transparent overlay to handle all touch events
        lv_obj_t* grid = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_width(grid, 240);
        lv_obj_set_height(grid, 216);
        lv_obj_align(grid, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 24);
        lv_style_init(&gridStyle);
        lv_style_set_bg_opa(&gridStyle, LV_STATE_DEFAULT, LV_OPA_0);
        lv_obj_set_user_data(grid, this);
        lv_obj_set_event_cb(grid, onPadEvent);
      }
      break;
    default:
      break;
  }

  midiService.SetEventCallback(this, OnMidiEvent);
  systemTask.PushMessage(System::Messages::DisableSleeping);
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Midi::~Midi() {
  lv_obj_clean(lv_scr_act());
  lv_task_del(taskRefresh);
  systemTask.PushMessage(System::Messages::EnableSleeping);
}

void Midi::Refresh() {
  statusIcons.Update();
  if (dirty) {
    if (mode == MODE_XY) {
      lv_line_set_points(lineX, lineXPoints, 2);
      lv_line_set_points(lineY, lineYPoints, 2);
    }
    dirty = false;
  }
}

void Midi::onPadEvent(lv_obj_t* obj, lv_event_t event) {
  auto screen = static_cast<Midi*>(obj->user_data);
  uint32_t id = lv_btnmatrix_get_active_btn(screen->padMatrix);

  switch (event) {
    case LV_EVENT_VALUE_CHANGED:
      if (screen->touchedPad < 16)
        screen->onPadRelease(screen->touchedPad);
      screen->touchedPad = id;
      screen->onPadPress(screen->touchedPad);
      break;
    case LV_EVENT_PRESSED:
      screen->touchedPad = id;
      screen->onPadPress(screen->touchedPad);
      break;
    case LV_EVENT_RELEASED:
      if (screen->touchedPad < 16)
        screen->onPadRelease(screen->touchedPad);
      screen->touchedPad = -1;
      break;
    default:
      break;
  }
}

void Midi::onPadPress(uint8_t pad) {
  if (pad > 15)
    return;
  Pinetime::Controllers::MidiService::MidiEvent midiEvent;
  midiEvent.cmd = Pinetime::Controllers::MidiService::MIDI_NOTEON;
  midiEvent.val1 = 48 + pad;
  midiEvent.val2 = 100;
  midiService.SendCommand(midiEvent);
  lv_obj_set_hidden(icon, false);
}

void Midi::onPadRelease(uint8_t pad) {
  if (pad > 15)
    return;
  Pinetime::Controllers::MidiService::MidiEvent midiEvent;
  midiEvent.cmd = Pinetime::Controllers::MidiService::MIDI_NOTEOFF;
  midiEvent.val1 = 48 + pad;
  midiEvent.val2 = 100;
  midiService.SendCommand(midiEvent);
  lv_obj_set_hidden(icon, true);
}

bool Midi::OnTouchEvent(uint16_t x, uint16_t y) {
  static uint8_t lastCCX, lastCCY, val;
  switch (mode) {
    case MODE_XY:
      if (lineXPoints[0].x != x || lineYPoints[0].y != y) {
        lineXPoints[0].x = x;
        lineXPoints[1].x = x;
        lineYPoints[0].y = y;
        lineYPoints[1].y = y;
        lv_line_set_points(lineX, lineXPoints, 2);
        lv_line_set_points(lineY, lineYPoints, 2);
        val = 127 * x / LV_HOR_RES_MAX;
        if (val != lastCCX) {
          Pinetime::Controllers::MidiService::MidiEvent midiEvent;
          midiEvent.cmd = Pinetime::Controllers::MidiService::MIDI_CC;
          midiEvent.val1 = ccX;
          midiEvent.val2 = val;
          midiService.SendCommand(midiEvent);
          lastCCX = val;
          dirty = true;
        }
        val = 127 * y / LV_VER_RES_MAX;
        if (val != lastCCY) {
          Pinetime::Controllers::MidiService::MidiEvent midiEvent;
          midiEvent.cmd = Pinetime::Controllers::MidiService::MIDI_CC;
          midiEvent.val1 = ccY;
          midiEvent.val2 = val;
          midiService.SendCommand(midiEvent);
          lastCCY = val;
          dirty = true;
        }
      }
      break;
    case MODE_PADS:
      break;
    default:
      break;
  }
  return true;
}

bool Midi::OnTouchEvent(TouchEvents /*event*/) {
    return true; // Avoid any gesture interpretation
}

void Midi::OnMidiEvent(void* target, const Pinetime::Controllers::MidiService::MidiEvent& event) {
  const Midi* screen = static_cast<const Midi*>(target);
  char txt[32];

  switch(event.cmd & 0xF0) {
    case Pinetime::Controllers::MidiService::MIDI_NOTEON:
      if (event.val2) {
        if (screen->mode == Pinetime::Applications::Screens::Midi::MODE_PADS) {
          if (event.val1 < 16) {
            //uint8_t row = event.val1 % 4;
            //uint8_t col = event.val1 / 4;

            if (event.val2 == 0) {
              //lv_table_set_cell_value(screen->gridDisplay, row, col, "");
              //lv_label_set_text_static(screen->pads[event.val1].obj, "");
            }
            if (event.val2 < 4) {
              //lv_table_set_cell_type(screen->gridDisplay, row, col, event.val2);
              //lv_table_set_cell_value(screen->gridDisplay, row, col, ">");
              //!@todo Stop flashing
            } else if (event.val2 < 30) {
              //lv_table_set_cell_type(screen->gridDisplay, row, col, event.val2);
              //lv_table_set_cell_value(screen->gridDisplay, row, col, ">");
              //!@todo Stop flashing
            } else if (event.val2 < 34) {
              //lv_table_set_cell_type(screen->gridDisplay, row, col, event.val2 - 30);
              //!@todo Startflashing
            } else if (event.val2 < 64) {
              //lv_table_set_cell_type(screen->gridDisplay, row, col, event.val2 - 30);
              //!@todo Start flashing
            } else if (event.val2 < 90) {
              //lv_table_set_cell_type(screen->gridDisplay, row, col, event.val2 - 60);
              //lv_table_set_cell_value(screen->gridDisplay, row, col, ">");
              //!@todo Start flashing
            }
          } else if (event.val2 == screen->metroLow) {
            // vibrate low
            //!@todo Vibrate for metronome within BLE MIDI driver
          } else if (event.val2 == screen->metroHigh) {
            // vibrate high
          }
        }
        sprintf(txt, "Note %d ON vel:%d", event.val1, event.val2);
        break;
      }
      [[fallthrough]]; // Note-on with zero velocity treated as note-off
    case Pinetime::Controllers::MidiService::MIDI_NOTEOFF:
        sprintf(txt, "Note %d OFF", event.val1);
      break;
    case Pinetime::Controllers::MidiService::MIDI_CC:
        sprintf(txt, "CC %d val:%d", event.val1, event.val2);
      break;
    default:
      sprintf(txt, "Unknown event: 0x%02x", event.cmd);
      break;
  }
  lv_label_set_text_static(screen->title, txt);
}
