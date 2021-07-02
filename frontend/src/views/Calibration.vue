<template>
  <div id="calibration" class="noselect">
    <div v-show="stage == 0">
      <img alt="Head shape for calibration" src="../assets/calibration.svg" />
      <p>Start calibration.</p>
    </div>
    <div v-show="stage == 1">
      <img alt="Head shape for calibration, left ear highlighted" src="../assets/left_ear.svg" />
      <p>Select the left ear in the brain image.</p>
    </div>
    <div v-show="stage == 2">
      <img alt="Head shape for calibration, right ear highlighted" src="../assets/right_ear.svg" />
      <p>Select the right ear in the brain image.</p>
    </div>
    <div v-show="stage == 3">
      <img alt="Head shape for calibration, nasion highlighted" src="../assets/nasion.svg" />
      <p>Select the nasion in the brain image.</p>
    </div>
    <div v-show="stage == 4">
      <img alt="Head shape for calibration, left ear highlighted" src="../assets/left_ear.svg" />
      <p>Point the tracker to the left ear.</p>
    </div>
    <div v-show="stage == 5">
      <img alt="Head shape for calibration, right ear highlighted" src="../assets/right_ear.svg" />
      <p>Point the tracker to the right ear.</p>
    </div>
    <div v-show="stage == 6">
      <img alt="Head shape for calibration, nasion highlighted" src="../assets/nasion.svg" />
      <p>Point the tracker to the nasion.</p>
    </div>

    <a v-show="stage > 0" v-on:click="previous()" class="previous round">&#8249;</a>
    <a v-on:click="next()" class="next round">&#8250;</a>
  </div>
</template>

<script>
export default {
  data() {
    return {
      stage: 0
    };
  },

  methods: {
    previous() {
      this.stage -= 1;
    },
    next() {
      this.stage += 1;
    }
  },

  sockets: {
  }
};
</script>

<style scoped lang="scss">
@import "../styles/_colors.scss";

a {
  text-decoration: none;
  display: inline-block;
  padding: 8px 16px;
}

a:hover {
  background-color: #ddd;
  color: black;
}

.previous {
  background-color: #f1f1f1;
  color: black;
}

.next {
  background-color: #04AA6D;
  color: white;
}

.round {
  border-radius: 50%;
}
</style>
