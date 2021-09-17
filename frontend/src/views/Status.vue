<template>
  <div class="status">
    <b>Connections</b>
    <p>
      <font-awesome-icon icon="circle" class="connected" v-if="connections['backend']" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!connections['backend']" />
      Core
      <br>
      <font-awesome-icon icon="circle" class="connected" v-if="connections['neuronavigation']" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!connections['neuronavigation']" />
      Neuronavigation
    </p>
    <b>Devices</b>
    <p>
      <font-awesome-icon icon="circle" class="connected" v-if="connections['pedal']" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!connections['pedal']" />
      Trigger pedal
      <br>
      <font-awesome-icon icon="circle" class="connected" v-if="connections['ttl']" />
      <font-awesome-icon icon="circle" class="not-connected" v-if="!connections['ttl']" />
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
        ttl: false
      },
      stimulating: false,
      recharging: false
    };
  },

  sockets: {
    connect() {
      this.connections["backend"] = true;
    },

    disconnect() {
      this.connections["backend"] = false;
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
