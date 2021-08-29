import Vue from "vue";
import VueRouter from "vue-router";
import Eeg from "@/views/Eeg.vue";
import Calibration from "@/views/Calibration.vue";

Vue.use(VueRouter);

const routes = [
  {
    path: "/calibration",
    name: "Calibration",
    component: Calibration
  },
  {
    path: "/eeg",
    name: "Eeg",
    component: Eeg
  }
];

const router = new VueRouter({
  mode: "history",
  base: process.env.BASE_URL,
  routes
});

export default router;
