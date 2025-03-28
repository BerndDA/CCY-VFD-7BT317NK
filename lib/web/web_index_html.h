/*
 * @Description: 
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-08-22 13:54:14
 * @LastEditTime: 2023-08-24 10:43:33
 */

/***********************************************************************************************
 * Copyright Statement:
 * The copyright of this source code belongs to [saisaiwa].
 *
 * Without the explicit authorization of the copyright owner, no part of this code may be used for commercial purposes, including but not limited to sale, rental, licensing, or publication.
 * It is only for personal learning, research, and non-profit purposes. If you have other usage needs, please contact [yustart@foxmail.com] for authorization.
 *
 * You are free to modify, use, and distribute this code under the following conditions:
 * - You must retain all content of this copyright statement.
 * - You must not use it for any illegal, abusive, or malicious activities.
 * - Your use should not damage the reputation of any individual, organization, or author.
 *
 * The author does not assume any warranty liability arising from the use of this code. The author is not responsible for any direct or indirect losses caused by the use of this code.
 * Github: https://github.com/ccy-studio/CCY-VFD-7BT317NK
 ***********************************************************************************************/

/*
 * @Description: 
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-08-18 14:52:18
 * @LastEditTime: 2023-08-21 22:58:46
 */
#ifndef __HTML_INDEX_H
#define __HTML_INDEX_H
#include <Arduino.h>
const char HTML_INDEX[] PROGMEM = R"HTML(
 <!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>VFD Backend Configuration</title>
    <link rel="stylesheet" href="https://unpkg.com/element-ui/lib/theme-chalk/index.css">
    <style>
        html {
            font-family: helvetica neue, Helvetica, pingfang sc, hiragino sans gb, microsoft yahei, 微软雅黑, Arial, sans-serif;
            background-color: #eee;
        }
        body {
            margin: 0;
        }
        .header {
            background-color: #fff;
            text-align: center;
            display: flex;
        }
        .header span {
            flex: 1;
            align-self: center;
            font-size: 18px;
            font-weight: 700;
        }
        .bq {
            font-size: 12px;
            color: #909399;
        }
    </style>
</head>
<body>
    <div id="app">
        <el-container>
            <el-header class="header">
                <span>VFD Desktop Clock Backend Configuration By: SAISAIWA</span>
            </el-header>
            <el-main>
                <el-alert effect="dark" style="margin-bottom: 20px;" title="Remember to click the save button after modification" type="success"></el-alert>
                <el-card class="box-card">
                    <el-form ref="form" :model="form" label-width="100px" label-position="top">
                        <el-form-item label="G1 Animation">
                            <el-switch v-model="form.annoOpen"></el-switch>
                        </el-form-item>
                        <el-form-item label="Scrolling Text">
                            <span slot="label">Scrolling Text
                                <el-tag effect="plain" style="margin-left: 5px;" size="mini" type="info">English numbers, punctuation marks under English. Chinese is not supported</el-tag>
                            </span>
                            <el-input maxlength="20" show-word-limit v-model="form.customLongText" placeholder="English form symbol number" autosize clearable></el-input>
                            <p style="font-size:12px;color:#f56c6c">Do not modify the scrolling text in scrolling mode, otherwise errors may occur!</p>
                        </el-form-item>
                        <el-form-item label="Scrolling Text Frame Rate (ms)">
                            <el-input @change="customFrameCheckChange" v-model="form.customLongTextFrame" placeholder="Enter frame rate 50~300" type="number" clearable></el-input>
                        </el-form-item>
                        <el-form-item label="Scheduled Power On/Off">
                            <el-switch v-model="form.autoPower"></el-switch>
                        </el-form-item>
                        <el-form-item label="Power On Time Setting" v-if="form.autoPower">
                            <el-time-picker value-format="HH:mm:ss" clearable v-model="form.autoPowerOpenTime" placeholder="Any time point"></el-time-picker>
                        </el-form-item>
                        <el-form-item label="Power Off Time Setting" v-if="form.autoPower">
                            <el-time-picker clearable v-model="form.autoPowerCloseTime" value-format="HH:mm:ss" placeholder="Any time point"></el-time-picker>
                        </el-form-item>
                        <el-form-item label="Power On/Off Effective Conditions" v-if="form.autoPower">
                            <el-checkbox-group v-model="form.autoPowerEnableDays">
                                <el-checkbox :label="1">Monday</el-checkbox>
                                <el-checkbox :label="2">Tuesday</el-checkbox>
                                <el-checkbox :label="3">Wednesday</el-checkbox>
                                <el-checkbox :label="4">Thursday</el-checkbox>
                                <el-checkbox :label="5">Friday</el-checkbox>
                                <el-checkbox :label="6">Saturday</el-checkbox>
                                <el-checkbox :label="0">Sunday</el-checkbox>
                            </el-checkbox-group>
                        </el-form-item>
                        <el-form-item label="Alarm Clock">
                            <el-switch v-model="form.alarmClock"></el-switch>
                            <el-time-picker style="margin-left: 10px;" v-if="form.alarmClock" clearable value-format="HH:mm:ss" v-model="form.alarmClockTime" placeholder="Any time point"></el-time-picker>
                        </el-form-item>
                        <el-form-item label="Alarm Clock Effective Conditions" v-if="form.alarmClock">
                            <el-checkbox-group v-model="form.alarmClockEnableDays">
                                <el-checkbox :label="1">Monday</el-checkbox>
                                <el-checkbox :label="2">Tuesday</el-checkbox>
                                <el-checkbox :label="3">Wednesday</el-checkbox>
                                <el-checkbox :label="4">Thursday</el-checkbox>
                                <el-checkbox :label="5">Friday</el-checkbox>
                                <el-checkbox :label="6">Saturday</el-checkbox>
                                <el-checkbox :label="0">Sunday</el-checkbox>
                            </el-checkbox-group>
                        </el-form-item>
                        <el-form-item label="Countdown">
                            <span slot="label">Countdown
                                <el-tag style="margin-left: 5px;" type="primary">{{form.countdown ? "Pending Execution" : "Not Set"}}</el-tag>
                            </span>
                            <el-switch v-model="form.countdown"></el-switch>
                            <el-time-picker style="margin-left: 10px;" v-if="form.countdown" value-format="HH:mm:ss" clearable :picker-options="{ start: '00:00:01', step: '00:00:01', end: '12:59:59' }" v-model="form.countdownTime" placeholder="Hours Minutes Seconds"></el-time-picker>
                        </el-form-item>
                        <el-button :loading="btnLoading" @click="saveSetting" style="margin-top: 30px;" icon="el-icon-check" type="primary" round>Save Changes</el-button>
                    </el-form>
                </el-card>
            </el-main>
            <el-footer>
                <p class="bq">Copyright © saisaiwa 2023~2025. All rights reserved.<br>Without the explicit permission of saisaiwa, no part of this webpage content may be reproduced, distributed, or modified in any form.<br>Disclaimer: The content of this webpage is for reference and educational purposes only. The author is not responsible for any direct or indirect losses or damages caused by the use of this webpage content. Users assume their own risk.<br>All third-party trademarks, logos, and names are used for illustrative purposes only and may be the property of their respective owners.<br>If you have any questions, please contact: yustart@foxmail.com</p>
                <el-link href="http://saisaiwa.com" type="primary" target="_blank">Create By SAISAIWA</el-link>
            </el-footer>
        </el-container>
    </div>
    <script src="https://unpkg.com/vue@2/dist/vue.js"></script>
    <script src="https://unpkg.com/axios/dist/axios.min.js"></script>
    <script src="https://unpkg.com/element-ui/lib/index.js"></script>
    <script>
        var vm = new Vue({
            el: "#app",
            data: function() {
                return {
                    btnLoading: false,
                    form: {
                        annoOpen: false,
                        customLongText: "Hello World",
                        customLongTextFrame: 200,
                        autoPower: false,
                        autoPowerOpenTime: null,
                        autoPowerCloseTime: null,
                        autoPowerEnableDays: [],
                        alarmClock: false,
                        alarmClockTime: null,
                        alarmClockEnableDays: [],
                        countdown: false,
                        countdownTime: null
                    }
                };
            },
            mounted() {
                this.getSettingData();
            },
            methods: {
                getSettingData() {
                    axios.get("get-setting").then(function(response) {
                        var data = response.data;
                        vm.form = data;
                        vm.form.alarmClockEnableDays = data.alarmClockEnableDays.filter(function(day) {
                            return day !== 0;
                        });
                        vm.form.autoPowerEnableDays = data.autoPowerEnableDays.filter(function(day) {
                            return day !== 0;
                        });
                        console.log(vm.form);
                    });
                },
                saveSetting() {
                    this.btnLoading = true;
                    this.form.customLongText = this.form.customLongText.trim();
                    axios({
                        url: "/save-setting",
                        method: "POST",
                        data: this.form
                    }).then(function(response) {
                        console.log(response.data);
                        if (response.data === "success") {
                            vm.btnLoading = false;
                            vm.$notify({
                                title: "Success",
                                message: "Saved successfully",
                                type: "success"
                            });
                        } else {
                            vm.$notify({
                                title: "Error",
                                message: "System exception",
                                type: "error"
                            });
                        }
                    });
                },
                customFrameCheckChange(value) {
                    if (value < 50) {
                        this.form.customLongTextFrame = 50;
                    }
                    if (value > 300) {
                        this.form.customLongTextFrame = 300;
                    }
                }
            }
        });
    </script>
</body>
</html>
  )HTML";
#endif