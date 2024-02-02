#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"
#include "components/ble/MidiService.h"
#include "displayapp/widgets/StatusIcons.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {
      class Midi : public Screen {
      public:
        enum mode_enum {
          MODE_NAVIGATE1,
          MODE_NAVIGATE2,
          MODE_PADS,
          MODE_ENCODERS,
          MODE_SETTINGS,
          MODE_BLE,
          MODE_MIDICHAN,
          MODE_CCX,
          MODE_CCY,
          MODE_METROHIGH,
          MODE_METROLOW,
          MODE_TIMEOUT,
          MODE_BRIGHTNESS,
          MODE_XY,
          MODE_NUM_0, MODE_NUM_1, MODE_NUM_2, MODE_NUM_3, MODE_NUM_4, MODE_NUM_5, MODE_NUM_6, MODE_NUM_7, MODE_NUM_8, MODE_NUM_9,
          MODE_NONE
        };

        Midi(uint8_t mode,
            Controllers::MidiService& midiService,
            System::SystemTask& systemTask,
            const Controllers::Battery& batteryController,
            const Controllers::Ble& bleController);
        ~Midi() override;
        void Refresh() override;
        static void OnMidiEvent(void* target, const Pinetime::Controllers::MidiService::MidiEvent& event);

        struct Pad {
          lv_obj_t* obj;
          Midi* screen;
          uint8_t pad;
        };

        static void onPadEvent(lv_obj_t* obj, lv_event_t event);
        void onPadPress(uint8_t pad);
        void onPadRelease(uint8_t pad);
        bool OnTouchEvent(TouchEvents event) override;
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        uint8_t mode = MODE_PADS;
        Controllers::MidiService& midiService;
        System::SystemTask& systemTask;
        lv_task_t* taskRefresh;
        lv_obj_t* title;
        lv_obj_t* icon;

        // Pads
        static constexpr int colWidth = LV_HOR_RES_MAX / 4;
        static constexpr int rowHeight = (LV_VER_RES_MAX - 24) / 4;
        lv_obj_t* pads[16];
        lv_style_t padStyles[16];
        lv_style_t gridStyle;

        uint8_t touchedPad = -1;

        lv_obj_t* padMatrix;

        uint8_t ccX = 21;
        uint8_t ccY = 22;
        lv_obj_t* lineX;
        lv_obj_t* lineY;
        lv_point_t lineXPoints[2] {{120, 20}, {120, 240}};
        lv_point_t lineYPoints[2] {{0, 120}, {240, 120}};
        bool dirty = true;

        uint8_t metroLow = 48;
        uint8_t metroHigh = 49;

        Widgets::StatusIcons statusIcons;
      };
    }
    
    template <>
    struct AppTraits<Apps::MidiPads> {
      static constexpr Apps app = Apps::MidiPads;
      static constexpr const char* icon = Screens::Symbols::drum;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Midi(Pinetime::Applications::Screens::Midi::MODE_PADS, *controllers.midiService, *controllers.systemTask, controllers.batteryController, controllers.bleController);
      }
    };
    template <>
    struct AppTraits<Apps::MidiXY> {
      static constexpr Apps app = Apps::MidiXY;
      static constexpr const char* icon = Screens::Symbols::cloudMeatball;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Midi(Pinetime::Applications::Screens::Midi::MODE_XY, *controllers.midiService, *controllers.systemTask, controllers.batteryController, controllers.bleController);
      }
    };
  }
}