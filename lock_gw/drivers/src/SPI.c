#include "stm32f10x_spi.h"
#include "spi.h"

void SPI1_Init(void){
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

		RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1 , ENABLE );
	
    //GPIO_PinRemapConfig(GPIO_Remap_SPI1,ENABLE);
	
    GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7 ;	  
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    SPI_InitStructure.SPI_Direction 				= SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��  SPI_Direction_1Line_Tx
    SPI_InitStructure.SPI_Mode 							= SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize 					= SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL 							= SPI_CPOL_Low;		//ѡ���˴���ʱ�ӵ���̬:ʱ������low
    SPI_InitStructure.SPI_CPHA 							= SPI_CPHA_1Edge;	//���ݲ����ڵ�1��ʱ����
    SPI_InitStructure.SPI_NSS 							= SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ�����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ16, 128
    SPI_InitStructure.SPI_FirstBit 					= SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial		  = 7;	//CRCֵ����Ķ���ʽ
    SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

    SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����
}

//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   (SPI 36M@sys 72M)
//SPI_BaudRatePrescaler_8   8��Ƶ   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_16  16��Ƶ  (SPI 4.5M@sys 72M)
//SPI_BaudRatePrescaler_256 256��Ƶ (SPI 281.25K@sys 72M)
void SPI1_SetSpeed(u8 SpeedSet){
    SPI1->CR1 &= 0XFFC7;
    SPI1->CR1 |= SpeedSet;
    SPI_Cmd(SPI1, ENABLE);
}

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI1_ReadWriteByte(u8 TxData){
    u8 retry;
		
		retry = 0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
    {
        retry++;
        if (retry>200)return 0;
    }
    SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ������
    
		retry=0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
    {
        retry++;
        if (retry>200)return 0;
    }
    return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����
}