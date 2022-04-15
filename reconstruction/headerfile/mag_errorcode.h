#ifndef _MAG_ERRORCODE_H_
#define _MAG_ERRORCODE_H_

#define SEV_MAG			(0xE<<28)
#define FACILITY_MAG	(0x4AE<<16)

#define MAG_ERROR_CODE(e)	(SEV_MAG | FACILITY_MAG | ((e) & 0xFFFF) )

//error codes for magnity
#define MAG_INVALIDPARAMETER		MAG_ERROR_CODE(0x1)//函数输入参数错误
#define MAG_INSUFFICIENTMEMORY		MAG_ERROR_CODE(0x2)//内存new失败
#define MAG_UNCLASSED				MAG_ERROR_CODE(0x3)//未归类的错误
#define MAG_NOTINITIALIZED			MAG_ERROR_CODE(0x4)//尚未调用MAG_Initialize()
#define MAG_NOAVAILABLECAMERA		MAG_ERROR_CODE(0x5)//尚未连接相机
#define MAG_NOAVAILABLEDATA			MAG_ERROR_CODE(0x6)//尚没有可输出的数据
#define MAG_COMMUNICATIONERROR		MAG_ERROR_CODE(0x7)//通讯错误
#define MAG_COMMUNICATIONTIMEOUT	MAG_ERROR_CODE(0x8)//通讯超时
#define MAG_FILEIOERROR				MAG_ERROR_CODE(0x9)//文件读写错误
#define MAG_DUPLICATEDOPERATION		MAG_ERROR_CODE(0xA)//重复操作，例如重复调用MAG_NewChannel()
#define MAG_NOTREADY				MAG_ERROR_CODE(0xB)//所需使用的资源尚未准备好
#define MAG_OEMERROR				MAG_ERROR_CODE(0xC)//定制类产品类别不符

//error codes for mag_ethernet
#define MAG_NOTCONNECTED			MAG_ERROR_CODE(0x101)//尚未连接以太网
#define MAG_SOCKETERROR				MAG_ERROR_CODE(0x102)//socket错误
#define MAG_SOCKETPORTERROR			MAG_ERROR_CODE(0x103)//socket端口错误

#endif
