#ifndef KADR_FILE_PICKER_H
#define KADR_FILE_PICKER_H

#include "../graphics.h"
#include "file_dialogs.h"

namespace FilePicker {

struct Result {
    bool edited = false;
    bool browse = false;
};

static Result draw(const char* label,
    const char* id,
    std::string* value,
    float min_width = 100.0f,
    const char* browse_label = "browse...") {
    Result r;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine();

    float browse_w = ImGui::CalcTextSize(browse_label).x + ImGui::GetStyle().FramePadding.x * 2.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float input_w = std::max(ImGui::GetContentRegionAvail().x - browse_w - spacing, min_width);

    ImGui::SetNextItemWidth(input_w);
    r.edited = ImGui::InputText(id, value, ImGuiInputTextFlags_ElideLeft);

    ImGui::SameLine();
    r.browse = ImGui::Button(browse_label);

    return r;
}

}  // namespace FilePicker

#endif  //KADR_FILE_PICKER_H
