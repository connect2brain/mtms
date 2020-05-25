<template>
  <div class="eeg">
    <div id="container">
      <svg id="area" :height="height" :width="width"></svg>
      <select v-model="channel">
        <option v-for="channel in channels"
                v-bind:key="channel"
                v-bind:value="channel">
          Channel {{ channel + 1 }}
        </option>
      </select>
      <p>
        The latest backend call took {{ requestTime / 1000 }} seconds.
      </p>
    </div>
  </div>
</template>

<script>
import axios from "axios";
import * as d3 from "d3";

export default {
  name: "EegVisualizer",
  data: function () {
    return {
      eegData: [],
      channel: 0,
      channels: [...Array(64).keys()],
      requestTime: 0,
      errors: [],
      minTime: -25,
      updateInterval: 1000
    }
  },
  props: {
    width: String,
    height: String
  },
  methods: {
    loadData: function() {
      this.requestStartTime = Date.now()

      // TODO: URL hard-coded for now, change to obtain host and port via config
      axios.get("http://localhost:5000/eeg_data", {
	params: {
	  from: this.minTime
	}})
	.then(response => {
	  this.requestTime = Date.now() - this.requestStartTime;
	  this.eegData = response.data;
	})
	.catch(e => {
	  this.errors.push(e)
	})
    },
    updateGraph: function() {
      var margin = {
	top: 50,
	right: 50,
	bottom: 30,
	left: 40
      }

      // Re-initialize the SVG element
      var svg = d3.select("#area")
      svg.selectAll("*").remove();

      if (this.eegData.length == 0) {
	return;
      }

      // Create x- and y-scale
      var channelData = this.eegData.map(x => x.data[this.channel])
      var yMax = Math.max(Math.abs(Math.min(...channelData)),
                           Math.abs(Math.max(...channelData)))

      var xScale = d3.scaleLinear()
	.domain([this.minTime, 0])
	.range([margin.left, this.width - margin.right]);
      var xAxisY0 = margin.top + (this.height - margin.top - margin.bottom) / 2;

      var yScale = d3.scaleLinear()
	.domain([-yMax, yMax])
	.range([margin.top, this.height - margin.bottom]);
      var yAxisX0 = this.width - margin.right;

      // Draw the data
      var line = d3.line()
	  .x(d => { return xScale(d.timestamp); })
	  .y(d => { return yScale(d.data[this.channel]); })

      svg.append("g")
	.append("path")
	.datum(this.eegData)
	.attr("class", "line")
	.attr("d", line);

      // Draw x-axis
      svg.append("g")
	.attr("transform", "translate(0, " + xAxisY0 + ")")
	.call(d3.axisBottom(xScale));

      // Draw y-axis
      svg.append("g")
	.attr("transform", "translate(" + yAxisX0 + ", 0)")
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

    setInterval(() => {
      this.loadData();
    }, this.updateInterval)
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
