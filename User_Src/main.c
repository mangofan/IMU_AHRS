/* main.c file
��д�ߣ�lisn3188
��ַ��www.chiplab7.com
����E-mail��lisn3188@163.com
���뻷����MDK-Lite  Version: 4.23
����ʱ��: 2012-05-05
���ԣ� ���������ڵ���ʵ���ҵ�mini IMU����ɲ���
Mini IMU AHRS ģ��ٷ����۵�ַ��Http://chiplab7.taobao.com
���ܣ�
1.��ʼ��������������
2.������̬����͸߶Ȳ���
3.���������̬�͸���������������ϴ��� MiniIMU AHRS �������
4.��Ӧ PC���͵�����
------------------------------------
*/

#include "common.h"  //�������е����� ͷ�ļ�

//�ϴ����ݵ�״̬��
#define REIMU  0x01 //�ϴ��������̬����
#define REMOV  0x02	//�ϴ������������
#define REHMC  0x03	//�ϴ������Ƶı궨ֵ

#define Upload_Speed  100   //�����ϴ��ٶ�  ��λ Hz
#define upload_time (1000000/Upload_Speed)/2  //�����ϴ���ʱ�䡣��λΪus

int16_t ax, ay, az;	
int16_t gx, gy, gz;
int16_t hx, hy, hz;
int32_t Temperature = 0, Pressure = 0, Altitude = 0;
uint32_t system_micrsecond;
int16_t hmcvalue[3];
u8 state= REIMU;  //�����ض�֡ ��״̬��
///////
//float ypr[3]; // yaw pitch roll
/**************************ʵ�ֺ���********************************************
*����ԭ��:		int main(void)
*��������:		������
*******************************************************************************/
int main(void)
{
	int16_t Math_hz=0;
	unsigned char PC_comm; //PC ����ؼ��ֽ�	 
	float ypr[3]; // yaw pitch roll
	/* ����ϵͳʱ��Ϊ72M ʹ���ⲿ8M����+PLL*/      
    //SystemInit();
	delay_init(72);		//��ʱ��ʼ��
    Initial_LED_GPIO();	//��ʼ��STM32-SDK�����ϵ�LED�ӿ�
	Initial_PWMLED();
	Initial_UART1(115200L);
	Initial_UART2(115200L);
	load_config();  //��flash�ж�ȡ������Ϣ -->eeprom.c
	IIC_Init();	 //��ʼ��I2C�ӿ�
	delay_ms(300);	//�ȴ������ϵ�
	//UART1_Put_String("Initialize...\r\n");
	IMU_init(); //��ʼ��IMU�ʹ�����
	system_micrsecond=micros();
	while(1){	//��ѭ��
		
	//delay_ms(1); //��ʱ����Ҫ����ô�졣
	IMU_getYawPitchRoll(ypr); //��̬����
	Math_hz++; //������� ++
	BMP180_Routing(); //����BMP018 ���� ����ת���Ͷ�ȡ�����������ӳ����н��� 

//-------------��λ��------------------------------
	//�Ƿ��˸��� ��λ����ʱ���ˣ�
	if((micros()-system_micrsecond)>upload_time){
	switch(state){ 
	case REIMU:
	BMP180_getTemperat(&Temperature); //��ȡ������¶�ֵ
	BMP180_getPress(&Pressure);	   //��ȡ�������ѹ����ֵ
	BMP180_getAlt(&Altitude);	   //��ȡ��Ը߶�
	UART1_ReportIMU((int16_t)(ypr[0]*10.0),(int16_t)(ypr[1]*10.0),
	(int16_t)(ypr[2]*10.0),Altitude/10,Temperature,Pressure/10,Math_hz*16);
	UART2_ReportIMU((int16_t)(ypr[0]*10.0),(int16_t)(ypr[1]*10.0),
	(int16_t)(ypr[2]*10.0),Altitude/10,Temperature,Pressure/10,Math_hz*Upload_Speed);
	Math_hz=0;
	state = REMOV; //����״̬��
	break;
	case REMOV:
	MPU6050_getlastMotion6(&ax, &ay, &az, &gx, &gy, &gz);
	HMC58X3_getlastValues(&hx,&hy,&hz);
	UART1_ReportMotion(ax,ay,az,gx,gy,gz,hx,hy,hz);
	UART2_ReportMotion(ax,ay,az,gx,gy,gz,hx,hy,hz);
	state = REIMU;
	if(HMC5883_calib)state = REHMC; //��Ҫ���͵�ǰ�����Ʊ궨ֵ
	break;
	default: 
	UART2_ReportHMC(HMC5883_maxx,HMC5883_maxy,HMC5883_maxz,
		 HMC5883_minx,HMC5883_miny,HMC5883_minz,0);//���ͱ궨ֵ
	state = REIMU;
	break;
	}//switch(state) 			 
	system_micrsecond=micros();	 //ȡϵͳʱ�� ��λ us 
	LED_Change();	//LED1�ı�����
	}
//--------------------------------------------------
	//����PC������������
	if((PC_comm=UART2_CommandRoute())!=0xff)
	{
	switch(PC_comm){ //��������ʶ
	case Gyro_init:			MPU6050_InitGyro_Offset(); break; //��ȡ��������ƫ
	case High_init:			BMP180_ResetAlt(0); 	break;		//��ѹ�߶� ����
	case HMC_calib_begin:	HMC5883L_Start_Calib();	break; //��ʼ�����Ʊ궨
	case HMC_calib:		HMC5883L_Save_Calib();	break;   //��������Ʊ궨
	}
	}// ����PC ���͵�����

	}//��ѭ�� while(1) ����

}  //main	

//------------------End of File----------------------------
