<template>
  <div id="app">
    <router-view></router-view>
    <nav-tab :activeTab="1" v-if="this.$store.state.isLoggedIn"></nav-tab>
  </div>
</template>

<script>
import NavTab from './components/NavTab.vue';

export default {
  name: 'app',
  components: {
    NavTab,
  },
};
</script>

<style>
#app {
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
}
</style>
