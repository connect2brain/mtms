<template>
  <div id="calibration" class="noselect">
    <div v-show="waiting_for_fiducial !== null">
      <img alt="Head shape" src="../assets/calibration.svg" />
      <p>Registering the value...</p>
    </div>
    <div v-show="waiting_for_fiducial === null">
      <div v-show="stage === 0">
        <img alt="Head shape" src="../assets/calibration.svg" />
        <p>Start calibration.</p>
      </div>

      <div v-show="stage === 1">
        <img
          alt="Head shape, left ear highlighted"
          src="../assets/left_ear.svg"
        />
        <p>Select the left ear in the brain image.</p>
      </div>

      <div v-show="stage === 2">
        <img
          alt="Head shape, right ear highlighted"
          src="../assets/right_ear.svg"
        />
        <p>Select the right ear in the brain image.</p>
      </div>

      <div v-show="stage === 3">
        <img alt="Head shape, nasion highlighted" src="../assets/nasion.svg" />
        <p>Select the nasion in the brain image.</p>
      </div>

      <div v-show="stage === 4">
        <img
          alt="Head shape, left ear highlighted"
          src="../assets/left_ear.svg"
        />
        <p>Point the tracker to the left ear.</p>
      </div>

      <div v-show="stage === 5">
        <img
          alt="Head shape, right ear highlighted"
          src="../assets/right_ear.svg"
        />
        <p>Point the tracker to the right ear.</p>
      </div>

      <div v-show="stage === 6">
        <img alt="Head shape, nasion highlighted" src="../assets/nasion.svg" />
        <p>Point the tracker to the nasion.</p>
      </div>

      <a v-show="stage > 0" v-on:click="previous()" class="previous round"
        >&#8249;</a
      >
      <a v-on:click="next()" class="next round">&#8250;</a>
    </div>
  </div>
</template>

<script>
export default {
  data() {
    return {
      stage: 0,
      fiducial_name: null,
      waiting_for_fiducial: null
    };
  },

  created() {
    this.STAGE_TO_FIDUCIAL_NAME = {
      1: "LEI",
      2: "REI",
      3: "NAI"
    };
  },

  methods: {
    setStage(newStage) {
      this.stage = newStage;
      this.fiducial_name = this.STAGE_TO_FIDUCIAL_NAME[newStage] || null;
    },

    previous() {
      this.setStage(this.stage - 1);
    },

    next() {
      if (this.fiducial_name !== null) {
        this.$socket.emit("calibration.set_fiducial", {
          fiducial_name: this.fiducial_name
        });
        this.waiting_for_fiducial = this.fiducial_name;
      }
      this.setStage(this.stage + 1);
    }
  },

  sockets: {
    "calibration.fiducial_set"(fiducial_name) {
      if (this.waiting_for_fiducial === fiducial_name) {
        this.waiting_for_fiducial = null;
      }
    }
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
  background-color: #04aa6d;
  color: white;
}

.round {
  border-radius: 50%;
}
</style>
