<template>
  <div v-if="loading">
    Loading...
  </div>
  <div v-else id="planner" class="noselect">
    <table>
      <tr>
        <th class="visibility-column">
          <font-awesome-icon icon="eye" />
        </th>
        <th class="name-column">
          asdf
        </th>
        <th class="type-column">
          Type
        </th>
        <th class="comment-column">
          <font-awesome-icon icon="comment-alt" />
          Comment
        </th>
      </tr>
    </table>

    <div class="canvas">
      <table>
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

    <div id="actions">
      <font-awesome-icon icon="plus" class="fa-fw" v-on:click="addPoint()" />
      <font-awesome-icon icon="minus" class="fa-fw" v-on:click="removePoint()" />
      <font-awesome-icon icon="bullseye" class="fa-fw" v-on:click="toggleTarget()" />
    </div>

    <!-- Show a circle indicating if the coil is at the target or not. -->
    <p v-if="isCoilAtTarget">
      <font-awesome-icon icon="circle" class="coil-at-target" />
      Coil at the target
    </p>
    <p v-else>
      <font-awesome-icon icon="circle" class="coil-not-at-target" />
      Coil not at the target
    </p>

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
      isCoilAtTarget: false,
      position: undefined,
      points: []
    };
  },

  components: {
    Editable
  },

  created() {
    this.$socket.emit("planner.request_state", {});
  },

  methods: {
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
    getSelectedPoints() {
      return this.points.filter((point) => point["selected"]);
    },
    removePoint() {
      this.getSelectedPoints().forEach((point) => {
        this.$socket.emit("point.remove", {
          name: point['name']
        });
      });
    },
    toggleTarget() {
      const selectedPoints = this.getSelectedPoints();
      if (selectedPoints.length === 1) {
        this.$socket.emit("planner.point.toggle_target", {
          name: selectedPoints[0]["name"]
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

.visibility-column {
  width: $visibility-column-width;
}

.name-column {
  width: $name-column-width;
}

.type-column {
  width: $type-column-width;
}

.comment-column {
  width: $comment-column-width;
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

.coil-at-target {
  color: green;
}

.coil-not-at-target {
  color: red;
}
</style>
