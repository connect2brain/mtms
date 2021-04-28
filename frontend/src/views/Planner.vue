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
          Comments
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
            <span v-on:dblclick="edit_name(row)" v-show="!row.edit">
              {{ row.name }}
            </span>
            <input
              type="text"
              ref="renameInput"
              :data-key="row.name"
              v-model="row.name"
              v-show="row.edit"
              v-on:blur="rename(row)"
              v-on:keyup.enter="rename(row)"
            />
          </td>
          <td class="type-column"></td>
          <td class="comment-column"></td>
        </tr>
      </table>
    </div>

    <div id="actions">
      <font-awesome-icon icon="plus" class="fa-fw" v-on:click="add_point()" />
      <font-awesome-icon icon="minus" class="fa-fw" />
    </div>

    <div>
      Current position: ({{ position[0].toFixed(0) }},
      {{ position[1].toFixed(0) }}, {{ position[2].toFixed(0) }})
    </div>
  </div>
</template>

<script>
export default {
  data() {
    return {
      isConnected: false,
      position: [0, 0, 0],
      points: [],
      selectedRows: []
    };
  },

  methods: {
    add_point() {
      this.points.push({
        position: self.position,
        name: "Target-" + Math.floor(Math.random() * 200),
        visible: false,
        edit: false
      });
    },
    toggle_visible(row) {
      row.visible = !row.visible;
    },
    edit_name(row) {
      row.edit = true;

      // Set focus to the input field and select the text.
      this.$nextTick(() => {
        const inputFields = this.$refs.renameInput.filter(
          el => el.getAttribute("data-key") === row.name
        );
        if (inputFields.length > 1) {
          throw "Found several rows with the same name";
        }
        const inputField = inputFields[0];
        inputField.focus();
        inputField.select();
      });
    },
    rename(row) {
      row.edit = false;
    }
  }
};
</script>

<style scoped lang="scss">
$visibility-column-width: 25px;
$name-column-width: 200px;
$type-column-width: 100px;
$comment-column-width: 100px;

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
