<template>
  <div>
    <h3>Parameters</h3>

    <p>Intensity [mV]: {{ parameters["intensity"] }}</p>

    <p>Inter-trial interval [ms]: {{ parameters["iti"] }}</p>

    <p>Number of stimuli: {{ parameters["number_of_stimuli"] }}</p>

    <h3>State</h3>

    <p v-if="stimulating">Stimulating: &#x2611;</p>
    <p v-else>Stimulating: &#x2610;</p>

    <p v-if="recharging">Recharging: &#x2611;</p>
    <p v-else>Recharging: &#x2610;</p>

    <h3>Commands</h3>

    <p>
      <button @click="stimulate()">Stimulate</button>
      <button @click="recharge()">Recharge</button>
      <button @click="abort()">Abort</button>
    </p>

    <h3>Messages</h3>

    <p>{{ latestMessage }}</p>
  </div>
</template>

<script>
export default {
  data() {
    return {
      // State
      stimulating: false,
      recharging: false,

      // Parameters
      parameters: {},

      latestMessage: ""
    };
  },

  sockets: {
    // Fired when the server updates the state.
    update_state(data) {
      const stateVariable = data["state_variable"];
      const rawValue = String.fromCharCode.apply(
        null,
        new Uint8Array(data["value"])
      );

      const value = rawValue == "True";

      let updated = false;
      switch (stateVariable) {
        case "stimulating":
          updated = value != this.stimulating;
          this.stimulating = value;
          break;
        case "recharging":
          updated = value != this.recharging;
          this.recharging = value;
          break;
      }

      if (updated) {
        this.latestMessage = `State updated: ${stateVariable} = ${value}`;
      }
    },

    // Fired when the server updates the parameters.
    update_parameter(data) {
      const parameterName = data["name"];
      const rawValue = String.fromCharCode.apply(
        null,
        new Uint8Array(data["value"])
      );

      const value = parseInt(rawValue);

      this.parameters[parameterName] = value;
      this.latestMessage = `Parameter updated: ${parameterName} = ${value}`;
    }
  },

  methods: {
    stimulate() {
      this.$socket.emit("command", "stimulate");
      this.latestMessage = "Command sent: stimulate";
    },
    recharge() {
      this.$socket.emit("command", "recharge");
      this.latestMessage = "Command sent: recharge";
    },
    abort() {
      this.$socket.emit("command", "abort");
      this.latestMessage = "Command sent: abort";
    }
  }
};
</script>
