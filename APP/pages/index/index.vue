<template>
	<view class="warp">
		<view class="dev-area">
			
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">温度</view>
					<image class="dev-logo" src="../../static/temp.png" mode=""></image>
				</view>
				<view class="dev-data">{{temp}}℃</view>
			</view>
			
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">湿度</view>
					<image class="dev-logo" src="../../static/humi.png" mode=""></image>
				</view>
				<view class="dev-data">{{humi}}%</view>
			</view>
			
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">光照强度</view>
					<image class="dev-logo" src="../../static/brig.png" mode=""></image>
				</view>
				<view class="dev-data">{{brig}}%</view>
			</view>
			
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">台灯</view>
					<image class="dev-logo" src="../../static/led.png" mode=""></image>
				</view>
				<switch :checked="led" @change="onLedSwitch" color="#0b6b11"/>
			</view>
			
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">风扇</view>
					<image class="dev-logo" src="../../static/fan.png" mode=""></image>
				</view>
				<switch :checked="fan" @change="onFanSwitch" color="#0b6b11"/>
			</view>
			
			<view class="device-cart-l">
				<view>
					<view class="dev-name">目标温度</view>
					<view class="dev-name">{{temp_tar}}℃</view>
				</view>
				<view class="ctrl-slider">
					<slider :value="temp_tar" @change="sliderChange($event, 'slider1')" min="0" max="100" step="1"
						block-size="20" show-value />
				</view>
			</view>
			
			<view class="device-cart-l">
				<view>
					<view class="dev-name">目标光照强度</view>
					<view class="dev-name">{{brig_tar}}%</view>
				</view>
				<view class="ctrl-slider">
					<slider :value="brig_tar" @change="sliderChange($event, 'slider2')" min="0" max="100" step="1"
						block-size="20" show-value />
				</view>
			</view>
			
			<view class="dev-cart">
				<view class="">
					3120002342
				</view>
			</view>

		</view>
	</view>

	</view>

</template>

<script>
	import {
		stringify
	} from 'querystring';
	const {
		createCommonToken
	} = require('@/key.js')
	export default {
		data() {
			return {
				temp: '',
				humi: '',
				brig: '',
				led: false,
				fan: false,
				token: '',
				temp_tar: 25,
				brig_tar: 60,
			}
		},
		onLoad() {
			const params = {
				author_key: 'f5bSzioAO9Q7r6YXJfKc9jDJz053gBcu9SDI5IFY2krI/qfZqKwqqMeUrdwzVSikKqtBao7X1Kvc+j0kgWmE9A==',
				version: '2022-05-01',
				user_id: '377780',
			}
			this.token = createCommonToken(params);
		},
		onShow() {
			this.fetchDevData();
			setInterval(()=>{
				this.fetchDevData();
			},3000)
		},
		methods: {
			fetchDevData() {
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/query-device-property', //仅为示例，并非真实接口地址。
					method: 'GET',
					data: {
						product_id: 'ArzhL4445I',
						device_name: 'd1'
					},
					header: {
						'authorization': this.token //自定义请求头信息
					},
					success: (res	) => {
						console.log(res.data);
						this.temp = res.data.data[4].value;
						this.humi = res.data.data[2].value;
						this.brig = res.data.data[0].value;
						this.led = res.data.data[3].value === 'true';
						this.fan = res.data.data[1].value === 'true';
					}
				});
			},
			onLedSwitch(event) {
				console.log(event.detail.value);
				let value = event.detail.value;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property', //仅为示例，并非真实接口地址。
					method: 'POST',
					data: {
						product_id: 'ArzhL4445I',
						device_name: 'd1',
						params: {
							"led": value
						}
					},
					header: {
						'authorization': this.token //自定义请求头信息
					},
					success: (res) => {
						console.log('LED' + (value ? 'ON' : 'OFF') + ' !');
					}
				});
			},
			onFanSwitch(event) {
				console.log(event.detail.value);
				let value = event.detail.value;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property', //仅为示例，并非真实接口地址。
					method: 'POST',
					data: {
						product_id: 'ArzhL4445I',
						device_name: 'd1',
						params: {
							"fan": value
						}
					},
					header: {
						'authorization': this.token //自定义请求头信息
					},
					success: (res) => {
						console.log('FAN' + (value ? 'ON' : 'OFF') + ' !');
					}
				});
			},
			// 滑动条变化事件的方法
			sliderChange(e, id) {
				console.log(id)
				console.log('value 发生变化：' + e.detail.value) // 控制台打印信息，调试用
				let key_tar = {}
				if (id == 'slider1') {
					this.temp_tar = e.detail.value
					// 更新温度阈值
					this.key_tar = {
						temp: this.temp_tar
					};
				} else if (id == 'slider2') {
					this.brig_tar = e.detail.value
					// 更新湿度阈值
					this.key_tar = {
						brig: this.brig_tar
					};
				}
				console.log('value 发生变化：>>>>>' + stringify(this.key_tar) + '<<<<<<<')
				// 向后端发送设备属性更新请求
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property', // 仅为示例，并非真实接口地址。
					method: 'POST',
					data: {
						product_id: 'ArzhL4445I',
						device_name: 'd1',
						params: this.key_tar
					},
					header: {
						'authorization': this.token // 自定义请求头信息
					},
					// 请求成功，打印提示信息，主要用于调试，可自定义
					success: () => {
						console.log(this.key_tar);
					}
				});
			}
		}
	}
</script>

<style>
	.warp {
		padding: 30rpx;
	}

	.dev-area {
		display: flex;
		justify-content: space-between;
		flex-wrap: wrap;
	}

	.dev-cart {
		height: 150rpx;
		width: 320rpx;
		border-radius: 30rpx;
		margin-top: 30rpx;
		display: flex;
		justify-content: space-around;
		align-items: center;
		box-shadow: 0 0 15rpx #ccc;
	}
	
	/* 长设备卡片样式 */
	.device-cart-l {
		height: 150rpx;
		/* 设置高度为150像素 */
		width: 700rpx;
		/* 设置宽度为700像素 */
		border-radius: 30rpx;
		/* 设置边框圆角为30像素 */
		margin-top: 30rpx;
		/* 设置上外边距为30像素 */
		display: flex;
		/* 使用弹性盒子布局 */
		justify-content: space-around;
		/* 在弹性容器中均匀分布子元素，两端对齐 */
		align-items: center;
		/* 在弹性容器中垂直居中对齐子元素 */
		box-shadow: 0 0 15rpx #ccc;
		/* 设置盒子阴影，颜色为灰色 */
	}

	.dev-name {
		font-size: 20rpx;
		text-align: center;
		color: #6d6d6d;
	}

	.dev-logo {
		width: 70rpx;
		height: 70rpx;
		margin-top: 10rpx;
	}

	.dev-data {
		font-size: 50rpx;
		color: #6d6d6d;
	}

	/* 滑动条样式 */
	.ctrl-slider {
		width: 580rpx;
		/* 设置宽度为580像素 */
	}
</style>