<template>
  <div>
    <p>
      <button @click="stimulate()">Stimulate</button>
      <button @click="recharge()">Recharge</button>
      <button @click="abort()">Abort</button>
    </p>
  </div>
</template>

<script>
export default {
  methods: {
    stimulate() {
      this.$socket.emit("command", "stimulate");
    },
    recharge() {
      this.$socket.emit("command", "recharge");
    },
    abort() {
      this.$socket.emit("command", "abort");
    }
  }
};
</script>
