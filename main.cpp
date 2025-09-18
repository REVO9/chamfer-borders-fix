#include "globals.hpp"

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() { return HYPRLAND_API_VERSION; }

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    if (HASH != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(
            PHANDLE, "[MyPlugin] Mismatched headers! Can't proceed.",
            CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[MyPlugin] Version mismatch");
    }

    HyprlandAPI::addNotification(
        PHANDLE,
        "[chamfer-borders-fix] this plugin has become obsolete, it's behavior "
        "is now native to hyprland",
        CHyprColor{1.0, 0.8, 0.2, 1.0}, 5000);

    return {"chamfer-borders-fix", "fixes chamfered borders", "revo", "0.1"};
}
