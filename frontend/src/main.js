import Vue from "vue";
import VueSocketIO from "vue-socket.io";
import SocketIO from "socket.io-client";

import App from "./App.vue";
import router from "./router";

Vue.config.productionTip = false;

// XXX(okahilak): Connection objects must be used here for CORS to work in Socket.IO connection,
//   see https://github.com/MetinSeylan/Vue-Socket.io/issues/295.
const socketConnectionState = SocketIO('http://localhost:5000');

Vue.use(new VueSocketIO({
    debug: false,
    connection: socketConnectionState,
  })
);

new Vue({
  router,
  render: h => h(App)
}).$mount("#app");
