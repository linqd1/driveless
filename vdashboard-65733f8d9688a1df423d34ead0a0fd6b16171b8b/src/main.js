import Vue from 'vue';
import Buefy from 'buefy';
import 'buefy/dist/buefy.css';
import '@mdi/font/css/materialdesignicons.min.css';

import App from './App.vue'
import router from './routes';
import store from './store';

Vue.use(Buefy);
Vue.config.productionTip = false;

new Vue({
  render: h => h(App),
  router,
  store,
}).$mount('#app');
