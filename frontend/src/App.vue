<template>
  <div id="app">
    <div id="nav">
      <router-link to="/">Home</router-link> |
      <router-link to="/eeg">EEG</router-link> |
      <router-link to="/planner">Planner</router-link> |
      <router-link to="/tms">TMS</router-link> |
      <router-link to="/about">About</router-link>
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
  }
};
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
}

#nav {
  padding: 30px;
}

#nav a {
  font-weight: bold;
  color: #2c3e50;
}

#nav a.router-link-exact-active {
  color: #42b983;
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
  background: #f1f1f1;
  border: 1px solid #707070;
}

::-webkit-scrollbar-thumb {
  background: #707070;
  border-radius: 10px;
}

::-webkit-scrollbar-thumb:hover {
  background: #606060;
}
</style>
