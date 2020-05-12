////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"
#include "logger.hpp"

#include "../../../src/cs-core/GuiManager.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-utils/convert.hpp"

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

void from_json(nlohmann::json const& j, Plugin::Settings::Layer& o) {
  cs::core::Settings::deserialize(j, "url", o.mURL);
  cs::core::Settings::deserialize(j, "layer", o.mLayer);
  cs::core::Settings::deserialize(j, "attribution", o.mAttribution);
}

void to_json(nlohmann::json& j, Plugin::Settings::Layer const& o) {
  cs::core::Settings::serialize(j, "url", o.mURL);
  cs::core::Settings::serialize(j, "layer", o.mLayer);
  cs::core::Settings::serialize(j, "attribution", o.mAttribution);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings& o) {
  cs::core::Settings::deserialize(j, "targets", o.mTargets);
}

void to_json(nlohmann::json& j, Plugin::Settings const& o) {
  cs::core::Settings::serialize(j, "targets", o.mTargets);
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

  // Add newly created bookmarks.
  mOnBookmarkAddedConnection = mGuiManager->onBookmarkAdded().connect(
      [this](uint32_t bookmarkID, cs::core::Settings::Bookmark const& bookmark) {
        onAddBookmark(mSolarSystem->pActiveBody.get(), bookmarkID, bookmark);
      });

  // Remove deleted bookmarks.
  mOnBookmarkRemovedConnection = mGuiManager->onBookmarkRemoved().connect(
      [this](uint32_t bookmarkID, cs::core::Settings::Bookmark const& /*bookmark*/) {
        mGuiManager->getGui()->callJavascript("CosmoScout.minimap.removeBookmark", bookmarkID);
      });

  // Update bookmarks if active body changes.
  mActiveBodyConnection = mSolarSystem->pActiveBody.connectAndTouch(
      [this](std::shared_ptr<cs::scene::CelestialBody> const& body) {
        mGuiManager->getGui()->callJavascript("CosmoScout.minimap.removeBookmarks");

        // Add all bookmarks with positions for this body.
        if (body) {
          for (auto const& [id, bookmark] : mGuiManager->getBookmarks()) {
            onAddBookmark(body, id, bookmark);
          }
        }
      });

  // Load initial settings.
  onLoad();

  logger().info("Loading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  logger().info("Unloading plugin...");

  mAllSettings->onLoad().disconnect(mOnLoadConnection);
  mAllSettings->onSave().disconnect(mOnSaveConnection);

  mGuiManager->onBookmarkAdded().disconnect(mOnBookmarkAddedConnection);
  mGuiManager->onBookmarkRemoved().disconnect(mOnBookmarkRemovedConnection);

  mGuiManager->getGui()->callJavascript("CosmoScout.gui.unregisterHtml", "minimap-template");
  mGuiManager->getGui()->callJavascript("CosmoScout.gui.unregisterCss", "css/csp-minimap.css");
  mGuiManager->getGui()->executeJavascript("document.querySelector('#minimap').remove()");

  mGuiManager->removeTimelineButton("Toggle Minimap");
  mGuiManager->getGui()->unregisterCallback("minimap.toggle");

  mSolarSystem->pActiveBody.disconnect(mActiveBodyConnection);

  logger().info("Unloading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::onAddBookmark(std::shared_ptr<cs::scene::CelestialBody> const& activeBody,
    uint32_t bookmarkID, cs::core::Settings::Bookmark const& bookmark) {

  if (bookmark.mLocation && bookmark.mLocation.value().mPosition) {
    if (activeBody->getCenterName() == bookmark.mLocation.value().mCenter) {
      auto radii = activeBody->getRadii();
      auto p     = cs::utils::convert::toLngLatHeight(
          bookmark.mLocation.value().mPosition.value(), radii[0], radii[0])
                   .xy();
      p      = cs::utils::convert::toDegrees(p);
      auto c = bookmark.mColor.value_or(glm::vec3(0.8F, 0.8F, 1.0F)) * 255.F;
      mGuiManager->getGui()->callJavascript("CosmoScout.minimap.addBookmark", bookmarkID,
          fmt::format("rgb({}, {}, {})", c.r, c.g, c.b), p.x, p.y);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::onLoad() {

  // Read settings from JSON.
  from_json(mAllSettings->mPlugins.at("csp-minimap"), mPluginSettings);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::minimap
