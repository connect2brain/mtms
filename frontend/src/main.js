import Vue from "vue";
import VueSocketIO from "vue-socket.io";
import SocketIO from "socket.io-client";

import { library } from "@fortawesome/fontawesome-svg-core";
import { FontAwesomeIcon } from "@fortawesome/vue-fontawesome";

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
import router from "./router";

library.add(
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

Vue.component("font-awesome-icon", FontAwesomeIcon);

Vue.config.productionTip = false;

const host = process.env.VUE_APP_BACKEND_HOST;
const port = process.env.VUE_APP_BACKEND_PORT;
const socketAddress = `http://${host}:${port}`;

// XXX(okahilak): Connection objects must be used here for CORS to work in Socket.IO connection,
//   see https://github.com/MetinSeylan/Vue-Socket.io/issues/295.
const socketConnectionState = SocketIO(socketAddress);

Vue.use(
  new VueSocketIO({
    debug: false,
    connection: socketConnectionState
  })
);

new Vue({
  router,
  render: h => h(App)
}).$mount("#app");
