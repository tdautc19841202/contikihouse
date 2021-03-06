/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "debug.h"
/***********************************/
#include"cc253x.h"

/***********************************/
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"
/***********************************/

#define DEVICE_ID               0x02
/***********************************/
#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
/***********************************/
uint8_t light_state=0;
uip_ipaddr_t server_ipaddr;
static struct uip_udp_conn *client_conn;
static uint8_t buf[5]={0};
static struct uip_udp_conn *server_conn;
extern uint8_t in_net_flag;//if set 1,reter "has join in the net "
uint8_t sended_flag=0;// if set 1 ,refer "has send address to root"
uint8_t temperature_now=0x25;//default 25 centigrede degree
uint8_t oshi_now=0x11;
uint8_t onoff_now=0x00;
static unsigned char cmd[6][9]=
{
 {0xF7,0x7F,0x00,0x09,0x01,0x02,0x00,0x01,0x84},//frozen on  25 centigrade degree
 {0xF7,0x7F,0x00,0x09,0x01,0x02,0x00,0x02,0x85},//off
 {0xF7,0x7F,0x00,0x09,0x01,0x02,0x00,0x03,0x86},//24 centigrade degree
 {0xF7,0x7F,0x00,0x09,0x01,0x02,0x00,0x04,0x87},//23 centigrade degree
 {0xF7,0x7F,0x00,0x09,0x01,0x02,0x00,0x05,0x88},//26 centigrade degree
 {0xF7,0x7F,0x00,0x09,0x01,0x02,0x00,0x06,0x89},//27 centigrade degree
};
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
void InitUart(void)
{ 
    PERCFG = 0x00;           //外设控制寄存器 USART 0的IO位置:0为P0口位置1 
    P0SEL = 0x0c;            //P0_2,P0_3用作串口（外设功能）
    P2DIR &= ~0xC0;          //P0优先作为UART0
    
    U0CSR |= 0x80;           //设置为UART方式
    U0GCR |= 11;				       
    U0BAUD |= 216;           //波特率设为115200
    UTX0IF = 0;              //UART0 TX中断标志初始置位0
    //U0CSR |= 0x40;           //允许接收 
    IEN0 |= 0x80;            //开总中断允许接收中断 
     P1DIR |= 0x01;
       
}
/*---------------------------------------------------------------------------*/
void login(void)
{
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x01);

 
 	buf[0] = 0xEF;// start  of cmd 
	buf[1] = DEVICE_ID;// deviceID
        buf[2] = 0x00;//"0x00" refer " send a address report to root "     
	buf[3] = 0x00;// data length 0
        buf[4] = 0xFE; //end of cmd
uip_udp_packet_sendto(client_conn, buf, 5, &server_ipaddr, UIP_HTONS(5775));

}

/*------------------set_temp when under mode 1--------------------------------*/
void set_temp(unsigned char temp)
{
 uint8_t i,j;
  //if(mode_now==0x11)
{
  switch(temp)
     {
       case 0x23:i=3;break;
       case 0x24:i=2;break;
       case 0x25:i=0;break;
       case 0x26:i=4;break;
       case 0x27:i=5;break;
       default:i=0;
     }
for(j=0;j<9;j++)
{
         U0DBUF =cmd[i][j];
         //__asm nop __endasm;       
        while(UTX0IF == 0);
        UTX0IF = 0;
if(j==22)
{ U0DBUF =cmd[i][j];
        while(UTX0IF == 0);
        UTX0IF = 0;
}
}
temperature_now =temp;
}

}

/*---------------------------------------------------------------------------*/
void turn_off(void)
{
  uint8_t j;

        U0DBUF =cmd[1][0];
        while(UTX0IF == 0);
        UTX0IF = 0;
        U0DBUF =cmd[1][1];
        while(UTX0IF == 0);
        UTX0IF = 0;
        U0DBUF =cmd[1][2];
        while(UTX0IF == 0);
        UTX0IF = 0;
        U0DBUF =cmd[1][3];
        while(UTX0IF == 0);
        UTX0IF = 0;
        U0DBUF =cmd[1][4];
        while(UTX0IF == 0);
        UTX0IF = 0;
        U0DBUF =cmd[1][5];
        while(UTX0IF == 0);
        UTX0IF = 0;        
        U0DBUF =cmd[1][6];
        while(UTX0IF == 0);
        UTX0IF = 0;
        U0DBUF =cmd[1][6];
        while(UTX0IF == 0);
        UTX0IF = 0;
        U0DBUF =cmd[1][7];
        while(UTX0IF == 0);
        UTX0IF = 0;
        U0DBUF =cmd[1][7];
        while(UTX0IF == 0);
        UTX0IF = 0;
        U0DBUF =cmd[1][8];
        while(UTX0IF == 0);
        UTX0IF = 0;

//InitUart();
/*
while(1)
{
        ;
j++;
if(j==9)break;
}
*/
  onoff_now=0x00;
}
/*---------------------------------------------------------------------------*/
void turn_on(void)
{
  uint8_t j;
//InitUart();
 for(j=0;j<9;j++)
{
         U0DBUF =cmd[0][j];
        while(UTX0IF == 0);
        UTX0IF = 0;
if(j==4)
{ U0DBUF =cmd[0][j];
        while(UTX0IF == 0);
        UTX0IF = 0;
}
if(j==6)
{ U0DBUF =cmd[0][j];
        while(UTX0IF == 0);
        UTX0IF = 0;
}
}

 onoff_now=0x01;
}

/*---------------------------------------------------------------------------*/
void DelayMS(uint32_t msec)//not enough 1ms
{  
    uint32_t i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<1070; j++);
}
/*---------------------------------------------------------------------------*/
 void send_state(void)
{
	

  memset(buf,0,sizeof(buf));
    uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x01);

 
 	buf[0] = 0xEF;// start  of cmd 
	buf[1] = DEVICE_ID;// deviceID
        buf[2] = 0xAA;// refer "this is a state report"
        buf[3] =0x03;//data length
	buf[4] = 0x11;//0x01:air conditioner on  and 0x01:off
        buf[5] = onoff_now;//mode of air conditioner 0x11:frozen 0x22:heat 0x33:...
	buf[6] = temperature_now;// configure temperature at present
        buf[7] = 0xFE; //end of cmd
    uip_udp_packet_sendto(client_conn, buf, 8, &server_ipaddr, UIP_HTONS(5775));

}
/*---------------------------------------------------------------------------*/

 void
tcpip_handler(void)
{
  uint8_t * p=uip_appdata;
 
 if(p[0]==0xEF&&p[1]==DEVICE_ID&&p[2]==0xCA&&p[3]==0x00&&p[4]==0xFE)send_state();
else if(p[0]==0xEF&&p[1]==DEVICE_ID&&p[2]==0xC1&&p[3]==0x00&&p[4]==0xFE)turn_on();
else if(p[0]==0xEF&&p[1]==DEVICE_ID&&p[2]==0xC0&&p[3]==0x00&&p[4]==0xFE)turn_off();
else if(p[0]==0xEF&&p[1]==DEVICE_ID&&p[3]==0x00&&p[4]==0xFE)set_temp(p[2]);
else send_state();
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  
     static struct etimer et;
    PROCESS_BEGIN();
   
    //PRINTF("UDP client process started\n\r");
    set_global_address();


    client_conn = udp_new(NULL, UIP_HTONS(5775), NULL);
     /*-------------two conn must seprate--------------------*/      
    udp_bind(client_conn, UIP_HTONS(5885));
    server_conn = udp_new(NULL, UIP_HTONS(0), NULL);  
    udp_bind(server_conn, UIP_HTONS(5885));
    
//CLKCONCMD &= ~0x40;                        //设置系统时钟源为32MHZ晶振
 // while(CLKCONSTA & 0x40);                   //等待晶振稳定为32M
  // CLKCONCMD &= ~0x47;                        //设置系统主时钟频率为32MHZ      
//InitUart();
   
 DelayMS(3000);


etimer_set(&et,10*CLOCK_SECOND);
  while(1) 
 {
   
    PROCESS_YIELD();

if(ev == tcpip_event)
    {
      tcpip_handler();

    }
  else if((sended_flag!=1)&&etimer_expired(&et)) 
{

    if(in_net_flag==1)
  {
     leds_toggle(LEDS_GREEN);
      login();
      sended_flag=1;
   }
  else etimer_set(&et,10*CLOCK_SECOND);

}
 


 }


     PROCESS_END();
}
/*---------------------------------------------------------------------------*/
