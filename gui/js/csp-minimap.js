/* global IApi, CosmoScout */

(() => {
  /**
   * FlyTo Api
   */
  class MinimapApi extends IApi {
    /**
     * @inheritDoc
     * @type {string}
     */
    name = 'minimap';

    init() {
      let minimap = CosmoScout.gui.loadTemplateContent('minimap');
      document.getElementById('cosmoscout').appendChild(minimap);
    }
  }

  CosmoScout.init(MinimapApi);
})();
