
#include "DialogMoveZ.hpp"
#include "ScreenHandler.hpp"
#include "png_resources.hpp"
#include "marlin_client.hpp"
#include "menu_vars.h"

bool DialogMoveZ::DialogShown = false;

DialogMoveZ::DialogMoveZ()
    : AddSuperWindow<IDialog>(GuiDefaults::EnableDialogBigLayout ? GuiDefaults::RectScreen : GuiDefaults::RectScreenNoFoot)
    , value(marlin_vars()->pos[2])
    , lastQueuedPos(marlin_vars()->pos[2])
    , axisText(this, text_rc, is_multiline::no, is_closed_on_click_t::no, _(axisLabel))
    , infoText(this, infoText_rc, is_multiline::yes, is_closed_on_click_t::no, _(infoTextContent))
    , closeText(this, closeText_rc, is_multiline::no, is_closed_on_click_t::no, _(closeTextContent))
#if (PRINTER_TYPE == PRINTER_PRUSA_XL || PRINTER_TYPE == PRINTER_PRUSA_IXL) // XL moves bed down while Z goes up
    , rightText(this, rightText_rc, is_multiline::no, is_closed_on_click_t::no, _(downTextContent))
    , leftText(this, leftText_rc, is_multiline::no, is_closed_on_click_t::no, _(upTextContent))
#else  /*PRINTER_TYPE*/
    , rightText(this, rightText_rc, is_multiline::no, is_closed_on_click_t::no, _(upTextContent))
    , leftText(this, leftText_rc, is_multiline::no, is_closed_on_click_t::no, _(downTextContent))
#endif /*PRINTER_TYPE*/
    , arrows(this, text_rc.TopRight(), { 0, 6, 0, 6 })
    , numb(this, numb_rc, marlin_vars()->pos[2], "%d mm", GuiDefaults::FontBig)
    , header(this, _(headerLabel))
    , icon(this, icon_rc, &png::turn_knob_81x55) {
    DialogShown = true;

    prev_accel = marlin_vars()->travel_acceleration;
    marlin_gcode("M204 T200");
    /// using window_t 1bit flag
    flags.close_on_click = is_closed_on_click_t::yes;
    header.SetIcon(&png::z_axis_16x16);

    constexpr static padding_ui8_t padding({ 6, 0, 6, 0 });

    //  info text
    infoText.SetPadding(padding);
    infoText.font = GuiDefaults::FontMenuSpecial;
    infoText.SetAlignment(Align_t::Center());

    //  close text
    closeText.SetPadding(padding);
    closeText.font = GuiDefaults::FontMenuSpecial;
    closeText.SetAlignment(Align_t::Center());

    //  UP DOWN texts
    rightText.font = GuiDefaults::FontBig;
    rightText.SetAlignment(Align_t::Left());
    leftText.font = GuiDefaults::FontBig;
    leftText.SetAlignment(Align_t::Right());

    // axis text
    axisText.font = GuiDefaults::FontBig;
    axisText.SetPadding({ 6, 0, 15, 0 });
    axisText.SetAlignment(Align_t::RightCenter());

    // numb
    numb.padding = { 15, 0, 6, 0 };
    numb.PrintAsInt32();
    numb.SetAlignment(Align_t::LeftCenter());

    // arrows
    arrows.SetState(WindowArrows::State_t::undef);
};

void DialogMoveZ::windowEvent(EventLock, [[maybe_unused]] window_t *sender, GUI_event_t event, void *param) {
#if PRINTER_TYPE == PRINTER_PRUSA_MINI
    constexpr static uint8_t len = 4;
#else
    constexpr static uint8_t len = 12;
#endif

    switch (event) {
    case GUI_event_t::CLICK: {
        /// has set is_closed_on_click_t
        /// todo
        /// GUI_event_t::CLICK could bubble into window_t::windowEvent and close dialog
        /// so CLICK could be left unhandled here
        /// but there is a problem with focus !!!parrent window of this dialog has it!!!
        if (flags.close_on_click == is_closed_on_click_t::yes)
            Screens::Access()->Close();
        return;
    }
    case GUI_event_t::ENC_DN: {
        const int enc_change = int(param);
        change(-enc_change);
        numb.SetValue(value);
#if (PRINTER_TYPE == PRINTER_PRUSA_XL || PRINTER_TYPE == PRINTER_PRUSA_IXL) // XL moves bed down while Z goes up
        arrows.SetState(WindowArrows::State_t::up);
#else  /*PRINTER_TYPE*/
        arrows.SetState(WindowArrows::State_t::down);
#endif /*PRINTER_TYPE*/
        return;
    }
    case GUI_event_t::ENC_UP: {
        const int enc_change = int(param);
        change(enc_change);
        numb.SetValue(value);
#if (PRINTER_TYPE == PRINTER_PRUSA_XL || PRINTER_TYPE == PRINTER_PRUSA_IXL) // XL moves bed down while Z goes up
        arrows.SetState(WindowArrows::State_t::down);
#else  /*PRINTER_TYPE*/
        arrows.SetState(WindowArrows::State_t::up);
#endif /*PRINTER_TYPE*/
        return;
    }
    case GUI_event_t::LOOP: {

        if (marlin_vars()->pqueue <= len) {
            int difference = value - lastQueuedPos;
            uint8_t freeSlots = len - marlin_vars()->pqueue;
            if (difference != 0) {
                for (uint8_t i = 0; i < freeSlots && lastQueuedPos != (int)value; i++) {
                    if (difference > 0) {
                        lastQueuedPos++;
                        difference--;
                    } else if (difference < 0) {
                        lastQueuedPos--;
                        difference++;
                    }
                    marlin_move_axis(lastQueuedPos, MenuVars::GetManualFeedrate()[2], 2);
                }
            }
        }
        return;
    }
    default:
        return;
    }
}
void DialogMoveZ::change(int diff) {
    int32_t val = diff + value;
    auto range = MenuVars::GetAxisRanges()[2];
    value = std::clamp(val, (int32_t)range[0], (int32_t)range[1]);
}

DialogMoveZ::~DialogMoveZ() {
    DialogShown = false;
    char msg[20];
    snprintf(msg, sizeof(msg), "M204 T%f", (double)prev_accel);
    marlin_gcode(msg);
}
void DialogMoveZ::Show() {
    // checking nesting to not open over some other blocking dialog
    // when blocking dialog is open, the nesting is larger than one
    if (!DialogShown && gui_get_nesting() <= 1) {
        DialogMoveZ moveZ;
        moveZ.MakeBlocking();
    }
}
