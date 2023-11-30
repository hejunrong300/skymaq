/*
 * prb0400Lib.h
 *
 *  Created on: 2022年11月15日
 *      Author: PC
 */

#ifndef PRB0400LIB_H_
#define PRB0400LIB_H_
#include "types.h"
#include <stdio.h>
#include <string.h>

// 当前驱动支持的最大端口数目
#define RIO_MAX_MPORTS 2

// Inbound门铃接收处理回调函数指针原型声明
typedef void (*doorbellRecvProc)(u8 portId, u16 srcRioID, u16 dbellInfo);
// Inbound消息接收处理回调函数指针原型声明
typedef void (*msgRecvProc)(u32 *pData);
typedef void (*msgAllRecvProc)(void *msgInfo);
// system tick
#define SYS_CLOCK_HZ 48000000 // 48Mhz

// 中断计数
extern u32 prb0400_intr_dev_int_0_num;
extern u32 prb0400_intr_num;

// 由于消息队列满导致的Inbound门铃事务接收丢失总计数
extern u32 IBDoorbellLostNum;
// 由于消息队列满导致的Inbound消息事务接收丢失总计数
extern u32 IBMSGLostNum;

// OB消息等待信号量超时时间,默认5000ms
extern u32 ob_msg_waite_time_out;

// OB消息阻塞方式传输使能，默认1，即使用阻塞方式发送消息
// 配置为1时，驱动中消息发送采用阻塞方式实现，驱动每次发送完毕一次message报文会等待中断传递的信号量，并且根据中断状态寄存器的值判定这次消息发送是否成功
// 配置为0时，驱动中消息发送采用非阻塞方式实现，循环队列深度256。若发送请求太多导致中断处理不及时，将待发送的循环队列填满时会返回-16设备忙错误码
extern u8 ob_msg_block_enable;

// 调试信息等级配置，默认0，关闭全部调试信息
// 其中bit0-bit11每位代表相应的模块调试信息使能开关，具体含义如下
extern u32 prb0400_dbg_level; // Debugging output level (default: 0 = none)

/* Debug output filtering masks */
#define BIT(nr) ((1) << (nr))
enum
{
	DBG_NONE = 0,
	DBG_INIT = BIT(0),	  /* driver init */
	DBG_EXIT = BIT(1),	  /* driver exit */
	DBG_MPORT = BIT(2),	  /* mport add/remove */
	DBG_DMA = BIT(3),	  /* DMA transfer messages */
	DBG_DMAV = BIT(4),	  /* verbose DMA transfer messages */
	DBG_IBW = BIT(5),	  /* inbound window */
	DBG_EVENT = BIT(6),	  /* event handling messages */
	DBG_OBW = BIT(7),	  /* outbound window messages */
	DBG_DBELL = BIT(8),	  /* inbound doorbell messages */
	DBG_OMSG = BIT(9),	  /* outbound MBOX messages */
	DBG_IMSG = BIT(10),	  /* inbound MBOX messages */
	DBG_ODBELL = BIT(11), /* outbound doorbell messages */
	DBG_ALL = ~0,
};

// RIO mport device attributes struct
struct rio_mport_attr
{
	int link_speed; // SRIO link speed value (as defined by RapidIO specification)
	int link_width; // link_width: SRIO link width value (as defined by RapidIO specification)
};

// srio controler configuration
struct prb0400_srio_config
{
	// global
	u8 srio_port_num; // curren pcie device on this system, maximum 2, default 1
	int pcie_mrrs; // PCIe MRRS override value, -1 = keep platform setting 0 = 128B, 1 = 256B, 2 = 512B, 3 = 1024B, 4 =
				   // 2048B 5 = 4096B(default)
	u8 sys_size;   // RapidIO common transport system size. 0=Small size, 256 devices 1=Large size, 65536 devices
				 // (default: 0 = small size)
	u8 intr_mode; // PCIE interrupt mode control. 1=MSI 2=INTx(default: 1 = use MSI mode), note only support INTA&MSI
				  // interrupt mode
	// perport
	u8 mapping_wr_type[RIO_MAX_MPORTS]; // Mapping Engine write type. 1=NWRITE or SWRITE 4=NWRITE_R 8=NWRITE (default: 1
										// = NWRITE or SWRITE)
	u8 speed_sel[RIO_MAX_MPORTS]; // SRIO link speed configuration. 1=1.25Gbps 2=2.5Gbps 3=3.125Gb 4=5.0Gbps 0xff=use
								  // hardware configuration (default: 0xff = use hardware configuration)
	u8 port_width_ovrd[RIO_MAX_MPORTS]; // SRIO port width overide. 0=no override 1=1xlane0 2=1xlaneR 3=2x 4=4x
										// (default: 0 = no override)
	u8 idle_sel[RIO_MAX_MPORTS];		// RIO IDLE sequence selection. 1=IDLE1 2=IDLE2 (default: 1 = IDLE1)
	u8 dlb_en[RIO_MAX_MPORTS]; // Digital equipment loopback mode control. 0=don't enable DLB 1=enable DLB (default: 0 =
							   // Don't enable DLB)
};

extern struct prb0400_srio_config prb0400SrioCfg;

// PCIE slot position
typedef struct
{
	u32 pcieCapExp; // PCIe capability structure start offset
	u32 busNo;
	u32 deviceN0;
	u32 funcNo;
} pcieChipLocation_s;

// IB message data structure
struct PRB0400MsgRcv
{
	u8 portid;
	u16 srcID;
	u8 mbox;
	u8 letter;
	u16 lengh; // Bytes unit
	u64 addr;  // the received IBMSG buffer address
};
/*************************************************
  函数标题: RIO接口初始化
  函数名:    rioInit
  调用函数:
  输入参数:  无
  输出参数:  无
  返回值:    0成功,非0失败
  函数说明:  无
  注意项:   无
*************************************************/
int rioInit(void);

/*************************************************
  函数标题: 注册port-write接收回调函数
  函数名:   rioPwIsrRegister
  调用函数:
  输入参数: rioPwIsr: port-write接收回调函数,函数原型为void (*rioPwIsr)(char reg[16])

  输出参数:
  返回值:    0:成功， -22：失败，非法参数
  函数说明:  无
  注意项:
注册的portwrite回调函数直接在中断服务程序中同步调用，因此要求该函数不能有睡眠等长时间占用程序执行的实现，应当尽可能快速返回
							 如果多次注册回调函数，则驱动调用最后一次注册的回调函数。
*************************************************/
int rioPwIsrRegister(void (*rioPwIsr)(char reg[16]));

/*************************************************
  函数标题: RIO门铃发送功能
  函数名:   rioDbellSend
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口
		u16 destID:目标RapidIO 地址
		u16 dbellInfo:门铃信息
  输出参数: 无
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -5   IO错误
			 -11  重传达到最大值（100次）错误
			 -22  非法参数
			 -79  超时
			 -143 端口未初始化
  函数说明: 门铃发送接口在驱动里实现了软件重传，若收到的门铃响应包为重传响应则启动软件重传，最大重传次数100次
  注意项:   无
*************************************************/
int rioDbellSend(u8 portId, u16 destID, u16 dbellInfo);

/*************************************************
  函数标题: 注册门铃接收回调函数
  函数名:   setDoorbellRecvCallBack
  调用函数:
  输入参数: dbellFunc: 门铃回调函数,函数原型为typedef void (*doorbellRecvProc )(u8 portId , u16 srcRioID, u16 dbellInfo)

  输出参数: 无
  返回值:   0:成功，-22：失败，非法参数
  函数说明: 无
  注意项:  如果多次注册回调函数，则驱动调用最后一次注册的回调函数。
*************************************************/
int setDoorbellRecvCallBack(doorbellRecvProc dbellFunc);

/*************************************************
  函数标题: 打开port-write包接收
  函数名:   openPW
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口

  输出参数: 无
  返回值:   0成功,非0失败
			返回值错误号列表：
			-22  非法参数
			-143 端口未初始化

  函数说明: 无
  注意项:   无
*************************************************/
int openPW(u8 portId);

/*************************************************
  函数标题: 关闭port-write包接收
  函数名:   closePW
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口

  输出参数: 无
  返回值:   0成功,非0失败
			返回值错误号列表：
			-22  非法参数
			-143 端口未初始化

  函数说明: 无
  注意项:   无
*************************************************/
int closePW(u8 portId);

/**************************************************************
  函数标题: 	维护读操作
  函数名:		rioMaintRead
  输入参数:
			u8 portNum   设备使用的物理端口
			u32 destid, 远端rioid ，如果是读取本地，此参数不做判断
			u16 hopCount,   跳数,等于0xFFFF时为写本地节点，其它为写远端节点
			u32 offset ,    偏移量
  输出参数:	 u32 *val, 将读取的4字节数据填充到val中
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -5   IO错误
			 -22  非法参数
			 -143 端口未初始化
  说明: 使用BDMA通道7发送维护请求
  注意项:  维护操作使用最高优先级prio：2 crf：1进行通信
***************************************************************/
int rioMaintRead(u8 portNum, u16 destid, u16 hopcount, u32 offset, u32 *val);

/**************************************************************
  函数标题: 	远端维护写操作
  函数名:		rioMaintWrite
  输入参数:
			u8 portNum   设备使用的物理端口
			u16 destid, 目标RapidIO 地址
			u16 hopCount,   跳数,等于0xFFFF时为写本地节点，其它为写远端节点
			u32 offset ,    偏移量
			u32 val,     写入的寄存器值
  输出参数:	 无
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -5   IO错误
			 -22  非法参数
			 -143 端口未初始化
 说明: 使用BDMA通道7发送维护请求
  注意项:  维护操作使用最高优先级prio：2 crf：1进行通信
***************************************************************/
int rioMaintWrite(u8 portNum, u16 destid, u16 hopcount, u32 offset, u32 val);

/*************************************************
  函数标题: RIO消息机制发送功能
  函数名:   rioMsgSend
	调用函数:
	输入参数:   u8 portId: 设备使用的硬件端口
		  u16 destRioID: 目标RapidIO 地址
		  char *pData: 发送数据的指针
		  u32 nDataLen: 发送数据长度，长度最大为4KB，最小8B，若非8字节对齐，按向上取整8字节对齐发送
  输出参数:    无
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -5 IO错误
			 -16  设备忙
			 -22  非法参数
			 -143 端口未初始化
			  其他：ACoreOs_status_code枚举类型，为ACoreOs_semaphore_obtain()函数返回值，典型值3
超时（只有阻塞模式会上报） 函数说明: 发送消息的mbox为0，letter为0，优先级为0 注意项:
消息发送模式受ob_msg_block_enable参数控制，默认1，即使用阻塞方式发送消息。驱动自动选择OBMSG0-7即8个发送通道中状态为空闲的1个进行消息发送，如果当前请求无空闲通道则返回-16错误码。
						  配置为1时，驱动中消息发送采用阻塞方式实现，驱动每次发送完毕一次message报文会等待中断传递的信号量，并且根据中断状态寄存器的值判定这次消息发送是否成功。
						  配置为0时，驱动中消息发送采用非阻塞方式实现，循环队列深度256。若发送请求太多导致中断处理不及时，将待发送的循环队列填满时会返回-16设备忙错误码
。
*************************************************/
int rioMsgSend(u8 portId, u16 destRioID, char *pData, u32 nDataLen);

/*************************************************
  函数标题: 注册消息接收回调函数
  函数名:   rioMsgRecvCallBackReg
  调用函数:
  输入参数: callbackFunc 回调函数指针，函数原型为typedef void (*msgRecvProc)(unsigned u32 *pData)

  输出参数: 无
  返回值:   0:成功，-22：失败，非法参数
  函数说明: 注册RIO消息接收回调函数，驱动接收到RIO消息后，调用回调函数将数据推送给上层应用软件
  注意项:  如果多次注册回调函数，则驱动调用最后一次注册的回调函数。
*************************************************/
int rioMsgRecvCallBackReg(msgRecvProc callbackFunc);

/*************************************************
  函数标题: 注册消息接收回调函数
  函数名:   rioMsgAllRecvCallBackReg
  调用函数:
  输入参数: callbackFunc 回调函数指针，函数原型为typedef void (*msgAllRecvProc)(void *msgInfo)
		其中msgInfo为指向接收到的消息的信息的结构体，原型如下
		struct PRB0400MsgRcv
		{
			u8 portid;
			u16 srcID;
			u8 mbox;
			u8 letter;
			u16 lengh;	 //Bytes unit
			u64 addr;	//the received IBMSG buffer address
		};
  输出参数: 无
  返回值:   0:成功，-22：失败，非法参数
  函数说明: 注册RIO消息接收回调函数，驱动接收到RIO消息后，调用回调函数将数据推送给上层应用软件
  注意项:  如果多次注册回调函数，则驱动调用最后一次注册的回调函数。
*************************************************/
int rioMsgAllRecvCallBackReg(msgAllRecvProc ibmsgFunc);

/*************************************************
  函数标题: RIO内存映射机制发送功能
  函数名:   rioNwrite
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口
		u16 destID:目标RapidIO 地址
		u32 rioRegionStart：RapidId 空间起始地址
		void *pBlkBuff：待发数数据的指针
		u32 length：发送数据长度
		BOOL bNeedRsp：送数据是否采用确认方式发送,为TRUE时需要确认发送（ALL NWRITE_R）,反之不需要（ALL NWRITE）
  输出参数: 无

  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -5   IO错误
			 -16  设备忙
			 -22  非法参数
			 -143 端口未初始化
  函数说明: 驱动自动选择BDMA0-6即7个发送通道中状态为空闲的1个进行数据搬移，如果当前请求无空闲通道则返回-16错误码，
			上层软件检查到设备忙后需要重传这次请求
  注意项:
该函数利用桥片内部BDMA实现数据传输，对于tsi721只支持该参数为TRUE配置（器件功能限制）；对于PRB0400支持该参数配置为TRUE或FALSE
*************************************************/
int rioNwrite(u8 portId, u16 destID, u32 rioRegionStart, void *pBlkBuff, u32 length, BOOL bNeedRsp);

/*************************************************
  函数标题: RIO内存映射机制接收功能
  函数名:   rioNread
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口
		u16 destID:目标RapidIO 地址
		u32 rioRegionStart：RapidId 空间起始地址
		void *pBlkBuff：接收到的数据指针，nread请求的数据结果放入该起始地址中
		u32 length：发送数据长度
  输出参数: 无

  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -5   IO错误
			 -16  设备忙
			 -22  非法参数
			 -143 端口未初始化
  函数说明: 驱动自动选择BDMA0-6即7个发送通道中状态为空闲的1个进行数据搬移，如果当前请求无空闲通道则返回-16错误码，
			上层软件检查到设备忙后需要重传这次请求
  注意项:  该函数桥片内部BDMA实现数据传输
*************************************************/
int rioNread(u8 portId, u16 destID, u32 rioRegionStart, void *pBlkBuff, u32 length);

/*************************************************
  函数标题: RIO内存映射功能
  函数名:   rioMemInbMap
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口
		u32 localRegionStart:本地映射起始地址
		u32 rioRegionStart：  srio总线起始地址
		u32 regionLen：映射空间大小

  输出参数:  无
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -5   IO错误
			 -12  内存不够
			 -14  错误地址
			 -16  设备忙
			 -22  非法参数
			 -143 端口未初始化
  函数说明:  无
  注意项:  regionLen为2的幂次方，最小0x1000；localRegionStart与rioRegionStart需要与regionLen地址对齐
						 最大支持同时开8个Inbound映射窗口，注意开多窗口时各窗对应的SRIO总线地址不能重叠
*************************************************/
int rioMemInbMap(u8 portId, u32 localRegionStart, u32 rioRegionStart, u32 regionLen);

/*************************************************
  函数标题: 获取本节点分配的RIO ID
  函数名:   getRioDeviceId
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口

  输出参数: 无
  返回值:   本节点分配的RIO ID；如果遇到错误返回固定值0x5A5A
  函数说明: 无
  注意项:   无
*************************************************/
u16 getRioDeviceId(u8 portId);

/*************************************************
  函数标题: 关闭或打开RIO端口开关
  函数名:   rioPortSwitch
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口
		u32 isEnable, 使能标志 0—不使能；1—使能

  输出参数: 无
  返回值:   0成功,非0失败
			返回值错误号列表：
			-22  非法参数
			-143 端口未初始化

  函数说明: 打开RIO端口才可建链发包；关闭SRIO端口后SRIO不建链，设备不响应对端发出的任何请求包
  注意项:  在调用SRIO初始化接口prb0400_rioInit()时会关闭SRIO端口，需要由应用程序在发包之前调用该接口以打开SRIO端口。
*************************************************/
int rioPortSwitch(u8 portId, u32 isEnable);

/*************************************************
  函数标题:  SRIO端口初始化参数配置
  函数名:   prb0400_srioCfg
  调用函数:
  输入参数: struct prb0400_srio_config cfg: SRIO端口初始化配置结构体

  输出参数: 无
  返回值:   0成功,非0失败
			返回值错误号列表：
			-22  非法参数

  函数说明:
  注意项: 详见struct prb0400_srio_config类型，使用时需要注意参数是否区分port，
   默认配置如下：  1,5,0,1, {1,1},{0xff,0xff},{0,0},{1,1},{0,0}
*************************************************/
int prb0400_srioCfg(struct prb0400_srio_config cfg);

/*************************************************
  函数标题:  查询SRIO端口属性，获取当前建链状态以及建链速率的值
  函数名:   prb0400_query_mport
  调用函数:
  输入参数: u8 portid: 设备使用的硬件端口
  输出参数: struct rio_mport_attr *attr: 当前SRIO端口属性
  返回值:   0成功,非0失败
			返回值错误号列表：
			-22  非法参数
			-143 端口未初始化
  函数说明:
  注意项:
*************************************************/
int prb0400_query_mport(u8 portid, struct rio_mport_attr *attr);

/*************************************************
  函数标题:  Outbound窗口地址映射
  函数名:   rioMemOutbMap
  调用函数:
  输入参数:  u8 portId: 设备使用的硬件端口
		 u16 destid:本地映射起始地址
		 u32 rioRegionStart：srio总线起始地址
		 u32 regionLen：映射空间大小

  输出参数:  u64 *localRegionStart：  本地映射空间起始地址
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -12  内存不够
			 -16  设备忙
			 -22  非法参数
			 -143 端口未初始化
  函数说明:  无
  注意项:  regionLen为2的幂次方，最小0x8000；rioRegionStart需要与regionLen地址对齐
						  最大支持同时开8个Outbound映射窗口，注意开多窗口时各窗对应的SRIO总线地址不能重叠
*************************************************/
int rioMemOutbMap(u8 portId, u16 destid, u64 *localRegionStart, u32 rioRegionStart, u32 regionLen);

/*************************************************
  函数标题:  释放Outbound窗口地址映射
  函数名:   rioMemOutbFree
  调用函数:
  输入参数:  u8 portId: 设备使用的硬件端口
		 u16 destid:本地映射起始地址
		 u32 rioRegionStart：srio总线起始地址

  输出参数:  无
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -22  非法参数
			 -143 端口未初始化
  函数说明:  无
  注意项:   无
*************************************************/
int rioMemOutbFree(u8 portId, u16 destid, u32 rioRegionStart);

/*************************************************
  函数标题:  释放Inbound窗口地址映射
  函数名:   rioMemInbFree
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口
		u32 localRegionStart:本地映射起始地址

  输出参数:  无
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -22  非法参数
			 -143 端口未初始化
  函数说明:  无
  注意项:   无
*************************************************/
int rioMemInbFree(u8 portId, u32 localRegionStart);

/*************************************************
  函数标题:  PRB0400端口性能统计
  函数名:   prb0400_show_counters
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口
		u8 flag: 端口计数统计属性  0-统计 Pkt包计数   ； 1-统计 Drop包计数

  输出参数:  无
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -22  非法参数
			 -143 端口未初始化
  函数说明:  无
  注意项:   该接口只适用于PRB0400，tsi721无该功能；使用时需要先调用一次该接口以开启包计数统计，
							  之后再次调用该接口则显示当前端口计数统计
*************************************************/
int prb0400_show_counters(u8 portid, u8 flag);

/*************************************************
  函数标题:  PRB0400 SRIO发包间隔配置
  函数名:   prb0400_srio_gap
  调用函数:
  输入参数: u8 portId: 设备使用的硬件端口
		u32 interval: 发包间隔配置，单位ns，最大1020ns； 配0关闭发包间隔，此配置为默认配置

  输出参数:  无
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -22  非法参数
			 -143 端口未初始化
  函数说明:  无
  注意项:   该接口只适用于PRB0400，tsi721无该功能；通过该接口实现SRIO发包间隔配置，用于匹配性能较低的设备
*************************************************/
int prb0400_srio_gap(u8 portid, u32 interval);

/*************************************************
  函数标题:  PRB0400 PCIE初始化配置
  函数名:   prb0400_pcieCfg
  调用函数:
  输入参数: void *dev: 桥片配置结构体指针
		void *pdev: 桥片所链接的RC配置结构体指针

  输出参数:  无
  返回值:   无
  函数说明:  无
  注意项:   如果系统只有1个port，无需调用该接口。如果使用第2个port，调用如下函数配置第2个port
PCIE初始化参数，需要将下面配置结构体中的{0}进行适当替换。 void setPcieLocationParm(void)
{
	pcieChipLocation_s dev[RIO_MAX_MPORTS] = {
			{0x40, 3, 0, 0},
			{0}
	};
	pcieChipLocation_s pdev[RIO_MAX_MPORTS] = {
			{ 0xc0, 0, 2, 0},
			{0}
	};
	prb0400_pcieCfg(&dev[0],&pdev[0]);
}
*************************************************/
void prb0400_pcieCfg(void *dev, void *pdev);

/*************************************************
  函数标题:  PRB0400 PCIE CPLD超时配置
  函数名:   prb0400_set_pcie_ctv
  调用函数:
  输入参数: u8 portid: 设备使用的硬件端口
		u8 ctv: PCIE完成报文超时时间配置，可配参数如下：
		0x0- 50us to 50ms  0x1- 50us to 100us  0x2- 1ms to 10ms(default) 0x5- 16ms to 55ms
			0x6- 65ms to 210ms 0x9- 260ms to 900ms 0xA- 1s to 3.5s           0xD- 4s to 13s      0xE- 17s to 64s

  输出参数:  无
  返回值:    0成功,非0失败
			 返回值错误号列表：
			 -22  非法参数
			 -143 端口未初始化
  函数说明:  无
*************************************************/
int prb0400_set_pcie_ctv(u8 portid, u8 ctv);

/*************************************************
  函数标题: 获取PRB0400 PCIE CPLD超时参数
  函数名:   prb0400_get_pcie_ctv
  调用函数:
  输入参数: u8 portid: 设备使用的硬件端口
  输出参数:  无
  返回值:    >=0成功,<0失败
			 返回值错误号列表：
			 -22  非法参数
			 -143 端口未初始化
  函数说明:  无
*************************************************/
int prb0400_get_pcie_ctv(u8 portid);

/*************************************************
  函数标题: 配置prb0400驱动IB门铃以及IB消息接收任务的亲和属性
  函数名:   prb0400_set_task_affinity
  调用函数:
  输入参数: u32 affinity: 默认0，配置0-3绑定对应核，配置0xffffffff由系统自动指定
  输出参数:  无
  返回值:    0
  函数说明:  无
*************************************************/
int prb0400_set_task_affinity(u32 affinity);

/*************************************************
  函数标题: 获取prb0400驱动IB门铃以及IB消息接收任务的亲和属性
  函数名:   prb0400_get_task_affinity
  调用函数:
  输入参数:  无
  输出参数:  无
  返回值:    亲和属性配置值
  函数说明:  无
*************************************************/
int prb0400_get_task_affinity(void);

#endif /* PRB0400LIB_H_ */
