#include "stm32f10x.h"                  // Device header
#include <string.h>
#include <stdlib.h>
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"			// ���ڵ���
#include "LED.h"
#include "utils.h"
#include "ESP8266.h"
#include "mytiming.h"

// ---- ����ȫ�ֱ��� ----
// ��ʪŨ�Ƚṹ��
Temperature_Typedef temp;	            // �¶Ƚṹ��
Humidity_Typedef humi;		            // ʪ�Ƚṹ��
Concentration_Typedef concentration;  // PM2.5Ũ�Ƚṹ��

// ESP8266ģ��ṹ��
ESP8266_InitTypeDef ESP8266_InitStructure; // WiFiģ���ʼ���ṹ��
ESP8266_HTTP_PostDataTypedef ESP8266_HTTP_PostDataTypestructure; // �ϴ���ʪ������API�ṹ��
ESP8266_HTTP_SendSmsTypedef ESP8266_HTTP_SendSmsTypestructure; // ���Ͷ���API�ṹ��

// ��ʶ
uint8_t mainProgramSuccessInit = 0;       // ��������Ƿ���ɳ�ʼ��
uint8_t ESP8266GetThresholdSuccess = 0;   // ESP8266�Ƿ��ȡ����ֵ(״̬)
uint8_t ESP8266GetThreshold = 0;          // ESP8266�Ƿ��ȡ��ֵ(����)
uint8_t ESP8266GetDataSuccess = 0;        // ESP8266�Ƿ��ȡ���¶ȡ�ʪ�ȡ�Ũ������(״̬)
uint8_t ESP8266GetData = 0;               // ESP8266�Ƿ��ȡ�¶ȡ�ʪ�ȡ�Ũ������(����)
// ---- ����ȫ�ֱ��� ----

int main(void)
{
	// ��ʼ��ǰ����NVIC�жϷ���
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			// ����NVICΪ����2

	// �����ʼ��
	OLED_Init();			    // OLED��ʼ��
	Serial_Init();		    // ���ڳ�ʼ��
	LED_Init();				    // LED��ʼ��
	ESP8266_Init();	      // ESP8266ģ���ʼ��
  Mytiming_Init();      // ��ʱ����ʼ��

  // ---- ���ڵ��Բ��� ----
  Serial_Printf("\r\n����������!\r\n");
  // ---- ���ڵ��Բ��� ----

	// ��ʾ��ӭ��Ϣ
	Utils_ShowWelcomeMessage();

  // ---- ESP8266�����ʼ�� ----
  OLED_ShowString(4, 4, "ESP8266...");  // ��ʾ��ʾ��Ϣ
  ESP8266_InitStructure.ESP8266_WIFI_SSID = "bwifaa";
  ESP8266_InitStructure.ESP8266_WIFI_PASSWORD = "123456789";
  // "116.62.81.138": ��֪����������IP��ַ
  // "192.168.41.242": �������ڵĵ���TCP�ͻ���IP��ַ
  // "47.93.208.134": �����ۺ�ʵ�������IP��ַ
  // "8.130.11.139": �����ۺ�ʵ�������IP��ַ
  ESP8266_InitStructure.ESP8266_SERVER_ADDRESS = "8.130.11.139";
  ESP8266_InitStructure.ESP8266_SERVER_PORT = 8080;
  uint8_t res_ESP8266Connetion = ESP8266_InitConnection(&ESP8266_InitStructure); // ���ӷ�����
  if(!res_ESP8266Connetion)
  {
    OLED_Clear();
  }
  // ---- ESP8266�����ʼ�� ----

  // ---- OLED��ʾ��ʼ�� ----
  // �¶�
	OLED_ShowChinese(0,0,2);		// ��
	OLED_ShowChinese(0,16,4);		// ��
	OLED_ShowString(1, 5, ":");
	OLED_ShowString(1, 9, ".");
  // ʪ��
	OLED_ShowChinese(2,0,3);		// ʪ
	OLED_ShowChinese(2,16,4);		// ��
	OLED_ShowString(2, 5, ":");
	OLED_ShowString(2, 9, ".");
  // PM2.5Ũ��
  OLED_ShowString(3,1,"PM2.5");
  OLED_ShowChinese(4,40,27);		// Ũ
	OLED_ShowChinese(4,56,4);		// ��
	OLED_ShowString(3, 10, ":");
	OLED_ShowString(3, 14, ".");
  // ---- OLED��ʾ��ʼ�� ----

  // ---- ���� ----
	uint8_t alarmFlag = 0; // ������־, 0:����, 1:�¶ȸ�, 2:ʪ�ȸ�, 3:PM2.5Ũ�ȸ�
	uint8_t lastAlarmFlag = 0; // ��һ�ξ�����־
  uint8_t mainProgramShowLoadingSuccess = 0; // �Ƿ�ɹ���ʾ������(ֻ��һ��)
  uint8_t mainProgramDoubliGetOLEDInitSuccess = 0;  // �ɹ���ȡ��ֵ������֮���Ƿ������Ļ(ֻ��һ��)
  // ---- ���� ----

  mainProgramSuccessInit = 1; // ��ɳ�ʼ��

	while (1)
	{
    // ---- �����ж�(������) ----
    if(mainProgramShowLoadingSuccess == 0)
    {
      // ��ʾ��ȡ������
      OLED_Clear();
      OLED_ShowChinese(0,0,28);   // ��
      OLED_ShowChinese(0,16,29);  // ��
      OLED_ShowChinese(0,32,30);  // ��
      OLED_ShowChinese(0,48,31);  // ��
      OLED_ShowChinese(0,64,32);  // ��
      mainProgramShowLoadingSuccess = 1;
    }

    if(ESP8266GetThresholdSuccess && ESP8266GetDataSuccess)
    {
      if(mainProgramDoubliGetOLEDInitSuccess == 0)
      {
        // ����
        OLED_Clear();
        // ��ʾ����
        OLED_ShowChinese(0,0,2);		// ��
        OLED_ShowChinese(0,16,4);		// ��
        OLED_ShowString(1, 5, ":");
        OLED_ShowString(1, 9, ".");
        OLED_ShowChinese(2,0,3);		// ʪ
        OLED_ShowChinese(2,16,4);		// ��
        OLED_ShowString(2, 5, ":");
        OLED_ShowString(2, 9, ".");
        OLED_ShowString(3,1,"PM2.5");
        OLED_ShowChinese(4,40,27);		// Ũ
        OLED_ShowChinese(4,56,4);		// ��
        OLED_ShowString(3, 10, ":");
        OLED_ShowString(3, 14, ".");
        mainProgramDoubliGetOLEDInitSuccess = 1;
      }
      // �Ѿ���ȡ����ֵ������
      // �ж��Ƿ񳬹���ֵ
      alarmFlag = Utils_IsOverThreshold(&humi,&temp,&concentration);
      if(alarmFlag == 1 && alarmFlag != lastAlarmFlag)
      {
        // ��ǰ�¶ȳ�����ֵ
        // LED_On();
        OLED_Clear();
        // �¶ȹ���
        OLED_ShowChinese(0,0,2);		// ��
        OLED_ShowChinese(0,16,4);		// ��
        OLED_ShowChinese(0,32,5);		// ��
        OLED_ShowChinese(0,48,6);		// ��
        // ��ʾ�����¶�ֵ
        OLED_ShowChinese(2,0,2);		// ��
        OLED_ShowChinese(2,16,4);		// ��
        OLED_ShowString(2, 5, ":");
        OLED_ShowString(2, 9, ".");
        OLED_ShowNum(2,7,temp.Temp_H,2);    // �¶� ��������
        OLED_ShowNum(2,10,temp.Temp_L,1);   // �¶� С������

        // ���Ͷ���
        // ESP8266_HTTP_SendSmsTypestructure.timeout = 20;
        // ESP8266_HTTPSendSms(&ESP8266_HTTP_SendSmsTypestructure);
      }
      else if(alarmFlag == 2 && alarmFlag != lastAlarmFlag)
      {
        // ��ǰʪ�ȳ�����ֵ
        // LED_On();
        OLED_Clear();
        // ʪ�ȹ���
        OLED_ShowChinese(0,0,3);		// ʪ
        OLED_ShowChinese(0,16,4);		// ��
        OLED_ShowChinese(0,32,5);		// ��
        OLED_ShowChinese(0,48,6);		// ��
        // ��ʾ����ʪ��ֵ
        OLED_ShowChinese(2,0,3);		// ʪ
        OLED_ShowChinese(2,16,4);		// ��
        OLED_ShowString(2, 5, ":");
        OLED_ShowString(2, 9, ".");
        OLED_ShowNum(2,7,humi.Humi_H,2);    // ʪ�� ��������
        OLED_ShowNum(2,10,humi.Humi_L,1);   // ʪ�� С������

        // ���Ͷ���
        // ESP8266_HTTP_SendSmsTypestructure.timeout = 20;
        // ESP8266_HTTPSendSms(&ESP8266_HTTP_SendSmsTypestructure);
      }
      else if(alarmFlag == 3 && alarmFlag != lastAlarmFlag)
      {
        // ��ǰŨ�ȳ�����ֵ
        // LED_On();
        OLED_Clear();
        // Ũ�ȹ���
        OLED_ShowChinese(0,0,27);		// Ũ
        OLED_ShowChinese(0,16,4);		// ��
        OLED_ShowChinese(0,32,5);		// ��
        OLED_ShowChinese(0,48,6);   // ��
        // ��ʾ����Ũ��ֵ
        OLED_ShowString(2,1,"PM2.5");
        OLED_ShowChinese(2,40,27);	// Ũ
        OLED_ShowChinese(2,56,4);		// ��
        OLED_ShowString(2, 10, ":");
        OLED_ShowString(2, 14, ".");
        OLED_ShowNum(2,11,concentration.Concentration_H,3);   // Ũ�� ��������
        OLED_ShowNum(2,15,concentration.Concentration_L,1);   // Ũ�� С������

        // ���Ͷ���
        // ESP8266_HTTP_SendSmsTypestructure.timeout = 20;
        // ESP8266_HTTPSendSms(&ESP8266_HTTP_SendSmsTypestructure);
      }
      else if(alarmFlag == 0)
      {
        // ��ǰ����
        LED_Off();
        if(alarmFlag != lastAlarmFlag)
        {
          // ����
          OLED_Clear();
          // ��ʾ����
          OLED_ShowChinese(0,0,2);		// ��
          OLED_ShowChinese(0,16,4);		// ��
          OLED_ShowString(1, 5, ":");
          OLED_ShowString(1, 9, ".");
          OLED_ShowChinese(2,0,3);		// ʪ
          OLED_ShowChinese(2,16,4);		// ��
          OLED_ShowString(2, 5, ":");
          OLED_ShowString(2, 9, ".");
          OLED_ShowString(3,1,"PM2.5");
          OLED_ShowChinese(4,40,27);		// Ũ
          OLED_ShowChinese(4,56,4);		// ��
          OLED_ShowString(3, 10, ":");
          OLED_ShowString(3, 14, ".");
        }
        // ��ʾ�¶ȡ�ʪ�ȡ�Ũ������
        OLED_ShowNum(1,7,temp.Temp_H,2);	 	                  // �¶� ��������
        OLED_ShowNum(1,10,temp.Temp_L,1);		                  // �¶� С������
        OLED_ShowNum(2,7,humi.Humi_H,2);  	                  // ʪ�� ��������
        OLED_ShowNum(2,10,humi.Humi_L,1); 	                  // ʪ�� С������
        OLED_ShowNum(3,11,concentration.Concentration_H,3);   // Ũ�� ��������
        OLED_ShowNum(3,15,concentration.Concentration_L,1);   // Ũ�� С������
      }
      lastAlarmFlag = alarmFlag;	// ���¾�����־
    }
    // ---- �����ж�(������) ----

    // ---- ESP8266��ȡ��ֵ ----
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
      // Serial_Printf("��ȡ��ֵ:\r\n%s\r\n", res_json); // ��ӡ JSON ����

      // ��ȡ�¶�, ʪ��, PM2.5Ũ��ֵ
      char* name = "temperature";
      Utils_GetInformationOfThreshold_8(res, name, &temp.Temp_H_Threshold, &temp.Temp_L_Threshold);
      name = "humidity";
      Utils_GetInformationOfThreshold_8(res, name, &humi.Humi_H_Threshold, &humi.Humi_L_Threshold);
      name = "concentration";
      Utils_GetInformationOfThreshold_16(res, name, &concentration.Concentration_H_Threshold, &concentration.Concentration_L_Threshold);

      Serial_Printf("temp*:%d.%d\r\n", temp.Temp_H_Threshold, temp.Temp_L_Threshold);
      Serial_Printf("humi*:%d.%d\r\n", humi.Humi_H_Threshold, humi.Humi_L_Threshold);
      Serial_Printf("concentration*:%d.%d\r\n", concentration.Concentration_H_Threshold, concentration.Concentration_L_Threshold);

      // ���±�־λ
      ESP8266GetThresholdSuccess = 1;
      ESP8266GetThreshold = 0;
    }
    // ---- ESP8266��ȡ��ֵ ---

    // ---- ESP8266��ȡ���� ----
    if(ESP8266GetData)
    {
      char* res = ESP8266_HTTP_Get("/thcp?access=9mzdDx3K3pgmGdkTcq2ONsUKMp3VSTkIxoUwnHk7bljH4GrkYSeJXFtBp8HYN4f0DJAYPa", 100);
      // Serial_Printf("��ȡ����:\r\n%s\r\n", res);
      char* res_end;
      do
      {
        res = ESP8266_GetBuffer();
        res_end = strstr(res, "}\"}");
        Delay_ms(100);
      }while(res_end == NULL);
      // char* res_json = strchr(res, '{');
      // Serial_Printf("��ȡ����JSON:\r\n%s\r\n", res_json); // ��ӡ JSON ����

      // ��ȡ�¶�, ʪ��, PM2.5Ũ��ֵ
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
    // ---- ESP8266��ȡ���� ----

		// ---- ��ʱ ----
		// Delay_ms(1000);
		// ---- ��ʱ ----
	}
}

// ---- ��ʱ ----

/**
 * @brief  1s�жϺ���������
 */
void Mytiming_callback_1s(void)
{

}

/**
 * @brief  3s�жϺ���������
 */
void Mytiming_callback_3s(void)
{

}

/**
 * @brief  5s�жϺ���������
 */
void Mytiming_callback_5s(void)
{
  ESP8266GetThreshold = 1;
  ESP8266GetData = 1;
}

// ��ʱ�ü�����
uint32_t Mytiming_count_1s = 0;
uint32_t Mytiming_count_3s = 0;
uint32_t Mytiming_count_5s = 0;

/**
 * @brief  TIM2�жϺ���������, 1ms����һ��
 * @param  ��
 * @retval ��
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

      // ����������
      Mytiming_count_1s++;
      Mytiming_count_3s++;
      Mytiming_count_5s++;
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}
// ---- ��ʱ ----
