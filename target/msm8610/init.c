/* Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <debug.h>
#include <platform/iomap.h>
#include <platform/irqs.h>
#include <reg.h>
#include <target.h>
#include <platform.h>
#include <uart_dm.h>
#include <mmc.h>
#include <platform/gpio.h>
#include <spmi.h>
#include <board.h>
#include <smem.h>
#include <baseband.h>
#include <dev/keys.h>
#include <pm8x41.h>
#include <hsusb.h>
#include <kernel/thread.h>
#include <arch/defines.h>
#include <stdlib.h>
#include <scm.h>
#include <partition_parser.h>
#include <platform/clock.h>
#include <platform/timer.h>
#include <shutdown_detect.h>
#include <vibrator.h>

#define PMIC_ARB_CHANNEL_NUM    0
#define PMIC_ARB_OWNER_ID       0

#define TLMM_VOL_UP_BTN_GPIO    72
#define VIBRATE_TIME    250

enum target_subtype {
	HW_PLATFORM_SUBTYPE_SKUAA = 1,
	HW_PLATFORM_SUBTYPE_SKUF = 2,
	HW_PLATFORM_SUBTYPE_SKUAB = 3,
};

static void set_sdc_power_ctrl(void);

static uint32_t mmc_pwrctl_base[] =
	{ MSM_SDC1_BASE, MSM_SDC2_BASE };

static uint32_t mmc_sdhci_base[] =
	{ MSM_SDC1_SDHCI_BASE, MSM_SDC2_SDHCI_BASE };

static uint32_t  mmc_sdc_pwrctl_irq[] =
	{ SDCC1_PWRCTL_IRQ, SDCC2_PWRCTL_IRQ };

struct mmc_device *dev;

void target_early_init(void)
{
#if WITH_DEBUG_UART
	uart_dm_init(2, 0, BLSP1_UART1_BASE);
#endif
}
enum mtp_subtype
{
	MTP_SUBTYPE_WVGA,
	MTP_SUBTYPE_720P,
	MTP_SUBTYPE_APQ_WVGA,
};

void set_mtp_baseband(struct board_data *board)
{
	uint32_t platform_subtype;
	platform_subtype = board->platform_subtype;

	switch(platform_subtype) {
	case MTP_SUBTYPE_WVGA:
	case MTP_SUBTYPE_720P:
		board->baseband = BASEBAND_MSM;
		break;
	case MTP_SUBTYPE_APQ_WVGA:
		board->baseband = BASEBAND_APQ;
		break;
	default:
		dprintf(CRITICAL, "MTP platform subtype :%u is not supported\n",
				platform_subtype);
		ASSERT(0);
	};
}

/* Return 1 if vol_up pressed */
static int target_volume_up()
{
	uint8_t status = 0;

	gpio_tlmm_config(TLMM_VOL_UP_BTN_GPIO, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA, GPIO_ENABLE);

	/* Wait for the configuration to complete.*/
	thread_sleep(1);
	/* Get status of GPIO */
	status = gpio_status(TLMM_VOL_UP_BTN_GPIO);

	/* Active low signal. */
	return !status;
}

/* Return 1 if vol_down pressed */
uint32_t target_volume_down()
{
	/* Volume down button tied in with PMIC RESIN. */
	return pm8x41_resin_status();
}

static void target_keystatus()
{
	keys_init();

	if(target_volume_down())
		keys_post_event(KEY_VOLUMEDOWN, 1);

	if(target_volume_up())
		keys_post_event(KEY_VOLUMEUP, 1);
}

void target_sdc_init()
{
	struct mmc_config_data config;

	/* Set drive strength & pull ctrl values */
	set_sdc_power_ctrl();

	config.bus_width = DATA_BUS_WIDTH_8BIT;
	config.max_clk_rate = MMC_CLK_200MHZ;

	/* Try slot 1*/
	config.slot = 1;
	config.sdhc_base = mmc_sdhci_base[config.slot - 1];
	config.pwrctl_base = mmc_pwrctl_base[config.slot - 1];
	config.pwr_irq     = mmc_sdc_pwrctl_irq[config.slot - 1];

	if (!(dev = mmc_init(&config)))
	{
		/* Try slot 2 */
		config.slot = 2;
		config.sdhc_base = mmc_sdhci_base[config.slot - 1];
		config.pwrctl_base = mmc_pwrctl_base[config.slot - 1];
		config.pwr_irq     = mmc_sdc_pwrctl_irq[config.slot - 1];

		if (!(dev = mmc_init(&config)))
		{
			dprintf(CRITICAL, "mmc init failed!");
			ASSERT(0);
		}
	}

	/* MMC initialization is complete, read the partition table info */
	if (partition_read_table())
	{
		dprintf(CRITICAL, "Error reading the partition table info\n");
		ASSERT(0);
	}
}

void target_init(void)
{
	dprintf(INFO, "target_init()\n");

	spmi_init(PMIC_ARB_CHANNEL_NUM, PMIC_ARB_OWNER_ID);

	target_keystatus();

	target_sdc_init();

	shutdown_detect();

	/* turn on vibrator to indicate that phone is booting up to end user */
	vib_timed_turn_on(VIBRATE_TIME);

	/* Display splash screen if enabled */
	dprintf(SPEW, "Display Init: Start\n");
	display_init();
	dprintf(SPEW, "Display Init: Done\n");
}

void target_uninit(void)
{
        mmc_put_card_to_sleep(dev);

	/* wait for the vibrator timer is expried */
	wait_vib_timeout();
}

#define SSD_CE_INSTANCE         1

void target_load_ssd_keystore(void)
{
	uint64_t ptn;
	int      index;
	uint64_t size;
	uint32_t *buffer;

	if (!target_is_ssd_enabled())
		return;

	index = partition_get_index("ssd");

	ptn = partition_get_offset(index);
	if (ptn == 0){
		dprintf(CRITICAL, "Error: ssd partition not found\n");
		return;
	}

	size = partition_get_size(index);
	if (size == 0) {
		dprintf(CRITICAL, "Error: invalid ssd partition size\n");
		return;
	}

	buffer = memalign(CACHE_LINE, ROUNDUP(size, CACHE_LINE));
	if (!buffer) {
		dprintf(CRITICAL, "Error: allocating memory for ssd buffer\n");
		return;
	}

	if (mmc_read(ptn, buffer, size)) {
		dprintf(CRITICAL, "Error: cannot read data\n");
		free(buffer);
		return;
	}

	clock_ce_enable(SSD_CE_INSTANCE);
	scm_protect_keystore(buffer, size);
	clock_ce_disable(SSD_CE_INSTANCE);
	free(buffer);
}

/* Do any target specific intialization needed before entering fastboot mode */
void target_fastboot_init(void)
{
	/* Set the BOOT_DONE flag in PM8110 */
	pm8x41_set_boot_done();

	if (target_is_ssd_enabled()) {
		clock_ce_enable(SSD_CE_INSTANCE);
		target_load_ssd_keystore();
	}

}

/* Detect the target type */
void target_detect(struct board_data *board)
{
	/*
	* already fill the board->target on board.c
	*/

}

/* Detect the modem type */
void target_baseband_detect(struct board_data *board)
{
	uint32_t platform;
	uint32_t platform_hardware;

	platform         = board->platform;
	platform_hardware = board->platform_hw;

	switch(platform_hardware) {
	case HW_PLATFORM_MTP:
		set_mtp_baseband(board);
	break;
	case HW_PLATFORM_SURF:
	case HW_PLATFORM_QRD:
		board->baseband = BASEBAND_MSM;
	break;
	default:
		dprintf(CRITICAL, "Platform type : %u is not supported defaulting the baseband to MSM\n", platform_hardware);
		board->baseband = BASEBAND_MSM;
	};
}

unsigned target_baseband()
{
	return board_baseband();
}

void target_serialno(unsigned char *buf)
{
	uint32_t serialno;
	if (target_is_emmc_boot()) {
		serialno = mmc_get_psn();
		snprintf((char *)buf, 13, "%x", serialno);
	}
}

unsigned check_reboot_mode(void)
{
	uint32_t restart_reason = 0;

	/* Read reboot reason and scrub it */
	restart_reason = readl(RESTART_REASON_ADDR);
	writel(0x00, RESTART_REASON_ADDR);

	return restart_reason;
}

void reboot_device(unsigned reboot_reason)
{
	int ret = 0;

	writel(reboot_reason, RESTART_REASON_ADDR);

	/* Configure PMIC for warm reset */
	pm8x41_reset_configure(PON_PSHOLD_WARM_RESET);

	ret = scm_halt_pmic_arbiter();
	if (ret)
		dprintf(CRITICAL , "Failed to halt pmic arbiter: %d\n", ret);

	/* Drop PS_HOLD for MSM */
	writel(0x00, MPM2_MPM_PS_HOLD);

	mdelay(5000);

	dprintf(CRITICAL, "Rebooting failed\n");
}

int target_cont_splash_screen()
{
	int ret = 0;

	switch(board_hardware_id())
	{
		case HW_PLATFORM_QRD:
		case HW_PLATFORM_MTP:
		case HW_PLATFORM_SURF:
			dprintf(SPEW, "Target_cont_splash=1\n");
			ret = 1;
			break;
		default:
			dprintf(SPEW, "Target_cont_splash=0\n");
			ret = 0;
	}
	return ret;
}

unsigned target_pause_for_battery_charge(void)
{
	uint8_t pon_reason = pm8x41_get_pon_reason();
	uint8_t is_cold_boot = pm8x41_get_is_cold_boot();
	dprintf(INFO, "%s : pon_reason is %d cold_boot:%d\n", __func__,
					pon_reason, is_cold_boot);
#ifdef DISABLE_POWER_OFF_CHARGER
//disable power off charger when mini version
	return 0;
#endif
	/*In case of fastboot reboot, adb reboot or if we see the power key
	 * pressed we do not want go into charger mode.
	 * fastboot reboot is warm boot with PON hard reset bit not set
	 * adb reboot is a cold boot with PON hard reset bit set
	 */
	if (is_cold_boot &&
					(!(pon_reason & HARD_RST)) &&
					(!(pon_reason & KPDPWR_N)) &&
					((pon_reason & USB_CHG) || (pon_reason & DC_CHG)))
			return 1;
	else
			return 0;
}
/**
*Bug#: 636472
*read boot command from misc
*/
int emmc_recovery_init(void)
{
    return _emmc_recovery_init();
}

void target_usb_stop(void)
{
	/* Disable VBUS mimicing in the controller. */
	ulpi_write(ULPI_MISC_A_VBUSVLDEXTSEL | ULPI_MISC_A_VBUSVLDEXT, ULPI_MISC_A_CLEAR);
}

void target_usb_init(void)
{
	uint32_t val;

	/* Select and enable external configuration with USB PHY */
	ulpi_write(ULPI_MISC_A_VBUSVLDEXTSEL | ULPI_MISC_A_VBUSVLDEXT, ULPI_MISC_A_SET);

	/* Enable sess_vld */
	val = readl(USB_GENCONFIG_2) | GEN2_SESS_VLD_CTRL_EN;
	writel(val, USB_GENCONFIG_2);

	/* Enable external vbus configuration in the LINK */
	val = readl(USB_USBCMD);
	val |= SESS_VLD_CTRL;
	writel(val, USB_USBCMD);
}


unsigned board_machtype(void)
{
	return 0;
}

static void set_sdc_power_ctrl()
{
	/* Drive strength configs for sdc pins */
	struct tlmm_cfgs sdc1_hdrv_cfg[] =
	{
		{ SDC1_CLK_HDRV_CTL_OFF,  TLMM_CUR_VAL_16MA, TLMM_HDRV_MASK },
		{ SDC1_CMD_HDRV_CTL_OFF,  TLMM_CUR_VAL_10MA, TLMM_HDRV_MASK },
		{ SDC1_DATA_HDRV_CTL_OFF, TLMM_CUR_VAL_6MA,  TLMM_HDRV_MASK },
	};

	/* Pull configs for sdc pins */
	struct tlmm_cfgs sdc1_pull_cfg[] =
	{
		{ SDC1_CLK_PULL_CTL_OFF,  TLMM_NO_PULL, TLMM_PULL_MASK },
		{ SDC1_CMD_PULL_CTL_OFF,  TLMM_PULL_UP, TLMM_PULL_MASK },
		{ SDC1_DATA_PULL_CTL_OFF, TLMM_PULL_UP, TLMM_PULL_MASK },
	};

	/* Set the drive strength & pull control values */
	tlmm_set_hdrive_ctrl(sdc1_hdrv_cfg, ARRAY_SIZE(sdc1_hdrv_cfg));
	tlmm_set_pull_ctrl(sdc1_pull_cfg, ARRAY_SIZE(sdc1_pull_cfg));
}

struct mmc_device *target_mmc_device()
{
	return dev;
}

/* Configure PMIC and Drop PS_HOLD for shutdown */
void shutdown_device()
{
	dprintf(CRITICAL, "Going down for shutdown.\n");

	/* Configure PMIC for shutdown */
	pm8x41_reset_configure(PON_PSHOLD_SHUTDOWN);

	/* Drop PS_HOLD for MSM */
	writel(0x00, MPM2_MPM_PS_HOLD);

	mdelay(5000);

	dprintf(CRITICAL, "shutdown failed\n");

	ASSERT(0);
}
