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

    _bookmarks = {};

    _lastLng = 0;
    _lastLat = 0;
    _locked  = true;

    _customControls = L.Control.extend({
      options: {position: 'topleft'},
      onAdd:
          () => {
            let createButton = (icon, tooltip, callback) => {
              var button       = L.DomUtil.create('a');
              button.innerHTML = `<i class="material-icons">${icon}</i>`;
              button.setAttribute('title', tooltip);
              button.dataset.toggle    = 'tooltip';
              button.dataset.placement = 'left';
              L.DomEvent.on(button, 'click', callback);

              // Don't let mouse events get to the map.
              L.DomEvent.on(button, 'dblclick', L.DomEvent.stop);
              L.DomEvent.on(button, 'mousedown', L.DomEvent.stop);
              L.DomEvent.on(button, 'mouseup', L.DomEvent.stop);

              return button;
            };

            let zoomInButton = createButton('add', 'Zoom in', (e) => {
              this._minimap.zoomIn();
              L.DomEvent.stop(e);
            });

            let zoomOutButton = createButton('remove', 'Zoom out', (e) => {
              this._minimap.zoomOut();
              L.DomEvent.stop(e);
            });

            let zoomResetButton = createButton('zoom_out_map', 'Fit map', (e) => {
              this._minimap.setView([0, 0], 0);
              L.DomEvent.stop(e);
            });

            let lockButton = createButton('lock', 'Lock Minimap to Observer', (e) => {
              this._locked = !this._locked;
              if (this._locked) {
                e.target.querySelector('i').textContent = 'lock';
              } else {
                e.target.querySelector('i').textContent = 'lock_open';
              }

              L.DomEvent.stop(e);
            });

            let centerButton = createButton('my_location', 'Center Minimap on Observer', (e) => {
              this._centerMapOnObserver(true);
              L.DomEvent.stop(e);
            });

            let bookmarkButton = createButton('bookmark', 'Add a Bookmark', (e) => {
              CosmoScout.bookmarkEditor.addNewBookmark();
              L.DomEvent.stop(e);
            });

            let container = L.DomUtil.create('div');

            let groupA = L.DomUtil.create('div');
            groupA.classList.add('leaflet-bar')
            groupA.appendChild(zoomInButton);
            groupA.appendChild(zoomOutButton);
            groupA.appendChild(zoomResetButton);
            container.appendChild(groupA);

            let groupB = L.DomUtil.create('div');
            groupB.classList.add('leaflet-bar')
            groupB.style.marginTop = "10px";
            groupB.appendChild(centerButton);
            groupB.appendChild(lockButton);
            container.appendChild(groupB);

            let groupC = L.DomUtil.create('div');
            groupC.classList.add('leaflet-bar')
            groupC.style.marginTop = "10px";
            groupC.appendChild(bookmarkButton);
            container.appendChild(groupC);

            return container;
          }
    });

    init() {
      // Add the minimap window.
      this._minimapDiv = CosmoScout.gui.loadTemplateContent('minimap');
      document.getElementById('cosmoscout').appendChild(this._minimapDiv);

      // Create the Leaflet map.
      this._minimap = L.map(document.querySelector('#minimap .window-content'), {
        attributionControl: false,
        zoomControl: false,
        center: [0, 0],
        zoom: 0,
        worldCopyJump: true,
        maxBounds: [[-90, -180], [90, 180]]
      });

      // Add our custom buttons.
      this._minimap.addControl(new this._customControls());

      // Add the attribution control to the bottom left.
      L.control.attribution({position: 'bottomleft'}).addTo(this._minimap);

      // Create a marker for the user's position.
      let crossHair = L.icon({
        iconUrl: 'img/observer.png',
        shadowUrl: 'img/observer_shadow.png',
        iconSize: [28, 28],
        shadowSize: [28, 28],
        iconAnchor: [14, 14],
        shadowAnchor: [14, 14],
        popupAnchor: [14, 30]
      });

      this._observerMarker =
          L.marker(
               [0, 0], {icon: crossHair, interactive: false, keyboard: false, zIndexOffset: 1000})
              .addTo(this._minimap);

      this._wmslayer = L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
                          attribution: '&copy; OpenStreetMap contributors'
                        }).addTo(this._minimap);

      // Move the observer when clicked on the minimap.
      this._minimap.on('click', (e) => {
        let lng    = parseFloat(e.latlng.lng);
        let lat    = parseFloat(e.latlng.lat);
        let center = CosmoScout.state.activePlanetCenter;
        let height = this._zoomToHeight(this._minimap.getZoom());

        if (center !== "") {
          CosmoScout.callbacks.navigation.setBodyLongLatHeightDuration(center, lng, lat, height, 2);
        }
      });

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

    // Update minimap based on observer state.
    update() {
      // Do not do anything if invisible.
      if (this._minimapDiv.classList.contains("visible")) {
        this._centerMapOnObserver(false);
      }
    }

    addBookmark(bookmarkID, bookmarkName, bookmarkColor, lng, lat) {
      this._bookmarks[bookmarkID] = L.marker([lat, lng], {
                                       icon: L.divIcon({
                                         className: 'minimap-bookmark-icon',
                                         html: `<div style="border-color: ${bookmarkColor}"></div>`
                                       })
                                     }).addTo(this._minimap);
    }

    removeBookmark(bookmarkID) {
      if (bookmarkID in this._bookmarks) {
        this._bookmarks[bookmarkID].removeFrom(this._minimap);
        delete this._bookmarks[bookmarkID];
      }
    }

    removeBookmarks() {
      for (let i in this._bookmarks) {
        this._bookmarks[i].removeFrom(this._minimap);
      }

      this._bookmarks = {};
    }

    // These are quite a crude conversions from the minimap zoom level to observer height. It
    // works quite well for middle latitudes and the standard field of view.
    _zoomToHeight(zoom) {
      return 0.01 * this._minimapDiv.clientWidth * CosmoScout.state.activePlanetRadius[0] /
             Math.pow(2, zoom);
    }

    _heightToZoom(height) {
      return Math.log2(0.01 * this._minimapDiv.clientWidth *
                       CosmoScout.state.activePlanetRadius[0] / Math.max(100, height));
    }

    // Update minimap based on observer state.
    _centerMapOnObserver(force) {
      if (CosmoScout.state.observerLngLatHeight) {

        // Update position of observer marker.
        let [lng, lat] = CosmoScout.state.observerLngLatHeight;

        // Center the minimap around the observer.
        if (force || (this._lastLng !== lng || this._lastLat !== lat)) {
          this._observerMarker.setLatLng([lat, lng]);

          if (force || this._locked) {
            this._minimap.setView([lat, lng], [this._minimap.getZoom()]);
          }

          this._lastLng = lng;
          this._lastLat = lat;
        }
      }
    }
  }

  CosmoScout.init(MinimapApi);
})();
