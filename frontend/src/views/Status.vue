<template>
  <div class="status">
    <b>Connections</b>
    <p>
      <font-awesome-icon icon="circle" class="connected" v-if="connections.backend" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!connections.backend" />
      Core
      <br>
      <font-awesome-icon icon="circle" class="connected" v-if="connections.neuronavigation" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!connections.neuronavigation" />
      Neuronavigation
    </p>
    <b>Devices</b>
    <p>
      <font-awesome-icon icon="circle" class="connected" v-if="connections.pedal" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!connections.pedal" />
      Trigger pedal
      <br>
      <font-awesome-icon icon="circle" class="connected" v-if="connections.serialPort" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!connections.serialPort" />
      TTL link
    </p>
    <b>Stimulation</b>
    <p>
      <font-awesome-icon icon="circle" class="connected" v-if="stimulating" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!stimulating" />
      Stimulating
      <br>
      <font-awesome-icon icon="circle" class="connected" v-if="recharging" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!recharging" />
      Recharging
    </p>
  </div>
</template>

<script>
export default {
  name: "Status",
  data: function() {
    return {
      connections: {
        backend: false,
        neuronavigation: false,
        pedal: false,
        serialPort: false
      },
      stimulating: false,
      recharging: false,

      // Currently not shown anywhere.
      latestMessage: ""
    };
  },

  sockets: {
    connect() {
      this.connections["backend"] = true;
    },

    disconnect() {
      this.connections["backend"] = false;
    },

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

    "status.serial_port_connection"(data) {
      this.connections.serialPort = data
    }
  }
};
</script>

<style scoped>
.status {
  font-size: 14px;
  text-align: left;
}

.connected {
  color: green;
}

.not-connected {
  color: gray;
}
</style>
