#ifndef KADR_TIMED_TRIGGER_H
#define KADR_TIMED_TRIGGER_H

#include "../graphics.h"

namespace timing {

inline bool trigger(bool* p_trigger, float duration_ms) {
    struct State {
        float remaining_ms = 0.0f;
        bool last_input = false;
    };

    static ImGuiStorage s_storage;
    ImGuiID id = (ImGuiID)(intptr_t)p_trigger;

    State* s = (State*)s_storage.GetVoidPtr(id);
    if (!s) {
        s = IM_NEW(State);
        s_storage.SetVoidPtr(id, s);
    }

    const float dt_ms = ImGui::GetIO().DeltaTime * 1000.0f;

    if (*p_trigger && !s->last_input) s->remaining_ms = duration_ms;

    s->last_input = *p_trigger;

    if (s->remaining_ms > 0.0f) {
        s->remaining_ms -= dt_ms;
        if (s->remaining_ms <= 0.0f) {
            s->remaining_ms = 0.0f;
            *p_trigger = false;
        }
        return true;
    }

    return false;
}

}  // namespace timing

#endif  //KADR_TIMED_TRIGGER_H
