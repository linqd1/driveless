import Vue from 'vue';
import Router from 'vue-router';

import store from '../store';

import EventListView from '../views/EventListView.vue';
import EventDetailView from '../views/EventDetailView.vue';
import RecordView from '../views/RecordView.vue';
import MeView from '../views/MeView.vue';
import LoginView from '../views/LoginView.vue';

Vue.use(Router);

const routes = [
  {
    path: '/',
    name: 'eventList',
    component: EventListView,
    beforeEnter(from, to, next) {
      store.commit('updateTab', 'event');
      next();
    },
  },
  {
    path: '/event/:eventId',
    name: 'eventDetail',
    component: EventDetailView,
    beforeEnter(from, to, next) {
      store.commit('updateTab', 'event');
      next();
    },
  },
  {
    path: '/record',
    name: 'record',
    component: RecordView,
    beforeEnter(from, to, next) {
      store.commit('updateTab', 'record');
      next();
    },
  },
  {
    path: '/me',
    name: 'me',
    component: MeView,
    beforeEnter(from, to, next) {
      store.commit('updateTab', 'me');
      next();
    },
  },
  {
    path: '/login',
    name: 'login',
    component: LoginView,
  }
];

const router = new Router({
  routes,
});

router.beforeEach((to, from, next) => {
  if (to.name === 'login' || store.state.isLoggedIn) {
    next();
  } else {
    next({ name: 'login' });
  }
});

export default router;
