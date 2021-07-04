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
            <th class="isi-column">
              ISI
            </th>
            <th class="mode-duration-column">
              Mode Duration
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
              100
            </td>
            <td class="isi-column">
              100
            </td>
            <td class="mode-duration-column">
              80
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
import Editable from "@/components/Editable.vue";

export default {
  data() {
    return {
      loading: true,
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
    this.$socket.emit("planner.request_state", {});
  },

  methods: {
    openTab(tab) {
      this.tab = tab;
    },

    addPoint() {
      if (this.position !== undefined) {
        this.$socket.emit("point.add", {
          position: this.position
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
        this.$socket.emit("point.remove", {
          name: point['name']
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
      this.$socket.emit("planner.point.toggle_select", {
        name: row["name"]
      });
    },
    isSelected(row) {
      return row["selected"];
    },
    toggleNavigating() {
      if (this.isReadyToNavigate) {
        this.$socket.emit("planner.toggle_navigating");
      }
    }
  },

  sockets: {
    "planner.position"(position) {
      this.position = position;
    },

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
$intensity-column-width: 60px;
$isi-column-width: 20px;
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

.isi-column {
  width: $isi-column-width;
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
