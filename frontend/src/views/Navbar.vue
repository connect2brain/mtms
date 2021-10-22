<template>
  <div class="navbar">
    <div id="navbar">
      <!-- XXX: It is not a good practice to have a bogus value for href attribute,
                see:

                https://developer.mozilla.org/en-US/docs/Web/API/Window/open#best_practices

                However, maybe it works well enough for our use case.
      -->
      <a href="#" v-on:click.stop.prevent="openWindow('Calibration')">Calibration</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('EEG')">EEG</a>
    </div>
  </div>
</template>

<script>
export default {
  name: "Navbar",

  created() {
    this.WINDOW_INFO = {
      EEG: {
        url: '/eeg',
        width: 740,
        height: 580
      },
      Calibration: {
        url: '/calibration',
        width: 310,
        height: 400
      }
    }
  },

  methods: {
    openWindow: function (windowName) {
      const info = this.WINDOW_INFO[windowName];

      const url = info['url'];
      const width = info['width'];
      const height = info['height'];

      const windowFeatures = `width=${width}, height=${height}`;

      const windowReference = window.open(url, windowName, windowFeatures);
      windowReference.focus();

      return false;
    }
  }
};
</script>

<style scoped lang="scss">
@import "../styles/_colors.scss";

#navbar {
  margin-bottom: 20px;
}

#navbar a {
  color: $default-font-color;
}
</style>