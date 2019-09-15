<template>
  <b-collapse class="card" :open="!event.status">
    <div
      slot="trigger" 
      slot-scope="props"
      class="card-header"
      role="button">
      <p class="card-header-title">
        <b-tooltip :label="eventDecider" v-if="event.status" position="is-right" type="is-info">
          <b-icon :icon="eventDeciderIcon" class="is-info"></b-icon>
        </b-tooltip>
        &nbsp;
        <b-icon
          :icon="eventStatusIcon"
          :style="{ color: eventStatusIconColor }"></b-icon>
        &nbsp;
        <b-tag :type='event.type.level'>{{ event.type.name }}</b-tag>
        &nbsp;
        <router-link :to="{ name: 'eventDetail', params: { eventId: event.id }}">#{{ event.id }}</router-link>
      </p>
      <a class="card-header-icon">
        <b-icon
          :icon="props.open ? 'menu-down' : 'menu-up'">
        </b-icon>
      </a>
    </div>
    <div class="card-content" @click="goToDetail">
      <div class="content">
        <div class="level">
          <div class="level-left">
            <div class="level-item">
              <p>发起人：</p>
            </div>
          </div>
          <div class="level-right">
            <div class="level-item">
              <p>{{ event.from }}</p>
            </div>
          </div>
        </div>
        <div class="level" v-for="(v, k) in event.data" :key="k">
          <div class="level-left">
            <div class="level-item">
              <p>{{ k }}：</p>
            </div>
          </div>
          <div class="level-right">
            <div class="level-item">
              <p>{{ v }}</p>
            </div>
          </div>
        </div>
      </div>
    </div>
    <footer class="card-footer">
      <a class="card-footer-item" style="color: red;" v-if="!expired">拒绝</a>
      <a class="card-footer-item" style="color: green;" v-if="!expired">同意</a>
      <div class="card-footer-item" v-if="expired && !event.status">
        <b-icon icon="exclamation" ></b-icon>
        已交由车辆自动决策
      </div>
      <router-link :to="{ name: 'eventDetail', params: { eventId: event.id }}" class="card-footer-item">
        <b-icon icon="dots-horizontal"></b-icon>
      </router-link>
    </footer>
  </b-collapse>
</template>

<script>
export default {
  name: 'EventCard',
  props: {
    event: Object,
  },
  data() {
    return {
      expired: false,
    };
  },
  computed: {
    eventStatusIcon() {
      if (this.event.status === 'accept') {
        return 'check';
      }
      if (this.event.status === 'reject') {
        return 'close';
      }
      return 'checkbox-blank-outline';
    },
    eventStatusIconColor() {
      if (this.event.status === 'accept') {
        return 'green';
      }
      if (this.event.status === 'reject') {
        return 'red';
      }
      return '';
    },
    eventDeciderIcon() {
      if (this.event.decider === 'human') {
        return 'hand';
      }
      if (this.event.decider === 'car') {
        return 'steering';
      }
      return '';
    },
    eventDecider() {
      if (this.event.decider === 'human') {
        return '事件由人类决策';
      }
      if (this.event.decider === 'car') {
        return '事件由车辆自动决策';
      }
      return '';
    },
  },
  methods: {
    goToDetail() {
      this.$router.push({ name: 'eventDetail', params: { eventId: this.event.id }});
    }
  },
  created() {
    if (this.event.expire <= Date.now() + 200) {
      this.expired = true;
    } else {
      this.expired = false;
      setTimeout(() => {
        this.expired = true;
      }, this.event.expire - Date.now() - 200);
    }
  }
}
</script>

<style scoped>
.card-content {
  cursor: pointer;
}

.card {
  margin-bottom: 15px;
}
</style>
