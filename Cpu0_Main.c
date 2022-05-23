/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2020,��ɿƼ�
 * All rights reserved.
 * ��������QQȺ����Ⱥ��824575535
 *
 * �����������ݰ�Ȩ������ɿƼ����У�δ��������������ҵ��;��
 * ��ӭ��λʹ�ò������������޸�����ʱ���뱣����ɿƼ��İ�Ȩ������
 *
 * @file            main
 * @company         �ɶ���ɿƼ����޹�˾
 * @author          ��ɿƼ�(QQ3184284598)
 * @version         �鿴doc��version�ļ� �汾˵��
 * @Software        ADS v1.2.2
 * @Target core     TC264D
 * @Taobao          https://seekfree.taobao.com/
 * @date            2020-3-23
 ********************************************************************************************************************/


#include "headfile.h"
#pragma section all "cpu0_dsram"
//���������#pragma section all restore���֮���ȫ�ֱ���������CPU0��RAM��


uint8 otsuThreshold(uint8 *image, uint16 col, uint16 row)
{
    #define GrayScale 256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j, pixelSum = width * height;
    uint8 threshold = 0;
    uint8* data = image;  //ָ���������ݵ�ָ��
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            pixelCount[(int)data[i * width + j]]++;  //������ֵ��Ϊ����������±�
        }
    }

    //����ÿ������������ͼ���еı���
    float maxPro = 0.0;
    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
        if (pixelPro[i] > maxPro)
        {
            maxPro = pixelPro[i];
        }
    }

    //�����Ҷȼ�[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
    for (i = 0; i < GrayScale; i++)     // i��Ϊ��ֵ
    {
        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
        for (j = 0; j < GrayScale; j++)
        {
            if (j <= i)   //��������
            {
                w0 += pixelPro[j];
                u0tmp += j * pixelPro[j];
            }
            else   //ǰ������
            {
                w1 += pixelPro[j];
                u1tmp += j * pixelPro[j];
            }
        }
        u0 = u0tmp / w0;
        u1 = u1tmp / w1;
        u = u0tmp + u1tmp;
        deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
        if (deltaTmp > deltaMax)
        {
            deltaMax = deltaTmp;
            threshold = (uint8)i;
        }
    }

    return threshold;
}


uint8 image_threshold;  //ͼ����ֵ
uint32 use_time;

//���̵��뵽���֮��Ӧ��ѡ�й���Ȼ����refreshˢ��һ��֮���ٱ���
//����Ĭ������Ϊ�ر��Ż��������Լ��һ�����ѡ��properties->C/C++ Build->Setting
//Ȼ�����Ҳ�Ĵ������ҵ�C/C++ Compiler->Optimization->Optimization level�������Ż��ȼ�
//һ��Ĭ���½����Ĺ��̶���Ĭ�Ͽ�2���Ż�����˴��Ҳ��������Ϊ2���Ż�

//����TCϵ��Ĭ���ǲ�֧���ж�Ƕ�׵ģ�ϣ��֧���ж�Ƕ����Ҫ���ж���ʹ��enableInterrupts();�������ж�Ƕ��
//�򵥵�˵ʵ���Ͻ����жϺ�TCϵ�е�Ӳ���Զ�������disableInterrupts();���ܾ���Ӧ�κε��жϣ������Ҫ�����Լ��ֶ�����enableInterrupts();�������жϵ���Ӧ��
int core0_main(void)
{
    int i,j,k;
    int temp;
    uint8 data;
    uint8 *p;

    get_clk();//��ȡʱ��Ƶ��  ��ر���

    //Ŀǰ�Ŀ�ɼ������ ͼ�����һ��Ϊ�̶��ĺ�ɫ
    //�������ڵ�Ƭ����ɵģ���������ͷ������

    ips200_init();  //��ʼ��IPS��Ļ
    ips200_showstr(0, 0, "SEEKFREE MT9V03x");
    ips200_showstr(0, 1, "Initializing...");

    uart_init(UART_0, 115200, UART0_TX_P14_0, UART0_RX_P14_1);
    mt9v03x_init(); //��ʼ������ͷ
    //�����Ļһֱ��ʾ��ʼ����Ϣ����������ͷ����
    //���ʹ�����壬һֱ����while(!uart_receive_flag)�������Ƿ�������OK?��������ͷ�����ô����뵥Ƭ�������Ƿ���ȷ
    //���ͼ��ֻ�ɼ�һ�Σ����鳡�ź�(VSY)�Ƿ�����OK?
	IfxCpu_emitEvent(&g_cpuSyncEvent);
	IfxCpu_waitEvent(&g_cpuSyncEvent, 0xFFFF);
    enableInterrupts();

    //ע�� ��V1.1.6�汾֮��  printf��ӡ����Ϣ�Ӵ�������������ѧϰ������6-Printf_Demo
    //ע�� ��V1.1.6�汾֮��  printf��ӡ����Ϣ�Ӵ�������������ѧϰ������6-Printf_Demo
    //ע�� ��V1.1.6�汾֮��  printf��ӡ����Ϣ�Ӵ�������������ѧϰ������6-Printf_Demo
    while (TRUE)
    {
        if(mt9v03x_finish_flag)
        {
            systick_start(STM1);
            //image_threshold = otsuThreshold(mt9v03x_image[0],MT9V03X_W,MT9V03X_H);  //��򷨼�����ֵ
            //use_time = systick_getval_us(STM1);                        //�����򷨳�������ʱ�䣬��λ΢�롣
            //printf("use_time: %ld\n", use_time);
            image_threshold = 140;

            //systick_delay_ms(STM0, 30);

            //���Ͷ�ֵ��ͼ������λ��
            p = mt9v03x_image[0];
            //uart_putchar(UART_0,0x0f);
//            for(i=0; i<MT9V03X_W*MT9V03X_H; i++)
//            {
//                 if(p[i]>image_threshold)    uart_putchar(UART_0,0xff);
//                 else                        uart_putchar(UART_0,0x00);
//            }
            temp=0;
            //p[0]=255;
            uart_putchar(UART_0,'m');
            for(i=0; i<MT9V03X_H*MT9V03X_W/8; i++)
            {

                data=0x00;
                for(k=0;k<8;k++){
               if(p[temp++]>image_threshold){
                   data |= 1 << (7-k);}
               }
                uart_putchar(UART_0,data);




//                 if(p[i]>image_threshold)    uart_putchar(UART_0,0x00);
//
//                 else                        uart_putchar(UART_0,0x00);
            }


            //uart_putchar(UART_0,0x02);
            ips200_displayimage032(mt9v03x_image[0], MT9V03X_W, MT9V03X_H);
            //seekfree_sendimg_03x(UART_0, mt9v03x_image[0], sMT9V03X_W, MT9V03X_H);

            mt9v03x_finish_flag = 0;//��ͼ��ʹ����Ϻ�  ��������־λ�����򲻻Ὺʼ�ɼ���һ��ͼ��
            //ע�⣺һ��Ҫ��ͼ��ʹ����Ϻ�������˱�־λ

        }

    }
}

#pragma section all restore


