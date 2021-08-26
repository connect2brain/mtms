<template>
  <div id="app">
    <div id="nav" v-if="$route.name === 'Home'">
      <!-- XXX: It is not a good practice to have a bogus value for href attribute,
                see:

                https://developer.mozilla.org/en-US/docs/Web/API/Window/open#best_practices

                However, maybe it works well enough for our use case.
      -->
      <a href="#" v-on:click.stop.prevent="openWindow('Calibration')">Calibration</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('Planner')">Planner</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('TMS')">TMS</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('EEG')">EEG</a> |
      <a href="#" v-on:click.stop.prevent="openWindow('About')">About</a>
    </div>
    <health-check />
    <router-view />
  </div>
</template>

<script>
import HealthCheck from "./components/HealthCheck.vue";

export default {
  components: {
    HealthCheck
  },

  created() {
    this.WINDOW_INFO = {
      EEG: {
        url: '/eeg',
        width: 980,
        height: 750
      },
      Calibration: {
        url: '/calibration',
        width: 300,
        height: 480
      },
      Planner: {
        url: '/planner',
        width: 420,
        height: 420
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

<style lang="scss">
@import "styles/_colors.scss";

#app {
  font-family: "Segoe UI";
  src: local("Segoe UI"), url(./fonts/segoeui.ttf) format("truetype");

  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: $default-font-color;
}

#nav {
  padding: 30px;
}

#nav a {
  font-weight: bold;
  color: $default-font-color;
}

#nav a.router-link-exact-active {
  color: $navigator-active-link-font-color;
}

/* As a default, input elements use their own font. Instead, make the elements inherit
   the font from the parent element. */
input {
  font-family: inherit;
}

/* Disable text selection with mouse.

   From: https://stackoverflow.com/questions/826782/how-to-disable-text-selection-highlighting */
.noselect {
  -webkit-touch-callout: none; /* iOS Safari */
  -webkit-user-select: none; /* Safari */
  -khtml-user-select: none; /* Konqueror HTML */
  -moz-user-select: none; /* Old versions of Firefox */
  -ms-user-select: none; /* Internet Explorer/Edge */
  user-select: none; /* Non-prefixed version, currently
                                  supported by Chrome, Edge, Opera and Firefox */
}

/* Scrollbar */
::-webkit-scrollbar {
  width: 12px;
}

::-webkit-scrollbar-track {
  background: $lighter-gray;
  border: 1px solid $dark-gray;
}

::-webkit-scrollbar-thumb {
  background: $dark-gray;
  border-radius: 10px;
}

::-webkit-scrollbar-thumb:hover {
  background: $darker-gray;
}
</style>
