<template>
  <div class="eeg">
    <div id="container">
      <svg id="area" :height="height" :width="width"></svg>
      <select v-model="channel">
        <option
          v-for="channel in channels"
          v-bind:key="channel"
          v-bind:value="channel"
        >
          Channel {{ channel + 1 }}
        </option>
      </select>
      <p>The latest backend call took {{ requestTime / 1000 }} seconds.</p>
    </div>
  </div>
</template>

<script>
import * as d3 from "d3";

const eegChannels = 64;

export default {
  name: "EegVisualizer",
  data: function() {
    return {
      eegData: [],
      channel: 0,
      channels: [...Array(eegChannels).keys()],
      requestTime: 0,
      errors: [],
      minTime: -25,
      updateInterval: 1000
    };
  },
  props: {
    width: {
      type: Number,
      default: 640
    },
    height: {
      type: Number,
      default: 480
    }
  },
  sockets: {
    eeg_data(data) {
      this.requestTime = Date.now() - this.requestStartTime;
      this.eegData = data;
    }
  },
  methods: {
    loadData: function() {
      this.requestStartTime = Date.now();
      this.$socket.emit("eeg_data", {
        from: this.minTime,
        to: 0
      });
    },
    updateGraph: function() {
      const margin = {
        top: 50,
        right: 50,
        bottom: 30,
        left: 40
      };

      // Re-initialize the SVG element
      const svg = d3.select("#area");
      svg.selectAll("*").remove();

      if (this.eegData.length == 0) {
        return;
      }

      // Create x- and y-scale
      const channelData = this.eegData.map(x => x.data[this.channel]);
      const yMax = Math.max(
        Math.abs(Math.min(...channelData)),
        Math.abs(Math.max(...channelData))
      );

      const xScale = d3
        .scaleLinear()
        .domain([this.minTime, 0])
        .range([margin.left, this.width - margin.right]);
      const xAxisY0 =
        margin.top + (this.height - margin.top - margin.bottom) / 2;

      const yScale = d3
        .scaleLinear()
        .domain([-yMax, yMax])
        .range([margin.top, this.height - margin.bottom]);
      const yAxisX0 = this.width - margin.right;

      // Draw the data
      const line = d3
        .line()
        .x(d => {
          return xScale(d.timestamp);
        })
        .y(d => {
          return yScale(d.data[this.channel]);
        });

      svg
        .append("g")
        .append("path")
        .datum(this.eegData)
        .attr("class", "line")
        .attr("d", line);

      // Draw x-axis
      svg
        .append("g")
        .attr("transform", `translate(0, ${xAxisY0})`)
        .call(d3.axisBottom(xScale));

      // Draw y-axis
      svg
        .append("g")
        .attr("transform", `translate(${yAxisX0}, 0)`)
        .call(d3.axisRight(yScale));
    }
  },
  watch: {
    eegData: function() {
      this.updateGraph();
    },
    channel: function() {
      this.updateGraph();
    }
  },
  mounted: function() {
    this.loadData();
    this.intervalTimer = setInterval(() => {
      this.loadData();
    }, this.updateInterval);
  },
  beforeDestroy() {
    clearInterval(this.intervalTimer);
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style>
h3 {
  margin: 40px 0 0;
}
ul {
  list-style-type: none;
  padding: 0;
}
li {
  display: inline-block;
  margin: 0 10px;
}
a {
  color: #42b983;
}
.line {
  fill: none;
  stroke: #ffab00;
  stroke-width: 3;
}
</style>
