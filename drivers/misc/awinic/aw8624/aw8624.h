#ifndef _AW8624_H_
#define _AW8624_H_

/*********************************************************
 *
 * kernel version
 *
 ********************************************************/

/*********************************************************
 *
 * aw8624.h
 *
 ********************************************************/
#include <linux/regmap.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/leds.h>
/*********************************************************
 *
 * marco
 *
 ********************************************************/
#define MAX_I2C_BUFFER_SIZE 65536

#define AW8624_REG_MAX                      0xff

#define AW8624_SEQUENCER_SIZE               8
#define AW8624_SEQUENCER_LOOP_SIZE          4

#define AW8624_RTP_I2C_SINGLE_MAX_NUM       512

#define HAPTIC_MAX_TIMEOUT                  10000

#define AW8624_HAPTIC_F0_PRE                2600
#define AW8624_HAPTIC_F0_CALI_PERCEN        7       /*-7%~7% */
#define AW8624_HAPTIC_CONT_DRV_LVL          125 /*80 //125=2.98v */
#define AW8624_HAPTIC_CONT_DRV_LVL_OV       155 /*  127  //155=3.69v */
#define AW8624_HAPTIC_CONT_TD               0xf06c  /*0x005d */
#define AW8624_HAPTIC_CONT_ZC_THR           0x0ff1 /* 0x009a */
#define AW8624_HAPTIC_CONT_NUM_BRK          3
#define AW8624_HAPTIC_F0_COEFF              260    /*2.604167 */


enum aw8624_flags {
	AW8624_FLAG_NONR = 0,
	AW8624_FLAG_SKIP_INTERRUPTS = 1,
};

enum aw8624_chipids {
	AW8624_ID = 1,
};

enum aw8624_haptic_read_write {
	AW8624_HAPTIC_CMD_READ_REG = 0,
	AW8624_HAPTIC_CMD_WRITE_REG = 1,
};


enum aw8624_haptic_work_mode {
	AW8624_HAPTIC_STANDBY_MODE = 0,
	AW8624_HAPTIC_RAM_MODE = 1,
	AW8624_HAPTIC_RTP_MODE = 2,
	AW8624_HAPTIC_TRIG_MODE = 3,
	AW8624_HAPTIC_CONT_MODE = 4,
	AW8624_HAPTIC_RAM_LOOP_MODE = 5,
};

enum aw8624_haptic_bst_mode {
	AW8624_HAPTIC_BYPASS_MODE = 0,
	AW8624_HAPTIC_BOOST_MODE = 1,
};

enum aw8624_haptic_activate_mode {
	AW8624_HAPTIC_ACTIVATE_RAM_MODE = 0,
	AW8624_HAPTIC_ACTIVATE_CONT_MODE = 1,
};

enum aw8624_haptic_vbat_comp_mode {
	AW8624_HAPTIC_VBAT_SW_COMP_MODE = 0,
	AW8624_HAPTIC_VBAT_HW_COMP_MODE = 1,
};

enum aw8624_haptic_f0_flag {
	AW8624_HAPTIC_LRA_F0 = 0,
	AW8624_HAPTIC_CALI_F0 = 1,
};

enum aw8624_haptic_pwm_mode {
	AW8624_PWM_48K = 0,
	AW8624_PWM_24K = 1,
	AW8624_PWM_12K = 2,
};

/*********************************************************
 *
 * struct
 *
 ********************************************************/
struct fileops {
	unsigned char cmd;
	unsigned char reg;
	unsigned char ram_addrh;
	unsigned char ram_addrl;
};

struct ram {
	unsigned int len;
	unsigned int check_sum;
	unsigned int base_addr;
	unsigned char version;
	unsigned char ram_shift;
	unsigned char baseaddr_shift;
};

struct haptic_ctr {
	unsigned char cmd;
	unsigned char play;
	unsigned char wavseq;
	unsigned char loop;
	unsigned char gain;
};

struct haptic_audio {
	struct mutex lock;
	struct hrtimer timer;
	struct work_struct work;
	int delay_val;
	int timer_val;
	unsigned char cnt;
	struct haptic_ctr ctr[256];
};

struct aw8624 {
	struct i2c_client *i2c;
	struct device *dev;

	struct mutex lock;
	struct hrtimer timer;
	struct work_struct vibrator_work;
	struct work_struct irq_work;
	struct work_struct rtp_work;
	struct delayed_work ram_work;

	struct led_classdev to_dev;

	struct fileops fileops;
	struct ram ram;

	int reset_gpio;
	int irq_gpio;
	int reset_gpio_ret;
	int irq_gpio_ret;

	unsigned char hwen_flag;
	unsigned char flags;
	unsigned char chipid;

	unsigned char play_mode;
	unsigned char activate_mode;

	int state;
	int duration;
	int amplitude;
	int index;
	int vmax;
	int gain;

	unsigned char seq[AW8624_SEQUENCER_SIZE];
	unsigned char loop[AW8624_SEQUENCER_SIZE];

	unsigned int rtp_cnt;
	unsigned int rtp_file_num;

	unsigned char rtp_init;
	unsigned char ram_init;

	unsigned int f0;
	unsigned int f0_pre;
	unsigned int cont_td;
	unsigned int cont_f0;
	unsigned int cont_zc_thr;
	unsigned char cont_drv_lvl;
	unsigned char cont_drv_lvl_ov;
	unsigned char cont_num_brk;
	unsigned char max_pos_beme;
	unsigned char max_neg_beme;
	unsigned char f0_cali_flag;
	bool IsUsedIRQ;
	bool ram_update_delay;
	bool dts_addr_real;
	unsigned int real_i2c_addr;
	struct haptic_audio haptic_audio;
};

struct aw8624_container {
	int len;
	unsigned char data[];
};


/*********************************************************
 *
 * ioctl
 *
 ********************************************************/
struct aw8624_seq_loop {
	unsigned char loop[AW8624_SEQUENCER_SIZE];
};

struct aw8624_que_seq {
	unsigned char index[AW8624_SEQUENCER_SIZE];
};


#define AW8624_HAPTIC_IOCTL_MAGIC         'h'

#define AW8624_HAPTIC_SET_QUE_SEQ         _IOWR(AW8624_HAPTIC_IOCTL_MAGIC, 1, struct aw8624_que_seq*)
#define AW8624_HAPTIC_SET_SEQ_LOOP        _IOWR(AW8624_HAPTIC_IOCTL_MAGIC, 2, struct aw8624_seq_loop*)
#define AW8624_HAPTIC_PLAY_QUE_SEQ        _IOWR(AW8624_HAPTIC_IOCTL_MAGIC, 3, unsigned int)
#define AW8624_HAPTIC_SET_BST_VOL         _IOWR(AW8624_HAPTIC_IOCTL_MAGIC, 4, unsigned int)
#define AW8624_HAPTIC_SET_BST_PEAK_CUR    _IOWR(AW8624_HAPTIC_IOCTL_MAGIC, 5, unsigned int)
#define AW8624_HAPTIC_SET_GAIN            _IOWR(AW8624_HAPTIC_IOCTL_MAGIC, 6, unsigned int)
#define AW8624_HAPTIC_PLAY_REPEAT_SEQ     _IOWR(AW8624_HAPTIC_IOCTL_MAGIC, 7, unsigned int)

#endif
