<template>
  <div class="eeg">
    <h1>EEG Visualizer</h1>
    <EegVisualizer :width="640" :height="480" />
  </div>
</template>

<script>
import EegVisualizer from "@/components/EegVisualizer.vue";

export default {
  name: "Eeg",
  components: {
    EegVisualizer
  }
};
</script>
