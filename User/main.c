#include "stm32f10x.h"                  // Device header
#include <string.h>
#include <stdlib.h>
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"			// 串口调试
#include "LED.h"
#include "utils.h"
#include "ESP8266.h"
#include "mytiming.h"

// ---- 定义全局变量 ----
// 温湿浓度结构体
Temperature_Typedef temp;	            // 温度结构体
Humidity_Typedef humi;		            // 湿度结构体
Concentration_Typedef concentration;  // PM2.5浓度结构体

// ESP8266模块结构体
ESP8266_InitTypeDef ESP8266_InitStructure; // WiFi模块初始化结构体
ESP8266_HTTP_PostDataTypedef ESP8266_HTTP_PostDataTypestructure; // 上传温湿度数据API结构体
ESP8266_HTTP_SendSmsTypedef ESP8266_HTTP_SendSmsTypestructure; // 发送短信API结构体

// 标识
uint8_t mainProgramSuccessInit = 0;       // 整体程序是否完成初始化
uint8_t ESP8266GetThresholdSuccess = 0;   // ESP8266是否获取到阈值(状态)
uint8_t ESP8266GetThreshold = 0;          // ESP8266是否获取阈值(动作)
uint8_t ESP8266GetDataSuccess = 0;        // ESP8266是否获取到温度、湿度、浓度数据(状态)
uint8_t ESP8266GetData = 0;               // ESP8266是否获取温度、湿度、浓度数据(动作)
// ---- 定义全局变量 ----

int main(void)
{
	// 初始化前配置NVIC中断分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			// 配置NVIC为分组2

	// 外设初始化
	OLED_Init();			    // OLED初始化
	Serial_Init();		    // 串口初始化
	LED_Init();				    // LED初始化
	ESP8266_Init();	      // ESP8266模块初始化
  Mytiming_Init();      // 定时器初始化

  // ---- 串口调试测试 ----
  Serial_Printf("\r\n程序已启动!\r\n");
  // ---- 串口调试测试 ----

	// 显示欢迎信息
	Utils_ShowWelcomeMessage();

  // ---- ESP8266软件初始化 ----
  OLED_ShowString(4, 4, "ESP8266...");  // 显示提示信息
  ESP8266_InitStructure.ESP8266_WIFI_SSID = "bwifaa";
  ESP8266_InitStructure.ESP8266_WIFI_PASSWORD = "123456789";
  // "116.62.81.138": 心知天气服务器IP地址
  // "192.168.41.242": 局域网内的电脑TCP客户端IP地址
  // "47.93.208.134": 测试综合实验服务器IP地址
  // "8.130.11.139": 最终综合实验服务器IP地址
  ESP8266_InitStructure.ESP8266_SERVER_ADDRESS = "8.130.11.139";
  ESP8266_InitStructure.ESP8266_SERVER_PORT = 8080;
  uint8_t res_ESP8266Connetion = ESP8266_InitConnection(&ESP8266_InitStructure); // 连接服务器
  if(!res_ESP8266Connetion)
  {
    OLED_Clear();
  }
  // ---- ESP8266软件初始化 ----

  // ---- OLED显示初始化 ----
  // 温度
	OLED_ShowChinese(0,0,2);		// 温
	OLED_ShowChinese(0,16,4);		// 度
	OLED_ShowString(1, 5, ":");
	OLED_ShowString(1, 9, ".");
  // 湿度
	OLED_ShowChinese(2,0,3);		// 湿
	OLED_ShowChinese(2,16,4);		// 度
	OLED_ShowString(2, 5, ":");
	OLED_ShowString(2, 9, ".");
  // PM2.5浓度
  OLED_ShowString(3,1,"PM2.5");
  OLED_ShowChinese(4,40,27);		// 浓
	OLED_ShowChinese(4,56,4);		// 度
	OLED_ShowString(3, 10, ":");
	OLED_ShowString(3, 14, ".");
  // ---- OLED显示初始化 ----

  // ---- 其他 ----
	uint8_t alarmFlag = 0; // 警报标志, 0:正常, 1:温度高, 2:湿度高, 3:PM2.5浓度高
	uint8_t lastAlarmFlag = 0; // 上一次警报标志
  uint8_t mainProgramShowLoadingSuccess = 0; // 是否成功显示加载中(只需一次)
  uint8_t mainProgramDoubliGetOLEDInitSuccess = 0;  // 成功获取阈值和数据之后是否清空屏幕(只需一次)
  // ---- 其他 ----

  mainProgramSuccessInit = 1; // 完成初始化

	while (1)
	{
    // ---- 本地判断(主程序) ----
    if(mainProgramShowLoadingSuccess == 0)
    {
      // 显示获取数据中
      OLED_Clear();
      OLED_ShowChinese(0,0,28);   // 数
      OLED_ShowChinese(0,16,29);  // 据
      OLED_ShowChinese(0,32,30);  // 加
      OLED_ShowChinese(0,48,31);  // 载
      OLED_ShowChinese(0,64,32);  // 中
      mainProgramShowLoadingSuccess = 1;
    }

    if(ESP8266GetThresholdSuccess && ESP8266GetDataSuccess)
    {
      if(mainProgramDoubliGetOLEDInitSuccess == 0)
      {
        // 清屏
        OLED_Clear();
        // 显示汉字
        OLED_ShowChinese(0,0,2);		// 温
        OLED_ShowChinese(0,16,4);		// 度
        OLED_ShowString(1, 5, ":");
        OLED_ShowString(1, 9, ".");
        OLED_ShowChinese(2,0,3);		// 湿
        OLED_ShowChinese(2,16,4);		// 度
        OLED_ShowString(2, 5, ":");
        OLED_ShowString(2, 9, ".");
        OLED_ShowString(3,1,"PM2.5");
        OLED_ShowChinese(4,40,27);		// 浓
        OLED_ShowChinese(4,56,4);		// 度
        OLED_ShowString(3, 10, ":");
        OLED_ShowString(3, 14, ".");
        mainProgramDoubliGetOLEDInitSuccess = 1;
      }
      // 已经获取到阈值与数据
      // 判断是否超过阈值
      alarmFlag = Utils_IsOverThreshold(&humi,&temp,&concentration);
      if(alarmFlag == 1 && alarmFlag != lastAlarmFlag)
      {
        // 当前温度超过阈值
        // LED_On();
        OLED_Clear();
        // 温度过高
        OLED_ShowChinese(0,0,2);		// 温
        OLED_ShowChinese(0,16,4);		// 度
        OLED_ShowChinese(0,32,5);		// 过
        OLED_ShowChinese(0,48,6);		// 高
        // 显示具体温度值
        OLED_ShowChinese(2,0,2);		// 温
        OLED_ShowChinese(2,16,4);		// 度
        OLED_ShowString(2, 5, ":");
        OLED_ShowString(2, 9, ".");
        OLED_ShowNum(2,7,temp.Temp_H,2);    // 温度 整数部分
        OLED_ShowNum(2,10,temp.Temp_L,1);   // 温度 小数部分

        // 发送短信
        // ESP8266_HTTP_SendSmsTypestructure.timeout = 20;
        // ESP8266_HTTPSendSms(&ESP8266_HTTP_SendSmsTypestructure);
      }
      else if(alarmFlag == 2 && alarmFlag != lastAlarmFlag)
      {
        // 当前湿度超过阈值
        // LED_On();
        OLED_Clear();
        // 湿度过高
        OLED_ShowChinese(0,0,3);		// 湿
        OLED_ShowChinese(0,16,4);		// 度
        OLED_ShowChinese(0,32,5);		// 过
        OLED_ShowChinese(0,48,6);		// 高
        // 显示具体湿度值
        OLED_ShowChinese(2,0,3);		// 湿
        OLED_ShowChinese(2,16,4);		// 度
        OLED_ShowString(2, 5, ":");
        OLED_ShowString(2, 9, ".");
        OLED_ShowNum(2,7,humi.Humi_H,2);    // 湿度 整数部分
        OLED_ShowNum(2,10,humi.Humi_L,1);   // 湿度 小数部分

        // 发送短信
        // ESP8266_HTTP_SendSmsTypestructure.timeout = 20;
        // ESP8266_HTTPSendSms(&ESP8266_HTTP_SendSmsTypestructure);
      }
      else if(alarmFlag == 3 && alarmFlag != lastAlarmFlag)
      {
        // 当前浓度超过阈值
        // LED_On();
        OLED_Clear();
        // 浓度过高
        OLED_ShowChinese(0,0,27);		// 浓
        OLED_ShowChinese(0,16,4);		// 度
        OLED_ShowChinese(0,32,5);		// 过
        OLED_ShowChinese(0,48,6);   // 高
        // 显示具体浓度值
        OLED_ShowString(2,1,"PM2.5");
        OLED_ShowChinese(2,40,27);	// 浓
        OLED_ShowChinese(2,56,4);		// 度
        OLED_ShowString(2, 10, ":");
        OLED_ShowString(2, 14, ".");
        OLED_ShowNum(2,11,concentration.Concentration_H,3);   // 浓度 整数部分
        OLED_ShowNum(2,15,concentration.Concentration_L,1);   // 浓度 小数部分

        // 发送短信
        // ESP8266_HTTP_SendSmsTypestructure.timeout = 20;
        // ESP8266_HTTPSendSms(&ESP8266_HTTP_SendSmsTypestructure);
      }
      else if(alarmFlag == 0)
      {
        // 当前正常
        LED_Off();
        if(alarmFlag != lastAlarmFlag)
        {
          // 清屏
          OLED_Clear();
          // 显示汉字
          OLED_ShowChinese(0,0,2);		// 温
          OLED_ShowChinese(0,16,4);		// 度
          OLED_ShowString(1, 5, ":");
          OLED_ShowString(1, 9, ".");
          OLED_ShowChinese(2,0,3);		// 湿
          OLED_ShowChinese(2,16,4);		// 度
          OLED_ShowString(2, 5, ":");
          OLED_ShowString(2, 9, ".");
          OLED_ShowString(3,1,"PM2.5");
          OLED_ShowChinese(4,40,27);		// 浓
          OLED_ShowChinese(4,56,4);		// 度
          OLED_ShowString(3, 10, ":");
          OLED_ShowString(3, 14, ".");
        }
        // 显示温度、湿度、浓度数据
        OLED_ShowNum(1,7,temp.Temp_H,2);	 	                  // 温度 整数部分
        OLED_ShowNum(1,10,temp.Temp_L,1);		                  // 温度 小数部分
        OLED_ShowNum(2,7,humi.Humi_H,2);  	                  // 湿度 整数部分
        OLED_ShowNum(2,10,humi.Humi_L,1); 	                  // 湿度 小数部分
        OLED_ShowNum(3,11,concentration.Concentration_H,3);   // 浓度 整数部分
        OLED_ShowNum(3,15,concentration.Concentration_L,1);   // 浓度 小数部分
      }
      lastAlarmFlag = alarmFlag;	// 更新警报标志
    }
    // ---- 本地判断(主程序) ----

    // ---- ESP8266获取阈值 ----
    if(ESP8266GetThreshold)
    {
      char* res = ESP8266_HTTP_Get("/fixthreshold?access=9mzdDx3K3pgmGdkTcq2ONsUKMp3VSTkIxoUwnHk7bljH4GrkYSeJXFtBp8HYN4f0DJAYPa", 100);
      char* res_end;
      do
      {
        res = ESP8266_GetBuffer();
        res_end = strstr(res, "}]}");
        Delay_ms(100);
      }while(res_end == NULL);
      // char* res_json = strchr(res, '{');
      // Serial_Printf("获取阈值:\r\n%s\r\n", res_json); // 打印 JSON 数据

      // 获取温度, 湿度, PM2.5浓度值
      char* name = "temperature";
      Utils_GetInformationOfThreshold_8(res, name, &temp.Temp_H_Threshold, &temp.Temp_L_Threshold);
      name = "humidity";
      Utils_GetInformationOfThreshold_8(res, name, &humi.Humi_H_Threshold, &humi.Humi_L_Threshold);
      name = "concentration";
      Utils_GetInformationOfThreshold_16(res, name, &concentration.Concentration_H_Threshold, &concentration.Concentration_L_Threshold);

      Serial_Printf("temp*:%d.%d\r\n", temp.Temp_H_Threshold, temp.Temp_L_Threshold);
      Serial_Printf("humi*:%d.%d\r\n", humi.Humi_H_Threshold, humi.Humi_L_Threshold);
      Serial_Printf("concentration*:%d.%d\r\n", concentration.Concentration_H_Threshold, concentration.Concentration_L_Threshold);

      // 更新标志位
      ESP8266GetThresholdSuccess = 1;
      ESP8266GetThreshold = 0;
    }
    // ---- ESP8266获取阈值 ---

    // ---- ESP8266获取数据 ----
    if(ESP8266GetData)
    {
      char* res = ESP8266_HTTP_Get("/thcp?access=9mzdDx3K3pgmGdkTcq2ONsUKMp3VSTkIxoUwnHk7bljH4GrkYSeJXFtBp8HYN4f0DJAYPa", 100);
      // Serial_Printf("获取数据:\r\n%s\r\n", res);
      char* res_end;
      do
      {
        res = ESP8266_GetBuffer();
        res_end = strstr(res, "}\"}");
        Delay_ms(100);
      }while(res_end == NULL);
      // char* res_json = strchr(res, '{');
      // Serial_Printf("获取数据JSON:\r\n%s\r\n", res_json); // 打印 JSON 数据

      // 获取温度, 湿度, PM2.5浓度值
      char* name = "temperature";
      Utils_GetInformationOfThcp_8(res, name, &temp.Temp_H, &temp.Temp_L);
      name = "humidity";
      Utils_GetInformationOfThcp_8(res, name, &humi.Humi_H, &humi.Humi_L);
      name = "concentration";
      Utils_GetInformationOfThcp_16(res, name, &concentration.Concentration_H, &concentration.Concentration_L);

      Serial_Printf("temp:%d.%d\r\n", temp.Temp_H, temp.Temp_L);
      Serial_Printf("humi:%d.%d\r\n", humi.Humi_H, humi.Humi_L);
      Serial_Printf("concentration:%d.%d\r\n", concentration.Concentration_H, concentration.Concentration_L);

      ESP8266GetDataSuccess = 1;
      ESP8266GetData = 0;
    }
    // ---- ESP8266获取数据 ----

		// ---- 延时 ----
		// Delay_ms(1000);
		// ---- 延时 ----
	}
}

// ---- 计时 ----

/**
 * @brief  1s中断函数处理函数
 */
void Mytiming_callback_1s(void)
{

}

/**
 * @brief  3s中断函数处理函数
 */
void Mytiming_callback_3s(void)
{

}

/**
 * @brief  5s中断函数处理函数
 */
void Mytiming_callback_5s(void)
{
  ESP8266GetThreshold = 1;
  ESP8266GetData = 1;
}

// 计时用计数器
uint32_t Mytiming_count_1s = 0;
uint32_t Mytiming_count_3s = 0;
uint32_t Mytiming_count_5s = 0;

/**
 * @brief  TIM2中断函数处理函数, 1ms触发一次
 * @param  无
 * @retval 无
*/
void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
  {
    if(mainProgramSuccessInit)
    {
      if(Mytiming_count_1s>=999)
      {
        Mytiming_callback_1s();
        Mytiming_count_1s = 0;
      }
      if(Mytiming_count_3s>=2999)
      {
        Mytiming_callback_3s();
        Mytiming_count_3s = 0;
      }
      if(Mytiming_count_5s>=4999)
      {
        Mytiming_callback_5s();
        Mytiming_count_5s = 0;
      }

      // 计数器增加
      Mytiming_count_1s++;
      Mytiming_count_3s++;
      Mytiming_count_5s++;
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}
// ---- 计时 ----
