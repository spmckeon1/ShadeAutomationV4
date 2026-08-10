
#include "appEvents.h"

AppEvents appEvents;

/*----  SEND THE EVENT SUBSCROBED TO  ----*/

void AppEvents::emit(AppEvent event) {
  uint8_t index = static_cast<uint8_t>(event);
  if (event == AppEvent::Count)
    return;
  for (uint8_t i = 0; i < MAX_SUBSCRIBERS; ++i) {
    if (_handlers[index][i] != nullptr) {
        _handlers[index][i]();
    }
  }
}

/*----  SET UP THE SUBSCRIPTION  ----*/

bool AppEvents::on(AppEvent event, AppEventHandler handler)
{
    uint8_t index = static_cast<uint8_t>(event);

    if (event == AppEvent::Count || handler == nullptr)
        return false;

    for (uint8_t i = 0; i < MAX_SUBSCRIBERS; ++i)
    {
        if (_handlers[index][i] == nullptr)
        {
            _handlers[index][i] = handler;
            return true;
        }
    }

    return false;
}

/*----  CANCEL THE SUBSCRIPTION  ----*/

bool AppEvents::off(AppEvent event, AppEventHandler handler)
{
    uint8_t index = static_cast<uint8_t>(event);

    if (event == AppEvent::Count || handler == nullptr)
        return false;

    for (uint8_t i = 0; i < MAX_SUBSCRIBERS; ++i)
    {
        if (_handlers[index][i] == handler)
        {
            _handlers[index][i] = nullptr;
            return true;
        }
    }

    return false;
}