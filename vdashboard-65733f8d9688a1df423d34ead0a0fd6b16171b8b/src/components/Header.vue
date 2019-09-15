<template>
  <section class="hero is-bold is-primary">
    <div class="hero-body">
      <div class="container">
        <h1 class="title">
          VChain - Dashboard
        </h1>
        <h2 class="subtitle">
          {{ title }}
        </h2>
      </div>
    </div>
  </section>
</template>

<script>
export default {
  name: 'Header',
  props: {
    title: String,
  }
}
</script>
