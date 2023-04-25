#include<reg51.h>
#include<intrins.h>
 
typedef unsigned char uchar;
typedef unsigned int uint;
 
sbit Bus=P3^0;//数据单总线
sbit RS=P3^3;
sbit RW=P3^4;
sbit E=P3^5;
 
void Delay10us(void);//10us延时函数
void Delay600us(void);//600us延时子函数  
void Delay(uint n);//LCD1602中延时子函数
void Delay1ms(uint t);//t毫秒延时子函数
 
void Init_Ds(void);//DS18B20初始化
void Write_Ds(uchar com);//向DS18B20写入一字节
uchar Read_Ds(void);//从DS18B20读出一字节
uint Get_Tem(void);//获取温度值
 
void Change(uint x);//把整型数值x转换为字符串
void Write_com(uchar com);//写命令子函数
void Write_dat(uchar dat);//写数据子函数
void Init_1602(void);//LCD1602初始化子函数
void Show(uchar x,uchar y,uchar *str);//LCD1602显示子函数
 
 
uchar str[4];//储存转换值对应的字符串
 
void main()
 
{
 
    unsigned int temp;
	Init_1602();
	temp=Get_Tem();
	Change(temp);
	Show(1,1,"T:");
	Show(1,3,str);
	while(1);
 
}
 
/***************************************延时函数体**************************/
void Delay10us(void)//10us延时函数
{
    unsigned char a,b;
    for(b=1;b>0;b--)
        for(a=1;a>0;a--);
}
void Delay600us(void)//600us延时函数
{
    unsigned char a,b;
    for(b=119;b>0;b--)
        for(a=1;a>0;a--);
}
void Delay(uint n)//LCD1602中延时函数                       
{ 
    uint x,y;  
    for(x=n;x>0;x--) 
        for(y=110;y>0;y--); 
}
void Delay1ms(uint t)//t毫秒延时函数
{
    unsigned char a,b;
	uint i;
	for(i=0;i<t;i++)
    	for(b=199;b>0;b--)
        	for(a=1;a>0;a--);
}
 
/********************************DS18B20初始化函数*************************/
void Init_Ds(void)//DS18B20初始化
{
	Bus=0;//主动拉低480-960us（此处选择600us）
	Delay600us();
	Bus=1;//释放总线,传感器15-60us后拉低总线
	while(Bus);//等待传感器拉低；
	while(!Bus);//度过传感器被拉低的时间（60-240us）后主动拉高
	Bus=1;//主动拉高
}
/********************************向DS18B20写入一字节***********************/
void Write_Ds(uchar com)//从低位开始写入
{
	uchar mask;
	for(mask=0x01;mask!=0;mask<<=1)
	{
		//该位为0，先拉低，15us后在拉高，并通过延时使整个周期为60us
	    //该位为1,先拉低并在15us内(此处选择5us)拉高，并通过延时使整个周期为60us	
		Bus=0;
		_nop_();_nop_();_nop_();_nop_();_nop_();//先拉低5us
		if((com&mask)==0)//该位是0
		{
			Bus=0;	
		}
		else//该位是1
		{
			Bus=1;		
		}
		Delay10us();Delay10us();Delay10us();Delay10us();Delay10us();;//延时60us
		_nop_();_nop_();_nop_();_nop_();_nop_();
		Bus=1;//拉高
		_nop_();_nop_();//写两个位之间至少有1us的间隔（此处选择2us）
	}
}
/********************************从DS18B20读出一字节***********************/
uchar Read_Ds(void)//先读的是低位，整个读周期至少为60us，但控制器采样要在15us内完成，相邻“位”之间至少间隔1us
{
	uchar value=0,mask;
	for(mask=0x01;mask!=0;mask<<=1)
	{
		Bus=0;//先把总线拉低超过1us（此处选择2us）后释放
		_nop_();_nop_();
		Bus=1;
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();//再延时6us后读总线数据
		if(Bus==0)//如果该位是0
		{
			value&=(~mask);
		}
		else
		{
			value|=mask;
		}
		Delay10us();Delay10us();Delay10us();Delay10us();Delay10us();//再延时52us，凑够至少60us的采样周期
		_nop_();_nop_();
		Bus=1;
		_nop_();_nop_();//写两个位之间至少有1us的间隔（此处选择2us）
	}
	return value;	
}
/**********************************获取温度值函数***************************/
uint Get_Tem(void)
{
	uint temp=0;
	float tp;
	uchar LSB=0,MSB=0;
	Delay1ms(10);//延时10ms度过不稳定期
 
	Init_Ds();//Ds18b20初始化
	Delay1ms(1);
	Write_Ds(0xcc);//跳过ROM寻址
	Write_Ds(0x44);//启动一次温度转换
	Delay1ms(1000);//延时1s等待转化
 
	Init_Ds();//Ds18b20初始化
	Delay1ms(1);
	Write_Ds(0xcc);//跳过ROM寻址
	Write_Ds(0xbe);//发送读值命令·
	LSB=Read_Ds();
	MSB=Read_Ds();
	temp=MSB;
	temp<<=8;
	temp|=LSB;
	tp=temp*0.0625;
	temp=tp;
	if(tp-temp>=0.5)
	{
		temp+=1;
	}
	return temp;
}
/******************************把整型数据转换为字符串**********************/
void Change(uint x)
{
	str[0]=x/100+48;
	str[1]=(x/10)%10+48;
	str[2]=x%10+48;
	str[3]='\0';
}
 
 
/********************************写命令函数体****************************/
void Write_com(uchar com)
{
	RS=0;
	P2=com;
	Delay(5);
	E=1;
	Delay(5);
	E=0;
}
/********************************写数据函数体****************************/
void Write_dat(uchar dat)
{
	RS=1;
	P2=dat;
	Delay(5);	 
	E=1;
	Delay(5);
	E=0;
}
/*****************************LCD1602初始化函数体*************************/
void Init_1602()
{
	uchar i=0;
	RW=0;
	Write_com(0x38);//屏幕初始化
	Write_com(0x0c);//打开显示 无光标 无光标闪烁
	Write_com(0x06);//当读或写一个字符是指针后一一位
	Write_com(0x01);//清屏
	Write_com(0x80);//设置位置
}
/*******************************显示内容函数体**************************/
void Show(uchar x,uchar y,uchar *str)
{
	unsigned char addr;
  	if (x==1)
	{
		addr=0x00+y-1; //从第一行、第y列开始显示
	}
	else
	{
		addr=0x40+y-1; //第二行、第y列开始显示
	}					
	Write_com(addr+0x80);
	while (*str!='\0')
	{
		Write_dat(*str++);
	}
 
}
 
 
 