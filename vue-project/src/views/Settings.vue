<template>
  <el-form :model="form" label-width="120px">
    <el-form-item label="WIFI名称">
      <el-input v-model="form.wifi.ssid" />
    </el-form-item>
    <el-form-item label="WIFI密码">
      <el-input v-model="form.wifi.password" :show-password="false" />
    </el-form-item>
    <el-form-item label="发件服务器">
      <el-input v-model="form.email.smtpServer" />
    </el-form-item>
    <el-form-item label="发件人">
      <el-input v-model="form.email.emailFrom" />
    </el-form-item>
    <el-form-item label="邮箱密码">
      <el-input v-model="form.email.passwordemail" :show-password="false" />
    </el-form-item>
    <el-form-item label="收件人">
      <el-input v-model="form.email.emailTo" />
    </el-form-item>
    <el-form-item label="BlinkerAUTH">
      <el-input v-model="form.blinker.auth" />
    </el-form-item>
    <el-form-item label="发件日期">
      <el-input v-model="form.sender.sendday" />
    </el-form-item>
    <el-form-item label="发件日期时间">
      <el-input v-model="form.sender.sendtime" />
    </el-form-item>
    <el-form-item>
      <el-button type="primary" @click="onSubmit">Submit</el-button>
      <el-button @click="onUpdate">Update</el-button>
    </el-form-item>
  </el-form>
</template>

<script lang="ts">
import { reactive } from "vue";
import axios from "axios";

// do not use same name with ref
const form = reactive({
  wifi: {
    ssid: "请输入wifi名称",
    password: "请输入wifi密码",
  },
  email: {
    smtpServer: "smtp.qq.com",
    emailFrom: "",
    passwordemail: "",
    emailTo: "",
  },
  blinker: {
    auth: "",
  },
  sender: {
    sendday: 1,
    sendtime: 1,
  },
});

export default {
  data() {
    return {
      form: form,
    };
  },
  mounted() {
    axios
      .get("/api/v1/config")
      .then((response) => (this.form = response.data))
      .catch(function (error) {
        console.log(error);
      });
  },
  methods: {
    onSubmit() {
      console.log("submit: " + this.form);
      axios
        .post("/api/v1/config", this.form, {
          headers: {
            "Content-Type": "application/json",
          },
        })
        .then((res) => {
          console.log(res.data);
        })
        .catch((Error) => {
          console.log(Error);
        });
    },
    onUpdate() {
      axios
        .get("/api/v1/config")
        .then((response) => (this.form = response.data))
        .catch(function (error) {
          console.log(error);
        });
    },
  },
};
</script>
