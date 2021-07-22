<template>
  <header class="header">
    <div>
      <img src="../assets/img/kramer_logo_word.svg" class="logo-word" alt="" />
      <span class="modelName">KDS-DEC-6X</span>
    </div>
    <img src="../assets/img/logo.svg" class="logo" alt="">
  </header>
</template>

<script>
export default {
  name: 'headerBar'
}
</script>

<style lang="less" scoped>
  .header{
    height: 70px;
    padding: 15px 20px;
    background: #000000;
    box-sizing: border-box;
    display: flex;
    align-items: center;
    justify-content: space-between;
    .logo-word{
      padding-right: 42px;
    }
    .modelName{
      font-size: 24px;
      color: #fff;
    }
    .logo{
      width: 50px;
      height: 50px;
      //border-radius: 8px;
    }
  }
</style>
