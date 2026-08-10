#pragma once

#include <stdint.h>

enum class AppEvent {
    ParkingBrakeChanged,
    Count
};

using AppEventHandler = void (*)();

class AppEvents
{
public:
  bool on(AppEvent event, AppEventHandler handler);
  bool off(AppEvent event, AppEventHandler handler);
  void emit(AppEvent event);

private:
  static constexpr uint8_t MAX_SUBSCRIBERS = 2;

  AppEventHandler _handlers[
    static_cast<uint8_t>(AppEvent::Count)
  ][MAX_SUBSCRIBERS] = {};
};

extern AppEvents appEvents;