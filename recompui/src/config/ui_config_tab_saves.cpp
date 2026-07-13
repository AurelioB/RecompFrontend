#include "recompui/config.h"

#include <optional>

namespace {
std::optional<recompui::config::saves::SaveSettingsProvider> provider;
bool tab_created = false;
std::string last_location;
std::string last_status;
}

namespace recompui::config {

void saves::set_settings_provider(SaveSettingsProvider new_provider) {
    provider = std::move(new_provider);
    refresh_status();
}

void saves::clear_settings_provider() {
    provider.reset();
}

void saves::refresh_status() {
    if (!tab_created || !provider) {
        return;
    }
    auto& config = get_config(saves::id);
    const std::string location = provider->location ? provider->location() : "App storage";
    const std::string status = provider->operation_status ? provider->operation_status() : std::string{};
    if (location != last_location) {
        config.update_option_value(saves::options::location, location);
        last_location = location;
    }
    if (status != last_status) {
        config.update_option_value(saves::options::operation_status, status);
        last_status = status;
    }
}

void create_save_management_tab(const std::string& name) {
    tab_created = true;
    auto& config = create_config_tab(name, saves::id, false);
    config.add_info_option(saves::options::location, "Save Location",
        "The active save destination. Android document folders remain outside app-owned storage.",
        provider && provider->location ? provider->location() : "App storage");
    config.add_info_option(saves::options::operation_status, "Last Operation",
        "Import, export, and folder synchronization status.",
        provider && provider->operation_status ? provider->operation_status() : "Ready");
    config.add_action_option(saves::options::import_save, "Import Save",
        "Select a save file. Files are validated before the active save is replaced.", "Import...", [] {
            if (provider && provider->import_save) provider->import_save();
        });
    config.add_action_option(saves::options::export_save, "Export Save",
        "Create a byte-exact snapshot of the active save.", "Export...", [] {
            if (provider && provider->export_save) provider->export_save();
        });
    config.add_action_option(saves::options::choose_folder, "Choose Save Folder",
        "Choose an Android document folder. Existing recognized saves are never overwritten automatically.", "Choose Folder...", [] {
            if (provider && provider->choose_folder) provider->choose_folder();
        });
    config.add_action_option(saves::options::reset_folder, "Reset to App Storage",
        "Stop synchronizing to the selected document folder. Existing files are kept.", "Reset", [] {
            if (provider && provider->reset_folder) provider->reset_folder();
        });
    saves::refresh_status();
}

} // namespace recompui::config
