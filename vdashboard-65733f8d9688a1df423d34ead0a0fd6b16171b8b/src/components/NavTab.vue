<template>
  <section class="section navtab">
    <div class="tabs is-fullwidth">
      <ul>
        <li :class="{ 'is-active': this.$store.state.activatedTab === 'event' }">
          <router-link :to="{ name: 'eventList' }">事件</router-link>
        </li>
        <li :class="{ 'is-active': this.$store.state.activatedTab === 'record' }">
          <router-link :to="{ name: 'record' }">记录</router-link>
        </li>
        <li :class="{ 'is-active': this.$store.state.activatedTab === 'me' }">
          <router-link :to="{ name: 'me' }">我的</router-link>
        </li>
      </ul>
    </div>
  </section>
</template>

<script>
export default {
  name: 'NavTab',
  props: {
    activeTab: Number,
  }
}
</script>

<style scoped>
.navtab {
  position: fixed;
  bottom: 0px;
  width: 100%;
  padding: 0 0 0.5rem 0;
}
</style>
