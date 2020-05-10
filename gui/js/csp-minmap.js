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
  }

  CosmoScout.init(MinimapApi);
})();
