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
        v-bind:class="{ enabled: connections.trigger.connected, active: connections.trigger.data_received }" />
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
import ROSLIB from "roslib";

export default {
  name: "Status",

  props: [
    'ros'
  ],
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
          state: false
        },
        trigger: {
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

    this.add_pedal_listeners();
    this.add_trigger_listeners();
  },

  methods: {
    /* Pedal-related methods */

    add_pedal_listeners() {
      const pedal_connected_listener = new ROSLIB.Topic({
        ros : this.ros,
        name : '/pedal/connected',
        messageType : 'std_msgs/Bool',
      });

      pedal_connected_listener.subscribe(this.update_pedal_connected);

      const pedal_pressed_listener = new ROSLIB.Topic({
        ros : this.ros,
        name : '/pedal/pressed',
        messageType : 'std_msgs/Bool',
      });

      pedal_pressed_listener.subscribe(this.update_pedal_pressed);
    },

    update_pedal_connected(message) {
      this.connections.pedal.connected = message.data;
    },

    update_pedal_pressed(message) {
      this.connections.pedal.state = message.data;
    },

    /* Trigger-related methods */

    add_trigger_listeners() {
      const trigger_connected_listener = new ROSLIB.Topic({
        ros : this.ros,
        name : '/trigger/connected',
        messageType : 'std_msgs/Bool',
      });

      trigger_connected_listener.subscribe(this.update_trigger_connected);

      const triggered_listener = new ROSLIB.Topic({
        ros : this.ros,
        name : '/trigger/triggered',
        messageType : 'std_msgs/Bool',
      });

      triggered_listener.subscribe(this.update_triggered);
    },

    update_trigger_connected(message) {
      this.connections.trigger.connected = message.data;
    },

    update_triggered() {
      this.connections.trigger.data_received = true;

      setTimeout(() => {
        this.connections.trigger.data_received = false;
      }, this.DATA_RECEIVED_TIME_THRESHOLD);
    }
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
