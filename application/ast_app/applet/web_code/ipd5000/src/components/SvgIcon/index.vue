<template>
  <svg class="svg-icon" aria-hidden="true">
    <use :xlink:href="iconName"></use>
  </svg>
</template>

<script>
export default {
  name: 'icon-svg',
  props: {
    iconClass: {
      type: String,
      required: true
    }
  },
  computed: {
    iconName () {
      return `#icon-${this.iconClass}`
    }
  }
}
</script>

<style>
.svg-icon {
  width: 22px;
  height: 22px;
  vertical-align: -0.15em;
  fill: currentColor;
  color: #828283;
  overflow: hidden;
  vertical-align: middle;
}
</style>
