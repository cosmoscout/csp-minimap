////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_MINIMAP_PLUGIN_HPP
#define CSP_MINIMAP_PLUGIN_HPP

#include "../../../src/cs-core/PluginBase.hpp"

namespace csp::minimap {

class Plugin : public cs::core::PluginBase {
 public:
  struct Settings {};

  void init() override;
  void deInit() override;

 private:
  void onLoad();

  Settings mPluginSettings;

  int mOnLoadConnection = -1;
  int mOnSaveConnection = -1;
};

} // namespace csp::minimap

#endif // CSP_MINIMAP_PLUGIN_HPP
