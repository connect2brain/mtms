<template>
  <div class="healthcheck">
    <p v-if="isConnected">
      <font-awesome-icon icon="circle" class="connected" />
      Connected
    </p>
    <p v-else>
      <font-awesome-icon icon="circle" class="not-connected" />
      Not connected
    </p>
  </div>
</template>

<script>
export default {
  name: "HealthCheck",
  data: function() {
    return {
      isConnected: false
    };
  },

  sockets: {
    connect() {
      this.isConnected = true;
    },

    disconnect() {
      this.isConnected = false;
    }
  }
};
</script>

<style scoped>
.healthcheck {
  font-size: 14px;
}

.connected {
  color: green;
}

.not-connected {
  color: red;
}
</style>
