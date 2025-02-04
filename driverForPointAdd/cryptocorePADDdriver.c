#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>

#include "../include/cryptocore_ioctl_header.h"

#define GPIO1_BASE				0xFF709000
#define GPIO1_SPAN				0x00000078

#define H2F_BRIDGE_BASE			0xC0000000

#define MWMAC_RAM_BASE			0x00000000
#define MWMAC_RAM_SPAN			0x00000FFF

#define LW_H2F_BRIDGE_BASE		0xFF200000

#define LEDR_BASE          		0x00000000
#define LEDR_SPAN				0x0000000F

#define HEX3_HEX0_BASE			0x00000020
#define HEX3_HEX0_SPAN			0x0000000F

#define HEX5_HEX4_BASE			0x00000030
#define HEX5_HEX4_SPAN			0x0000000F

#define SW_BASE					0x00000040
#define SW_SPAN					0x0000000F

#define KEY_BASE				0x00000050
#define KEY_SPAN				0x0000000F

#define MWMAC_CMD_BASE			0x00000060
#define MWMAC_CMD_SPAN			0x00000007

#define MWMAC_IRQ_BASE         	0x00000070
#define MWMAC_IRQ_SPAN			0x0000000F

#define TRNG_CMD_BASE			0x00000080
#define TRNG_CMD_SPAN			0x0000000F

#define TRNG_CTR_BASE			0x00000090
#define TRNG_CTR_SPAN			0x0000000F

#define TRNG_TSTAB_BASE			0x000000A0
#define TRNG_TSTAB_SPAN			0x0000000F

#define TRNG_TSAMPLE_BASE		0x000000B0
#define TRNG_TSAMPLE_SPAN		0x0000000F

#define TRNG_IRQ_BASE			0x000000C0
#define TRNG_IRQ_SPAN			0x0000000F

#define TRNG_FIFO_BASE			0x00001000
#define TRNG_FIFO_SPAN			0x00000FFF

#define TIMER_BASE            	0x00002000
#define TIMER_SPAN				0x0000001F

#define KEY_IRQ	 			73
#define MWMAC_IRQ	 		74
#define TRNG_IRQ			75

#define DRIVER_NAME "cryptocore" /* Name des Moduls */

// CryptoCore Operations:

#define MONTMULT			0x0
#define MONTR				0x1
#define MONTR2				0x2
#define MONTEXP				0x3
#define MODADD				0x4
#define MODSUB				0x5
#define COPYH2V				0x6
#define COPYV2V				0x7
#define COPYH2H				0x8
#define COPYV2H				0x9
#define MONTMULT1			0xA

// CryptoCore RAM Locations

#define MWMAC_RAM_B1 			0x0
#define MWMAC_RAM_B2 			0x1
#define MWMAC_RAM_B3 			0x2
#define MWMAC_RAM_B4 			0x3
#define MWMAC_RAM_B5 			0x4
#define MWMAC_RAM_B6 			0x5
#define MWMAC_RAM_B7 			0x6
#define MWMAC_RAM_B8 			0x7

#define MWMAC_RAM_P1			0x8
#define MWMAC_RAM_P2			0x9
#define MWMAC_RAM_P3			0xA
#define MWMAC_RAM_P4			0xB
#define MWMAC_RAM_P5			0xC
#define MWMAC_RAM_P6			0xD
#define MWMAC_RAM_P7			0xE
#define MWMAC_RAM_P8			0xF

#define MWMAC_RAM_TS1			0x10
#define MWMAC_RAM_TS2			0x11
#define MWMAC_RAM_TS3			0x12
#define MWMAC_RAM_TS4			0x13
#define MWMAC_RAM_TS5			0x14
#define MWMAC_RAM_TS6			0x15
#define MWMAC_RAM_TS7			0x16
#define MWMAC_RAM_TS8			0x17

#define MWMAC_RAM_TC1			0x18
#define MWMAC_RAM_TC2			0x19
#define MWMAC_RAM_TC3			0x1A
#define MWMAC_RAM_TC4			0x1B
#define MWMAC_RAM_TC5			0x1C
#define MWMAC_RAM_TC6			0x1D
#define MWMAC_RAM_TC7			0x1E
#define MWMAC_RAM_TC8			0x1F

#define MWMAC_RAM_A1			0x0
#define MWMAC_RAM_A2			0x1
#define MWMAC_RAM_A3			0x2
#define MWMAC_RAM_A4			0x3
#define MWMAC_RAM_A5			0x4
#define MWMAC_RAM_A6			0x5
#define MWMAC_RAM_A7			0x6
#define MWMAC_RAM_A8			0x7

#define MWMAC_RAM_E1			0x8
#define MWMAC_RAM_E2			0x9
#define MWMAC_RAM_E3			0xA
#define MWMAC_RAM_E4			0xB
#define MWMAC_RAM_E5			0xC
#define MWMAC_RAM_E6			0xD
#define MWMAC_RAM_E7			0xE
#define MWMAC_RAM_E8			0xF

#define MWMAC_RAM_X1			0x10
#define MWMAC_RAM_X2			0x11
#define MWMAC_RAM_X3			0x12
#define MWMAC_RAM_X4			0x13
#define MWMAC_RAM_X5			0x14
#define MWMAC_RAM_X6			0x15
#define MWMAC_RAM_X7			0x16
#define MWMAC_RAM_X8			0x17

static dev_t cryptocore_dev_number;
static struct cdev *driver_object;
static struct class *cryptocore_class;
static struct device *cryptocore_dev;

volatile u32 *HPS_GPIO1_ptr;
volatile u32 *LEDR_ptr;
volatile u32 *KEY_ptr;
volatile u32 *MWMAC_RAM_ptr;
volatile u32 *MWMAC_CMD_ptr;
volatile u32 *MWMAC_IRQ_ptr;
volatile u32 *TRNG_CMD_ptr;
volatile u32 *TRNG_CTR_ptr;
volatile u32 *TRNG_TSTAB_ptr;
volatile u32 *TRNG_TSAMPLE_ptr;
volatile u32 *TRNG_IRQ_ptr;
volatile u32 *TRNG_FIFO_ptr;

volatile u32 mwmac_irq_var;

volatile u32 trng_words_available;

// CryptoCore Driver Supported Precisions:
static u32 PRIME_PRECISIONS[13][2]={ {192,0x0}, {224,0x1}, {256,0x2}, {320,0x3}, {384,0x4}, {512,0x5}, {768,0x6}, {1024,0x7}, {1536,0x8}, {2048,0x9}, {3072,0xA}, {4096,0xB}, {448, 0xD}};
static u32 BINARY_PRECISIONS[16][2]={ {131,0x0}, {163,0x1}, {176,0x2}, {191,0x3}, {193,0x4}, {208,0x5}, {233,0x6}, {239,0x7}, {272,0x8}, {283,0x9}, {304,0xA}, {359,0xB}, {368,0xC}, {409,0xD}, {431,0xE}, {571,0xF} };

// CryptoCore Driver Function Prototyps:
static void Clear_MWMAC_RAM(void);
static void MWMAC_MontMult(MontMult_params_t *MontMult_params_ptr);
static void MWMAC_MontR(MontR_params_t *MontR_params_ptr);
static void MWMAC_MontR2(MontR2_params_t *MontR2_params_ptr);
static void MWMAC_MontExp(MontExp_params_t *MontExp_params_ptr);
static void MWMAC_ModAdd(ModAdd_params_t *ModAdd_params_ptr);
static void MWMAC_ModSub(ModSub_params_t *ModSub_params_ptr);
static void MWMAC_CopyH2V(CopyH2V_params_t *CopyH2V_params_ptr);
static void MWMAC_CopyV2V(CopyV2V_params_t *CopyV2V_params_ptr);
static void MWMAC_CopyH2H(CopyH2H_params_t *CopyH2H_params_ptr);
static void MWMAC_CopyV2H(CopyV2H_params_t *CopyV2H_params_ptr);
static void MWMAC_MontMult1(MontMult1_params_t *MontMult1_params_ptr);
// Add further Function Prototypes here...
static void MWMAC_ECPreparation(ECPreparation_params_t *ECPreparation_params_ptr);
static void MWMAC_ECMDJacobiTransform(ECMDJacobiTransform_params_t *ECMDJacobiTransform_params_ptr);
static void MWMAC_ECMDPointDBL(ECMDPointDBL_params_t *ECMDPointDBL_params_ptr);
static void MWMAC_ECMDPointADD(ECMDPointADD_params_t *ECMDPointADD_params_ptr);
static void MWMAC_ECMDJtoAffine(ECMDJtoAffine_params_t *ECMDJtoAffine_params_ptr);
static void MWMAC_ECMontBack(ECMontBack_params_t *ECMontBack_params_ptr);

irq_handler_t key_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 led_val;
	led_val = ioread32(LEDR_ptr);
	iowrite32((led_val ^ 0x00000001), LEDR_ptr); // Toggle LEDR[0]
	
	iowrite32(0x0000000F, KEY_ptr+3);
    
	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t mwmac_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 led_val;
	led_val = ioread32(LEDR_ptr);
	iowrite32((led_val ^ 0x00000001), LEDR_ptr); // Toggle LEDR[0]
	
	iowrite32(0x00000001, MWMAC_IRQ_ptr+3);
	mwmac_irq_var = 1;
    
	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t trng_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	iowrite32(0x00000001, TRNG_IRQ_ptr+3);

	trng_words_available = 1024;
    
	return (irq_handler_t) IRQ_HANDLED;
}

static int cryptocore_driver_open ( struct inode *inode, struct file *instance )
{
	dev_info( cryptocore_dev, "cryptocore_driver_open called\n" );
	return 0;
}

static int cryptocore_driver_close ( struct inode *inode, struct file *instance )
{
	dev_info( cryptocore_dev, "cryptocore_driver_close called\n" );
	return 0;
}

static long cryptocore_driver_ioctl( struct file *instance, unsigned int cmd, unsigned long arg)
{	
	// Add CryptoCore Structs here and allocate kernel memory... 		
	MontMult_params_t *MontMult_params_ptr = kmalloc(sizeof(MontMult_params_t), GFP_DMA);
	MontR_params_t *MontR_params_ptr = kmalloc(sizeof(MontR_params_t), GFP_DMA);
	MontR2_params_t *MontR2_params_ptr = kmalloc(sizeof(MontR2_params_t), GFP_DMA);
	MontExp_params_t *MontExp_params_ptr = kmalloc(sizeof(MontExp_params_t), GFP_DMA);
	ModAdd_params_t *ModAdd_params_ptr = kmalloc(sizeof(ModAdd_params_t), GFP_DMA);
	ModSub_params_t *ModSub_params_ptr = kmalloc(sizeof(ModSub_params_t), GFP_DMA);
	CopyH2V_params_t *CopyH2V_params_ptr = kmalloc(sizeof(CopyH2V_params_t), GFP_DMA);
	CopyV2V_params_t *CopyV2V_params_ptr = kmalloc(sizeof(CopyV2V_params_t), GFP_DMA);
	CopyH2H_params_t *CopyH2H_params_ptr = kmalloc(sizeof(CopyH2H_params_t), GFP_DMA);
	CopyV2H_params_t *CopyV2H_params_ptr = kmalloc(sizeof(CopyV2H_params_t), GFP_DMA);
	MontMult1_params_t *MontMult1_params_ptr = kmalloc(sizeof(MontMult1_params_t), GFP_DMA);

	ECPreparation_params_t *ECPreparation_params_ptr = kmalloc(sizeof(ECPreparation_params_t), GFP_DMA);
	ECMDJacobiTransform_params_t *ECMDJacobiTransform_params_ptr = kmalloc(sizeof(ECMDJacobiTransform_params_t), GFP_DMA);
	ECMDPointDBL_params_t *ECMDPointDBL_params_ptr = kmalloc(sizeof(ECMDPointDBL_params_t), GFP_DMA);
	ECMDPointADD_params_t *ECMDPointADD_params_ptr = kmalloc(sizeof(ECMDPointADD_params_t), GFP_DMA);
	ECMDJtoAffine_params_t *ECMDJtoAffine_params_ptr = kmalloc(sizeof(ECMDJtoAffine_params_t), GFP_DMA);
	ECMontBack_params_t *ECMontBack_params_ptr = kmalloc(sizeof(ECMontBack_params_t), GFP_DMA);

	int rc;
	u32 i;
	u32 trng_val = 0;

	mwmac_irq_var = 0;

	dev_info( cryptocore_dev, "cryptocore_driver_ioctl called 0x%4.4x %p\n", cmd, (void *) arg );

	switch(cmd) {
		case IOCTL_SET_TRNG_CMD:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_CMD_ptr);
			if(trng_val | 0x00000001) {
				for(i=0; i<60; i++){
					udelay(1000); // Give TRNG FIFO time to fill
				}
			}
			break;
		case IOCTL_SET_TRNG_CTR:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_CTR_ptr);
			break;
		case IOCTL_SET_TRNG_TSTAB:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_TSTAB_ptr);
			break;
		case IOCTL_SET_TRNG_TSAMPLE:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_TSAMPLE_ptr);
			break;
		case IOCTL_READ_TRNG_FIFO:
			trng_val = ioread32(TRNG_FIFO_ptr);
			trng_words_available--;
			put_user(trng_val, (u32 *)arg);
			if(trng_words_available == 0) {
				for(i=0; i<60; i++){
					udelay(1000); // Give TRNG FIFO time to fill
				}				
			}
			break;
		case IOCTL_MWMAC_MONTMULT:
			rc = copy_from_user(MontMult_params_ptr, (void *)arg, sizeof(MontMult_params_t));
			MWMAC_MontMult(MontMult_params_ptr);
			rc = copy_to_user((void *)arg, MontMult_params_ptr, sizeof(MontMult_params_t));
			break;
		case IOCTL_MWMAC_MONTR:
			rc = copy_from_user(MontR_params_ptr, (void *)arg, sizeof(MontR_params_t));
			MWMAC_MontR(MontR_params_ptr);
			rc = copy_to_user((void *)arg, MontR_params_ptr, sizeof(MontR_params_t));
			break;			
		case IOCTL_MWMAC_MONTR2:
			rc = copy_from_user(MontR2_params_ptr, (void *)arg, sizeof(MontR2_params_t));
			MWMAC_MontR2(MontR2_params_ptr);
			rc = copy_to_user((void *)arg, MontR2_params_ptr, sizeof(MontR2_params_t));
			break;			
		case IOCTL_MWMAC_MONTEXP:
			rc = copy_from_user(MontExp_params_ptr, (void *)arg, sizeof(MontExp_params_t));
			MWMAC_MontExp(MontExp_params_ptr);
			rc = copy_to_user((void *)arg, MontExp_params_ptr, sizeof(MontExp_params_t));
			break;			
		case IOCTL_MWMAC_MODADD:
			rc = copy_from_user(ModAdd_params_ptr, (void *)arg, sizeof(ModAdd_params_t));
			MWMAC_ModAdd(ModAdd_params_ptr);
			rc = copy_to_user((void *)arg, ModAdd_params_ptr, sizeof(ModAdd_params_t));
			break;
		case IOCTL_MWMAC_MODSUB:
			rc = copy_from_user(ModSub_params_ptr, (void *)arg, sizeof(ModSub_params_t));
			MWMAC_ModSub(ModSub_params_ptr);
			rc = copy_to_user((void *)arg, ModSub_params_ptr, sizeof(ModSub_params_t));
			break;
		case IOCTL_MWMAC_COPYH2V:
			rc = copy_from_user(CopyH2V_params_ptr, (void *)arg, sizeof(CopyH2V_params_t));
			MWMAC_CopyH2V(CopyH2V_params_ptr);
			rc = copy_to_user((void *)arg, CopyH2V_params_ptr, sizeof(CopyH2V_params_t));
			break;
		case IOCTL_MWMAC_COPYV2V:
			rc = copy_from_user(CopyV2V_params_ptr, (void *)arg, sizeof(CopyV2V_params_t));
			MWMAC_CopyV2V(CopyV2V_params_ptr);
			rc = copy_to_user((void *)arg, CopyV2V_params_ptr, sizeof(CopyV2V_params_t));
			break;
		case IOCTL_MWMAC_COPYH2H:
			rc = copy_from_user(CopyH2H_params_ptr, (void *)arg, sizeof(CopyH2H_params_t));
			MWMAC_CopyH2H(CopyH2H_params_ptr);
			rc = copy_to_user((void *)arg, CopyH2H_params_ptr, sizeof(CopyH2H_params_t));
			break;
		case IOCTL_MWMAC_COPYV2H:
			rc = copy_from_user(CopyV2H_params_ptr, (void *)arg, sizeof(CopyV2H_params_t));
			MWMAC_CopyV2H(CopyV2H_params_ptr);
			rc = copy_to_user((void *)arg, CopyV2H_params_ptr, sizeof(CopyV2H_params_t));
			break;
		case IOCTL_MWMAC_MONTMULT1:
			rc = copy_from_user(MontMult1_params_ptr, (void *)arg, sizeof(MontMult1_params_t));
			MWMAC_MontMult1(MontMult1_params_ptr);
			rc = copy_to_user((void *)arg, MontMult1_params_ptr, sizeof(MontMult1_params_t));
			break;
		// Add further CryptoCore commands here 	
		case IOCTL_MWMAC_ECPREPARATION:
			rc = copy_from_user(ECPreparation_params_ptr, (void *)arg, sizeof(ECPreparation_params_t));
			MWMAC_ECPreparation(ECPreparation_params_ptr);
			rc = copy_to_user((void *)arg, ECPreparation_params_ptr, sizeof(ECPreparation_params_t));
			break;		
		
		case IOCTL_MWMAC_ECMDJACOBITRANSFORM:
			rc = copy_from_user(ECMDJacobiTransform_params_ptr, (void *)arg, sizeof(ECMDJacobiTransform_params_t));
			MWMAC_ECMDJacobiTransform(ECMDJacobiTransform_params_ptr);
			rc = copy_to_user((void *)arg, ECMDJacobiTransform_params_ptr, sizeof(ECMDJacobiTransform_params_t));
			break;

		case IOCTL_MWMAC_ECMDPOINTDBL:
			rc = copy_from_user(ECMDPointDBL_params_ptr, (void *)arg, sizeof(ECMDPointDBL_params_t));
			MWMAC_ECMDPointDBL(ECMDPointDBL_params_ptr);
			rc = copy_to_user((void *)arg, ECMDPointDBL_params_ptr, sizeof(ECMDPointDBL_params_t));
			break;					
					
		case IOCTL_MWMAC_ECMDPOINTADD:
			rc = copy_from_user(ECMDPointADD_params_ptr, (void *)arg, sizeof(ECMDPointADD_params_t));
			MWMAC_ECMDPointADD(ECMDPointADD_params_ptr);
			rc = copy_to_user((void *)arg, ECMDPointADD_params_ptr, sizeof(ECMDPointADD_params_t));
			break;		
		case IOCTL_MWMAC_ECMDJTOAFFINE:
			rc = copy_from_user(ECMDJtoAffine_params_ptr, (void *)arg, sizeof(ECMDJtoAffine_params_t));
			MWMAC_ECMDJtoAffine(ECMDJtoAffine_params_ptr);
			rc = copy_to_user((void *)arg, ECMDJtoAffine_params_ptr, sizeof(ECMDJtoAffine_params_t));
			break;
		case IOCTL_MWMAC_ECMONTBACK:
			rc = copy_from_user(ECMontBack_params_ptr, (void *)arg, sizeof(ECMontBack_params_t));
			MWMAC_ECMontBack(ECMontBack_params_ptr);
			rc = copy_to_user((void *)arg, ECMontBack_params_ptr, sizeof(ECMontBack_params_t));
			break;

		default:
			printk("unknown IOCTL 0x%x\n", cmd);

			// Free allocated kernel memory for defined CryptoCore Structs here...
			kfree(MontMult_params_ptr);
			kfree(MontR_params_ptr);
			kfree(MontR2_params_ptr);
			kfree(MontExp_params_ptr);
			kfree(ModAdd_params_ptr);
			kfree(ModSub_params_ptr);
			kfree(CopyH2V_params_ptr);
			kfree(CopyV2V_params_ptr);
			kfree(CopyH2H_params_ptr);
			kfree(CopyV2H_params_ptr);
			kfree(MontMult1_params_ptr);
			
			kfree(ECPreparation_params_ptr);
			kfree(ECMDJacobiTransform_params_ptr);
			kfree(ECMDPointDBL_params_ptr);
			kfree(ECMDPointADD_params_ptr);
			kfree(ECMDJtoAffine_params_ptr);
			kfree(ECMontBack_params_ptr);
			
			return -EINVAL;
	}
	
	// Free allocated kernel memory for defined CryptoCore Structs here...
	kfree(MontMult_params_ptr);
	kfree(MontR_params_ptr);
	kfree(MontR2_params_ptr);
	kfree(MontExp_params_ptr);
	kfree(ModAdd_params_ptr);
	kfree(ModSub_params_ptr);
	kfree(CopyH2V_params_ptr);
	kfree(CopyV2V_params_ptr);
	kfree(CopyH2H_params_ptr);
	kfree(CopyV2H_params_ptr);
	kfree(MontMult1_params_ptr);
				
	kfree(ECPreparation_params_ptr);
	kfree(ECMDJacobiTransform_params_ptr);
	kfree(ECMDPointDBL_params_ptr);
	kfree(ECMDPointADD_params_ptr);
	kfree(ECMDJtoAffine_params_ptr);
	kfree(ECMontBack_params_ptr);
			
	return 0;
}

static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = cryptocore_driver_open,
   .release = cryptocore_driver_close,
   .unlocked_ioctl = cryptocore_driver_ioctl,
};

static int __init cryptocore_init( void )
{
   int value;

   if( alloc_chrdev_region(&cryptocore_dev_number, 0, 1, DRIVER_NAME) < 0 )
      return -EIO;

   driver_object = cdev_alloc(); /* Anmeldeobject reservieren */

   if( driver_object == NULL )
      goto free_device_number;

   driver_object->owner = THIS_MODULE;
   driver_object->ops = &fops;

   if( cdev_add(driver_object, cryptocore_dev_number, 1) )
      goto free_cdev;

   cryptocore_class = class_create( THIS_MODULE, DRIVER_NAME );

   if( IS_ERR( cryptocore_class ) ) {
      pr_err( "cryptocore: no udev support\n");
      goto free_cdev;
   }

   cryptocore_dev = device_create(cryptocore_class, NULL, cryptocore_dev_number, NULL, "%s", DRIVER_NAME );
   dev_info( cryptocore_dev, "cryptocore_init called\n" );

   if(!(request_mem_region(GPIO1_BASE, GPIO1_SPAN, DRIVER_NAME))) {
      pr_err( "timer: request mem_region (GPIO1) failed!\n");
      goto fail_request_mem_region_1;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (LEDR) failed!\n");
      goto fail_request_mem_region_2;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (KEY) failed!\n");
      goto fail_request_mem_region_3;
   }

   if(!(request_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_RAM) failed!\n");
      goto fail_request_mem_region_4;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_CMD) failed!\n");
      goto fail_request_mem_region_5;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_IRQ) failed!\n");
      goto fail_request_mem_region_6;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_CMD) failed!\n");
      goto fail_request_mem_region_7;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_CTR) failed!\n");
      goto fail_request_mem_region_8;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_TSTAB) failed!\n");
      goto fail_request_mem_region_9;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_TSAMPLE) failed!\n");
      goto fail_request_mem_region_10;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_IRQ) failed!\n");
      goto fail_request_mem_region_11;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_FIFO) failed!\n");
      goto fail_request_mem_region_12;
   }

   if(!(HPS_GPIO1_ptr = ioremap(GPIO1_BASE, GPIO1_SPAN))) {
      pr_err( "cryptocore: ioremap (GPIO1) failed!\n");
      goto fail_ioremap_1;
   }

   if(!(LEDR_ptr = ioremap(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN))) {
      pr_err( "cryptocore: ioremap (LEDR) failed!\n");
      goto fail_ioremap_2;
   }

   if(!(KEY_ptr = ioremap(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN))) {
      pr_err( "cryptocore: ioremap (KEY) failed!\n");
      goto fail_ioremap_3;
   }

   if(!(MWMAC_RAM_ptr = ioremap(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_RAM) failed!\n");
      goto fail_ioremap_4;
   }

   if(!(MWMAC_CMD_ptr = ioremap(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_CMD) failed!\n");
      goto fail_ioremap_5;
   }

   if(!(MWMAC_IRQ_ptr = ioremap(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_IRQ) failed!\n");
      goto fail_ioremap_6;
   }

   if(!(TRNG_CMD_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_CMD) failed!\n");
      goto fail_ioremap_7;
   }

   if(!(TRNG_CTR_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_CTR) failed!\n");
      goto fail_ioremap_8;
   }

   if(!(TRNG_TSTAB_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_TSTAB) failed!\n");
      goto fail_ioremap_9;
   }

   if(!(TRNG_TSAMPLE_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_TSAMPLE) failed!\n");
      goto fail_ioremap_10;
   }

   if(!(TRNG_IRQ_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_IRQ) failed!\n");
      goto fail_ioremap_11;
   }

   if(!(TRNG_FIFO_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_FIFO) failed!\n");
      goto fail_ioremap_12;
   }

   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x00000001, MWMAC_IRQ_ptr+3); 
   // Enable IRQ generation for the CryptoCore
   iowrite32(0x00000001, MWMAC_IRQ_ptr+2); 

   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x00000001, TRNG_IRQ_ptr+3);
   // Enable IRQ generation for the CryptoCore TRNG
   iowrite32(0x00000001, TRNG_IRQ_ptr+2);

   
   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x0000000F, KEY_ptr+3);
   // Enable IRQ generation for the 4 buttons
   iowrite32(0x0000000F, KEY_ptr+2);

   value = request_irq (KEY_IRQ, (irq_handler_t) key_irq_handler, IRQF_SHARED, "cryptocore_key_irq_handler", (void *) (key_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (KEY) failed!\n");
      goto fail_irq_1;
   }

   value = request_irq (MWMAC_IRQ, (irq_handler_t) mwmac_irq_handler, IRQF_SHARED, "cryptocore_mwmac_irq_handler", (void *) (mwmac_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (MWMAC) failed!\n");
      goto fail_irq_2;
   }

   value = request_irq (TRNG_IRQ, (irq_handler_t) trng_irq_handler, IRQF_SHARED, "cryptocore_trng_irq_handler", (void *) (trng_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (TRNG) failed!\n");
      goto fail_irq_3;
   } 

   // Turn on green User LED
   iowrite32(0x01000000, (HPS_GPIO1_ptr+1));
   iowrite32(0x01000000, (HPS_GPIO1_ptr));

   return 0;

fail_irq_3:
   free_irq (MWMAC_IRQ, (void*) mwmac_irq_handler);

fail_irq_2:
   free_irq (KEY_IRQ, (void*) key_irq_handler);

fail_irq_1:
   iounmap(TRNG_FIFO_ptr);

fail_ioremap_12:
   iounmap(TRNG_IRQ_ptr);

fail_ioremap_11:
   iounmap(TRNG_TSAMPLE_ptr);

fail_ioremap_10:
   iounmap(TRNG_TSTAB_ptr);

fail_ioremap_9:
   iounmap(TRNG_CTR_ptr);

fail_ioremap_8:
   iounmap(TRNG_CMD_ptr);

fail_ioremap_7:
   iounmap(MWMAC_IRQ_ptr);

fail_ioremap_6:
   iounmap(MWMAC_CMD_ptr);

fail_ioremap_5:
   iounmap(MWMAC_RAM_ptr);

fail_ioremap_4:
   iounmap(KEY_ptr);

fail_ioremap_3:
   iounmap(LEDR_ptr);

fail_ioremap_2:
   iounmap(HPS_GPIO1_ptr);

fail_ioremap_1:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN);

fail_request_mem_region_12:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN);

fail_request_mem_region_11:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN);

fail_request_mem_region_10:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN);

fail_request_mem_region_9:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN);

fail_request_mem_region_8:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN);

fail_request_mem_region_7:
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN);

fail_request_mem_region_6:
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN);

fail_request_mem_region_5:
   release_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN);

fail_request_mem_region_4:
   release_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN);

fail_request_mem_region_3:
   release_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN);

fail_request_mem_region_2:
   release_mem_region(GPIO1_BASE, GPIO1_SPAN);

fail_request_mem_region_1:
   device_destroy( cryptocore_class, cryptocore_dev_number );
   class_destroy( cryptocore_class );

free_cdev:
   kobject_put( &driver_object->kobj );

free_device_number:
   unregister_chrdev_region( cryptocore_dev_number, 1 );
   return -EIO;
}

static void __exit cryptocore_exit( void )
{
   dev_info( cryptocore_dev, "cryptocore_exit called\n" );

   iowrite32(0x00000000, (LEDR_ptr));
   iowrite32(0x00000000, (HPS_GPIO1_ptr));
   iowrite32(0x00000000, (HPS_GPIO1_ptr+1));

   free_irq (KEY_IRQ, (void*) key_irq_handler);
   free_irq (MWMAC_IRQ, (void*) mwmac_irq_handler);
   free_irq (TRNG_IRQ, (void*) trng_irq_handler);

   iounmap(TRNG_FIFO_ptr);
   iounmap(TRNG_IRQ_ptr);
   iounmap(TRNG_TSAMPLE_ptr);
   iounmap(TRNG_TSTAB_ptr);
   iounmap(TRNG_CTR_ptr);
   iounmap(TRNG_CMD_ptr);
   iounmap(MWMAC_IRQ_ptr);
   iounmap(MWMAC_CMD_ptr);
   iounmap(MWMAC_RAM_ptr);
   iounmap(KEY_ptr);
   iounmap(LEDR_ptr);
   iounmap(HPS_GPIO1_ptr);

   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN);
   release_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN);
   release_mem_region(GPIO1_BASE, GPIO1_SPAN);

   device_destroy( cryptocore_class, cryptocore_dev_number );
   class_destroy( cryptocore_class );

   cdev_del( driver_object );
   unregister_chrdev_region( cryptocore_dev_number, 1 );

   return;
}

static void Clear_MWMAC_RAM(void)
{
	u32 value;
	u32 i;
	
	value = 0x00000000;
	
	for(i=0; i<128; i++){
		// Clear B
		iowrite32(value, (MWMAC_RAM_ptr+0x3+i*0x4));
		// Clear P
		iowrite32(value, (MWMAC_RAM_ptr+0x2+i*0x4));
		// Clear TS
		iowrite32(value, (MWMAC_RAM_ptr+0x1+i*0x4));
		// Clear TC
		iowrite32(value, (MWMAC_RAM_ptr+0x0+i*0x4));
		// Clear A
		iowrite32(value, (MWMAC_RAM_ptr+0x200+i));
		// Clear E
		iowrite32(value, (MWMAC_RAM_ptr+0x280+i));
		// Clear X
		iowrite32(value, (MWMAC_RAM_ptr+0x300+i));
	}
	
}

static void MWMAC_MontMult(MontMult_params_t *MontMult_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontMult_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontMult_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontMult_params_ptr->prec % 32) {
					rw_prec = (MontMult_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontMult_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontMult_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontMult_params_ptr->prec;
			}
		}
	}	
	
	if(MontMult_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}	
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	
	// Write Parameter a to A Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+i));
	}

	// Write Parameter b to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}	
	
	// MontMult(A1, B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
	//			src_addr     			dest_addr    		src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontMult_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_MontR(MontR_params_t *MontR_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontR_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontR_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontR_params_ptr->prec % 32) {
					rw_prec = (MontR_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontR_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontR_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontR_params_ptr->prec;
			}
		}
	}	
	
	if(MontR_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontR_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result r from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontR_params_ptr->r[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_MontR2(MontR2_params_t *MontR2_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontR2_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontR2_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontR2_params_ptr->prec % 32) {
					rw_prec = (MontR2_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontR2_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontR2_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontR2_params_ptr->prec;
			}
		}
	}			
	
	if(MontR2_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontR2_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// MontR2(P1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR2 << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result r from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontR2_params_ptr->r2[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_MontExp(MontExp_params_t *MontExp_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontExp_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontExp_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontExp_params_ptr->prec % 32) {
					rw_prec = (MontExp_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontExp_params_ptr->prec;
				}	
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontExp_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontExp_params_ptr->prec;
			}
		}
	}	
	
	if(MontExp_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// Write Parameter e to E Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->e[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
	}	

	// Write Parameter b to X Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+i));
	}
	
	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// MontExp(A1, B1, E1, X1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTEXP << 8) 
	//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (MWMAC_RAM_E1 << 22) | (MWMAC_RAM_X1 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontExp_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_ModAdd(ModAdd_params_t *ModAdd_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(ModAdd_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ModAdd_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ModAdd_params_ptr->prec % 32) {
					rw_prec = (ModAdd_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ModAdd_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ModAdd_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ModAdd_params_ptr->prec;
			}
		}
	}		
	
	if(ModAdd_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
		
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModAdd_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	
	// Write Parameter a to TS Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModAdd_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x1+i*0x4));
	}	

	// Write Parameter b to TC Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModAdd_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x0+i*0x4));
	}	
	
	// ModAdd(TS1, TC1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODADD << 8) 
	//			src_addr      			dest_addr    src_addr_e   src_addr_x
			| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ModAdd_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_ModSub(ModSub_params_t *ModSub_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(ModSub_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ModSub_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ModSub_params_ptr->prec % 32) {
					rw_prec = (ModSub_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ModSub_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ModSub_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ModSub_params_ptr->prec;
			}
		}
	}		
	
	if(ModSub_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModSub_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	
	// Write Parameter a to TS Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModSub_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x1+i*0x4));
	}	

	// Write Parameter b to TC Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModSub_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x0+i*0x4));
	}	
	
	// ModSub(TS1, TC1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
	//			src_addr      			dest_addr    src_addr_e   src_addr_x
			| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;	
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ModSub_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

static void MWMAC_CopyH2V(CopyH2V_params_t *CopyH2V_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(CopyH2V_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==CopyH2V_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(CopyH2V_params_ptr->prec % 32) {
					rw_prec = (CopyH2V_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = CopyH2V_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==CopyH2V_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = CopyH2V_params_ptr->prec;
			}
		}
	}	
	
	if(CopyH2V_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter a to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(CopyH2V_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}
	
	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result acopy from A Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		CopyH2V_params_ptr->acopy[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x200+i);
	} 
}

static void MWMAC_CopyV2V(CopyV2V_params_t *CopyV2V_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(CopyV2V_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==CopyV2V_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(CopyV2V_params_ptr->prec % 32) {
					rw_prec = (CopyV2V_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = CopyV2V_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==CopyV2V_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = CopyV2V_params_ptr->prec;
			}
		}
	}			
	
	if(CopyV2V_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
		
	Clear_MWMAC_RAM();

	// Write Parameter a to A Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(CopyV2V_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+i));
	}
	
	// CopyV2V(A1, X1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2V << 8) 
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_A1 << 12) | (MWMAC_RAM_X1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result acopy from X Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		CopyV2V_params_ptr->acopy[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x300+i);
	} 
}

static void MWMAC_CopyH2H(CopyH2H_params_t *CopyH2H_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;
	
	if(CopyH2H_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==CopyH2H_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(CopyH2H_params_ptr->prec % 32) {
					rw_prec = (CopyH2H_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = CopyH2H_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==CopyH2H_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = CopyH2H_params_ptr->prec;
			}
		}
	}		
	
	if(CopyH2H_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	Clear_MWMAC_RAM();

	// Write Parameter a to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(CopyH2H_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}
	
	// CopyH2H(B1, TS1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
	//			src_addr      			dest_addr    			src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result acopy from TS Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		CopyH2H_params_ptr->acopy[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x1+i*0x4);
	} 	

}

static void MWMAC_CopyV2H(CopyV2H_params_t *CopyV2H_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;
	
	if(CopyV2H_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==CopyV2H_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(CopyV2H_params_ptr->prec % 32) {
					rw_prec = (CopyV2H_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = CopyV2H_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==CopyV2H_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = CopyV2H_params_ptr->prec;
			}
		}
	}		
	
	if(CopyV2H_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
		
	Clear_MWMAC_RAM();

	// Write Parameter a to A Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(CopyV2H_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+i));
	}
	
	// CopyV2H(A1, TS1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
	//			src_addr      			dest_addr    		src_addr_e   src_addr_x
			| (MWMAC_RAM_A1 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
	
	// Read Result acopy from TS Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		CopyV2H_params_ptr->acopy[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x1+i*0x4);
	} 
}

static void MWMAC_MontMult1(MontMult1_params_t *MontMult1_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontMult1_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontMult1_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontMult1_params_ptr->prec % 32) {
					rw_prec = (MontMult1_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontMult1_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontMult1_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontMult1_params_ptr->prec;
			}
		}
	}			
	
	if(MontMult1_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
		
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult1_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}
	
	// Write Parameter b to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontMult1_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}	
	
	// MontMult1(B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT1 << 8) 
	//			src_addr      dest_addr    src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;		
	
	// Read Result c from B Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		MontMult1_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} 
}

// Add further CryptoCore Functions here...
/*Her function basindaki tanimlar eksik. en basta, baslangicta 1 clear command olmali. READ degerleri de eksik. BUNLARI unutma */

// Add further CryptoCore Functions here...

/*  In General Case..
	We will focus to prime precisions GF(p) so:
		mwmac_f_sel -> 1 
	We care about timing..
		mwmac_sec_calc -> 0
	For ECC support precisions up to 512 bits
	Considering time issues 192 bit could be chosen.. but, to work with (.. [rw_prec/32-1-i]..) for easy implementations.
		mwmac_precision -> 0101 = 0x5
	operation will be chosen later..
		mwmac_op -> ????
		
		
Sorular:
	1- işlemlerde sonuç C, B üzerine yazılıyor. Bu yer B ile yapılan işlemin yerine mi geçiyor yoksa default olarak B1 yerine mi yazılıyor??
	2- Eger P = Q gibi bir durum olursa Point Doubling yapmak zorunda kalıyoruz..
		
		
		 */
// Start to Code for Point Addition:


/* !!!!!!!!!!! REGISTER MANAGEMENT AND LOOK UP TABLE  !!!!!!!!!!!!!!!!!!!

Steps of proper algorithm :
	
		GF(p) EC Preparation:
			in : (2,3,4,8,a,b,p)
			
			name 	op						register
			p		write					P1, P2...
			P			COPY H2H			COPY(P1,P2...)

			r**2 <- MontR2(p)				MontR2(P1, A2)// There is Clear_MWMAC_RAM(); command in MontR2 !!!!! ????????????????? + MontR2 saves A1 and B1
						
			2		write					B1
			MD_2 <- MontMult(r**2,2,p)		MontMult(A2, B1, P1)
			A1			COPY H2V			COPY(B1,A1)
						
			3		write					B1
			MD_2 <- MontMult(r**2,2,p)		MontMult(A2, B1, P1)
			A3			COPY H2V			COPY(B1,A3)

			4		write					B1
			MD_2 <- MontMult(r**2,2,p)		MontMult(A2, B1, P1)
			A4			COPY H2V			COPY(B1,A4)
						
			8		write					B1
			MD_2 <- MontMult(r**2,2,p)		MontMult(A2, B1, P1)
			A5			COPY H2V			COPY(B1,A5)
												
			a		write					B1
			a_MD <- MontMult(r**2,a,p)		MontMult(A2, B1, P1)
			A6			COPY H2V			COPY(B1,A6)					
							
			b		write					B1			
			b_MD <- MontMult(r**2,b,p)		MontMult(A2, B1, P1)
			A7			COPY H2V			COPY(B1,A7)
									
						COPY H2H			COPY(P1,TS1)
			2		write					TC1
			exp  <- ModSub(p,2,p)			ModSub(TS1, TC1, P1) //ModSub(TS1, TC1, P1)  TS1 & TC1 are default registers, P1 is source param.
			A8			COPY H2V			COPY(B1, A8)
		
			REGISTERS:
				P1,P2.. <-- P			A4	<-- 4_MD		A8	<-- exp
				A1		<-- 2_MD		A5	<-- 8_MD
				A2		<-- R**2		A6	<-- a_MD
				A3		<-- 3_MD		A7	<-- b_MD
			
			1xMontR, 1x MontR2, 6xMontMult, 1xMontSub, 2xCopyH2H
		
From here P. Addition, P.Doubling or P.Mult. can be done in different ways
	For P.Addition:	
	
		GF(p) EC Jacobi-Montgomery Transformation: 
		// Jacobi Transform is done by just introducing extra z_MD, x_MD & y_MD remains same 
			in : (xp, yp, xq, yq, r**2)
			
			
			name		op						register
			x_pG		write					B8
			x_PMDJ <- MontMult(x_p, r**2,p)	MontMult(A2, B8, P8)	
						COPY V2H			COPY(B8,X8)
			
			y_pG		write					B7
			y_PMDJ <- MontMult(y_p, r**2,p)	MontMult(A2, B7, P7)
						COPY V2H			COPY(B7,X7)
			
			z_PMDJ <- MontR(p)				MontR(P6,B6)
						COPY H2V			COPY(B6,X6)
		
			x_qG		write					B5
			x_QMDJ <- MontMult(x_p, r**2,p)	MontMult(A2, B5, P5)	
						COPY V2H			COPY(B5,X5)
		
			y_qG		write					B4
			y_QMDJ <- MontMult(y_p, r**2,p)	MontMult(A2, B4, P4) 
						COPY H2H			COPY(B4,X4)
			
			z_QMDJ <- MontR(p)				MontR(P3,B3)
						COPY H2V			COPY(B3,X3)
									
			return: (x_PMDJ, y_PMDJ, z_PMDJ, x_QMDJ, y_QMDJ, z_QMDJ)
			
			REGISTERS:
				B8, X8	<-- x_PMDJ		B5,	X5	<-- x_QMDJ
				B7, X7	<-- y_PMDJ		B4,	X4	<-- y_QMDJ
				B6,	X6	<-- z_PMDJ		B3,	X3	<-- z_QMDJ
			
			4xMontMult, 2xMontR, 4xCopyV2V, 2xCopyH2H  for Point Addition 
			
just an idea
			Here we may make some changes.. As professor suggested a trick we can 
		copy V2V from X4 to X1, X5 to X2, X6 to X3 registers. So that, we can generalize 
		this funciton for point doubling or multiplication too. 
			Let's say we insert Xp -> X4 and copied to X1, Yp -> X5 copied to X2, Zp -> X6 copied to X3
		now, we can use X4,X5,X6 registers again in Point ADD for Q..	

	Point Validation
	Point Multiplication are skipped for now..
	
		GF(p) EC Point Doubling	in MD	
		
		    name          op                            register
		
		    E8  <-  MontMult(X_PMDJ, X_PMDJ, p)     MontMult(X8, B8, P8)
		                COPY H2V                        COPYH2V(B8, E8)
		
		    E7  <-  MontMult(Y_PMDJ, Y_PMDJ, p)     MontMult(X7, B7, P7)
		                COPY H2V                        COPYH2V(B7, E7)
		
		    E6  <-  MontMult(E7, E7, p)             MontMult(E7, B7, P7)
		                COPY H2V                        COPYH2V(B7, E6)
		
		    E5  <-  MontMult(Z_PMDJ, Z_PMDJ, p)     MontMult(X6, B6, P6)
		                COPY H2V                        COPYH2V(B6, E5)
																				
						COPY	V2H					    COPYV2H(X8, TS1)
						COPY	V2H					    COPYV2H(E7, TC1)
		    E4  <-  ModAdd(X_PMDJ, E7, p)           ModAdd(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E4)
		
		    E3  <-  MontMult(E4, E4, p)             MontMult(E4, B1, P1)
		                COPY H2V                        COPYH2V(B1, E3)
		
						COPY	V2H					    COPYV2H(E3, TS1)
						COPY	V2H					    COPYV2H(E8, TC1)
		    E4  <-  ModSub(E3, E8, p)				ModSub(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E4)
		
						COPY	V2H					    COPYV2H(E4, TS1)
						COPY	V2H					    COPYV2H(E6, TC1)
		    E3  <-  ModSub(E4, E6, p)				ModSub(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E3)
		
		    E4  <-  MontMult(2_MD, E3, p)            MontMult(A1, B1, P1)
		                COPY H2V                        COPYH2V(B1, E4)
		
		    E3  <-  MontMult(3_MD, E8, p)            MontMult(A3, B8, P8)
		                COPY H2V                        COPYH2V(B8, E3)
		
		    E2  <-  MontMult(E5, E5, p)              MontMult(E5, B6, P6)
		                COPY H2V                        COPYH2V(B6, E2)
		
		    E1  <-  MontMult(a_MD, E2, p)            MontMult(A6, B6, P6)
		                COPY H2V                        COPYH2V(B6, E1)
		
						COPY	V2H					    COPYV2H(E3, TS1)
						COPY	V2H					    COPYV2H(E1, TC1)
		    E2  <-  ModAdd(E3, E1, p)                 ModAdd(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E2)
		
		    E3  <-  MontMult(E2, E2, p)               MontMult(E2, B1, P1)
		               COPY H2V                          COPYH2V(B1, E3)
		
		              COPY V2H                           COPYV2H(E4, B2)
		    E1  <-  MontMult(2_MD, E4, p)             MontMult(A1, B2, P2)
		              COPY H2V                           COPYH2V(B2, E1)
		
						COPY	V2H					    COPYV2H(E3, TS1)
						COPY	V2H					    COPYV2H(E1, TC1)
		 XR =E8  <-  ModSub(E3, E1, p)			 	 ModSub(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E8)
		
						COPY	V2H					    COPYV2H(E4, TS1)
						COPY	V2H					    COPYV2H(E8, TC1)
		    E1  <-  ModSub(E4, E8, p)				ModSub(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E1)
		
		    E4  <-  MontMult(E2, E1, p)               MontMult(E2, B1, P1)
		                COPY H2V                        COPYH2V(B1, E4)
		
		    E3  <-  MontMult(8_MD, E6, p)             MontMult(A5, B7, P7)
		                COPY H2V                        COPYH2V(B7, E3)
		
						COPY	V2H					    COPYV2H(E4, TS1)
						COPY	V2H					    COPYV2H(E3, TC1)
		YR = E6 <-  ModSub(E4, E3, p)			 	 ModSub(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E6)
		
						COPY	V2H					    COPYV2H(X7, TS1)
						COPY	V2H					    COPYV2H(X6, TC1)
		    E3  <-  ModAdd(Y_PMDJ, Z_PMDJ)          ModAdd(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E3)
		
		    E2  <-  MontMult(E3, E3, p)                MontMult(E3, B1, P1)
		                COPY H2V                        COPYH2V(B1, E2)
		
						COPY	V2H					    COPYV2H(E2, TS1)
						COPY	V2H					    COPYV2H(E7, TC1)
		    E3  <-  ModSub(E2, E7, p)			 	 ModSub(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E3)
		
						COPY	V2H					    COPYV2H(E3, TS1)
						COPY	V2H					    COPYV2H(E5, TC1)
		ZR  = E7 <- ModSub(E3, E5, p)			 	 ModSub(TS1, TC1, P1)
		                COPY H2V                        COPYH2V(B1, E7)
		
		                COPY V2V                        COPYV2V(E6, E5)  // YR = E5
		                COPY V2V                        COPYV2V(E8, E6)  // XR = E6  
		
		                			
					REGISTERS (IMPORTANTS):
						E5	<-	y_RMDJ
						E6	<-	x_RMDJ
						E7	<-	z_RMDJ				
		             
		

		
		GF(p) EC Point Addition	in MD
			
			 if P == Q then call point_double
			 or the inf thing
			
			name   		  op						register
			E8	<-	MontMult(z_PMDJ, z_PMDJ, p)		MontMult(X6, B6, P6)
						COPY	H2V					COPY(B6, E8)
			E7	<-	MontMult(z_QMDJ, z_QMDJ, p)		MontMult(X3, B3, P3)
						COPY	H2V					COPY(B3, E7)				
			E6	<-	MontMult(E7**	, x_PMDJ, p)	MontMult(E7**, B8, P8)
						COPY	H2V					COPY(B8, E6)
			E5	<-	MontMult(E8**	, x_QMDJ, p)	MontMult(E8**, B5, P5)
						COPY	H2V					COPY(B5, E5)
			E4	<-	MontMult(z_QMDJ, y_PMDJ, p)		MontMult(X3, B7, P7)
						COPY	H2V					COPY(B7, E4)
			E3	<-	MontMult(E7, E4, p) 			MontMult(E7**, B7, P7)  	//B7 is used because I suppose result of previous calculation is still there
						COPY	H2V					COPY(B7, E3)
			E4	<-	MontMult(z_PMDJ, y_QMDJ, p)		MontMult(X6, B4, P4)
						COPY	H2V					COPY(B4, E4)
			E2	<-	MontMult(E8, E4, p)				MontMult(E8, B4, P4)
						COPY 	H2V					COPY(B4, E2)
						
						COPY	V2H					COPY(E5, TS1)
						COPY	V2H					COPY(E6, TC1)
			E4	<-	ModSub(E5, E6 ,p)				ModSub(TS1, TC1, P1)
						COPY	H2V					COPY(B1, E4)
			E5	<-	MontMult(2_MD, E4, p)			MontMult(A1, B1, P1)
						COPY	H2V					COPY(B1, E5)
			E1	<-	MontMult(E5, E5, p)				MontMult(E5, B1, P1)
						COPY	H2V					COPY(B1, E1)
			E5	<-	MontMult(E4, E1, p)				MontMult(E4, B1, P1)
						COPY	H2V					COPY(B1, E5)			
			
						COPY	V2H					COPY(E2, TS1)
						COPY	V2H					COPY(E3, TC1)
			X2	<-	ModSub(E2, E3 ,p)				ModSub(TS1, TC1, P1)
						COPY	H2V					COPY(B1, X2)
			E2	<-	MontMult(2_MD, X2, p)			MontMult(A1, B1, P1)
						COPY	H2V					COPY(B1, E2)
			X2	<-	MontMult(E1, E6, p)				MontMult(E1, B8, P8)	// result of the E6 is still ready on B8			
						COPY	H2V					COPY(B8, X2)
			E6	<-	MontMult(E2, E2, p)				MontMult(E2, B1, P1)
						COPY	H2V					COPY(B1, E6)
			E1	<-	MontMult(2_MD, X2, p)			MontMult(A1, B8, P8)
						COPY	H2V					COPY(B8, E1)
	
						COPY	V2H					COPY(E6, TS1)
						COPY	V2H					COPY(E5, TC1)
			X1	<-	ModSub(E6, E5 ,p)				ModSub(TS1, TC1, P1)
						COPY	H2V					COPY(B1, X1)
	
						COPY	V2H					COPY(X1, TS1)
						COPY	V2H					COPY(E1, TC1)
X_RMDJ = 	E6	<-	ModSub(X1, E1 ,p)				ModSub(TS1, TC1, P1)
						COPY	H2V					COPY(B1, E6)
	
						COPY	V2H					COPY(X2, TS1)
						COPY	V2H					COPY(E6, TC1)
			E1	<-	ModSub(X2, E6 ,p)				ModSub(TS1, TC1, P1)
						COPY	H2V					COPY(B1, E1)
			X1	<-	MontMult(E2, E1, p)				MontMult(E2, B1, P1)
						COPY	H2V					COPY(B1, X1)
			X2	<-	MontMult(2_MD, E3, p)			MontMult(A1, B7, P7)
						COPY	H2V					COPY(B7, X2)
			E1	<-	MontMult(E5, X2, p)				MontMult(E5, B7, P7)
						COPY 	H2V					COPY(B7, E1)
	
						COPY	V2H					COPY(X1, TS1)
						COPY	V2H					COPY(E1, TC1)
Y_RMDJ =	E5	<-	ModSub(X1, E1 ,p)				ModSub(TS1, TC1, P1)
						COPY	H2V					COPY(B1, E5)
																		
						COPY	V2H					COPY(X6, TS1)
						COPY	V2H					COPY(X3, TC1)
			E3	<-	ModAdd(X6, X3 ,p)				ModAdd(TS1, TC1, P1)
						COPY	H2V					COPY(B1, E3)
			X1	<-	MontMult(E3, E3, p)				MontMult(E3, B1, P1)
						COPY	H2V					COPY(B1, X1)
			
						COPY	V2H					COPY(X1, TS1)
						COPY	V2H					COPY(E8, TC1)
			X2	<-	ModSub(X1, E8 ,p)				ModSub(TS1, TC1, P1)
						COPY	H2V					COPY(B1, X2)

						COPY	V2H					COPY(X2, TS1)
						COPY	V2H					COPY(E7, TC1)
			E1	<-	ModSub(X2, E7 ,p)				ModSub(TS1, TC1, P1)
						COPY	H2V					COPY(B1, E1)
Z_RMDJ	=	E7	<-	MontMult(E4, E1, p)				MontMult(E4, B1, P1)
						COPY	H2V					COPY(B1, E7)
			
			REGISTERS (IMPORTANTS):
				E5	<-	y_RMDJ
				E6	<-	x_RMDJ
				E7	<-	z_RMDJ				

	
			
												
		GF(p) EC Jacobi-to-Affine Transformation
			in: (x_RMDJ, y_RMDJ, ZRMDJ,p, exp)
			
			name     op											register
						COPY z
						COPY z
						COPY z
						COPY exp
			t1_MD <- MontExp(Z_RMDJ,Z_RMDJ,exp,Z_RMDJ,p)
					ther are already @ A1,B1
			t2_MD <- MontMult(t1_MD, t1_MD, p)
						COPY t2_save 
			t3_MD <- MontMult(t1_MD, t2_MD, p)
			y_RMD <- MontMult(y_RMDJ, t3_MD, p)
						COPY
						COPY t2
			x_RMD <- MontMult(x_RMDJ, t2_MD, p)
						COPY
			return(x_RMD, y_RMD)
		
		GF(p) EC Montgomery Back Transformation
			in: (x_RMD, y_RMD)
			
			name     op											register
			x_R	 <- MontMult1(x_R,p)
					COPY
			y_R	 <- MontMult1(y_R,p)
					COPY
		 */
		 


//GF(p) EC Preparation:  in : (2,3,4,8,a,b,p)	out: (r**2, MD_2, MD_3, MD_4, MD_8, a_MD, b_MD, exp)	 
static void MWMAC_ECPreparation(ECPreparation_params_t *ECPreparation_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0; // ???? There is mwmac_cmd[0] or mwmac_start??????
	u32 mwmac_cmd_prec = 0; // ???? again mwmac_precision ?????
	u32 mwmac_f_sel = 1; 
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;  // ???? what is rw_prec ?????????????????????????????????????????

	if(ECPreparation_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ECPreparation_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ECPreparation_params_ptr->prec % 32) {
					rw_prec = (ECPreparation_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ECPreparation_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ECPreparation_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ECPreparation_params_ptr->prec;
			}
		}
	}			
	
	if(ECPreparation_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	Clear_MWMAC_RAM(); 

// Parameters ----> Registers:
	// Write Parameter n to P Register ???? n is written
//P1
		for(i=0; i<rw_prec/32; i++){
			iowrite32(ECPreparation_params_ptr->p[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4)); // ???????? Here the "p" instead of n??
		}
	//Copy(P1,P2)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
		//			src_addr      			dest_addr    			src_addr_e   src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_P2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	//Copy(P1,P3)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
		//			src_addr      			dest_addr    			src_addr_e   src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_P3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	//Copy(P1,P4)	
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
		//			src_addr      			dest_addr    			src_addr_e   src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_P4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	//Copy(P1,P5)	
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
		//			src_addr      			dest_addr    			src_addr_e   src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_P5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	//Copy(P1,P6)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
		//			src_addr      			dest_addr    			src_addr_e   src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_P6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	//Copy(P1,P7)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
		//			src_addr      			dest_addr    			src_addr_e   src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_P7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	//Copy(P1,P8)	
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
		//			src_addr      			dest_addr    			src_addr_e   src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_P8 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//R2	
	//MontR2(P1, A2)			r**2 <- MontR2(p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR2 << 8)
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | 	(0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;

//2_MD

		// 	Write Parameter 2 to B1 Register  .. 1 Word is enough for this, rest is 0
		iowrite32(2,(MWMAC_RAM_ptr+0x3));				//Write number to first register
		for(i=1; i<rw_prec/32; i++){
			iowrite32(0, (MWMAC_RAM_ptr+0x3+i*0x4)); 	//Write 0 to rest..
		}

	//MontMult(A2, B1, P1)		MD_2 <- MontMult(r**2,2,p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B1,A1)
			//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
	
//3_MD
		// 	Write Parameter 3 to B1 Register 
		iowrite32(3,(MWMAC_RAM_ptr+0x3));
		for(i=1; i<rw_prec/32; i++){
			iowrite32(0, (MWMAC_RAM_ptr+0x3+i*0x4)); 
		}
		
	//MontMult(A2, B1, P1)		MD_3 <- MontMult(r**2,3,p)		
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	//COPYH2V(B1,A3)
			//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
//4_MD
		// 	Write Parameter 4 to B1 Register 
		iowrite32(4,(MWMAC_RAM_ptr+0x3));
		for(i=1; i<rw_prec/32; i++){
			iowrite32(0, (MWMAC_RAM_ptr+0x3+i*0x4)); 
		}
		
	//MontMult(A2, B1, P1)		MD_4 <- MontMult(r**2,4,p)		
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	//COPYH2V(B1,A4)
			//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
//8_MD
		// 	Write Parameter 8 to B1 Register 
		iowrite32(8,(MWMAC_RAM_ptr+0x3));
		for(i=1; i<rw_prec/32; i++){
			iowrite32(0, (MWMAC_RAM_ptr+0x3+i*0x4)); 
		}
					
	//MontMult(A2, B1, P1)		MD_8 <- MontMult(r**2,8,p)		
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	//COPYH2V(B1,A5)
			//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		
//a_MD
		// 	Write Parameter a to B1 Register 
		for(i=0; i<rw_prec/32; i++){
			iowrite32(ECPreparation_params_ptr->a[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4)); // ?????????? How we update parameter a or b?????????????*
		}
	
	//MontMult(A2, B1, P1)		a_MD <- MontMult(r**2,a,p)		
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B1,A6)
			//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
//b_MD
		// 	Write Parameter b to B1 Register 
		for(i=0; i<rw_prec/32; i++){
			iowrite32(ECPreparation_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4)); 
		}
		
	//MontMult(A2, B1, P1)		b_MD <- MontMult(r**2,b,p)		
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//COPYH2V(B1,A7)
			//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
			
//EXP
	// TC1 <-- 2
		// 	Write Parameter 2 to TC1 Register  .. 1 Word is enough for this, rest is 0
		iowrite32(2,(MWMAC_RAM_ptr+0x0));				//Write number to first register
		for(i=1; i<rw_prec/32; i++){
			iowrite32(0, (MWMAC_RAM_ptr+0x0+i*0x4)); 	//Write 0 to rest..
		}
	
	// CopyH2H(P1, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
		//			src_addr      			dest_addr    			src_addr_e   src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//ModSub(P1, TC1, P1)		exp  <- ModSub(p,2,p)	
		//ModSub(TS1, TC1, P1) 
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
		
	//COPYH2V(B1,A8)
			//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A8 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		/*
				P1,P2.. <-- P			A4	<-- 4_MD		A8	<-- exp
				A1		<-- 2_MD		A5	<-- 8_MD
				A2		<-- R**2		A6	<-- a_MD
				A3		<-- 3_MD		A7	<-- b_MD					*/
}
static void MWMAC_ECMDJacobiTransform(ECMDJacobiTransform_params_t *ECMDJacobiTransform_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0; // ???? There is mwmac_cmd[0] or mwmac_start??????
	u32 mwmac_cmd_prec = 0; // ???? again mwmac_precision ?????
	u32 mwmac_f_sel = 1; 
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;  // ???? what is rw_prec ?????????????????????????????????????????

	if(ECMDJacobiTransform_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ECMDJacobiTransform_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ECMDJacobiTransform_params_ptr->prec % 32) {
					rw_prec = (ECMDJacobiTransform_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ECMDJacobiTransform_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ECMDJacobiTransform_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ECMDJacobiTransform_params_ptr->prec;
			}
		}
	}			
	
	if(ECMDJacobiTransform_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
/*
????????????????????????????????????????????????????????????
in for loops xp yp xq and yq are used
???????????????????????????????????????????????
*/
//x_pG		write	B8
		// 	Write Parameter xp to B8 Register (0x1C3)
		for(i=0; i<rw_prec/32; i++){
			iowrite32(ECMDJacobiTransform_params_ptr->xp[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x1C3+i*0x4)); // ?????????? How we update parameter a or b?????????????*
		}
		
	//MontMult(A2, B8, P8)			x_PMDJ <- MontMult(x_p, r**2,p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B8,X8)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_X8 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
			
//y_pG		write	B7
		// 	Write Parameter yp to B7 Register (0x183)
		for(i=0; i<rw_prec/32; i++){
			iowrite32(ECMDJacobiTransform_params_ptr->yp[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x183+i*0x4)); // ?????????? How we update parameter a or b?????????????*
		}
		
	//MontMult(A2, B7, P7)			y_PMDJ <- MontMult(y_p, r**2,p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
	//COPYH2V(B7,X7)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_X7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;

//z_p				
	//MontR(P6,B6)					z_PMDJ <- MontR(p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_P6 << 12) | (MWMAC_RAM_B6 << 17) | (0x0 << 22) | 	(0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
	//COPYH2V(B6,X6)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B6 << 12) | (MWMAC_RAM_X6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
//x_qG		write	B5
		// 	Write Parameter xq to B5 Register (0x103)
		for(i=0; i<rw_prec/32; i++){
			iowrite32(ECMDJacobiTransform_params_ptr->xq[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x103+i*0x4)); // ?????????? How we update parameter a or b?????????????*
		}
		
	//MontMult(A2, B5, P5)		x_QMDJ <- MontMult(x_p, r**2,p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B5 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B5,X5)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B5 << 12) | (MWMAC_RAM_X5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
			
//y_qG		write	B4
		// 	Write Parameter yq to B4 Register  (0xC3)
		for(i=0; i<rw_prec/32; i++){
			iowrite32(ECMDJacobiTransform_params_ptr->yq[rw_prec/32-1-i], (MWMAC_RAM_ptr+0xC3+i*0x4)); // ?????????? How we update parameter a or b?????????????*
		}
		
	//MontMult(A2, B4, P4)		y_QMDJ <- MontMult(y_p, r**2,p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B4 << 12) | (MWMAC_RAM_A2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B4,X4)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B4 << 12) | (MWMAC_RAM_X4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//z_q	
	//MontR(P3,B3)					z_QMDJ <- MontR(p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_P3 << 12) | (MWMAC_RAM_B3 << 17) | (0x0 << 22) | 	(0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
	//COPYH2V(B3,X3)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B3 << 12) | (MWMAC_RAM_X3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	/*
				REGISTERS:
				B8, X8	<-- x_PMDJ		B5,	X5	<-- x_QMDJ
				B7, X7	<-- y_PMDJ		B4,	X4	<-- y_QMDJ
				B6,	X6	<-- z_PMDJ		B3,	X3	<-- z_QMDJ		*/
				
// Read Result x_pmdj from x8 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDJacobiTransform_params_ptr->x_pmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x370+i);
		} 
// Read Result y_pmdj from x7 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDJacobiTransform_params_ptr->y_pmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x360+i);
		} 
// Read Result z_pmdj from x6 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDJacobiTransform_params_ptr->z_pmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x350+i);
		} 
// Read Result x_qmdj from x5 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDJacobiTransform_params_ptr->x_qmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x340+i);
		} 
// Read Result y_qmdj from x4 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDJacobiTransform_params_ptr->y_qmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x330+i);
		} 
// Read Result z_qmdj from x3 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDJacobiTransform_params_ptr->z_qmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x320+i);
		} 							
}

static void MWMAC_ECMDPointDBL(ECMDPointDBL_params_t *ECMDPointDBL_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0; // ???? There is mwmac_cmd[0] or mwmac_start??????
	u32 mwmac_cmd_prec = 0; // ???? again mwmac_precision ?????
	u32 mwmac_f_sel = 1; 
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;  // ???? what is rw_prec ?????????????????????????????????????????

	if(ECMDPointDBL_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ECMDPointDBL_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ECMDPointDBL_params_ptr->prec % 32) {
					rw_prec = (ECMDPointDBL_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ECMDPointDBL_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ECMDPointDBL_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ECMDPointDBL_params_ptr->prec;
			}
		}
	}			
	
	if(ECMDPointDBL_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}

//E8
		//MontMult(X8, B8, P8)		    E8  <-  MontMult(X_PMDJ, X_PMDJ, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_X8 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		   
  
		//COPYH2V(B8, E8)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_E8 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E7		
		//MontMult(X7, B7, P7)		    E7  <-  MontMult(Y_PMDJ, Y_PMDJ, p) 
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_X7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

		//COPYH2V(B7, E7)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_E7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E6		
		//MontMult(E7, B7, P7)		    E6  <-  MontMult(E7, E7, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_E7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		             

		//COPYH2V(B7, E6)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_E6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E5		
		//MontMult(X6, B6, P6)		    E5  <-  MontMult(Z_PMDJ, Z_PMDJ, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B6 << 12) | (MWMAC_RAM_X6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		     

		//COPYH2V(B6, E5)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B6 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E4																				
		//COPYV2H(X8, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X8 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
						
		//COPYV2H(E7, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E7 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
		//ModAdd(TS1, TC1, P1)		    E4  <-  ModAdd(X_PMDJ, E7, p)
				//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODADD << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
	           

		//COPYH2V(B1, E4)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E3		
		//MontMult(E4, B1, P1)		    E3  <-  MontMult(E4, E4, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		             

		//COPYH2V(B1, E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		
//E4
		//COPYV2H(E3, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E3 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYV2H(E8, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E8 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ModSub(TS1, TC1, P1)		    E4  <-  ModSub(E3, E8, p)	
				//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
							

		//COPYH2V(B1, E4)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E3		
		//COPYV2H(E4, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E4 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYV2H(E6, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E6 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ModSub(TS1, TC1, P1)		    E3  <-  ModSub(E4, E6, p)
				//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
								

		//COPYH2V(B1, E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		
//E4         
		//MontMult(A1, B1, P1)		    E4  <-  MontMult(2_MD, E3, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		   

		//COPYH2V(B1, E4)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		
//E3
		//MontMult(A3, B8, P8)		    E3  <-  MontMult(3_MD, E8, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_A3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		            

		//COPYH2V(B8, E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E2		
		//MontMult(E5, B6, P6)		    E2  <-  MontMult(E5, E5, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B6 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		              

		//COPYH2V(B6, E2)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B6 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E1		
		//MontMult(A6, B6, P6)		    E1  <-  MontMult(a_MD, E2, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B6 << 12) | (MWMAC_RAM_A6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		            

		//COPYH2V(B6, E1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B6 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E2		
		//COPYV2H(E3, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E3 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYV2H(E1, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E1 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ModAdd(TS1, TC1, P1)		    E2  <-  ModAdd(E3, E1, p) 
				//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODADD << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
	                

		//COPYH2V(B1, E2)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E3		
		//MontMult(E2, B1, P1)		    E3  <-  MontMult(E2, E2, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		               

		//COPYH2V(B1, E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		
//E1
		//COPYV2H(E4, B2)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E4 << 12) | (MWMAC_RAM_B2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;

		//MontMult(A1, B2, P2)		    E1  <-  MontMult(2_MD, E4, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B2 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		             

		//COPYH2V(B2, E1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B2 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		
//XR = E8
		//COPYV2H(E3, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E3 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYV2H(E1, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E1 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ModSub(TS1, TC1, P1)		 XR =E8  <-  ModSub(E3, E1, p)	
				//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
						 	 

		//COPYH2V(B1, E8)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E8 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		
//E1
		//COPYV2H(E4, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E4 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYV2H(E8, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E8 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ModSub(TS1, TC1, P1)		    E1  <-  ModSub(E4, E8, p)	
				//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
							

		//COPYH2V(B1, E1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E4		
		//MontMult(E2, B1, P1)		    E4  <-  MontMult(E2, E1, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		               

		//COPYH2V(B1, E4)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E3		
		//MontMult(A5, B7, P7)		    E3  <-  MontMult(8_MD, E6, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_A5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		             

		//COPYH2V(B7, E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//YR= E6		
		//COPYV2H(E4, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E4 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYV2H(E3, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E3 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ModSub(TS1, TC1, P1)		YR = E6 <-  ModSub(E4, E3, p)
				//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
							 	 

		//COPYH2V(B1, E6)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E3		
		//COPYV2H(X7, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X7 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYV2H(X6, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X6 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ModAdd(TS1, TC1, P1)		    E3  <-  ModAdd(Y_PMDJ, Z_PMDJ)
				//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODADD << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
	          

		//COPYH2V(B1, E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E2		
		//MontMult(E3, B1, P1)		    E2  <-  MontMult(E3, E3, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		                

		//COPYH2V(B1, E2)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E3		
		//COPYV2H(E2, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E2 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYV2H(E7, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E7 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ModSub(TS1, TC1, P1)		    E3  <-  ModSub(E2, E7, p)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
									 	 

		//COPYH2V(B1, E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//ZR = E7		
		//COPYV2H(E3, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E3 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYV2H(E5, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E5 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ModSub(TS1, TC1, P1)		ZR  = E7 <- ModSub(E3, E5, p)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
									 	 

		//COPYH2V(B1, E7)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E5 <- E6		
		//COPYV2V(E6, E5)  	// YR = E5

		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2V << 8) 
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_E6 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
//E6 <- E8
		//COPYV2V(E8, E6)  // XR = E6
				//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2V << 8) 
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_E8 << 12) | (MWMAC_RAM_E6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		  
		  /*
			REGISTERS (IMPORTANTS):
									E5	<-	y_RMDJ
									E6	<-	x_RMDJ
									E7	<-	z_RMDJ			
		  */
// Read Result x_rmdj from E6 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDPointDBL_params_ptr->x_rmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x2D0+i);
		} 
// Read Result y_rmdj from E5 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDPointDBL_params_ptr->y_rmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x2C0+i);
		} 
// Read Result z_rmdj from E7 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDPointDBL_params_ptr->z_rmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x2E0+i);
		} 	
	
	
}
static void MWMAC_ECMDPointADD(ECMDPointADD_params_t *ECMDPointADD_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0; // ???? There is mwmac_cmd[0] or mwmac_start??????
	u32 mwmac_cmd_prec = 0; // ???? again mwmac_precision ?????
	u32 mwmac_f_sel = 1; 
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;  // ???? what is rw_prec ?????????????????????????????????????????

	if(ECMDPointADD_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ECMDPointADD_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ECMDPointADD_params_ptr->prec % 32) {
					rw_prec = (ECMDPointADD_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ECMDPointADD_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ECMDPointADD_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ECMDPointADD_params_ptr->prec;
			}
		}
	}			
	
	if(ECMDPointADD_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
//E8
	//MontMult(X6, B6, P6)				E8	<-	MontMult(z_PMDJ, z_PMDJ, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B6 << 12) | (MWMAC_RAM_X6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B6, E8)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B6 << 12) | (MWMAC_RAM_E8 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
	
//E7
	//MontMult(X3, B3, P3)				E7	<-	MontMult(z_QMDJ, z_QMDJ, p)		
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B3 << 12) | (MWMAC_RAM_X3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B3, E7)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B3 << 12) | (MWMAC_RAM_E7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
//E6
	//MontMult(E7**, B8, P8)				E6	<-	MontMult(E7** , x_PMDJ, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_E7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
			
	//COPYH2V(B8, E6)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_E6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//E5
	//MontMult(E8**, B5, P5)			E5	<-	MontMult(E8**	, x_QMDJ, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B5 << 12) | (MWMAC_RAM_E8 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B5, E5)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B5 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//E4
	//MontMult(X3, B7, P7)			E4	<-	MontMult(z_QMDJ, y_PMDJ, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_X3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B7, E4)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//E3
	//MontMult(E7**, B7, P7)  			E3	<-	MontMult(E7, E4, p) 	//B7 is used because I suppose result of previous calculation is still there
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_E7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B7, E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//E4
	//MontMult(X6, B4, P4)			E4	<-	MontMult(z_PMDJ, y_QMDJ, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B4 << 12) | (MWMAC_RAM_X6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B4, E4)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B4 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//E2
	//MontMult(E8, B4, P4)			E2	<-	MontMult(E8, E4, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B4 << 12) | (MWMAC_RAM_E8 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B4, E2)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B4 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
						
//E4
	//COPYV2H(E5, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E5 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//COPYV2H(E6, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E6 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//ModSub(TS1, TC1, P1)			E4	<-	ModSub(E5, E6 ,p)		
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
				
	//COPYH2V(B1, E4)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
	
	
//E5
	//MontMult(A1, B1, P1)			E5	<-	MontMult(2_MD, E4, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
			
	//COPYH2V(B1, E5)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	
//E1
	//MontMult(E5, B1, P1)			E1	<-	MontMult(E5, E5, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
	//COPYH2V(B1, E1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//E5
	//MontMult(E4, B1, P1)			E5	<-	MontMult(E4, E1, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//COPYH2V(B1, E5)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
			
			
//X2
	//COPYV2H(E2, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E2 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//COPYV2H(E3, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E3 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//ModSub(TS1, TC1, P1)			X2	<-	ModSub(E2, E3 ,p)	
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
					
	//COPYH2V(B1, X2)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_X2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//E2
	//MontMult(A1, B1, P1)			E2	<-	MontMult(2_MD, X2, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B1, E2)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//X2
	//MontMult(E1, B8, P8)			X2	<-	MontMult(E1, E6, p)		// result of the E6 is still ready on B8			
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B8, X2)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_X2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//E6
	//MontMult(E2, B1, P1)			E6	<-	MontMult(E2, E2, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//COPYH2V(B1, E6)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
//E1
	//MontMult(A1, B8, P8)			E1	<-	MontMult(2_MD, X2, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYH2V(B8, E1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B8 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;

//X1	
	//COPYV2H(E6, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E6 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//COPYV2H(E5, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E5 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//ModSub(TS1, TC1, P1)			X1	<-	ModSub(E6, E5 ,p)	
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
					
	//COPYH2V(B1, X1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_X1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;

//X_RMDJ = 	E6	

	//COPYV2H(X1, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X1 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//COPYV2H(E1, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E1 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	//ModSub(TS1, TC1, P1)			X_RMDJ = 	E6	<-	ModSub(X1, E1 ,p)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
						
	//COPYH2V(B1, E6)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	
//E1	
	//COPYV2H(X2, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X2 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYV2H(E6, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E6 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//ModSub(TS1, TC1, P1)			E1	<-	ModSub(X2, E6 ,p)	
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
									
	//COPYH2V(B1, E1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//X1
	//MontMult(E2, B1, P1)			X1	<-	MontMult(E2, E1, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
	//COPYH2V(B1, X1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_X1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
//X2
	//MontMult(A1, B7, P7)			X2	<-	MontMult(2_MD, E3, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
	//COPYH2V(B7, X2)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_X2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
//E1
	//MontMult(E5, B7, P7)			E1	<-	MontMult(E5, X2, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
	//COPYH2V(B7, E1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B7 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//Y_RMDJ =	E5	
	//COPYV2H(X1, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X1 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYV2H(E1, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E1 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//ModSub(TS1, TC1, P1)			Y_RMDJ =	E5	<-	ModSub(X1, E1 ,p)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
										
	//COPYH2V(B1, E5)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
																		
//E3
	//COPYV2H(X6, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X6 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYV2H(X3, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X3 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//ModAdd(TS1, TC1, P1)			E3	<-	ModAdd(X6, X3 ,p)	
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODADD << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
				
	//COPYH2V(B1, E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//X1
	//MontMult(E3, B1, P1)			X1	<-	MontMult(E3, E3, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
	//COPYH2V(B1, X1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_X1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
			
//X2
	//COPYV2H(X1, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X1 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYV2H(E8, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E8 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//ModSub(TS1, TC1, P1)			X2	<-	ModSub(X1, E8 ,p)	
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
									
	//COPYH2V(B1, X2)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_X2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//E1
	//COPYV2H(X2, TS1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_X2 << 12) | (MWMAC_RAM_TS1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//COPYV2H(E7, TC1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E7 << 12) | (MWMAC_RAM_TC1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//ModSub(TS1, TC1, P1)			E1	<-	ModSub(X2, E7 ,p)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8) 
		//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
										
	//COPYH2V(B1, E1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

//Z_RMDJ =	E7
	//MontMult(E4, B1, P1)			Z_RMDJ	=	E7	<-	MontMult(E4, E1, p)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				
	//COPYH2V(B1, E7)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E7 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
/*						
		REGISTERS (IMPORTANTS):
					E5	<-	y_RMDJ
					E6	<-	x_RMDJ
					E7	<-	z_RMDJ				
										*/	
// Read Result x_rmdj from E6 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDPointADD_params_ptr->x_rmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x2D0+i);
		} 
// Read Result y_rmdj from E5 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDPointADD_params_ptr->y_rmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x2C0+i);
		} 
// Read Result z_rmdj from E7 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDPointADD_params_ptr->z_rmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x2E0+i);
		} 												
}
static void MWMAC_ECMDJtoAffine(ECMDJtoAffine_params_t *ECMDJtoAffine_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0; // ???? There is mwmac_cmd[0] or mwmac_start??????
	u32 mwmac_cmd_prec = 0; // ???? again mwmac_precision ?????
	u32 mwmac_f_sel = 1; 
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;  // ???? what is rw_prec ?????????????????????????????????????????

	if(ECMDJtoAffine_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ECMDJtoAffine_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ECMDJtoAffine_params_ptr->prec % 32) {
					rw_prec = (ECMDJtoAffine_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ECMDJtoAffine_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ECMDJtoAffine_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ECMDJtoAffine_params_ptr->prec;
			}
		}
	}			
	
	if(ECMDJtoAffine_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
//A1&B1 <- Z^-1
		//COPYV2V(A1,E2)						COPY(2_MD, E2) to save
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2V << 8) 
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_A1 << 12) | (MWMAC_RAM_E2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;

		//MontR(P1,B1)	
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYH2V(B1,A1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;	
/*	
		//COPYV2V(E7,A1)						COPY(Z_RMDJ,A1)		//R
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2V << 8) 
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_E7 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
*/				
		//COPYV2V(E7,X1)						COPY(Z_RMDJ, X1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2V << 8) 
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_E7 << 12) | (MWMAC_RAM_X1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
/*			
		//COPYV2H(E7,B1)						COPY(Z_RMDJ, B1)	//R
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E7 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
*/				
		
		//COPYV2V(A8,E1)						COPY(exp, E1)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2V << 8) 
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_A8 << 12) | (MWMAC_RAM_E1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
		//MontExp(A1, B1, E1, X1, P1)		Z^-1 <- MontExp(Z_RMDJ,Z_RMDJ,exp,Z_RMDJ,p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTEXP << 8) 
		//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (MWMAC_RAM_E1 << 22) | (MWMAC_RAM_X1 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//ther are already @ A1,B1, no need to copy elsewhere

//B2<-(Z^-1)^2
		//MontMult(A1, B1, P1)					(Z^-1)^2 <- MontMult(Z^-1, Z^-1, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
		//COPYH2H(B1,B2)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2H << 8) 
		//			src_addr      			dest_addr    			src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_B2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
				

//x_RMD = E4
		//E4	<-	MontMult(E6, B2, P2)		x_RMD <- MontMult(x_RMDJ, (Z^-1)^2, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B2 << 12) | (MWMAC_RAM_E6 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYH2V(B2,E4)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B2 << 12) | (MWMAC_RAM_E4 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

		//MontMult(A1, B1, P1)					(Z^-1)^3 <- MontMult(Z^-1, (Z^-1)^2, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
//y_RMD = E3
		//E3	<-	MontMult(E5, B1, P1)		y_RMD <- MontMult(y_RMDJ, (Z^-1)^3, p)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8) 
		//			src_addr     			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E5 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
		//COPYH2V(B1,E3)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8) 
		//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_E3 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		

		//COPYV2V(E2,A1)						COPY(2_MD, A1), REPLACE IT
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2V << 8) 
		//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_E2 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
	
	/*
	REGISTERS:
		E4 <- x_RMD
		E3 <- y_RMD
	*/
	
}
static void MWMAC_ECMontBack(ECMontBack_params_t *ECMontBack_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0; // ???? There is mwmac_cmd[0] or mwmac_start??????
	u32 mwmac_cmd_prec = 0; // ???? again mwmac_precision ?????
	u32 mwmac_f_sel = 1; 
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;  // ???? what is rw_prec ?????????????????????????????????????????

	if(ECMontBack_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ECMontBack_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ECMontBack_params_ptr->prec % 32) {
					rw_prec = (ECMontBack_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ECMontBack_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ECMontBack_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ECMontBack_params_ptr->prec;
			}
		}
	}			
	
	if(ECMontBack_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}

//x_R	
	//COPYV2H(E4,B2)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E4 << 12) | (MWMAC_RAM_B2 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//MontMult1(B2,P2)	x_R	 <- MontMult1(x_R,p)
			//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT1 << 8) 
		//			src_addr      dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_B2 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;		
		

//y_R	
	//COPYV2H(E3,B1)
		//            Start     Abort       f_sel     sec_calc        precision              operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYV2H << 8) 
		//			src_addr      			dest_addr    		src_addr_e   src_addr_x
				| (MWMAC_RAM_E3 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;
		
	//MontMult1(B1,P1)	y_R	 <- MontMult1(y_R,p)
			//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT1 << 8) 
		//			src_addr      dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		while(!mwmac_irq_var);
		mwmac_irq_var = 0;		
		
		/*REGISTERS:
						x_R <- B2
						y_R <- B1  
		This function can be taken into the Jacobian to Affine function*/

 // Read Result x_r from B2 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMontBack_params_ptr->x_r[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x43+i*0x4);
		} 
// Read Result y_r from B1 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMontBack_params_ptr->y_r[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
		} 
}
module_init( cryptocore_init );
module_exit( cryptocore_exit );

MODULE_AUTHOR("MAI - Selected Topics of Embedded Software Development II");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("The driver for the FPGA-based CryptoCore");
MODULE_SUPPORTED_DEVICE("CryptoCore");
