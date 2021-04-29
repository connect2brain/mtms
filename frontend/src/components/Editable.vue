<template>
  <span>
    <span v-on:dblclick="edit()" v-show="!editing && value.trim() != ''">
      {{ value }}
    </span>
    <div
      class="empty-clickable-area"
      v-on:dblclick="edit()"
      v-show="!editing && value.trim() == ''"
    ></div>
    <input
      type="text"
      ref="renameInput"
      v-model="newValue"
      v-show="editing"
      v-on:blur="changed()"
      v-on:keyup.enter="changed()"
    />
  </span>
</template>

<script>
export default {
  name: "Editable",
  data: function() {
    return {
      editing: false,
      newValue: this.value
    };
  },
  props: {
    value: {
      type: String
    },
    finished: {
      type: Function
    },
    allowEmpty: {
      type: Boolean
    }
  },
  methods: {
    edit() {
      this.editing = true;

      // Set focus to the input field and select the text.
      this.$nextTick(() => {
        const inputField = this.$refs.renameInput;
        inputField.focus();
        inputField.select();
      });
    },

    changed() {
      if (!this.allowEmpty && this.newValue.trim() == "") {
        this.newValue = this.value;
      } else {
        this.$emit("changed", this.newValue);
      }
      this.editing = false;
    }
  }
};
</script>

<style scoped>
.empty-clickable-area {
  height: 14px;
  width: 170px;
}
</style>
