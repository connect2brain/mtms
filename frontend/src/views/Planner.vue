<template>
  <div id="planner" class="noselect">
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
    </table>

    <div class="canvas">
      <table>
        <tr v-for="row in points" :key="row.id" :row="row">
          <td class="visibility-column">
            <font-awesome-icon
              v-if="row.visible"
              v-on:click="toggle_visible(row)"
              icon="eye"
            />
            <font-awesome-icon
              v-if="!row.visible"
              v-on:click="toggle_visible(row)"
              icon="eye-slash"
            />
          </td>
          <td class="name-column">
            <font-awesome-icon icon="square" class="target-square" />
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
              v-on:changed="change_comment(row, $event)"
              :allowEmpty="true"
            />
          </td>
        </tr>
      </table>
    </div>

    <div id="actions">
      <font-awesome-icon icon="plus" class="fa-fw" v-on:click="add_point()" />
      <font-awesome-icon icon="minus" class="fa-fw" />
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
      isConnected: false,
      position: undefined,
      points: [],
      selectedRows: []
    };
  },

  components: {
    Editable
  },

  methods: {
    add_point() {
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
    toggle_visible(row) {
      row.visible = !row.visible;
    },
    rename(row, newName) {
      row.name = newName;
    },
    change_comment(row, newComment) {
      row.comment = newComment;
    }
  },

  sockets: {
    "Set cross focal point"(data) {
      this.position = data.position.slice(0, 3);
    },

    "point.add"(data) {
      this.points.push({
        visible: data.visible,
        name: data.name,
        type: data.type,
        comment: data.comment,
        position: data.position
      });
    }
  }
};
</script>

<style scoped lang="scss">
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
  background: #f4f4f4;

  color: #707070;
  font-weight: normal;
  font-size: 12px;

  border-bottom: 1px solid #707070;
  border-top: 1px solid #707070;
}

td {
  border-bottom: 1px solid #e0e0e0;
}

td,
input {
  color: #707070;
  font-size: 14px;
}

input {
  border-width: 0px;
}

.row-selected {
  background-color: #e2e2e2;
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
  color: #707070;
  font-size: 18px;
}

.target-square {
  color: red;
}
</style>
