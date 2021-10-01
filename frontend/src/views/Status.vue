<template>
  <div class="status">
    <b>Connections</b>
    <p>
      <font-awesome-icon
        class="circle"
        icon="circle"
        v-bind:class="{ enabled: connections.backend.connected }" />
      Core
      <br>
      <font-awesome-icon
        class="circle"
        icon="circle"
        v-bind:class="{ enabled: connections.neuronavigation.connected }" />
      Neuronavigation
    </p>
    <b>Devices</b>
    <p>
      <font-awesome-icon
        class="circle"
        icon="circle"
        v-bind:class="{ enabled: connections.pedal.connected, active: connections.pedal.state }" />
      Trigger pedal
      <br>
      <font-awesome-icon
        class="circle"
        icon="circle"
        v-bind:class="{ enabled: connections.serialPort.connected, active: connections.serialPort.data_received }" />
      TTL link
    </p>
    <b>Stimulation</b>
    <p>
      <font-awesome-icon
        class="circle"
        icon="circle"
        v-bind:class="{ enabled: stimulating }" />
      Stimulating
      <br>
      <font-awesome-icon
        class="circle"
        icon="circle"
        v-bind:class="{ enabled: recharging }" />
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
        backend: {
          connected: false,
          data_received: false
        },
        neuronavigation: {
          connected: false,
          data_received: false
        },
        pedal: {
          connected: false,
          data_received: false
        },
        serialPort: {
          connected: false,
          data_received: false
        }
      },
      stimulating: false,
      recharging: false,

      // Currently not shown anywhere.
      latestMessage: ""
    };
  },

  created() {
    this.DATA_RECEIVED_TIME_THRESHOLD = 200;
  },

  sockets: {
    connect() {
      this.connections.backend.connected = true;
    },

    disconnect() {
      this.connections.backend.connected = false;
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

    "status.serial_port_connection"(connected) {
      this.connections.serialPort.connected = connected;
    },

    "status.serial_port_pulse_triggered"() {
      this.connections.serialPort.data_received = true;

      setTimeout(() => {
        this.connections.serialPort.data_received = false;
      }, this.DATA_RECEIVED_TIME_THRESHOLD);
    },

    "status.pedal_connection"(connected) {
      this.connections.pedal.connected = connected;
    },

    "status.pedal_state_changed"(state) {
      this.connections.pedal.state = state;
    }
  }
};
</script>

<style scoped>
.status {
  font-size: 14px;
  text-align: left;
}

.circle {
  color: gray;
}

.enabled {
  color: green;
}

.active {
  color: lightgreen;
}
</style>
