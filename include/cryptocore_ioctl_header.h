/* 
* cryptocore_ioctl_header.h - the header file with the ioctl definitions.
* The declarations here have to be in a header file, because
* they need to be known both the kernel module in *_driver.c
* and the application *_app.c 
*/

#include <linux/ioctl.h>

// CryptoCore Struct Declarations:
typedef struct MontMult_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 a[16];
	__u32 b[16];
	__u32 c[16];
} MontMult_params_t;

typedef struct MontR_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 r[16];
} MontR_params_t;

typedef struct MontR2_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 r2[16];
} MontR2_params_t;

typedef struct MontExp_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 b[16];
	__u32 e[16];
	__u32 c[16];
} MontExp_params_t;

typedef struct ModAdd_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 a[16];
	__u32 b[16];
	__u32 c[16];
} ModAdd_params_t;

typedef struct ModSub_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 a[16];
	__u32 b[16];
	__u32 c[16];
} ModSub_params_t;

typedef struct CopyH2V_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 a[16];
	__u32 acopy[16];
} CopyH2V_params_t;

typedef struct CopyV2V_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 a[16];
	__u32 acopy[16];
} CopyV2V_params_t;

typedef struct CopyH2H_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 a[16];
	__u32 acopy[16];
} CopyH2H_params_t;

typedef struct CopyV2H_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 a[16];
	__u32 acopy[16];
} CopyV2H_params_t;

typedef struct MontMult1_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 b[16];
	__u32 c[16];
} MontMult1_params_t;

typedef struct ModExp_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 b[16];
	__u32 e[16];
	__u32 c[16];
} ModExp_params_t;


// Add CryptoCore Struct Declarations here...

typedef struct ECPreparation_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 p[16];
	__u32 a[16];
	__u32 b[16];
} ECPreparation_params_t;

typedef struct ECMDJacobiTransform_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 xp[16];
	__u32 yp[16];
	__u32 xq[16];
	__u32 yq[16];
	__u32 x_pmdj[16];
	__u32 y_pmdj[16];
	__u32 z_pmdj[16];
	__u32 x_qmdj[16];
	__u32 y_qmdj[16];
	__u32 z_qmdj[16];
} ECMDJacobiTransform_params_t;

typedef struct ECMDPointDBL_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 x_rmdj[16];
	__u32 y_rmdj[16];
	__u32 z_rmdj[16];
}ECMDPointDBL_params_t;

typedef struct ECMDPointADD_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 x_rmdj[16];
	__u32 y_rmdj[16];
	__u32 z_rmdj[16];
} ECMDPointADD_params_t;

typedef struct ECMDPointMUL_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 k[16];
} ECMDPointMUL_params_t;

typedef struct ECMDJtoAffine_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
} ECMDJtoAffine_params_t;

typedef struct ECMontBack_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 x_r[16];
	__u32 y_r[16];
} ECMontBack_params_t;



#define IOCTL_BASE 'k' 					// magic number

// NOTE: magic | cmdnumber | size of data to pass
#define 	IOCTL_SET_TRNG_CMD			_IOW(IOCTL_BASE,   1, __u32)
#define 	IOCTL_SET_TRNG_CTR			_IOW(IOCTL_BASE,   2, __u32)
#define 	IOCTL_SET_TRNG_TSTAB		_IOW(IOCTL_BASE,   3, __u32)
#define 	IOCTL_SET_TRNG_TSAMPLE		_IOW(IOCTL_BASE,   4, __u32)
#define 	IOCTL_READ_TRNG_FIFO		_IOR(IOCTL_BASE,   5, __u32)

#define		IOCTL_MWMAC_MONTMULT		_IOWR(IOCTL_BASE,  6, MontMult_params_t)
#define		IOCTL_MWMAC_MONTR			_IOWR(IOCTL_BASE,  7, MontR_params_t)
#define		IOCTL_MWMAC_MONTR2			_IOWR(IOCTL_BASE,  8, MontR2_params_t)
#define		IOCTL_MWMAC_MONTEXP			_IOWR(IOCTL_BASE,  9, MontExp_params_t)
#define		IOCTL_MWMAC_MODADD			_IOWR(IOCTL_BASE, 10, ModAdd_params_t)
#define		IOCTL_MWMAC_MODSUB			_IOWR(IOCTL_BASE, 11, ModSub_params_t)
#define		IOCTL_MWMAC_COPYH2V			_IOWR(IOCTL_BASE, 12, CopyH2V_params_t)
#define		IOCTL_MWMAC_COPYV2V			_IOWR(IOCTL_BASE, 13, CopyV2V_params_t)
#define		IOCTL_MWMAC_COPYH2H			_IOWR(IOCTL_BASE, 14, CopyH2H_params_t)
#define		IOCTL_MWMAC_COPYV2H			_IOWR(IOCTL_BASE, 15, CopyV2H_params_t)
#define		IOCTL_MWMAC_MONTMULT1		_IOWR(IOCTL_BASE, 16, MontMult1_params_t)
#define		IOCTL_MWMAC_MONTEXP_FULL	_IOWR(IOCTL_BASE, 17, MontExp_params_t)
#define		IOCTL_MWMAC_MODEXP			_IOWR(IOCTL_BASE, 18, ModExp_params_t)

// Define further IOCTL commands here...

#define		IOCTL_MWMAC_ECPREPARATION			_IOWR(IOCTL_BASE, 19, ECPreparation_params_t)
#define		IOCTL_MWMAC_ECMDJACOBITRANSFORM		_IOWR(IOCTL_BASE, 20, ECMDJacobiTransform_params_t)
#define		IOCTL_MWMAC_ECMDPOINTDBL			_IOWR(IOCTL_BASE, 21, ECMDPointDBL_params_t)
#define		IOCTL_MWMAC_ECMDPOINTADD			_IOWR(IOCTL_BASE, 22, ECMDPointADD_params_t)
#define		IOCTL_MWMAC_ECMDPOINTMUL			_IOWR(IOCTL_BASE, 23, ECMDPointMUL_params_t)
#define		IOCTL_MWMAC_ECMDJTOAFFINE			_IOWR(IOCTL_BASE, 24, ECMDJtoAffine_params_t)
#define		IOCTL_MWMAC_ECMONTBACK				_IOWR(IOCTL_BASE, 25, ECMontBack_params_t)
