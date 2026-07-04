#pragma once
// NUI: ResourceManager - unified access to resources (embedded or filesystem).
// Resources are first looked up in the embedded registry, then on disk.

#include <string>
#include <cstdint>
#include <cstddef>

namespace nui {

// Data pointer + size for a loaded resource
struct ResourceData {
    const uint8_t* data = nullptr;
    size_t         size = 0;
    bool           valid() const { return data != nullptr && size > 0; }
};

class ResourceManager {
public:
    // Initialize the resource manager.
    // If embed_generated is true, auto-registers embedded resources.
    static void Initialize();

    // Load a resource by path. Tries embedded first, then filesystem.
    // Returns empty ResourceData if not found.
    static ResourceData Load(const std::string& path);

    // Check if a resource exists (embedded or on disk)
    static bool Exists(const std::string& path);

    // Get the full filesystem path (only works for non-embedded resources)
    static std::string GetFilePath(const std::string& path);

private:
    static ResourceData LoadEmbedded(const std::string& path);
    static ResourceData LoadFile(const std::string& path);
};

} // namespace nui
