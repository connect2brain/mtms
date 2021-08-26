<template>
  <div class="nav">
    <div id="nav">
      <!-- XXX: It is not a good practice to have a bogus value for href attribute,
                see:

                https://developer.mozilla.org/en-US/docs/Web/API/Window/open#best_practices

                However, maybe it works well enough for our use case.
      -->
      <a href="#" v-on:click.stop.prevent="openWindow('Status')">Status</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('Calibration')">Calibration</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('Planner')">Planner</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('TMS')">TMS</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('EEG')">EEG</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('About')">About</a>
    </div>
  </div>
</template>

<script>
export default {
  name: "Home",

  created() {
    this.WINDOW_INFO = {
      Status: {
        url: '/status',
        width: 140,
        height: 400
      },
      EEG: {
        url: '/eeg',
        width: 980,
        height: 750
      },
      Calibration: {
        url: '/calibration',
        width: 310,
        height: 440
      },
      Planner: {
        url: '/planner',
        width: 400,
        height: 320
      },
      TMS: {
        url: '/tms',
        width: 400,
        height: 400
      },
      About: {
        url: '/about',
        width: 400,
        height: 400
      }
    };
  },

  methods: {
    openWindow: function (windowName) {
      const info = this.WINDOW_INFO[windowName];

      const url = info['url'];
      const width = info['width'];
      const height = info['height'];

      const windowFeatures = `width=${width}, height=${height}, menubar=1, toolbar=1, resizable=0`;

      const windowReference = window.open(url, windowName, windowFeatures);
      windowReference.focus();

      return false;
    }
  }
};
</script>

<style scoped lang="scss">
@import "../styles/_colors.scss";

#nav {
  padding: 30px;
}

#nav a {
  font-weight: bold;
  color: $default-font-color;
}
</style>