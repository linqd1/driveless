<template>
  <div class="hero is-bold is-primary is-fullheight">
    <div class="hero-body">
      <div class="container">
        <h1 class="title">
          VChain - Dashboard
        </h1>
        <h2 class="subtitle">
          登陆
        </h2>
        <br>
        <div class="container loginpanel">
          <b-field label="公钥">
            <b-input v-model="publicKey" expanded></b-input>
          </b-field>
          <br>
          <b-field label="私钥">
            <b-input v-model="privateKey" expanded></b-input>
          </b-field>
          <br>
          <b-button @click="login" type="is-primary">&nbsp;&nbsp;登陆&nbsp;&nbsp;</b-button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'LoginView',
  data() {
    return {
      publicKey: '',
      privateKey: '',
    };
  },
  methods: {
    login() {
      this.$store.commit('login', {
        publicKey: this.publicKey,
        privateKey: this.privateKey,
      });
      if (this.$store.state.isLoggedIn) {
        this.$router.push({ name: 'me' });
      }
    },
  },
};
</script>

<style scoped>
.loginpanel {
  background-color: white;
  padding: 100px;
  margin-top: 50px;
  border-radius: 30px;
}
</style>


