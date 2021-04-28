<template>
  <span>
    <span v-on:dblclick="edit()" v-show="!editing">
      {{ value }}
    </span>
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
    }
  },
  methods: {
    edit() {
      this.editing = true;
      console.log(this.$refs.renameInput);

      // Set focus to the input field and select the text.
      this.$nextTick(() => {
        const inputField = this.$refs.renameInput;
        inputField.focus();
        inputField.select();
      });
    },

    changed() {
      this.$emit("changed", this.newValue);
      this.editing = false;
    }
  }
};
</script>

<style scoped>
</style>
