#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <climits>

#include "imnodes.hpp"

// the structure of this file:
//
// [SECTION] internal enums
// [SECTION] internal data structures
// [SECTION] global and editor context structs
// [SECTION] object pool implementation

struct ImNodesContext;

extern ImNodesContext* GImNodes;

// [SECTION] internal enums

using ImNodesScope = int;
using ImNodesAttributeType = int;
using ImNodesClickInteractionType = int;
using ImNodesLinkCreationType = int;

enum ImNodesScope_ {
  ImNodesScope_None = 1,
  ImNodesScope_Editor = 1 << 1,
  ImNodesScope_Node = 1 << 2,
  ImNodesScope_Attribute = 1 << 3
};

enum ImNodesAttributeType_ {
  ImNodesAttributeType_None,
  ImNodesAttributeType_Input,
  ImNodesAttributeType_Output
};

enum ImNodesUIState_ {
  ImNodesUIState_None = 0,
  ImNodesUIState_LinkStarted = 1 << 0,
  ImNodesUIState_LinkDropped = 1 << 1,
  ImNodesUIState_LinkCreated = 1 << 2
};

enum ImNodesClickInteractionType_ {
  ImNodesClickInteractionType_Node,
  ImNodesClickInteractionType_Link,
  ImNodesClickInteractionType_LinkCreation,
  ImNodesClickInteractionType_Panning,
  ImNodesClickInteractionType_BoxSelection,
  ImNodesClickInteractionType_ImGuiItem,
  ImNodesClickInteractionType_None
};

enum ImNodesLinkCreationType_ {
  ImNodesLinkCreationType_Standard,
  ImNodesLinkCreationType_FromDetach
};

// [SECTION] internal data structures

// The object T must have the following interface:
//
// struct T
// {
//     T();
//
//     int id;
// };
template <typename T>
struct ImObjectPool {
  ImVector<T> Pool;
  ImVector<bool> InUse;
  ImVector<int> FreeList;
  ImGuiStorage IdMap;

  ImObjectPool() : Pool(), IdMap() {}
};

// Emulates std::optional<int> using the sentinel value `INVALID_INDEX`.
struct ImOptionalIndex {
  ImOptionalIndex() : m_index(INVALID_INDEX) {}
  ImOptionalIndex(const int value) : m_index(value) {}

  // Observers

  [[nodiscard]] inline bool HasValue() const {
    return m_index != INVALID_INDEX;
  }

  [[nodiscard]] inline int Value() const {
    IM_ASSERT(HasValue());
    return m_index;
  }

  // Modifiers

  inline ImOptionalIndex& operator=(const int value) {
    m_index = value;
    return *this;
  }

  inline void Reset() {
    m_index = INVALID_INDEX;
  }

  inline bool operator==(const ImOptionalIndex& rhs) const {
    return m_index == rhs.m_index;
  }

  inline bool operator==(const int rhs) const {
    return m_index == rhs;
  }

  inline bool operator!=(const ImOptionalIndex& rhs) const {
    return m_index != rhs.m_index;
  }

  inline bool operator!=(const int rhs) const {
    return m_index != rhs;
  }

  static constexpr int INVALID_INDEX = -1;

 private:
  int m_index;
};

struct ImNodeData {
  int Id;
  ImVec2 Origin;  // The node origin is in editor space
  ImRect TitleBarContentRect;
  ImRect Rect;

  struct {
    ImU32 Background, BackgroundHovered, BackgroundSelected, Outline, Titlebar, TitlebarHovered,
        TitlebarSelected;
  } ColorStyle;

  struct {
    float CornerRounding{};
    ImVec2 Padding;
    float BorderThickness{};
  } LayoutStyle;

  ImVector<int> PinIndices;
  bool Draggable{true};

  explicit ImNodeData(const int node_id)
      : Id(node_id),
        Origin(0.0F, 0.0F),
        Rect(ImVec2(0.0F, 0.0F), ImVec2(0.0F, 0.0F)),
        ColorStyle() {}

  ~ImNodeData() {
    Id = INT_MIN;
  }
};

struct ImPinData {
  int Id;
  int ParentNodeIdx{};
  ImRect AttributeRect;
  ImNodesAttributeType Type{ImNodesAttributeType_None};
  ImNodesPinShape Shape{ImNodesPinShape_CircleFilled};
  ImVec2 Pos;  // screen-space coordinates
  int Flags{ImNodesAttributeFlags_None};

  struct {
    ImU32 Background, Hovered;
  } ColorStyle;

  explicit ImPinData(const int pin_id) : Id(pin_id), ColorStyle() {}
};

struct ImLinkData {
  int Id;
  int StartPinIdx{};
  int EndPinIdx{};

  struct {
    ImU32 Base, Hovered, Selected;
  } ColorStyle;

  explicit ImLinkData(const int link_id) : Id(link_id), ColorStyle() {}
};

struct ImClickInteractionState {
  ImNodesClickInteractionType Type{ImNodesClickInteractionType_None};

  struct {
    int StartPinIdx{};
    ImOptionalIndex EndPinIdx;
    ImNodesLinkCreationType Type{};
  } LinkCreation;

  struct {
    ImRect Rect;  // Coordinates in grid space
  } BoxSelector;

  ImClickInteractionState() = default;
};

struct ImNodesColElement {
  ImU32 Color;
  ImNodesCol Item;

  ImNodesColElement(const ImU32 color, const ImNodesCol item) : Color(color), Item(item) {}
};

struct ImNodesStyleVarElement {
  ImNodesStyleVar Item;
  std::array<float, 2> FloatValue{};

  ImNodesStyleVarElement(const ImNodesStyleVar variable, const float value) : Item(variable) {
    FloatValue[0] = value;
  }

  ImNodesStyleVarElement(const ImNodesStyleVar variable, const ImVec2 value) : Item(variable) {
    FloatValue[0] = value.x;
    FloatValue[1] = value.y;
  }
};

// [SECTION] global and editor context structs

struct ImNodesEditorContext {
  ImObjectPool<ImNodeData> Nodes;
  ImObjectPool<ImPinData> Pins;
  ImObjectPool<ImLinkData> Links;

  ImVector<int> NodeDepthOrder;

  // ui related fields
  ImVec2 Panning;
  ImVec2 AutoPanningDelta;
  // Minimum and maximum extents of all content in grid space. Valid after final
  // ImNodes::EndNode() call.
  ImRect GridContentBounds;

  ImVector<int> SelectedNodeIndices;
  ImVector<int> SelectedLinkIndices;

  // Relative origins of selected nodes for snapping of dragged nodes
  ImVector<ImVec2> SelectedNodeOffsets;
  // Offset of the primary node origin relative to the mouse cursor.
  ImVec2 PrimaryNodeOffset;

  ImClickInteractionState ClickInteraction;

  // Mini-map state set by MiniMap()

  bool MiniMapEnabled{false};
  ImNodesMiniMapLocation MiniMapLocation{};
  float MiniMapSizeFraction{0.0F};
  ImNodesMiniMapNodeHoveringCallback MiniMapNodeHoveringCallback{nullptr};
  ImNodesMiniMapNodeHoveringCallbackUserData MiniMapNodeHoveringCallbackUserData{nullptr};

  // Mini-map state set during EndNodeEditor() call

  ImRect MiniMapRectScreenSpace;
  ImRect MiniMapContentScreenSpace;
  float MiniMapScaling{0.0F};

  ImNodesEditorContext() : Panning(0.F, 0.F), PrimaryNodeOffset(0.F, 0.F) {}
};

struct ImNodesContext {
  ImNodesEditorContext* DefaultEditorCtx;
  ImNodesEditorContext* EditorCtx;

  // Canvas draw list and helper state
  ImDrawList* CanvasDrawList;
  ImGuiStorage NodeIdxToSubmissionIdx;
  ImVector<int> NodeIdxSubmissionOrder;
  ImVector<int> NodeIndicesOverlappingWithMouse;
  ImVector<int> OccludedPinIndices;

  // Canvas extents
  ImVec2 CanvasOriginScreenSpace;
  ImRect CanvasRectScreenSpace;

  // Debug helpers
  ImNodesScope CurrentScope;

  // Configuration state
  ImNodesIO Io;
  ImNodesStyle Style;
  ImVector<ImNodesColElement> ColorModifierStack;
  ImVector<ImNodesStyleVarElement> StyleModifierStack;
  ImGuiTextBuffer TextBuffer;

  int CurrentAttributeFlags;
  ImVector<int> AttributeFlagStack;

  // UI element state
  int CurrentNodeIdx;
  int CurrentPinIdx;
  int CurrentAttributeId;

  ImOptionalIndex HoveredNodeIdx;
  ImOptionalIndex HoveredLinkIdx;
  ImOptionalIndex HoveredPinIdx;

  ImOptionalIndex DeletedLinkIdx;
  ImOptionalIndex SnapLinkIdx;

  // Event helper state
  // TODO: this should be a part of a state machine, and not a member of the global struct.
  // Unclear what parts of the code this relates to.
  int ImNodesUIState;

  int ActiveAttributeId;
  bool ActiveAttribute;

  // ImGui::IO cache

  ImVec2 MousePos;

  bool LeftMouseClicked;
  bool LeftMouseReleased;
  bool AltMouseClicked;
  bool LeftMouseDragging;
  bool AltMouseDragging;
  float AltMouseScrollDelta;
  bool MultipleSelectModifier;
};

namespace ImNodes {
static inline ImNodesEditorContext& EditorContextGet() {
  // No editor context was set! Did you forget to call ImNodes::CreateContext()?
  IM_ASSERT(GImNodes->EditorCtx != nullptr);
  return *GImNodes->EditorCtx;
}

// [SECTION] ObjectPool implementation

template <typename T>
static inline int ObjectPoolFind(const ImObjectPool<T>& objects, const int id) {
  const int index = objects.IdMap.GetInt(static_cast<ImGuiID>(id), -1);
  return index;
}

template <typename T>
static inline void ObjectPoolUpdate(ImObjectPool<T>& objects) {
  for (int i = 0; i < objects.InUse.size(); ++i) {
    const int id = objects.Pool[i].Id;

    if (!objects.InUse[i] && objects.IdMap.GetInt(id, -1) == i) {
      objects.IdMap.SetInt(id, -1);
      objects.FreeList.push_back(i);
      (objects.Pool.Data + i)->~T();
    }
  }
}

template <typename T>
static inline void ObjectPoolReset(ImObjectPool<T>& objects) {
  if (!objects.InUse.empty()) {
    memset(objects.InUse.Data, 0, objects.InUse.size_in_bytes());
  }
}

template <typename T>
static inline int ObjectPoolFindOrCreateIndex(ImObjectPool<T>& objects, const int id) {
  int index = objects.IdMap.GetInt(static_cast<ImGuiID>(id), -1);

  // Construct new object
  if (index == -1) {
    if (objects.FreeList.empty()) {
      index = objects.Pool.size();
      IM_ASSERT(objects.Pool.size() == objects.InUse.size());
      const int new_size = objects.Pool.size() + 1;
      objects.Pool.resize(new_size);
      objects.InUse.resize(new_size);
    } else {
      index = objects.FreeList.back();
      objects.FreeList.pop_back();
    }
    IM_PLACEMENT_NEW(objects.Pool.Data + index) T(id);
    objects.IdMap.SetInt(static_cast<ImGuiID>(id), index);
  }

  // Flag it as used
  objects.InUse[index] = true;

  return index;
}

template <typename T>
static inline T& ObjectPoolFindOrCreateObject(ImObjectPool<T>& objects, const int id) {
  const int index = ObjectPoolFindOrCreateIndex(objects, id);
  return objects.Pool[index];
}
}  // namespace ImNodes
