import Vue from "vue";
import VueRouter from "vue-router";
import Home from "@/views/Home.vue";
import Eeg from "@/views/Eeg.vue";
import Tms from "@/views/Tms.vue";
import About from "@/views/About.vue";

Vue.use(VueRouter);

const routes = [
  {
    path: "/",
    name: "Home",
    component: Home
  },
  {
    path: "/eeg",
    name: "Eeg",
    component: Eeg
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
