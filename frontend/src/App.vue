<template>
  <div id="app">
    <router-view />
  </div>
</template>

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
