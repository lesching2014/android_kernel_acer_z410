/*
 * Driver for CAM_CAL
 *
 *
 */

#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include "kd_camera_hw.h"
#include "cam_cal.h"
#include "cam_cal_define.h"
#include "imx135otp.h"
#include <asm/system.h>  // for SMP

//#define CAM_CALGETDLT_DEBUG
#define CAM_CAL_DEBUG
#ifdef CAM_CAL_DEBUG
#define CAM_CALDB printk
#else
#define CAM_CALDB(x,...)
#endif

#if defined(ACER_JADEL)
u8 VENDOR_ID;//pei_add
extern u8 s4AF_EEPROM_Reg_Read(u8 addr);
#endif
	
static DEFINE_SPINLOCK(g_CAM_CALLock); // for SMP
#define CAM_CAL_I2C_BUSNUM 3


/*******************************************************************************
*
********************************************************************************/
#define CAM_CAL_ICS_REVISION 1 //seanlin111208
/*******************************************************************************
*
********************************************************************************/
#define CAM_CAL_DRVNAME "CAM_CAL_DRV"
#define CAM_CAL_I2C_GROUP_ID 0
/*******************************************************************************
*
********************************************************************************/
static struct i2c_board_info __initdata kd_cam_cal_dev={ I2C_BOARD_INFO(CAM_CAL_DRVNAME, 0xA2>>1)};

static struct i2c_client * g_pstI2Cclient = NULL;

//81 is used for V4L driver
static dev_t g_CAM_CALdevno = MKDEV(CAM_CAL_DEV_MAJOR_NUMBER,0);
static struct cdev * g_pCAM_CAL_CharDrv = NULL;
//static spinlock_t g_CAM_CALLock;
//spin_lock(&g_CAM_CALLock);
//spin_unlock(&g_CAM_CALLock);

static struct class *CAM_CAL_class = NULL;
static atomic_t g_CAM_CALatomic;
//static DEFINE_SPINLOCK(kdcam_cal_drv_lock);
//spin_lock(&kdcam_cal_drv_lock);
//spin_unlock(&kdcam_cal_drv_lock);

/*******************************************************************************
*
********************************************************************************/
//Address: 1Byte, Data: 2Byte
int iWriteCAM_CAL(u8 a_u2Addr  , u32 a_u4Bytes, u8 puDataInBytes)
{
    int  i4RetValue = 0;
    int retry = 3;
    do {
        //CAM_CALDB("[CAM_CAL][iWriteCAM_CAL] Write 0x%x=0x%x \n",a_u2Addr, puDataInBytes);
    	//i4RetValue = i2c_master_send(g_pstI2Cclient, puSendCmd, 3);
    	i4RetValue=i2c_smbus_write_i2c_block_data(g_pstI2Cclient,a_u2Addr,a_u4Bytes,&puDataInBytes);
        if (i4RetValue < 0) {
            CAM_CALDB("[CAM_CAL] I2C send failed!!\n");
        }
        else {
            break;
    	}
        mdelay(10);
    } while ((retry--) > 0);    
   //CAM_CALDB("[CAM_CAL] iWriteCAM_CAL done!! \n");
    return 0;
}


//Address: 1Byte, Data: 2Byte
int iReadCAM_CAL(u8 a_u2Addr, u32 ui4_length, u8 * a_puBuff)
{
    int  i4RetValue = 0;

    //i4RetValue = i2c_master_send(g_pstI2Cclient, puReadCmd, 2);
    i4RetValue=i2c_smbus_read_i2c_block_data(g_pstI2Cclient,a_u2Addr,ui4_length,a_puBuff);
    //CAM_CALDB("[CAM_CAL][iReadCAM_CAL] Read 0x%x=0x%x \n", a_u2Addr, a_puBuff[0]);
    if (i4RetValue != ui4_length)
    {
        CAM_CALDB("[CAM_CAL] I2C read data failed!! \n");
        return -1;
    } 

    return 0;
}
#if 0
//go to page
static kal_uint8 IMX135_GotoPage(kal_uint8 page)
{
	kal_uint8 rdrdy_status = 0;
	kal_uint8 checktimes = 2,i = 0;
	u8  readbuff ;
	int ret;
	for(i= 0; i<checktimes;i++)
	{
		iWriteCAM_CAL(0x3b02, 1 , page);
    	iWriteCAM_CAL(0x3b00,1, 0x01);
   		mdelay(2);
    	ret = iReadCAM_CAL(0x3b01,1,&readbuff);
		rdrdy_status = readbuff;
		if( rdrdy_status == 1)
		{			
			return 1;
		}
		else
		{
			CAM_CALDB("OTP fail!\n");
		}
	}
	return 0;	
}

static kal_uint8 check_IMX135_otp_valid_AWBPage(void)
{
	kal_uint8 Pagemax = 12;
	kal_uint8 Pagemin = 9;
	kal_uint8 page = 0;
	kal_uint8 AWB_OK = 0x00;
	u8  readbuff;
	int ret;
	for(page = Pagemax ; page>=Pagemin;page--)
	{
	if (IMX135_GotoPage(page))
		{
			ret = iReadCAM_CAL(0x3b04,1,&readbuff);
			AWB_OK = readbuff;
			if (AWB_OK != 0)
			{
				CAM_CALDB("AWB otp is exist\n");
				break;
			}
		
		}
	}
	if ( page<Pagemin)
		{		
		CAM_CALDB("No AWB OTP data!");
		return 0;
		}
return page;

}

static kal_uint8 check_IMX135_otp_valid_AFPage(void)
{
	kal_uint8 Pagemax = 16;
	kal_uint8 Pagemin = 13;
	kal_uint8 page = 0;
	kal_uint8 AF_OK = 0x00;
	kal_uint8 i = 0;
	u8  readbuff;
	int ret;
	for(page = Pagemax ; page>=Pagemin;page--)
	{
	if (IMX135_GotoPage(page))
		{
		CAM_CALDB("[CAM_CAL]page= %d\n",page);
		for(i = 0; i<5 ;i++)
		{	
			ret = iReadCAM_CAL((0x3b04+i),1,&readbuff);
		    CAM_CALDB("[CAM_CAL](0x3b04+%d)= 0x%x\n",(0x3b04+i),readbuff);
			AF_OK = readbuff;
			if (AF_OK != 0)
			{
				break;
			}	
			}	
		}
			if (AF_OK != 0)
			{
				break;
			}	
	}
	if ( page<Pagemin)
		{		
		CAM_CALDB("[CAM_CAL]No AF OTP data!");
		return 0;
		}
	CAM_CALDB("[CAM_CAL]No AF OTP data!");

return page;

}

static kal_uint8 IMX135_ReadOtp(kal_uint8 page,kal_uint16 address,unsigned char *iBuffer,unsigned int buffersize)
{
		kal_uint16 i = 0;
		u8 readbuff ;
		int ret ;
		CAM_CALDB("[CAM_CAL]ENTER page:0x%x address:0x%x buffersize:%d\n ",page, address, buffersize);
		if (IMX135_GotoPage(page))
		{
			for(i = 0; i<buffersize; i++)
			{				
				ret= iReadCAM_CAL(address+i,1,&readbuff);
				CAM_CALDB("[CAM_CAL]address+i = 0x%x,readbuff = 0x%x\n",(address+i),readbuff );
				*(iBuffer+i) =(unsigned char)readbuff;
			}
		}
		return 0;
}

//Burst Write Data
static int iWriteData(unsigned int  ui4_offset, unsigned int  ui4_length, unsigned char * pinputdata)
{
   int  i4RetValue = 0;
   int  i4ResidueDataLength;
   u32 u4IncOffset = 0;
   u32 u4CurrentOffset;
   u8 * pBuff;
   CAM_CALDB("[CAM_CAL] iWriteData\n" );

   i4ResidueDataLength = (int)ui4_length;
   u4CurrentOffset = ui4_offset;
   pBuff = pinputdata;   

   CAM_CALDB("[CAM_CAL] iWriteData u4CurrentOffset is %d \n",u4CurrentOffset);   

   do 
   {
       CAM_CALDB("[CAM_CAL][iWriteData] Write 0x%x=0x%x \n",u4CurrentOffset, pBuff[0]);
       i4RetValue = iWriteCAM_CAL((u8)u4CurrentOffset, 1, pBuff[0]);
       if (i4RetValue != 0)
       {
            CAM_CALDB("[CAM_CAL] I2C iWriteData failed!! \n");
            return -1;
       }           
       u4IncOffset ++;
       i4ResidueDataLength --;
       u4CurrentOffset = ui4_offset + u4IncOffset;
       pBuff = pinputdata + u4IncOffset;
   }while (i4ResidueDataLength > 0);
   CAM_CALDB("[CAM_CAL] iWriteData done\n" );
 
   return 0;
}

//Burst Read Data
static int iReadData(unsigned int  ui4_offset, unsigned int  ui4_length, unsigned char * pinputdata)
{
   int  i4RetValue = 0;
    kal_uint8 page = 0;
	//1. check which page is valid
	if(ui4_length ==2)
	{
   		page = check_IMX135_otp_valid_AFPage();
  		 if (page!=0)
   		{
	  		 CAM_CALDB("[CAM_CAL]AF OTP exist! Valid page is %d\n",page);
   		}
   		else
   		{
	   		 CAM_CALDB("[CAM_CAL]No AF OTP Data!\n");
	  		 return -1;
   		}
	}
	else
    {
	  	page = check_IMX135_otp_valid_AWBPage();
	  	if (page!=-1)
	 	{
		  CAM_CALDB("[CAM_CAL]AWB OTP exist! Valid page is %d\n",page);
	  	}
	  	else
	  	{
		  CAM_CALDB("[CAM_CAL]No AWB OTP Data!\n");
		  return -1;
	  	}
   	}
    //2. read otp
    IMX135_ReadOtp(page, ui4_offset, pinputdata, ui4_length);
    for(i4RetValue = 0;i4RetValue<ui4_length;i4RetValue++){
    CAM_CALDB( "[[CAM_CAL]]pinputdata[%d]=%d\n", i4RetValue,*(pinputdata+i4RetValue));}
    CAM_CALDB(" [[CAM_CAL]]page = %d,ui4_length = %d,ui4_offset =%d\n ",page,ui4_length,ui4_offset);
    CAM_CALDB("[S24EEPORM] iReadData done\n" );
   return 0;
}


static void Enb_OTP_Read(int enb){
	u8 * pOneByteBuff = NULL;
	pOneByteBuff = (u8 *)kmalloc(I2C_UNIT_SIZE, GFP_KERNEL);
	//Enable Reading OTP
	CAM_CALDB("[CAM_CAL]Enb_OTP_Read=%d\n",enb);
	iReadData(0x3d21, I2C_UNIT_SIZE, pOneByteBuff);
	CAM_CALDB("[CAM_CAL]0x3d21=0x%x\n",*pOneByteBuff);
	if(enb){
		*pOneByteBuff = (*pOneByteBuff | 0x01);
	    iWriteData(0x3d21, I2C_UNIT_SIZE, pOneByteBuff );
	} else {
	    *pOneByteBuff = (*pOneByteBuff & 0xFE);
	    iWriteData(0x3d21, I2C_UNIT_SIZE, pOneByteBuff );
	}
}

static void Clear_OTP_Buff(void){
	u8 AllZero[OTP_SIZE]={0};
	iWriteData(OTP_START_ADDR, OTP_SIZE, AllZero);
}
#endif

/*******************************************************************************
*
********************************************************************************/
#define NEW_UNLOCK_IOCTL
#ifndef NEW_UNLOCK_IOCTL
static int CAM_CAL_Ioctl(struct inode * a_pstInode,
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
#else 
static long CAM_CAL_Ioctl(
    struct file *file, 
    unsigned int a_u4Command, 
    unsigned long a_u4Param
)
#endif
{
    int i4RetValue = 0;
    u8 * pBuff = NULL;
    u8 * pWorkingBuff = NULL;
    stCAM_CAL_INFO_STRUCT *ptempbuf;

#ifdef CAM_CALGETDLT_DEBUG
    struct timeval ktv1, ktv2;
    unsigned long TimeIntervalUS;
#endif

    if(_IOC_NONE == _IOC_DIR(a_u4Command))
    {
    }
    else
    {
        pBuff = (u8 *)kmalloc(sizeof(stCAM_CAL_INFO_STRUCT),GFP_KERNEL);

        if(NULL == pBuff)
        {
            CAM_CALDB("[CAM_CAL] ioctl allocate mem failed\n");
            return -ENOMEM;
        }

        if(_IOC_WRITE & _IOC_DIR(a_u4Command))
        {
            if(copy_from_user((u8 *) pBuff , (u8 *) a_u4Param, sizeof(stCAM_CAL_INFO_STRUCT)))
            {    //get input structure address
                kfree(pBuff);
                CAM_CALDB("[CAM_CAL] ioctl copy from user failed\n");
                return -EFAULT;
            }
        }
    }

    ptempbuf = (stCAM_CAL_INFO_STRUCT *)pBuff;
    pWorkingBuff = (u8*)kmalloc(ptempbuf->u4Length,GFP_KERNEL); 
    if(NULL == pWorkingBuff)
    {
        kfree(pBuff);
        CAM_CALDB("[CAM_CAL] ioctl allocate mem failed\n");
        return -ENOMEM;
    }
     //CAM_CALDB("[CAM_CAL] init Working buffer address 0x%8x  command is 0x%8x\n", (u32)pWorkingBuff, (u32)a_u4Command);

 
    if(copy_from_user((u8*)pWorkingBuff ,  (u8*)ptempbuf->pu1Params, ptempbuf->u4Length))
    {
        kfree(pBuff);
        kfree(pWorkingBuff);
        CAM_CALDB("[CAM_CAL] ioctl copy from user failed\n");
        return -EFAULT;
    } 
    
    switch(a_u4Command)
    {
        case CAM_CALIOC_S_WRITE:    
            CAM_CALDB("[CAM_CAL] Write CMD \n");
#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv1);
#endif            
            i4RetValue = iWriteCAM_CAL((u8)ptempbuf->u4Offset, ptempbuf->u4Length, pWorkingBuff);
#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv2);
            if(ktv2.tv_sec > ktv1.tv_sec)
            {
                TimeIntervalUS = ktv1.tv_usec + 1000000 - ktv2.tv_usec;
            }
            else
            {
                TimeIntervalUS = ktv2.tv_usec - ktv1.tv_usec;
            }
            printk("Write data %d bytes take %lu us\n",ptempbuf->u4Length, TimeIntervalUS);
#endif            
            break;
        case CAM_CALIOC_G_READ:
            CAM_CALDB("[CAM_CAL] Read CMD \n");
#ifdef CAM_CALGETDLT_DEBUG            
            do_gettimeofday(&ktv1);
#endif 
            CAM_CALDB("[CAM_CAL] offset %d \n", ptempbuf->u4Offset);
            CAM_CALDB("[CAM_CAL] length %d \n", ptempbuf->u4Length);
            //CAM_CALDB("[CAM_CAL] Before read Working buffer address 0x%8x \n", (u32)pWorkingBuff);

             //  Enb_OTP_Read(1); //Enable OTP Read
            //i4RetValue = iReadCAM_CAL((u8)(ptempbuf->u4Offset+OTP_START_ADDR), ptempbuf->u4Length, pWorkingBuff);
            i4RetValue = iReadCAM_CAL((u8)(ptempbuf->u4Offset), ptempbuf->u4Length, pWorkingBuff);
			
	    //Enb_OTP_Read(0); //Disable OTP Read
	    //Clear_OTP_Buff(); //Clean OTP buff
            CAM_CALDB("[CAM_CAL] After read Working buffer data  0x%2x --0x%2x\n", *pWorkingBuff,*(pWorkingBuff+1));

#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv2);
            if(ktv2.tv_sec > ktv1.tv_sec)
            {
                TimeIntervalUS = ktv1.tv_usec + 1000000 - ktv2.tv_usec;
            }
            else
            {
                TimeIntervalUS = ktv2.tv_usec - ktv1.tv_usec;
            }
            printk("Read data %d bytes take %lu us\n",ptempbuf->u4Length, TimeIntervalUS);
#endif            

            break;

#if defined(ACER_JADEL)

    case CAM_VENDOR_READ: 
		    
	CAM_CALDB("peisong [CAM_CAL] VENDOR_ID %d\n", VENDOR_ID);

			//VENDOR_ID;
			*pWorkingBuff = VENDOR_ID;
	 	
	      break;

#endif




        default :
      	     CAM_CALDB("[CAM_CAL] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    if(_IOC_READ & _IOC_DIR(a_u4Command))
    {
        //copy data to user space buffer, keep other input paremeter unchange.
        CAM_CALDB("[CAM_CAL] to user length %d \n", ptempbuf->u4Length);
        //CAM_CALDB("[CAM_CAL] to user  Working buffer address 0x%8x \n", (u32)pWorkingBuff);
        if(copy_to_user((u8 __user *) ptempbuf->pu1Params , (u8 *)pWorkingBuff , ptempbuf->u4Length))
        {
            kfree(pBuff);
            kfree(pWorkingBuff);
            CAM_CALDB("[CAM_CAL] ioctl copy to user failed\n");
            return -EFAULT;
        }
    }

    kfree(pBuff);
    kfree(pWorkingBuff);
    return i4RetValue;
}


static u32 g_u4Opened = 0;
//#define
//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
static int CAM_CAL_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    CAM_CALDB("[CAM_CAL] CAM_CAL_Open\n");
    spin_lock(&g_CAM_CALLock);
    if(g_u4Opened)
    {
        spin_unlock(&g_CAM_CALLock);
	CAM_CALDB("[CAM_CAL] Opened, return -EBUSY\n");
        return -EBUSY;
    }
    else
    {
        g_u4Opened = 1;
        //atomic_set(&g_CAM_CALatomic,0);
    }
    spin_unlock(&g_CAM_CALLock);

#if 0
    if(TRUE != hwPowerOn(MT65XX_POWER_LDO_VCAMA, VOL_2800, CAM_CAL_DRVNAME))
    {
        CAM_CALDB("[CAM_CAL] Fail to enable analog gain\n");
        return -EIO;
    }
#endif
#if defined(ACER_JADEL)
//pei_add

	VENDOR_ID=s4AF_EEPROM_Reg_Read(0x0C); // abilitycorp 0x00 or 0x01 or 0x55 ; lite on 0x02
	CAM_CALDB("pxs_camcal [CAM_CAL] VENDOR_ID %d\n", VENDOR_ID);
#endif
    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int CAM_CAL_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    CAM_CALDB("[CAM_CAL] CAM_CAL_Release\n");
    if(g_u4Opened==0)
    {
	CAM_CALDB("[CAM_CAL] CAM_CAL_Release, return -EBUSY\n");
        return -EBUSY;
    }
	
#if 0
    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2, CAM_CAL_DRVNAME))
    {
        CAM_CALDB("[CAM_CAL] Fail to disable analog gain\n");
        return -EIO;
    }
#endif
    spin_lock(&g_CAM_CALLock);

    g_u4Opened = 0;
	
    //atomic_set(&g_CAM_CALatomic,0);

    spin_unlock(&g_CAM_CALLock);

    return 0;
}

static const struct file_operations g_stCAM_CAL_fops =
{
    .owner = THIS_MODULE,
    .open = CAM_CAL_Open,
    .release = CAM_CAL_Release,
    //.ioctl = CAM_CAL_Ioctl
    .unlocked_ioctl = CAM_CAL_Ioctl
};

#define CAM_CAL_DYNAMIC_ALLOCATE_DEVNO 1
inline static int RegisterCAM_CALCharDrv(void)
{
    struct device* CAM_CAL_device = NULL;

#if CAM_CAL_DYNAMIC_ALLOCATE_DEVNO
    if( alloc_chrdev_region(&g_CAM_CALdevno, 0, 1,CAM_CAL_DRVNAME) )
    {
        CAM_CALDB("[CAM_CAL] Allocate device no failed\n");

        return -EAGAIN;
    }
#else
    if( register_chrdev_region(  g_CAM_CALdevno , 1 , CAM_CAL_DRVNAME) )
    {
        CAM_CALDB("[CAM_CAL] Register device no failed\n");

        return -EAGAIN;
    }
#endif

    //Allocate driver
    g_pCAM_CAL_CharDrv = cdev_alloc();

    if(NULL == g_pCAM_CAL_CharDrv)
    {
        unregister_chrdev_region(g_CAM_CALdevno, 1);

        CAM_CALDB("[CAM_CAL] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pCAM_CAL_CharDrv, &g_stCAM_CAL_fops);

    g_pCAM_CAL_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pCAM_CAL_CharDrv, g_CAM_CALdevno, 1))
    {
        CAM_CALDB("[CAM_CAL] Attatch file operation failed\n");

        unregister_chrdev_region(g_CAM_CALdevno, 1);

        return -EAGAIN;
    }

    CAM_CAL_class = class_create(THIS_MODULE, "CAM_CALdrv");
    if (IS_ERR(CAM_CAL_class)) {
        int ret = PTR_ERR(CAM_CAL_class);
        CAM_CALDB("Unable to create class, err = %d\n", ret);
        return ret;
    }
    CAM_CAL_device = device_create(CAM_CAL_class, NULL, g_CAM_CALdevno, NULL, CAM_CAL_DRVNAME);

    return 0;
}

inline static void UnregisterCAM_CALCharDrv(void)
{
    //Release char driver
    cdev_del(g_pCAM_CAL_CharDrv);

    unregister_chrdev_region(g_CAM_CALdevno, 1);

    device_destroy(CAM_CAL_class, g_CAM_CALdevno);
    class_destroy(CAM_CAL_class);
}


//////////////////////////////////////////////////////////////////////
#ifndef CAM_CAL_ICS_REVISION
static int CAM_CAL_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
#elif 0
static int CAM_CAL_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
#else
#endif
static int CAM_CAL_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int CAM_CAL_i2c_remove(struct i2c_client *);

static const struct i2c_device_id CAM_CAL_i2c_id[] = {{CAM_CAL_DRVNAME,0},{}};   
#if 0 //test110314 Please use the same I2C Group ID as Sensor
static unsigned short force[] = {CAM_CAL_I2C_GROUP_ID, IMX135OTP_DEVICE_ID, I2C_CLIENT_END, I2C_CLIENT_END};   
#else
//static unsigned short force[] = {IMG_SENSOR_I2C_GROUP_ID, OV5647OTP_DEVICE_ID, I2C_CLIENT_END, I2C_CLIENT_END};   
#endif
//static const unsigned short * const forces[] = { force, NULL };              
//static struct i2c_client_address_data addr_data = { .forces = forces,}; 


static struct i2c_driver CAM_CAL_i2c_driver = {
    .probe = CAM_CAL_i2c_probe,                                   
    .remove = CAM_CAL_i2c_remove,                           
//   .detect = CAM_CAL_i2c_detect,                           
    .driver.name = CAM_CAL_DRVNAME,
    .id_table = CAM_CAL_i2c_id,                             
};

#ifndef CAM_CAL_ICS_REVISION
static int CAM_CAL_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, CAM_CAL_DRVNAME);
    return 0;
}
#endif
static int CAM_CAL_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {             
    int i4RetValue = 0;
    CAM_CALDB("[CAM_CAL] Attach I2C \n");
//    spin_lock_init(&g_CAM_CALLock);

    //get sensor i2c client
    spin_lock(&g_CAM_CALLock); //for SMP
    g_pstI2Cclient = client;
    g_pstI2Cclient->addr = IMX135OTP_DEVICE_ID>>1;
    spin_unlock(&g_CAM_CALLock); // for SMP    
    
    CAM_CALDB("[CAM_CAL] g_pstI2Cclient->addr = 0x%8x \n",g_pstI2Cclient->addr);
    //Register char driver
    i4RetValue = RegisterCAM_CALCharDrv();

    if(i4RetValue){
        CAM_CALDB("[CAM_CAL] register char device failed!\n");
        return i4RetValue;
    }


    CAM_CALDB("[CAM_CAL] Attached!! \n");
    return 0;                                                                                       
} 

static int CAM_CAL_i2c_remove(struct i2c_client *client)
{
    return 0;
}

static int CAM_CAL_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&CAM_CAL_i2c_driver);
}

static int CAM_CAL_remove(struct platform_device *pdev)
{
    i2c_del_driver(&CAM_CAL_i2c_driver);
    return 0;
}

// platform structure
static struct platform_driver g_stCAM_CAL_Driver = {
    .probe		= CAM_CAL_probe,
    .remove	= CAM_CAL_remove,
    .driver		= {
        .name	= CAM_CAL_DRVNAME,
        .owner	= THIS_MODULE,
    }
};


static struct platform_device g_stCAM_CAL_Device = {
    .name = CAM_CAL_DRVNAME,
    .id = 0,
    .dev = {
    }
};

static int __init CAM_CAL_i2C_init(void)
{
    i2c_register_board_info(CAM_CAL_I2C_BUSNUM, &kd_cam_cal_dev, 1);

    if (platform_device_register(&g_stCAM_CAL_Device))
    {
        CAM_CALDB("failed to register CAM_CAL driver, 2nd time\n");
        return -ENODEV;
    }	

    if(platform_driver_register(&g_stCAM_CAL_Driver)){
        CAM_CALDB("failed to register CAM_CAL driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit CAM_CAL_i2C_exit(void)
{
	platform_driver_unregister(&g_stCAM_CAL_Driver);
}

module_init(CAM_CAL_i2C_init);
module_exit(CAM_CAL_i2C_exit);

MODULE_DESCRIPTION("CAM_CAL driver");
MODULE_AUTHOR("Sean Lin <Sean.Lin@Mediatek.com>");
MODULE_LICENSE("GPL");


