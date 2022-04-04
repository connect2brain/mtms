import Vue from "vue";
import ROSLIB from "roslib";

import { library } from "@fortawesome/fontawesome-svg-core";
import { FontAwesomeIcon } from "@fortawesome/vue-fontawesome";

import { faArrowAltCircleRight } from "@fortawesome/free-solid-svg-icons";
import { faBullseye } from "@fortawesome/free-solid-svg-icons";
import { faCommentAlt } from "@fortawesome/free-solid-svg-icons";
import { faCircle } from "@fortawesome/free-solid-svg-icons";
import { faEye } from "@fortawesome/free-solid-svg-icons";
import { faEyeSlash } from "@fortawesome/free-solid-svg-icons";
import { faFolder } from "@fortawesome/free-solid-svg-icons";
import { faMinus } from "@fortawesome/free-solid-svg-icons";
import { faPlus } from "@fortawesome/free-solid-svg-icons";
import { faRedo } from "@fortawesome/free-solid-svg-icons";
import { faSquare } from "@fortawesome/free-solid-svg-icons";

import App from "./App.vue";
import router from  "./router";

library.add(
  faArrowAltCircleRight,
  faBullseye,
  faEye,
  faEyeSlash,
  faFolder,
  faCircle,
  faCommentAlt,
  faPlus,
  faMinus,
  faRedo,
  faSquare
);

const ros = new ROSLIB.Ros({
  url : 'ws://localhost:9090'
});

Vue.component("font-awesome-icon", FontAwesomeIcon);

Vue.config.productionTip = false;

new Vue({
  router,
  render: h => h(App, {
    props: {
      'ros': ros
    }
  })
}).$mount("#app");
