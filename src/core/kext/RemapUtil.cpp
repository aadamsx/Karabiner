#include "RemapUtil.hpp"
#include "keycode.hpp"
#include "Config.hpp"
#include "util/PointingButtonStatus.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  ListFireConsumerKey listFireConsumerKey;
  ListFireRelativePointer listFireRelativePointer;
  FirePointingScroll firePointingScroll;

  bool
  RemapUtil::isInternalKeyboard(unsigned int keyboardType)
  {
    if (keyboardType == KeyboardType::MACBOOK) return true;
    if (keyboardType == KeyboardType::MACBOOK_COREDUO) return true;
    if (keyboardType == KeyboardType::POWERBOOK) return true;
    if (keyboardType == KeyboardType::POWERBOOK_G4) return true;
    if (keyboardType == KeyboardType::POWERBOOK_G4_TI) return true;
    if (keyboardType == KeyboardType::JIS_MACBOOK) return true;
    if (keyboardType == KeyboardType::JIS_MACBOOK_2008) return true;
    return false;
  }

  KeyCode::KeyCode
  RemapUtil::getModifierKeyCode(ModifierFlag::ModifierFlag flag)
  {
    if (flag == ModifierFlag::CAPSLOCK) return KeyCode::CAPSLOCK;
    if (flag == ModifierFlag::SHIFT_L) return KeyCode::SHIFT_L;
    if (flag == ModifierFlag::SHIFT_R) return KeyCode::SHIFT_R;
    if (flag == ModifierFlag::CONTROL_L) return KeyCode::CONTROL_L;
    if (flag == ModifierFlag::CONTROL_R) return KeyCode::CONTROL_R;
    if (flag == ModifierFlag::OPTION_L) return KeyCode::OPTION_L;
    if (flag == ModifierFlag::OPTION_R) return KeyCode::OPTION_R;
    if (flag == ModifierFlag::COMMAND_L) return KeyCode::COMMAND_L;
    if (flag == ModifierFlag::COMMAND_R) return KeyCode::COMMAND_R;
    if (flag == ModifierFlag::FN) return KeyCode::FN;

    //IOLog("KeyRemap4MacBook::getModifierKeyCode invalid args\n");
    return KeyCode::NONE;
  }

  ModifierFlag::ModifierFlag
  RemapUtil::getKeyCodeModifier(unsigned int keycode) {
    if (keycode == KeyCode::CAPSLOCK) return ModifierFlag::CAPSLOCK;
    if (keycode == KeyCode::SHIFT_L) return ModifierFlag::SHIFT_L;
    if (keycode == KeyCode::SHIFT_R) return ModifierFlag::SHIFT_R;
    if (keycode == KeyCode::CONTROL_L) return ModifierFlag::CONTROL_L;
    if (keycode == KeyCode::CONTROL_R) return ModifierFlag::CONTROL_R;
    if (keycode == KeyCode::OPTION_L) return ModifierFlag::OPTION_L;
    if (keycode == KeyCode::OPTION_R) return ModifierFlag::OPTION_R;
    if (keycode == KeyCode::COMMAND_L) return ModifierFlag::COMMAND_L;
    if (keycode == KeyCode::COMMAND_R) return ModifierFlag::COMMAND_R;
    if (keycode == KeyCode::FN) return ModifierFlag::FN;

    //IOLog("KeyRemap4MacBook::getKeyCodeModifier invalid args\n");
    return ModifierFlag::NONE;
  }

  namespace {
    inline bool isFromFlags(unsigned int flags, unsigned int fromFlags) {
      if (RemapUtil::isModifierOn(fromFlags, ModifierFlag::NONE)) {
        return (flags == ModifierFlag::stripNONE(fromFlags));
      } else {
        return ((flags & fromFlags) == fromFlags);
      }
    }
    void remapFlags(unsigned int fromFlags, unsigned int toFlags, unsigned int toKeyCode = KeyCode::NONE, bool isKeyDown = false) {
      // if to Modifier, we change the flag permanently, else change temporary.
      bool isToModifier = (RemapUtil::getKeyCodeModifier(toKeyCode) != ModifierFlag::NONE);

      for (int i = 0; i < ModifierFlag::listsize; ++i) {
        ModifierFlag::ModifierFlag m = ModifierFlag::list[i];
        FlagStatus::Item *p = FlagStatus::getFlagStatus(m);
        if (! p) continue;

        if (RemapUtil::isModifierOn(fromFlags, m)) {
          if (isToModifier) {
            if (isKeyDown) {
              p->decrease();
            } else {
              p->increase();
            }
          } else {
            // always decrease
            p->temporary_decrease();
          }
        }
        if (RemapUtil::isModifierOn(toFlags, m)) {
          if (isToModifier) {
            if (isKeyDown) {
              p->increase();
            } else {
              p->decrease();
            }
          } else {
            // always increase
            p->temporary_increase();
          }
        }
      }
    }
  }

  bool
  RemapUtil::keyToKey(const RemapParams &remapParams, KeyCode::KeyCode fromKeyCode, unsigned int fromFlags, KeyCode::KeyCode toKeyCode, unsigned int toFlags)
  {
    if (! isFromFlags(FlagStatus::makeFlags(remapParams), fromFlags)) return false;
    if (! RemapUtil::isKey(remapParams, fromKeyCode)) return false;

    ModifierFlag::ModifierFlag fromModifier = getKeyCodeModifier(fromKeyCode);
    ModifierFlag::ModifierFlag toModifier = getKeyCodeModifier(toKeyCode);

    FlagStatus::Item *fromStatus = FlagStatus::getFlagStatus(fromModifier);
    FlagStatus::Item *toStatus = FlagStatus::getFlagStatus(toModifier);

    bool isKeyDown = RemapUtil::isKeyDown(remapParams, fromKeyCode);

    if (fromStatus == NULL) {
      if (toStatus == NULL) {
        // key2key

      } else {
        // key2modifier
        if (isEvent_Down(remapParams)) {
          toStatus->increase();
        } else if (isEvent_Up(remapParams)) {
          toStatus->decrease();
        }
        (remapParams.params)->eventType = KeyEvent::MODIFY;
      }

    } else {
      if (toStatus == NULL) {
        // modifier2key
        if (isModifierOn(remapParams, fromModifier)) {
          fromStatus->decrease();
          (remapParams.params)->eventType = KeyEvent::DOWN;
        } else {
          fromStatus->increase();
          (remapParams.params)->eventType = KeyEvent::UP;
        }

      } else {
        // modifier2modifier
        if (isModifierOn(remapParams, fromModifier)) {
          toStatus->increase();
          fromStatus->decrease();
        } else {
          toStatus->decrease();
          fromStatus->increase();
        }
      }
    }

    (remapParams.params)->key = toKeyCode;
    remapFlags(fromFlags, toFlags, toKeyCode, isKeyDown);

    return true;
  }

  bool
  RemapUtil::keyToKey(const RemapParams& remapParams, KeyCode::KeyCode fromKeyCode, unsigned int fromFlags,
                      KeyCode::KeyCode toKeyCode1, unsigned int toFlags1,
                      KeyCode::KeyCode toKeyCode2, unsigned int toFlags2)
  {
    bool isKeyDown = RemapUtil::isKeyDown(remapParams, fromKeyCode);

    bool result = keyToKey(remapParams, fromKeyCode, fromFlags, toKeyCode1, toFlags1);
    if (! result) return false;

    if (isKeyDown) {
      // calc flags
      remapFlags(toFlags1, toFlags2, toKeyCode2, isKeyDown);
      unsigned int flags = FlagStatus::makeFlags(toKeyCode2);
      ListFireExtraKey::addKey(flags, toKeyCode2);
      // restore flags
      remapFlags(toFlags2, toFlags1, toKeyCode1, isKeyDown);
    }

    return true;
  }

  namespace {
    void
    keyToKeyCombination(const RemapParams &remapParams, KeyCode::KeyCode fromKeyCode,
                        KeyCode::KeyCode toKeyCode1, CharCode::CharCode toCharCode1,
                        KeyCode::KeyCode toKeyCode2, CharCode::CharCode toCharCode2)
    {
      if (! RemapUtil::isKeyDown(remapParams, fromKeyCode)) return;

      unsigned int flags = FlagStatus::makeFlags(remapParams);
      ListFireExtraKey::addKey(flags, toKeyCode1);
      if (toKeyCode2 != KeyCode::NONE) {
        ListFireExtraKey::addKey(flags, toKeyCode2);
      }
      (remapParams.params)->key = KeyCode::NONE;

      ExtraRepeatFunc::register_keyCombination(toKeyCode1, toCharCode1, toKeyCode2, toCharCode2);
      *(remapParams.ex_extraRepeatFunc) = ExtraRepeatFunc::extraRepeatFunc_keyCombination;
      *(remapParams.ex_extraRepeatFlags) = flags;
    }
  }

  bool
  RemapUtil::keyToKey_dependingShift(const RemapParams &remapParams, KeyCode::KeyCode fromKeyCode,
                                     KeyCode::KeyCode toKeyCode_noflag1, CharCode::CharCode toCharCode_noflag1,
                                     KeyCode::KeyCode toKeyCode_noflag2, CharCode::CharCode toCharCode_noflag2,
                                     KeyCode::KeyCode toKeyCode_shiftL1, CharCode::CharCode toCharCode_shiftL1,
                                     KeyCode::KeyCode toKeyCode_shiftL2, CharCode::CharCode toCharCode_shiftL2,
                                     KeyCode::KeyCode toKeyCode_shiftR1, CharCode::CharCode toCharCode_shiftR1,
                                     KeyCode::KeyCode toKeyCode_shiftR2, CharCode::CharCode toCharCode_shiftR2)
  {
    if (! RemapUtil::isKey(remapParams, fromKeyCode)) return false;

    if (FlagStatus::isHeldDown(ModifierFlag::SHIFT_L)) {
      FlagStatus::temporary_decrease(ModifierFlag::SHIFT_L);
      keyToKeyCombination(remapParams, fromKeyCode, toKeyCode_shiftL1, toCharCode_shiftL1, toKeyCode_shiftL2, toCharCode_shiftL2);

    } else if (FlagStatus::isHeldDown(ModifierFlag::SHIFT_R)) {
      FlagStatus::temporary_decrease(ModifierFlag::SHIFT_R);
      keyToKeyCombination(remapParams, fromKeyCode, toKeyCode_shiftR1, toCharCode_shiftR1, toKeyCode_shiftR2, toCharCode_shiftR2);

    } else {
      keyToKeyCombination(remapParams, fromKeyCode, toKeyCode_noflag1, toCharCode_noflag1, toKeyCode_noflag2, toCharCode_noflag2);
    }

    return true;
  }

  // ----------
  void
  RemapUtil::keyToPointingButton(const RemapParams &remapParams, KeyCode::KeyCode fromKeyCode, PointingButton::PointingButton toButton)
  {
    if (! RemapUtil::isKey(remapParams, fromKeyCode)) return;

    ModifierFlag::ModifierFlag fromModifier = getKeyCodeModifier(fromKeyCode);
    FlagStatus::Item *fromStatus = FlagStatus::getFlagStatus(fromModifier);

    if (fromStatus) {
      // clear flags
      if (RemapUtil::isKeyDown(remapParams, fromKeyCode)) {
        fromStatus->decrease();
      } else {
        fromStatus->increase();
      }
    }

    if (RemapUtil::isKeyDown(remapParams, fromKeyCode)) {
      listFireRelativePointer.add(toButton);

      PointingButtonStatus::set(toButton, true);

    } else {
      listFireRelativePointer.add(PointingButton::NONE);

      PointingButtonStatus::set(toButton, false);
    }

    (remapParams.params)->key = KeyCode::NONE;
  }

  bool
  RemapUtil::keyToConsumer(const RemapParams &remapParams,
                           KeyCode::KeyCode fromKeyCode, unsigned int fromFlags,
                           ConsumerKeyCode::ConsumerKeyCode toKeyCode, unsigned int toFlags)
  {
    if (! isFromFlags(FlagStatus::makeFlags(KeyCode::NONE), fromFlags)) return false;
    if (! RemapUtil::isKey(remapParams, fromKeyCode)) return false;

    remapFlags(fromFlags, toFlags);

    unsigned int eventType = (RemapUtil::isKeyDown(remapParams, fromKeyCode) ? KeyEvent::DOWN : KeyEvent::UP);
    unsigned int flags = FlagStatus::makeFlags(toKeyCode);
    listFireConsumerKey.add(eventType, flags, toKeyCode);

    (remapParams.params)->key = KeyCode::NONE;
    return true;
  }

  bool
  RemapUtil::consumerToKey(const RemapConsumerParams &remapParams,
                           ConsumerKeyCode::ConsumerKeyCode fromKeyCode, unsigned int fromFlags,
                           KeyCode::KeyCode toKeyCode, unsigned int toFlags)
  {
    if (! isFromFlags(FlagStatus::makeFlags(KeyCode::NONE), fromFlags)) return false;
    if (! isKey(remapParams, fromKeyCode)) return false;

    bool isKeyDown = ((remapParams.params)->eventType == KeyEvent::DOWN);
    remapFlags(fromFlags, toFlags, toKeyCode, isKeyDown);

    if (RemapUtil::getKeyCodeModifier(toKeyCode) != ModifierFlag::NONE) {
      (remapParams.params)->eventType = KeyEvent::MODIFY;
    }
    (remapParams.params)->key = ConsumerKeyCode::NONE;
    *(remapParams.ex_remapKeyCode) = toKeyCode;

    return true;
  }

  bool
  RemapUtil::consumerToConsumer(const RemapConsumerParams &remapParams,
                                ConsumerKeyCode::ConsumerKeyCode fromKeyCode, unsigned int fromFlags,
                                ConsumerKeyCode::ConsumerKeyCode toKeyCode, unsigned int toFlags)
  {
    if (! isFromFlags(FlagStatus::makeFlags(KeyCode::NONE), fromFlags)) return false;
    if (! isKey(remapParams, fromKeyCode)) return false;

    remapFlags(fromFlags, toFlags);

    (remapParams.params)->key = toKeyCode;
    (remapParams.params)->flavor = toKeyCode;
    return true;
  }

  bool
  RemapUtil::pointingButtonToPointingButton(const RemapPointingParams_relative &remapParams,
                                            PointingButton::PointingButton fromButton, unsigned int fromFlags,
                                            PointingButton::PointingButton toButton)
  {
    if (! isFromFlags(FlagStatus::makeFlags(KeyCode::NONE), fromFlags)) return false;
    if (! ((remapParams.params)->buttons & fromButton)) return false;

    // XXX: call fireModifiers if needed.
    remapFlags(fromFlags, 0);

    (remapParams.params)->buttons = ((remapParams.params)->buttons & ~fromButton);
    (remapParams.params)->buttons |= toButton;
    return true;
  }

  // --------------------
  void
  RemapUtil::fireModifiers(KeyboardEventCallback callback, const Params_KeyboardEventCallBack &params)
  {
    if (callback == NULL) return;

    static unsigned int lastFlags = 0;
    unsigned int toFlags = params.flags;

    if (lastFlags == toFlags) return;
#if 0
    printf("RemapUtil::fireModifiers from:%x to:%x\n", lastFlags, toFlags);
#endif

    // ----------------------------------------------------------------------
    bool modifierStatus[ModifierFlag::listsize];

    // setup modifierStatus
    for (int i = 0; i < ModifierFlag::listsize; ++i) {
      modifierStatus[i] = RemapUtil::isModifierOn(lastFlags, ModifierFlag::list[i]);
    }

    Params_KeyboardEventCallBack callbackparams = params;
    callbackparams.eventType = KeyEvent::MODIFY;

    // fire
    for (int i = 0; i < ModifierFlag::listsize; ++i) {
      ModifierFlag::ModifierFlag m = ModifierFlag::list[i];
      bool from = RemapUtil::isModifierOn(lastFlags, m);
      bool to = RemapUtil::isModifierOn(toFlags, m);

      if (from == to) continue;

      if (from) {
        modifierStatus[i] = false;
      } else {
        modifierStatus[i] = true;
      }

      unsigned int flags = 0;
      for (int j = 0; j < ModifierFlag::listsize; ++j) {
        if (modifierStatus[j]) {
          flags |= ModifierFlag::list[j];
        }
      }

      callbackparams.flags = flags;
      callbackparams.key = RemapUtil::getModifierKeyCode(m);
      callbackparams.apply(callback);
    }

    lastFlags = toFlags;
  }

  void
  RemapUtil::fireKey(KeyboardEventCallback callback, const Params_KeyboardEventCallBack &params)
  {
    if (! callback) return;
    if (params.key == KeyCode::NONE) return;

    Params_KeyboardEventCallBack p = params;
    KeyCode::reverseNormalizeKey(p.key, p.flags, p.keyboardType);

    RemapUtil::fireModifiers(callback, p);

    if (p.eventType == KeyEvent::DOWN || p.eventType == KeyEvent::UP) {
      p.apply(callback);

      if (p.eventType == KeyEvent::DOWN) {
        PressDownKeys::add(p.key, p.keyboardType);
      } else {
        PressDownKeys::remove(p.key, p.keyboardType);
      }
    }
  }

  void
  RemapUtil::fireConsumer(KeyboardSpecialEventCallback callback, const Params_KeyboardSpecialEventCallback &params)
  {
    if (! callback) return;
    if (params.key == ConsumerKeyCode::NONE) return;

    Params_KeyboardSpecialEventCallback p = params;
    p.apply(callback);
  }

  // --------------------
  void
  RemapUtil::jis_toggle_eisuu_kana(const RemapParams &params, KeyCode::KeyCode fromKeyCode)
  {
    if (! RemapUtil::isKey(params, fromKeyCode)) return;

    static KeyCode::KeyCode toKeyCode = KeyCode::NONE;
    if (RemapUtil::isKeyDown(params, fromKeyCode)) {
      if (params.inputmode == KeyRemap4MacBook_bridge::GetWorkspaceData::INPUTMODE_JAPANESE) {
        toKeyCode = KeyCode::JIS_EISUU;
      } else {
        toKeyCode = KeyCode::JIS_KANA;
      }
    }
    RemapUtil::keyToKey(params, fromKeyCode, toKeyCode);
  }

  // --------------------
  void
  RemapUtil::pointingRelativeToScroll(const RemapPointingParams_relative &remapParams)
  {
    *(remapParams.ex_dropEvent) = true;

    // ----------------------------------------
    int delta1 = - (remapParams.params)->dy;
    int delta2 = - (remapParams.params)->dx;

    if (config.option_pointing_disable_vertical_scroll) delta1 = 0;
    if (config.option_pointing_disable_horizontal_scroll) delta2 = 0;

    // ----------------------------------------
    // ignore minuscule move
    int abs1 = delta1 > 0 ? delta1 : -delta1;
    int abs2 = delta2 > 0 ? delta2 : -delta2;
    int SCALE = 100;

    if (abs1 > abs2) {
      // case y > x (ignore x if x is very small)
      if (abs2) {
        if ((abs1 * SCALE / abs2) > (10 * SCALE / 5)) delta2 = 0;
      }
    }
    if (abs1 < abs2) {
      if (abs1) {
        if ((abs2 * SCALE / abs1) > (10 * SCALE / 5)) delta1 = 0;
      }
    }

    if (delta1 == 0 && delta2 == 0) return;

    // ----------------------------------------
    int deltaAxis1 = (delta1 * config.pointing_relative2scroll_rate) / 1000;
    if (deltaAxis1 == 0 && delta1 != 0) {
      deltaAxis1 = delta1 > 0 ? 1 : -1;
    }
    int deltaAxis2 = (delta2 * config.pointing_relative2scroll_rate) / 1000;
    if (deltaAxis2 == 0 && delta2 != 0) {
      deltaAxis2 = delta2 > 0 ? 1 : -1;
    }

    // ----------------------------------------
    IOFixed fixedDelta1 = (delta1 * POINTING_FIXED_SCALE * config.pointing_relative2scroll_rate) / 1000;
    IOFixed fixedDelta2 = (delta2 * POINTING_FIXED_SCALE * config.pointing_relative2scroll_rate) / 1000;

    SInt32 pointDelta1 = (delta1 * POINTING_POINT_SCALE * config.pointing_relative2scroll_rate) / 1000;
    SInt32 pointDelta2 = (delta2 * POINTING_POINT_SCALE * config.pointing_relative2scroll_rate) / 1000;

    firePointingScroll.set(deltaAxis1, deltaAxis2, 0,
                           fixedDelta1, fixedDelta2, 0,
                           pointDelta1, pointDelta2, 0);
  }

  // ----------------------------------------------------------------------
  void
  FireFunc::firefunc_nop(const RemapParams &params)
  {
  }

  void
  FireFunc::firefunc_backslash(const RemapParams &params)
  {
    unsigned int flags = FlagStatus::makeFlags(params);
    ListFireExtraKey::addKey(flags, KeyCode::BACKSLASH);
  }

  void
  FireFunc::firefunc_commandO(const RemapParams &params)
  {
    // fire only if no-modifiers
    if (FlagStatus::makeFlags(params) != 0) return;

    ListFireExtraKey::add(KeyEvent::DOWN, ModifierFlag::COMMAND_L, KeyCode::O);
    ListFireExtraKey::add(KeyEvent::UP,   ModifierFlag::COMMAND_L, KeyCode::O);
  }

  void
  FireFunc::firefunc_commandSpace(const RemapParams &params)
  {
    // fire only if no-modifiers
    if (FlagStatus::makeFlags(params) != 0) return;

    ListFireExtraKey::add(KeyEvent::DOWN, ModifierFlag::COMMAND_L, KeyCode::SPACE);
    ListFireExtraKey::add(KeyEvent::UP,   ModifierFlag::COMMAND_L, KeyCode::SPACE);
  }

  void
  FireFunc::firefunc_enter(const RemapParams &params)
  {
    unsigned int flags = FlagStatus::makeFlags(params);
    ListFireExtraKey::addKey(flags, KeyCode::ENTER);
  }

  void
  FireFunc::firefunc_escape(const RemapParams &params)
  {
    unsigned int flags = FlagStatus::makeFlags(params);
    ListFireExtraKey::addKey(flags, KeyCode::ESCAPE);
  }

  void
  FireFunc::firefunc_exposeAll(const RemapParams &params)
  {
    unsigned int flags = FlagStatus::makeFlags(params);
    ListFireExtraKey::addKey(flags, KeyCode::EXPOSE_ALL);
  }

  void
  FireFunc::firefunc_return(const RemapParams &params)
  {
    unsigned int flags = FlagStatus::makeFlags(params);
    ListFireExtraKey::addKey(flags, KeyCode::RETURN);
  }

  void
  FireFunc::firefunc_space(const RemapParams &params)
  {
    unsigned int flags = FlagStatus::makeFlags(params);
    ListFireExtraKey::addKey(flags, KeyCode::SPACE);
  }

  void
  FireFunc::firefunc_tab(const RemapParams &params)
  {
    unsigned int flags = FlagStatus::makeFlags(params);
    ListFireExtraKey::addKey(flags, KeyCode::TAB);
  }

  void
  FireFunc::firefunc_emacsmode_controlK(const RemapParams &params, bool first)
  {
    static int counter = 0;
    if (first) counter = 0;

    if (counter % 2 == 0) {
      // Command+Shift+Right
      ListFireExtraKey::add(KeyEvent::DOWN, ModifierFlag::COMMAND_L | ModifierFlag::SHIFT_L, KeyCode::CURSOR_RIGHT);
      ListFireExtraKey::add(KeyEvent::UP,   ModifierFlag::COMMAND_L | ModifierFlag::SHIFT_L, KeyCode::CURSOR_RIGHT);

      // Command+X
      ListFireExtraKey::add(KeyEvent::DOWN, ModifierFlag::COMMAND_L, KeyCode::X);
      ListFireExtraKey::add(KeyEvent::UP,   ModifierFlag::COMMAND_L, KeyCode::X);

    } else {
      // Forward Delete
      ListFireExtraKey::add(KeyEvent::DOWN, 0, KeyCode::FORWARD_DELETE);
      ListFireExtraKey::add(KeyEvent::UP,   0, KeyCode::FORWARD_DELETE);
    }

    ++counter;
  }

  void
  FireFunc::firefunc_emacsmode_controlK_2nd(const RemapParams &params)
  {
    firefunc_emacsmode_controlK(params, false);
  }

  void
  FireFunc::firefunc_french_backslash(const RemapParams &params)
  {
    ListFireExtraKey::addKey(ModifierFlag::OPTION_L | ModifierFlag::SHIFT_L, KeyCode::DOT);
  }

  void
  FireFunc::firefunc_jis_kana(const RemapParams &params)
  {
    // fire only if no-modifiers
    if (FlagStatus::makeFlags(params) != 0) return;

    ListFireExtraKey::addKey(0, KeyCode::JIS_KANA);
  }

  void
  FireFunc::firefunc_jis_kana_x2(const RemapParams &params)
  {
    // fire only if no-modifiers
    if (FlagStatus::makeFlags(params) != 0) return;

    ListFireExtraKey::addKey(0, KeyCode::JIS_KANA);
    ListFireExtraKey::addKey(0, KeyCode::JIS_KANA);
  }

  void
  FireFunc::firefunc_jis_eisuu(const RemapParams &params)
  {
    // fire only if no-modifiers
    if (FlagStatus::makeFlags(params) != 0) return;

    ListFireExtraKey::addKey(0, KeyCode::JIS_EISUU);
  }

  void
  FireFunc::firefunc_jis_eisuu_x2(const RemapParams &params)
  {
    // fire only if no-modifiers
    if (FlagStatus::makeFlags(params) != 0) return;

    ListFireExtraKey::addKey(0, KeyCode::JIS_EISUU);
    ListFireExtraKey::addKey(0, KeyCode::JIS_EISUU);
  }

  void
  FireFunc::firefunc_jis_toggle_eisuu_kana(const RemapParams &params)
  {
    // fire only if no-modifiers
    if (FlagStatus::makeFlags(params) != 0) return;

    if (params.inputmode == KeyRemap4MacBook_bridge::GetWorkspaceData::INPUTMODE_JAPANESE) {
      ListFireExtraKey::addKey(0, KeyCode::JIS_EISUU);
    } else {
      ListFireExtraKey::addKey(0, KeyCode::JIS_KANA);
    }
  }

  // --------------------
  void
  ExtraRepeatFunc::extraRepeatFunc_enter(KeyboardEventCallback callback, OSObject *target, unsigned int flags, unsigned int keyboardType, AbsoluteTime ts, OSObject *sender, void *refcon)
  {
    if (callback == NULL) return;

    Params_KeyboardEventCallBack params = {
      target, KeyEvent::DOWN, flags, KeyCode::ENTER,
      0, 0, 0, 0,
      keyboardType, false, ts, sender, refcon,
    };
    RemapUtil::fireKey(callback, params);

    params.eventType = KeyEvent::UP;
    RemapUtil::fireKey(callback, params);
  }

  void
  ExtraRepeatFunc::extraRepeatFunc_space(KeyboardEventCallback callback, OSObject *target, unsigned int flags, unsigned int keyboardType, AbsoluteTime ts, OSObject *sender, void *refcon)
  {
    if (callback == NULL) return;

    Params_KeyboardEventCallBack params = {
      target, KeyEvent::DOWN, flags, KeyCode::SPACE,
      0, 0, 0, 0,
      keyboardType, false, ts, sender, refcon,
    };

    RemapUtil::fireKey(callback, params);

    params.eventType = KeyEvent::UP;
    RemapUtil::fireKey(callback, params);
  }

  void
  ExtraRepeatFunc::call_firefunc(FireFunc::FireFunc firefunc, KeyboardEventCallback callback, OSObject *target, unsigned int flags, unsigned int keyboardType, AbsoluteTime ts, OSObject *sender, void *refcon)
  {
    if (callback == NULL) return;

    ListFireExtraKey::reset();
    {
      RemapParams params;
      firefunc(params);
    }
    {
      Params_KeyboardEventCallBack params = {
        target, 0, flags, KeyCode::SPACE,
        0, 0, 0, 0,
        keyboardType, false, ts, sender, refcon,
      };
      ListFireExtraKey::fire(callback, params);
    }
  }

  void
  ExtraRepeatFunc::extraRepeatFunc_emacsmode_controlK(KeyboardEventCallback callback, OSObject *target, unsigned int flags, unsigned int keyboardType, AbsoluteTime ts, OSObject *sender, void *refcon)
  {
    call_firefunc(FireFunc::firefunc_emacsmode_controlK_2nd, callback, target, flags, keyboardType, ts, sender, refcon);
  }

  namespace ExtraRepeatFunc {
    struct KeyCombination {
      KeyCode::KeyCode keyCode1;
      KeyCode::KeyCode keyCode2;
      CharCode::CharCode charCode1;
      CharCode::CharCode charCode2;
    } keyCombination;
  }

  void
  ExtraRepeatFunc::register_keyCombination(KeyCode::KeyCode keyCode1, CharCode::CharCode charCode1, KeyCode::KeyCode keyCode2, CharCode::CharCode charCode2)
  {
    keyCombination.keyCode1 = keyCode1;
    keyCombination.keyCode2 = keyCode2;
    keyCombination.charCode1 = charCode1;
    keyCombination.charCode2 = charCode2;
  }

  void
  ExtraRepeatFunc::extraRepeatFunc_keyCombination(KeyboardEventCallback callback, OSObject *target, unsigned int flags, unsigned int keyboardType, AbsoluteTime ts, OSObject *sender, void *refcon)
  {
    if (callback == NULL) return;

    Params_KeyboardEventCallBack params = {
      target, 0, flags, 0,
      0, 0, 0, 0,
      keyboardType, false, ts, sender, refcon,
    };

    if (keyCombination.keyCode1 != KeyCode::NONE) {
      params.eventType = KeyEvent::DOWN;
      params.key = keyCombination.keyCode1;
      RemapUtil::fireKey(callback, params);

      params.eventType = KeyEvent::UP;
      RemapUtil::fireKey(callback, params);
    }
    if (keyCombination.keyCode2 != KeyCode::NONE) {
      params.eventType = KeyEvent::DOWN;
      params.key = keyCombination.keyCode2;
      RemapUtil::fireKey(callback, params);

      params.eventType = KeyEvent::UP;
      RemapUtil::fireKey(callback, params);
    }
  }

  // ----------------------------------------
  void
  KeyOverlaidModifier::remap(const RemapParams &remapParams, KeyCode::KeyCode fromKeyCode, ModifierFlag::ModifierFlag toFlag, FireFunc::FireFunc firefunc, ExtraRepeatFunc::ExtraRepeatFunc extraRepeatFunc)
  {
    if (! RemapUtil::isKey(remapParams, fromKeyCode)) {
      if (! RemapUtil::isEvent_Up(remapParams)) {
        useAsModifier = true;
      }
      return;
    }

    // ----------------------------------------
    bool isKeyDown = RemapUtil::isKeyDown(remapParams, fromKeyCode);

    KeyCode::KeyCode toKeyCode = RemapUtil::getModifierKeyCode(toFlag);
    RemapUtil::keyToKey(remapParams, fromKeyCode, toKeyCode);

    if (isKeyDown) {
      useAsModifier = false;
      ClickWatcher::set(&isClick);
      ic.begin();

      if (extraRepeatFunc) {
        FlagStatus::Item *status = FlagStatus::getFlagStatus(toFlag);
        if (status) {
          status->temporary_decrease();

          *(remapParams.ex_extraRepeatFunc) = extraRepeatFunc;
          *(remapParams.ex_extraRepeatFlags) = FlagStatus::makeFlags(remapParams);

          status->temporary_increase();
        }
      }

    } else {
      if (useAsModifier == false && isClick == false) {
        if (extraRepeatFunc == NULL || remapParams.ex_extraRepeatCounter == 0) {
          if (config.parameter_keyoverlaidmodifier_timeout <= 0 || ic.checkThreshold(config.parameter_keyoverlaidmodifier_timeout) == false) {
            firefunc(remapParams);
          }
        }
      }
      ClickWatcher::unset(&isClick);
    }
  }

  void
  KeyOverlaidModifierCombination::remap(const RemapParams &remapParams, ModifierFlag::ModifierFlag fromFlag1, ModifierFlag::ModifierFlag fromFlag2, FireFunc::FireFunc firefunc)
  {
    KeyCode::KeyCode keyCode1 = RemapUtil::getModifierKeyCode(fromFlag1);
    KeyCode::KeyCode keyCode2 = RemapUtil::getModifierKeyCode(fromFlag2);
    if (keyCode1 == KeyCode::NONE || keyCode2 == KeyCode::NONE) return;

    FlagStatus::Item *fromStatus1 = FlagStatus::getFlagStatus(fromFlag1);
    FlagStatus::Item *fromStatus2 = FlagStatus::getFlagStatus(fromFlag2);
    if (fromStatus1 == NULL || fromStatus2 == NULL) return;

    if ((remapParams.params)->key != static_cast<unsigned int>(keyCode2)) {
      isCallFireFunc = false;
      return;
    }

    if (fromStatus2->isHeldDown()) {
      isCallFireFunc = true;
      ClickWatcher::set(&isClick);

    } else {
      if (fromStatus1->isHeldDown() && isCallFireFunc && isClick == false) {
        fromStatus1->temporary_decrease();
        firefunc(remapParams);
      }
      ClickWatcher::unset(&isClick);
    }
  }

  void
  DoublePressModifier::remap(const RemapParams &remapParams, KeyCode::KeyCode fromKeyCode, ModifierFlag::ModifierFlag toFlag, FireFunc::FireFunc firefunc)
  {
    if (! RemapUtil::isKey(remapParams, fromKeyCode)) {
      pressCount = 0;
      return;
    }

    // ----------------------------------------
    bool isKeyDown = RemapUtil::isKeyDown(remapParams, fromKeyCode);

    KeyCode::KeyCode toKeyCode = RemapUtil::getModifierKeyCode(toFlag);
    RemapUtil::keyToKey(remapParams, fromKeyCode, toKeyCode);

    if (isKeyDown) {
      ++pressCount;
    } else {
      if (pressCount >= 2) {
        firefunc(remapParams);
      }
    }
  }

  // ----------------------------------------
  void
  ModifierHoldingKeyToKey::remap(const RemapParams &remapParams, ModifierFlag::ModifierFlag fromFlag, KeyCode::KeyCode fromKeyCode, KeyCode::KeyCode toKeyCode)
  {
    FlagStatus::Item *fromStatus = FlagStatus::getFlagStatus(fromFlag);
    if (fromStatus == NULL) return;

    KeyCode::KeyCode fromFlagKeyCode = fromStatus->getKeyCode();
    if ((remapParams.params)->key == static_cast<unsigned int>(fromFlagKeyCode)) {
      doremap = false;
      first = fromStatus->isHeldDown();
      ic.begin();
      return;
    }

    if (first) {
      first = false;
      if (ic.checkThreshold(config.parameter_modifierholdingkeytokey_wait)) {
        doremap = true;
      } else {
        doremap = false;
      }
    }

    if (! RemapUtil::isKey(remapParams, fromKeyCode)) return;
    if (! doremap) return;

    fromStatus->temporary_decrease();
    RemapUtil::keyToKey(remapParams, fromKeyCode, toKeyCode);
  }

  // ----------------------------------------
  void
  FirePointingScroll::fire(ScrollWheelEventCallback callback, OSObject *target, IOHIPointing *pointing, AbsoluteTime ts)
  {
    if (! enable) return;
    enable = false;

    if (callback == NULL) return;

    callback(target,
             deltaAxis1, deltaAxis2, deltaAxis3,
             fixedDelta1, fixedDelta2, fixedDelta3,
             pointDelta1, pointDelta2, pointDelta3,
             0, ts, pointing, 0);

    if (config.debug_pointing) {
#if __x86_64__
      const char *format = "sending scrollWheelEventCallback: deltaAxis(%d, %d, %d), fixedDelta(%ld, %ld, %ld), pointDelta(%d,%d,%d)\n";
#else
      const char *format = "sending scrollWheelEventCallback: deltaAxis(%d, %d, %d), fixedDelta(%d, %d, %d), pointDelta(%d,%d,%d)\n";
#endif
      printf(format,
             deltaAxis1, deltaAxis2, deltaAxis3,
             fixedDelta1, fixedDelta2, fixedDelta3,
             pointDelta1, pointDelta2, pointDelta3);
    }
  }

  // ------------------------------------------------------------
  void
  ButtonRelativeToScroll::remap(const RemapPointingParams_relative &remapParams, unsigned int button)
  {
    if (((remapParams.params)->buttons & button) == button) {
      // if the source button contains left button, we cancel left click for iPhoto, or some applications.
      // iPhoto store the scroll events when left button is pressed, and restore events after left button is released.
      // ButtonRelativeToScroll doesn't aim it, we release the left button and do normal scroll event.
      if (button & PointingButton::LEFT) {
        if (! isButtonHeldDown) {
          listFireRelativePointer.add(PointingButton::NONE);
        }
      }

      isButtonHeldDown = true;
      *(remapParams.ex_dropEvent) = true;
      RemapUtil::pointingRelativeToScroll(remapParams);
    } else {
      // ignore button up event.
      if (isButtonHeldDown) {
        isButtonHeldDown = false;
        *(remapParams.ex_dropEvent) = true;
      }
    }
  }
}
