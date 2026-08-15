#pragma once
#include <tuple>

namespace NodeSystem::Visual {
    class VisualBlock;

    enum class SelectionType { None, Node, Sensor, Device, InputSlot, OutputSlot, Flight };

    struct SelectionInfo {
        SelectionType type = SelectionType::None;
        VisualBlock* block = nullptr;
        int slotIndex = -1;
    };

    struct VisualLink {
        int fromDevice;
        int fromLevel;
        int fromIndex;
        int toDevice;
        int toLevel;
        int toIndex;
        int toSlot;
    };

    using PinKey = std::tuple<int, int, int, int>;
}