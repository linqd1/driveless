import Vue from 'vue';
import Vuex from 'vuex';
import { getEventList } from '../api/event';

let eventSocket;

Vue.use(Vuex);

export default new Vuex.Store({
  state: {
    activatedTab: 'event',
    eventList: [],
    currentAccount: {},
    isLoggedIn: false,
  },
  mutations: {
    updateTab(state, payload) {
      state.activatedTab = payload;
    },
    updateEventList(state, payload) {
      state.eventList = payload;
    },
    login(state, { publicKey, privateKey }) {
      // validate
      state.currentAccount = {
        publicKey,
        privateKey,
      };
      state.isLoggedIn = true;
    },
  },
  actions: {
    async fetchEventList({ state, commit }) {
      if (!state.isLoggedIn) return;
      if (!eventSocket) {
        eventSocket = new WebSocket('ws://api.v.noinfinity/ws/event');
        eventSocket.onopen = () => {
          eventSocket.send(JSON.stringify({
            public: state.currentAccount.publicKey,
          }));
        }
        eventSocket.onmessage = ({ data }) => {
          const { type, event } = JSON.parse(data);
          const evl = [];
          if (type === 'new') {
            evl.push(event);
            evl.push(...state.eventList);
          }
          if (type === 'update') {
            evl.push(...state.eventList.map((v) => {
              if (v.id === event.id) return event;
              return v;
            }));
          }
          commit('updateEventList', evl);
        }
      }
      const data = await getEventList();
      commit('updateEventList', data);
    },
  }
});
