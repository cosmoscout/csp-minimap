////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"
#include "logger.hpp"

#include "../../../src/cs-core/GuiManager.hpp"
#include "../../../src/cs-core/Settings.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::minimap::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::minimap {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings& o) {
}

void to_json(nlohmann::json& j, Plugin::Settings const& o) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {

  logger().info("Loading plugin...");

  // Call onLoad whenever the settings are reloaded.
  mOnLoadConnection = mAllSettings->onLoad().connect([this]() { onLoad(); });

  // Store the current settings on save.
  mOnSaveConnection = mAllSettings->onSave().connect(
      [this]() { mAllSettings->mPlugins["csp-minimap"] = mPluginSettings; });

  // Add resources to gui.
  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/third-party/js/leaflet.js");
  mGuiManager->addCssToGui("third-party/css/leaflet.css");

  mGuiManager->addCssToGui("css/csp-minimap.css");
  mGuiManager->addHtmlToGui("minimap-template", "../share/resources/gui/csp-minimap-template.html");
  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/js/csp-minimap.js");

  // Register a callback to toggle the minimap.
  std::string callback = "minimap.toggle";
  mGuiManager->getGui()->registerCallback(callback, "Toggles the Minimap.", std::function([this]() {
    mGuiManager->getGui()->executeJavascript(
        "document.querySelector('#minimap').classList.toggle('visible')");
  }));

  // Add a timeline button to toggle the minimap.
  mGuiManager->addTimelineButton("Toggle Minimap", "map", callback);

  // Load initial settings.
  onLoad();

  logger().info("Loading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  mAllSettings->onLoad().disconnect(mOnLoadConnection);
  mAllSettings->onSave().disconnect(mOnSaveConnection);

  mGuiManager->getGui()->callJavascript("CosmoScout.gui.unregisterHtml", "minimap-template");
  mGuiManager->getGui()->callJavascript("CosmoScout.gui.unregisterCss", "css/csp-minimap.css");
  mGuiManager->getGui()->executeJavascript("document.querySelector('#minimap').remove()");

  mGuiManager->removeTimelineButton("Toggle Minimap");
  mGuiManager->getGui()->unregisterCallback("minimap.toggle");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::onLoad() {

  // Read settings from JSON.
  from_json(mAllSettings->mPlugins.at("csp-minimap"), mPluginSettings);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::minimap
