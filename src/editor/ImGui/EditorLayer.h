#pragma once

namespace synapse {

class EditorLayer
{
public:
    EditorLayer() = default;
    ~EditorLayer() = default;

    EditorLayer(const EditorLayer&) = delete;
    EditorLayer& operator=(const EditorLayer&) = delete;
};

} // namespace synapse
