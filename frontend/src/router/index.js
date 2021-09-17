import Vue from "vue";
import VueRouter from "vue-router";
import Nav from "@/views/Nav.vue";
import Eeg from "@/views/Eeg.vue";
import Calibration from "@/views/Calibration.vue";
import Planner from "@/views/Planner.vue";
import Tms from "@/views/Tms.vue";
import About from "@/views/About.vue";

Vue.use(VueRouter);

const routes = [
  {
    path: "/",
    name: "Nav",
    component: Nav
  },
  {
    path: "/eeg",
    name: "Eeg",
    component: Eeg
  },
  {
    path: "/calibration",
    name: "Calibration",
    component: Calibration
  },
  {
    path: "/planner",
    name: "Planner",
    component: Planner
  },
  {
    path: "/tms",
    name: "Tms",
    component: Tms
  },
  {
    path: "/about",
    name: "About",
    component: About
  }
];

const router = new VueRouter({
  mode: "history",
  base: process.env.BASE_URL,
  routes
});

export default router;
