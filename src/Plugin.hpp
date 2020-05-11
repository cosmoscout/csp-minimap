////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_MINIMAP_PLUGIN_HPP
#define CSP_MINIMAP_PLUGIN_HPP

#include "../../../src/cs-core/PluginBase.hpp"
#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-scene/CelestialBody.hpp"

#include <map>
#include <vector>

namespace csp::minimap {

class Plugin : public cs::core::PluginBase {
 public:
  struct Settings {
    struct Layer {
      std::string                mURL;
      std::optional<std::string> mLayer;
      std::optional<std::string> mAttribution;
    };

    std::map<std::string, std::vector<Layer>> mTargets;
  };

  void init() override;
  void deInit() override;

 private:
  void onAddBookmark(std::shared_ptr<cs::scene::CelestialBody> const& activeBody,
      uint32_t bookmarkID, cs::core::Settings::Bookmark const& bookmark);
  void onLoad();

  Settings mPluginSettings;

  int mActiveBodyConnection        = -1;
  int mOnBookmarkAddedConnection   = -1;
  int mOnBookmarkRemovedConnection = -1;
  int mOnLoadConnection            = -1;
  int mOnSaveConnection            = -1;
};

} // namespace csp::minimap

#endif // CSP_MINIMAP_PLUGIN_HPP
