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

    _lastLng = 0;
    _lastLat = 0;

    init() {
      // Add the minimap window.
      this._minimapDiv = CosmoScout.gui.loadTemplateContent('minimap');
      document.getElementById('cosmoscout').appendChild(this._minimapDiv);

      // Create the Leaflet map.
      this._minimap = L.map(document.querySelector('#minimap .window-content'), {
        center: [0, 0],
        zoom: 0,
        worldCopyJump: true,
        maxBounds: [[-90, -180], [90, 180]],
        attributionControl: false
      });

      L.control.scale().addTo(this._minimap);

      // Create a marker for the user's position.
      this._observerMarker =
          L.circleMarker([0, 0], {radius: 5, weight: 2, color: "red", fill: false})
              .addTo(this._minimap);

      this._wmslayer = L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
                          attribution: '&copy; OpenStreetMap contributors'
                        }).addTo(this._minimap);

      // // Moving the planet with the minimap.
      // this._minimap.on('click', (e) => {
      //   // const location = {
      //   //   longitude: parseFloat(e.latlng.lng),
      //   //   latitude: parseFloat(e.latlng.lat),
      //   //   height: this.circumference / Math.pow(2, this._minimap.getZoom())
      //   // };

      //   // if (this.activePlanet !== "") {
      //   //   this.flyTo(this.activePlanet, location, 5);
      //   // }
      //   console.log("click");
      // });

      // Resize the leaflet map on minimap window resize events.
      this._resizeObserver = new ResizeObserver((entries) => {
        // We wrap it in requestAnimationFrame to avoid "ResizeObserver loop limit exceeded".
        // See https://stackoverflow.com/questions/49384120/resizeobserver-loop-limit-exceeded
        window.requestAnimationFrame(() => {
          if (!Array.isArray(entries) || !entries.length) {
            return;
          }
          this._minimap.invalidateSize();
        });
      });

      this._resizeObserver.observe(this._minimapDiv);
    }

    update() {
      // Do not do anything if invisible.
      if (this._minimapDiv.classList.contains("visible")) {

        // Update position of observer marker.
        if (CosmoScout.state.observerLngLatHeight) {

          let [lng, lat] = CosmoScout.state.observerLngLatHeight;
          this._observerMarker.setLatLng([lat, lng])

          // Center the minimap around the observer.
          if ((this._lastLng !== lng || this._lastLat !== lat)) {
            this._minimap.setView([lat, lng], [this._minimap.getZoom()]);
            this._lastLng = lng;
            this._lastLat = lat;
          }
        }
      }
    }
  }

  CosmoScout.init(MinimapApi);
})();
