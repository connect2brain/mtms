<template>
  <div v-if="loading">
    Loading...
  </div>
  <div v-else id="planner" class="noselect">
    <!-- Tab bar -->
    <div>
      <button
        class="tab-link"
        v-bind:class="{ 'active-tab': tab == 'targets' }"
        v-on:click="openTab('targets')">

        Targets
      </button>
      <button
        class="tab-link"
        v-bind:class="{ 'active-tab': tab == 'sequence' }"
        v-on:click="openTab('sequence')">

        Sequence
      </button>
    </div>

    <!-- Targets -->
    <div v-show="tab == 'targets'">
      <div class="canvas">
        <table>
          <tr>
            <th class="visibility-column">
              <font-awesome-icon icon="eye" />
            </th>
            <th class="name-column">
              Name
            </th>
            <th class="type-column">
              Type
            </th>
            <th class="comment-column">
              <font-awesome-icon icon="comment-alt" />
              Comment
            </th>
          </tr>
          <!-- XXX: When the planner is scrolled, the table header scrolls along. Ideally, it would remain
                    in place. However, if a separate table is created to form the header, the alignments of
                    the columns mismatch. There's a similar problem with Sequence tab.

                    See https://stackoverflow.com/questions/21168521/table-fixed-header-and-scrollable-body
                    for a potential solution. -->
          <tr
            v-for="row in points"
            :key="row.id"
            :row="row"
            v-on:click="toggleSelect(row)"
            v-bind:class="{ selected: isSelected(row) }"
          >
            <td class="visibility-column">
              <font-awesome-icon
                v-show="row.visible"
                v-on:click.stop.prevent="toggleVisible(row)"
                icon="eye"
              />
              <font-awesome-icon
                v-show="!row.visible"
                v-on:click.stop.prevent="toggleVisible(row)"
                icon="eye-slash"
              />
            </td>
            <td class="name-column">
              <font-awesome-icon icon="square" class="square" v-bind:class="{ target: row.target }" />
              <Editable
                :value="row.name"
                v-on:changed="rename(row, $event)"
                :allowEmpty="false"
              />
            </td>
            <td class="type-column">
              {{ row.type }}
            </td>
            <td class="comment-column">
              <Editable
                :value="row.comment"
                v-on:changed="changeComment(row, $event)"
                :allowEmpty="true"
              />
            </td>
          </tr>
        </table>
      </div>
    </div>

    <!-- Sequence -->
    <div v-show="tab == 'sequence'">
      <div class="canvas">
        <table>
          <tr>
            <th class="visibility-column">
              <font-awesome-icon icon="eye" />
            </th>
            <th class="number-column">
              #
            </th>
            <th class="name-column">
              Name
            </th>
            <th class="intensity-column">
              Intensity
            </th>
            <th class="iti-column">
              ITI
            </th>
            <th class="mode-duration-column">
              Mode duration
            </th>
          </tr>
          <tr
            v-for="row in targetPoints"
            :key="row.id"
            :row="row"
            v-on:click="toggleSelect(row)"
            v-bind:class="{ selected: isSelected(row) }"
          >
            <td class="visibility-column">
              <font-awesome-icon
                v-show="row.visible"
                v-on:click.stop.prevent="toggleVisible(row)"
                icon="eye"
              />
              <font-awesome-icon
                v-show="!row.visible"
                v-on:click.stop.prevent="toggleVisible(row)"
                icon="eye-slash"
              />
            </td>
            <td class="number-column">
              1
            </td>
            <td class="name-column">
              <font-awesome-icon icon="square" class="square" v-bind:class="{ target: row.target }" />
              <Editable
                :value="row.name"
                v-on:changed="rename(row, $event)"
                :allowEmpty="false"
              />
            </td>
            <td class="intensity-column">
              <Editable
                :value="row.intensity.toString()"
                v-on:changed="setIntensity(row, $event)"
                :allowEmpty="false"
              />
            </td>
            <td class="iti-column">
              <Editable
                :value="row.iti.toString()"
                v-on:changed="setIti(row, $event)"
                :allowEmpty="false"
              />
            </td>
            <td class="mode-duration-column">
              &mdash;
            </td>
          </tr>
        </table>
      </div>
    </div>

    <div id="actions">
      <font-awesome-icon icon="plus" class="fa-fw" v-on:click="addPoint()" />
      <font-awesome-icon icon="minus" class="fa-fw" v-on:click="removePoint()" />
      <font-awesome-icon icon="bullseye" class="fa-fw" v-on:click="toggleTarget()" />
      <font-awesome-icon
        icon="arrow-alt-circle-right"
        class="fa-fw toggle-navigating"
        v-bind:class="{ 'ready-to-navigate': isReadyToNavigate }"
        v-on:click="toggleNavigating()"
      />
      &nbsp;
      <font-awesome-icon
        icon="circle"
        class="fa-fw coil-at-target-indicator"
        v-bind:class="{ 'coil-at-target': isCoilAtTarget }"
      />
    </div>

    <!-- XXX: Keep the position display here for debugging purposes during the
              development, but remove later.
    -->
    <div v-if="position !== undefined">
      Current position: ({{ position[0].toFixed(0) }},
      {{ position[1].toFixed(0) }}, {{ position[2].toFixed(0) }})
    </div>
  </div>
</template>

<script>
import ROSLIB from "roslib";

import Editable from "@/components/Editable.vue";

export default {
  name: "Planner",

  props: [
    'ros'
  ],
  data() {
    return {
      loading: false,
      tab: 'targets',
      navigating: false,
      isCoilAtTarget: false,
      position: undefined,
      points: []
    };
  },
  computed: {
    targetPoints: function () {
      return this.points.filter((point) => point["target"]);
    },
    selectedPoints() {
      return this.points.filter((point) => point["selected"]);
    },
    isReadyToNavigate: function () {
      return this.points.some((point) => point["target"]);
    }
  },

  components: {
    Editable
  },

  created() {
    const listener = new ROSLIB.Topic({
      ros : this.ros,
      name : '/neuronavigation/focus',
      messageType : 'neuronavigation_interfaces/PoseUsingEulerAngles',
    });

    listener.subscribe(this.update_position);

    this.addTargetClient = new ROSLIB.Service({
      ros : this.ros,
      name : '/planner/add_target',
      serviceType : 'mtms_interfaces/AddTarget'
    });

    this.toggleSelectService = new ROSLIB.Service({
      ros : this.ros,
      name : '/planner/toggle_select',
      serviceType : 'mtms_interfaces/ToggleSelect'
    });

    this.removeTargetService = new ROSLIB.Service({
      ros : this.ros,
      name : '/planner/remove_target',
      serviceType : 'mtms_interfaces/RemoveTarget'
    });

    const stateListener = new ROSLIB.Topic({
      ros : this.ros,
      name : '/planner/state',
      messageType : 'mtms_interfaces/PlannerState',
    });

    stateListener.subscribe(this.update_state);
  },

  methods: {
    update_position(message) {
      this.position = [message.position.x, message.position.y, message.position.z];
    },

    update_state(message) {
      this.points = message.targets;
/*    [{
        visible: false,
        name: "",
        type: "Target",
        comment: "",
        selected: false,
        target: false,
        position: [2, 3, 4],
        direction: [1, 2, 3],

        intensity: 200,
        iti: 100,
      }]
 */
    },

    openTab(tab) {
      this.tab = tab;
    },

    addPoint() {
      if (this.position !== undefined) {
        const position = new ROSLIB.Message({
          x: this.position[0],
          y: this.position[1],
          z: this.position[2]
        });

        // TODO: Use proper values.
        const orientation = new ROSLIB.Message({
          alpha: 0.0,
          beta: 0.0,
          gamma: 0.0
        });

        const pose = new ROSLIB.Message({
          position: position,
          orientation: orientation
        });

        var request = new ROSLIB.ServiceRequest({
          target: pose,
        });

        this.addTargetClient.callService(request, function(result) {
          if (!result.success) {
            console.log('ERROR: Failed to add target: ');
            console.log(request.target);
          }
        });
      } else {
        /* XXX: Once we have a mechanism for showing user-visible errors, have this
                error shown to the user, instead, and also change the error message
                so that it makes sense to the user. */
        throw "Position is undefined.";
      }
    },
    removePoint() {
      this.selectedPoints.forEach((point) => {
        const request = new ROSLIB.ServiceRequest({
          name: point["name"]
        });

        this.removeTargetService.callService(request, function(result) {
          if (!result.success) {
            console.log('ERROR: Failed to remove target: ');
            console.log(request.name);
          }
        });
      });
    },
    toggleTarget() {
      if (this.selectedPoints.length === 1) {
        this.$socket.emit("planner.point.toggle_target", {
          name: this.selectedPoints[0]["name"]
        });
      } else {
        // TODO: Add some kind of an indicator to the user that the number of selected
        //       points is something else than one.
      }
    },
    toggleVisible(row) {
      row.visible = !row.visible;
    },
    rename(row, newName) {
      row.name = newName;
    },
    changeComment(row, newComment) {
      row.comment = newComment;
    },
    toggleSelect(row) {
      const request = new ROSLIB.ServiceRequest({
        name: row["name"]
      });

      this.toggleSelectService.callService(request, function(result) {
        if (!result.success) {
          console.log('ERROR: Failed to toggle select for target: ');
          console.log(request.name);
        }
      });
    },
    isSelected(row) {
      return row["selected"];
    },
    toggleNavigating() {
      if (this.isReadyToNavigate) {
        this.$socket.emit("planner.toggle_navigating");
      }
    },
    setIntensity(row, newIntensityString) {
      const name = row["name"];
      const newIntensity = parseInt(newIntensityString);

      if (!isNaN(newIntensity)) {
        this.$socket.emit("planner.point.set_intensity", {
          name: name,
          value: newIntensity
        });
        row.intensity = newIntensity;
      }
    },
    setIti(row, newItiString) {
      const name = row["name"];
      const newIti = parseInt(newItiString);

      if (!isNaN(newIti)) {
        this.$socket.emit("planner.point.set_iti", {
          name: name,
          value: newIti
        });
        row.iti = newIti;
      }
    }
  },

  sockets: {
    "planner.position"(position) {
      this.position = position;
    }
/*
    "planner.points"(points) {
      this.points = points;
    },

    "planner.coil_at_target"(state) {
      this.isCoilAtTarget = state;
    },

    "planner.state_sent"() {
      this.loading = false;
    },

    "planner.navigating"(navigating) {
      // TODO: The value of 'navigating' is not yet shown to the user.
      //
      this.navigating = navigating;
    }
*/
  }
};
</script>

<style scoped lang="scss">
@import "../styles/_colors.scss";

$visibility-column-width: 25px;
$name-column-width: 150px;
$type-column-width: 50px;
$comment-column-width: 150px;

$number-column-width: 20px;
$intensity-column-width: 55px;
$iti-column-width: 35px;
$mode-duration-column-width: 100px;

$total-width: $visibility-column-width + $name-column-width + $type-column-width +
  $comment-column-width;

#planner {
  text-align: left;
}

.canvas {
  height: 200px;
  width: $total-width + 8px;
  overflow-y: scroll;
}

table,
th,
td {
  border-spacing: 0px 2px;
}

th {
  background: $lighter-gray;
}

.tab-link {
  background: $light-gray;
}

.active-tab {
  background: white;
}

th,
.tab-link {
  color: $dark-gray;
  font-weight: normal;
  font-size: 12px;

  border-bottom: 1px solid $dark-gray;
  border-top: 1px solid $dark-gray;
}

td {
  border-bottom: 1px solid $light-gray;
}

td,
input {
  color: $dark-gray;
  font-size: 14px;
}

input {
  border-width: 0px;
}

.selected {
  background-color: $light-gray;
}

/* Common columns */

.visibility-column {
  width: $visibility-column-width;
}

.name-column {
  width: $name-column-width;
}

/* Columns on 'Targets' tab */

.type-column {
  width: $type-column-width;
}

.comment-column {
  width: $comment-column-width;
}

/* Columns on 'Sequence' tab */

.number-column {
  width: $number-column-width;
}

.intensity-column {
  width: $intensity-column-width;
}

.iti-column {
  width: $iti-column-width;
}

.mode-duration-column {
  width: $mode-duration-column-width;
}


#actions {
  color: $dark-gray;
  font-size: 18px;
}

.square {
  color: $non-target-color;
}

.target {
  color: $target-color;
}

.coil-at-target-indicator {
  color: red;
}

.coil-at-target {
  color: green;
}

.toggle-navigating {
  color: darkred;
}

.ready-to-navigate {
  color: darkgreen;
}
</style>
