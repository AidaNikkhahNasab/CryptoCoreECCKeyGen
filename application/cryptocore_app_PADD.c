#include <asm-generic/fcntl.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>

#include "../include/cryptocore_ioctl_header.h"

/* Prototypes for functions used to access physical memory addresses */
int open_physical (int);
void close_physical (int);

int main(void)
{
	int dd = -1;
	int ret_val;

	__u32 trng_val = 0;
	__u32 i = 0;
	
	double seconds;
	struct timespec tstart={0,0}, tend={0,0};

	if ((dd = open_physical (dd)) == -1)
      return (-1);

// Stop TRNG and clear FIFO
	trng_val = 0x00000010;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CMD, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	usleep(10);

// Configure Feedback Control Polynomial
	trng_val = 0x0003ffff;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CTR, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

// Configure Stabilisation Time
	trng_val = 0x00000050;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_TSTAB, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

// Configure Sample Time
	trng_val = 0x00000006;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_TSAMPLE, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

// Start TRNG
	trng_val = 0x00000001;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CMD, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	usleep(10);


//	Define parameter structure like input spaces and output spaces here

	ECPreparation_params_t ECPreparation_512_test ={512,
	1,
	0,
	{ 0xaadd9db8, 0xdbe9c48b, 0x3fd4e6ae, 0x33c9fc07, 
	  0xcb308db3, 0xb3c9d20e, 0xd6639cca, 0x70330871, 
	  0x7d4d9b00, 0x9bc66842, 0xaecda12a, 0xe6a380e6, 
	  0x2881ff2f, 0x2d82c685, 0x28aa6056, 0x583a48f3 }, // p
	{ 0x7830a331, 0x8b603b89, 0xe2327145, 0xac234cc5,
	  0x94cbdd8d, 0x3df91610, 0xa83441ca, 0xea9863bc,
	  0x2ded5d5a, 0xa8253aa1, 0x0a2ef1c9, 0x8b9ac8b5,
	  0x7f1117a7, 0x2bf2c7b9, 0xe7c1ac4d, 0x77fc94ca }, // a
	{ 0x3df91610, 0xa83441ca, 0xea9863bc, 0x2ded5d5a,
	  0xa8253aa1, 0x0a2ef1c9, 0x8b9ac8b5, 0x7f1117a7,
	  0x2bf2c7b9, 0xe7c1ac4d, 0x77fc94ca, 0xdc083e67,
	  0x984050b7, 0x5ebae5dd, 0x2809bd63, 0x8016f723 }, // b
	};

	ECMDJacobiTransform_params_t ECMDJacobiTransform_512_test ={512,
	1,
	0,
	{ 0x81aee4bd, 0xd82ed964, 0x5a21322e, 0x9c4c6a93,
	  0x85ed9f70, 0xb5d916c1, 0xb43b62ee, 0xf4d0098e,
	  0xff3b1f78, 0xe2d0d48d, 0x50d1687b, 0x93b97d5f,
	  0x7c6d5047, 0x406a5e68, 0x8b352209, 0xbcb9f822 }, // xp
	{ 0x7dde385d, 0x566332ec, 0xc0eabfa9, 0xcf7822fd,
	  0xf209f700, 0x24a57b1a, 0xa000c55b, 0x881f8111,
	  0xb2dcde49, 0x4a5f485e, 0x5bca4bd8, 0x8a2763ae,
	  0xd1ca2b2f, 0xa8f05406, 0x78cd1e0f, 0x3ad80892 }, // yp	
	{ 0x8840b668, 0x2ef2f0bb, 0xfd8baa18, 0xa8c94453, 
		0x5cb1d3f4, 0xa7feb48f, 0xa6dda6e2, 0x7d046f22, 
		0x4827e4c2, 0xb3e7fc0b, 0x284137ae, 0x087581e3, 
	    0x6644734e, 0xadf3a240, 0x1f1b9538, 0xbfc5072e }, //xq
		
	{ 0x766745b2, 0x73774d42, 0x4938b979, 0xd0121c24, 
		0xb2d767a0, 0x3a68bfd2, 0xc6b4af90, 0x6d85f75d, 
		0x4e4202b3, 0x77ce49ab, 0xf0b2c6cc, 0xca44375c, 
		0xa1a580a1, 0x280ee9b9, 0x5fee02ff, 0x9a5246bd }, // yq	
		
	{  }, // x_pmdj
	{  }, // y_pmdj
	{  }, // z_pmdj
	{  }, // x_qmdj
	{  }, // y_qmdj
	{  }, // z_qmdj
	
	};

	ECMDPointADD_params_t ECMDPointADD_512_test ={512,
	1,
	0,
	{  }, // x_rmdj
	{  }, // y_rmdj
	{  }, // z_rmdj
	};

	ECMDJtoAffine_params_t ECMDJtoAffine_512_test ={512,
	1,
	0,
	};

	ECMontBack_params_t ECMontBack_512_test ={512,
	1,
	0,
	{  },	//x_r
	{  },	//y_r
	};
	
	

// Show the initial values if they are complete
	printf("P : 0x");
	for(i=0; i<ECPreparation_512_test.prec/32; i++){
		printf("%08x", ECPreparation_512_test.p[i]);
	}
	printf("\n\n");	
	
	printf("A : 0x");
	for(i=0; i<ECPreparation_512_test.prec/32; i++){
		printf("%08x", ECPreparation_512_test.a[i]);
	}
	printf("\n\n");	
	
	printf("B : 0x");
	for(i=0; i<ECPreparation_512_test.prec/32; i++){
		printf("%08x", ECPreparation_512_test.b[i]);
	}
	printf("\n\n");	
	
	printf("xp: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.xp[i]);
	}
	printf("\n\n");
	
	printf("yp: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.yp[i]);
	}
	printf("\n\n");


	printf("xq: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.xq[i]);
	}
	printf("\n\n");
	
	printf("yq: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.yq[i]);
	}
	printf("\n\n");	
	// Start timer-----------------------------------------------------------------------------
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	
	// call ECPreparation()
	ret_val = ioctl(dd, IOCTL_MWMAC_ECPREPARATION, &ECPreparation_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	// call ECMDJacobiTransform()
	ret_val = ioctl(dd, IOCTL_MWMAC_ECMDJACOBITRANSFORM, &ECMDJacobiTransform_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	printf("Affine to Jacobi Results : ");
	printf("\n\n");
	
	// read P_MDJ :
	
	printf("X_pmdj: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.x_pmdj[i]);
	}
	printf("\n\n");
	printf("Y_pmdj: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.y_pmdj[i]);
	}
	printf("\n\n");
	printf("Z_pmdj: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.z_pmdj[i]);
	}
	// read Q_MDJ :	
	printf("\n\n");	
	printf("X_qmdj: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.x_qmdj[i]);
	}
	printf("\n\n");
	printf("Y_qmdj: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.y_qmdj[i]);
	}
	printf("\n\n");
	printf("Z_qmdj: 0x");
	for(i=0; i<ECMDJacobiTransform_512_test.prec/32; i++){
		printf("%08x", ECMDJacobiTransform_512_test.z_qmdj[i]);
	}
	printf("\n\n");	

	// call ECMDPointADD()
	ret_val = ioctl(dd, IOCTL_MWMAC_ECMDPOINTADD, &ECMDPointADD_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	printf("\n\nPoint Addition Results : ");
	printf("\n\n");
	
	// read R_MDJ :
	
	printf("X_rmdj: 0x");
	for(i=0; i<ECMDPointADD_512_test.prec/32; i++){
		printf("%08x", ECMDPointADD_512_test.x_rmdj[i]);
	}
	printf("\n\n");
	printf("Y_rmdj: 0x");
	for(i=0; i<ECMDPointADD_512_test.prec/32; i++){
		printf("%08x", ECMDPointADD_512_test.y_rmdj[i]);
	}
	printf("\n\n");
	printf("Z_rmdj: 0x");
	for(i=0; i<ECMDPointADD_512_test.prec/32; i++){
		printf("%08x", ECMDPointADD_512_test.z_rmdj[i]);
	}	

	// call ECMDJtoAffine()
	ret_val = ioctl(dd, IOCTL_MWMAC_ECMDJTOAFFINE, &ECMDJtoAffine_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	// call ECMontBack()
	ret_val = ioctl(dd, IOCTL_MWMAC_ECMONTBACK, &ECMontBack_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	printf("\n\nRESULTS : ");
	printf("\n\n");
	
	// read R :
	
	printf("XR: 0x");
	for(i=0; i<ECMontBack_512_test.prec/32; i++){
		printf("%08x", ECMontBack_512_test.x_r[i]);
	}
	printf("\n\n");
	printf("YR: 0x");
	for(i=0; i<ECMontBack_512_test.prec/32; i++){
		printf("%08x", ECMontBack_512_test.y_r[i]);
	}	

	clock_gettime(CLOCK_MONOTONIC, &tend);
	// End timer-----------------------------------------------------------------------------
/*
	printf("rand: ");
	for(i=0; i<TRNG_512_test.prec/32; i++){
		printf("%08x", TRNG_512_test.rand[i]);
	}
	printf("\n\n");
*/
	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("Reading 512 random bits took about %.5f ms\n", seconds*1000.0);
	else 
		printf("Reading 512 random bits took about %.5f us\n", seconds*1000000.0);	

	close_physical (dd);   // close /dev/cryptocore
	return 0;
}

// Open /dev/cryptocore, if not already done, to give access to physical addresses
int open_physical (int dd)
{
   if (dd == -1)
      if ((dd = open( "/dev/cryptocore", (O_RDWR | O_SYNC))) == -1)
      {
         printf ("ERROR: could not open \"/dev/cryptocore\"...\n");
         return (-1);
      }
   return dd;
}

// Close /dev/mem to give access to physical addresses
void close_physical (int dd)
{
   close (dd);
}

/*		
In MWMAC_ECPreparation : 
	there are 
				p, a  b	 		    WRITTEN to the device memory
In MWMAC_ECMDJacobiTransform:
	there are 
				xp, yp, xq, yq		 WRITTEN to the device memory
		
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
		
 In MWMAC_ECMDPointADD:
 
// Read Result x_rmdj from x6 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDPointADD_params_ptr->x_rmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3D0+i);
		} 
// Read Result y_rmdj from x5 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDPointADD_params_ptr->y_rmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3C0+i);
		} 
// Read Result z_rmdj from x7 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMDPointADD_params_ptr->z_rmdj[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3E0+i);
		} 	
In MWMAC_ECMDJtoAffine:
	-----------

In MWMAC_ECMontBack:
	
 // Read Result x_r from B2 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMontBack_params_ptr->x_r[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x43+i*0x4);
		} 
// Read Result y_r from B1 Register Memory 
 	for(i=0; i<rw_prec/32; i++){
		ECMontBack_params_ptr->y_r[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
		} 
 
 
 
 */
