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
      current_fiducial: null,
      waiting_for_fiducial: null
    };
  },

  created() {
    this.STAGE_TO_FIDUCIAL = {
      0: null,
      1: {
        name: "LE",
        type: "image"
      },
      2: {
        name: "RE",
        type: "image"
      },
      3: {
        name: "NA",
        type: "image"
      },
      4: {
        name: "LE",
        type: "tracker"
      },
      5: {
        name: "RE",
        type: "tracker"
      },
      6: {
        name: "NA",
        type: "tracker"
      }
    };
  },

  methods: {
    setStage(newStage) {
      this.stage = newStage;
      this.current_fiducial = this.STAGE_TO_FIDUCIAL[newStage];
    },

    previous() {
      this.setStage(this.stage - 1);
    },

    next() {
      if (this.current_fiducial !== null) {
        this.$socket.emit("calibration.set_fiducial", {
          fiducial: this.current_fiducial
        });
        this.waiting_for_fiducial = this.current_fiducial;
      }
      this.setStage(this.stage + 1);
    }
  },

  sockets: {
    "calibration.fiducial_set"(fiducial) {
      if (this.waiting_for_fiducial["name"] === fiducial["name"] &&
          this.waiting_for_fiducial["type"] === fiducial["type"]) {

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
