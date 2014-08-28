/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <debug.h>
#include <err.h>
#include <smem.h>
#include <msm_panel.h>
#include <board.h>
#include <mipi_dsi.h>

#include "include/panel.h"
#include "panel_display.h"

/*---------------------------------------------------------------------------*/
/* GCDB Panel Database                                                       */
/*---------------------------------------------------------------------------*/
#include "include/panel_truly_wvga_cmd.h"
#include "include/panel_truly_wvga_video.h"
#include "include/panel_hx8379a_wvga_video.h"
#include "include/panel_hx8389b_qhd_video.h"
#include "include/panel_otm8018b_fwvga_video.h"
#include "include/panel_nt35590_720p_video.h"
#include "include/panel_tianma_tm040ydh65_ili9806c_wvga_video.h"
/*---------------------------------------------------------------------------*/
/* static panel selection variable                                           */
/*---------------------------------------------------------------------------*/
enum {
TRULY_WVGA_CMD_PANEL,
TRULY_WVGA_VIDEO_PANEL,
HX8379A_WVGA_VIDEO_PANEL,
OTM8018B_FWVGA_VIDEO_PANEL,
NT35590_720P_VIDEO_PANEL,
HX8389B_QHD_VIDEO_PANEL,
ILI9806C_WVGA_VIDEO_PANEL,
};

enum {
QRD_DEF,
QRD_SKUAA,
QRD_SKUAB = 3,
};

static uint32_t panel_id;

int oem_panel_rotation()
{
	/* OEM can keep there panel spefic on instructions in this
	function */
	return NO_ERROR;
}


int oem_panel_on()
{
	/* OEM can keep there panel spefic on instructions in this
	function */
	return NO_ERROR;
}

int oem_panel_off()
{
	/* OEM can keep there panel spefic off instructions in this
	function */
	return NO_ERROR;
}

static bool init_panel_data(struct panel_struct *panelstruct,
			struct msm_panel_info *pinfo,
			struct mdss_dsi_phy_ctrl *phy_db)
{
	switch (panel_id) {
	case ILI9806C_WVGA_VIDEO_PANEL:
		panelstruct->paneldata    = &tianma_tm040ydh65_ili9806c_wvga_video_panel_data;
		panelstruct->panelres     = &tianma_tm040ydh65_ili9806c_wvga_video_panel_res;
		panelstruct->color        = &tianma_tm040ydh65_ili9806c_wvga_video_color;
		panelstruct->videopanel   = &tianma_tm040ydh65_ili9806c_wvga_video_video_panel;
		panelstruct->commandpanel = &tianma_tm040ydh65_ili9806c_wvga_video_command_panel;
		panelstruct->state        = &tianma_tm040ydh65_ili9806c_wvga_video_state;
		panelstruct->laneconfig   = &tianma_tm040ydh65_ili9806c_wvga_video_lane_config;
		panelstruct->paneltiminginfo  	= &tianma_tm040ydh65_ili9806c_wvga_video_timing_info;
		panelstruct->panelresetseq	= &tianma_tm040ydh65_ili9806c_wvga_video_reset_seq;
		panelstruct->backlightinfo = &tianma_tm040ydh65_ili9806c_wvga_video_backlight;
		pinfo->mipi.panel_cmds	  = tianma_tm040ydh65_ili9806c_wvga_video_on_command;
		pinfo->mipi.num_of_panel_cmds	= TIANMA_TM040YDH65_ILI9806C_WVGA_VIDEO_ON_COMMAND;
		memcpy(phy_db->timing, tianma_tm040ydh65_ili9806c_wvga_video_timings, TIMING_SIZE);
		break;
	case TRULY_WVGA_CMD_PANEL:
		panelstruct->paneldata    = &truly_wvga_cmd_panel_data;
		panelstruct->panelres     = &truly_wvga_cmd_panel_res;
		panelstruct->color        = &truly_wvga_cmd_color;
		panelstruct->videopanel   = &truly_wvga_cmd_video_panel;
		panelstruct->commandpanel = &truly_wvga_cmd_command_panel;
		panelstruct->state        = &truly_wvga_cmd_state;
		panelstruct->laneconfig   = &truly_wvga_cmd_lane_config;
		panelstruct->paneltiminginfo
					 = &truly_wvga_cmd_timing_info;
		panelstruct->panelresetseq
					 = &truly_wvga_cmd_reset_seq;
		panelstruct->backlightinfo = &truly_wvga_cmd_backlight;
		pinfo->mipi.panel_cmds
					= truly_wvga_cmd_on_command;
		pinfo->mipi.num_of_panel_cmds
					= TRULY_WVGA_CMD_ON_COMMAND;
		memcpy(phy_db->timing,
			truly_wvga_cmd_timings, TIMING_SIZE);
		break;
	case TRULY_WVGA_VIDEO_PANEL:
		panelstruct->paneldata    = &truly_wvga_video_panel_data;
		panelstruct->panelres     = &truly_wvga_video_panel_res;
		panelstruct->color        = &truly_wvga_video_color;
		panelstruct->videopanel   = &truly_wvga_video_video_panel;
		panelstruct->commandpanel = &truly_wvga_video_command_panel;
		panelstruct->state        = &truly_wvga_video_state;
		panelstruct->laneconfig   = &truly_wvga_video_lane_config;
		panelstruct->paneltiminginfo
					 = &truly_wvga_video_timing_info;
		panelstruct->panelresetseq
					 = &truly_wvga_video_reset_seq;
		panelstruct->backlightinfo = &truly_wvga_video_backlight;
		pinfo->mipi.panel_cmds
					= truly_wvga_video_on_command;
		pinfo->mipi.num_of_panel_cmds
					= TRULY_WVGA_VIDEO_ON_COMMAND;
		memcpy(phy_db->timing,
				truly_wvga_video_timings, TIMING_SIZE);
		break;
	case HX8379A_WVGA_VIDEO_PANEL:
		panelstruct->paneldata    = &hx8379a_wvga_video_panel_data;
		panelstruct->panelres     = &hx8379a_wvga_video_panel_res;
		panelstruct->color        = &hx8379a_wvga_video_color;
		panelstruct->videopanel   = &hx8379a_wvga_video_video_panel;
		panelstruct->commandpanel = &hx8379a_wvga_video_command_panel;
		panelstruct->state        = &hx8379a_wvga_video_state;
		panelstruct->laneconfig   = &hx8379a_wvga_video_lane_config;
		panelstruct->paneltiminginfo
					 = &hx8379a_wvga_video_timing_info;
		panelstruct->panelresetseq
					 = &hx8379a_wvga_video_reset_seq;
		panelstruct->backlightinfo = &hx8379a_wvga_video_backlight;
		pinfo->mipi.panel_cmds
					= hx8379a_wvga_video_on_command;
		pinfo->mipi.num_of_panel_cmds
					= HX8379A_WVGA_VIDEO_ON_COMMAND;
		memcpy(phy_db->timing,
				hx8379a_wvga_video_timings, TIMING_SIZE);
		break;
	case OTM8018B_FWVGA_VIDEO_PANEL:
		panelstruct->paneldata    = &otm8018b_fwvga_video_panel_data;
		panelstruct->panelres     = &otm8018b_fwvga_video_panel_res;
		panelstruct->color        = &otm8018b_fwvga_video_color;
		panelstruct->videopanel   = &otm8018b_fwvga_video_video_panel;
		panelstruct->commandpanel = &otm8018b_fwvga_video_command_panel;
		panelstruct->state        = &otm8018b_fwvga_video_state;
		panelstruct->laneconfig   = &otm8018b_fwvga_video_lane_config;
		panelstruct->paneltiminginfo
					 = &otm8018b_fwvga_video_timing_info;
		panelstruct->panelresetseq
					 = &otm8018b_fwvga_video_reset_seq;
		panelstruct->backlightinfo = &otm8018b_fwvga_video_backlight;
		pinfo->mipi.panel_cmds
					= otm8018b_fwvga_video_on_command;
		pinfo->mipi.num_of_panel_cmds
					= OTM8018B_FWVGA_VIDEO_ON_COMMAND;
		memcpy(phy_db->timing,
				otm8018b_fwvga_video_timings, TIMING_SIZE);
		break;
	case NT35590_720P_VIDEO_PANEL:
		panelstruct->paneldata    = &nt35590_720p_video_panel_data;
		panelstruct->panelres     = &nt35590_720p_video_panel_res;
		panelstruct->color        = &nt35590_720p_video_color;
		panelstruct->videopanel   = &nt35590_720p_video_video_panel;
		panelstruct->commandpanel = &nt35590_720p_video_command_panel;
		panelstruct->state        = &nt35590_720p_video_state;
		panelstruct->laneconfig   = &nt35590_720p_video_lane_config;
		panelstruct->paneltiminginfo
					 = &nt35590_720p_video_timing_info;
		panelstruct->panelresetseq
					 = &nt35590_720p_video_panel_reset_seq;
		panelstruct->backlightinfo = &nt35590_720p_video_backlight;
		pinfo->mipi.panel_cmds
					= nt35590_720p_video_on_command;
		pinfo->mipi.num_of_panel_cmds
					= NT35590_720P_VIDEO_ON_COMMAND;
		memcpy(phy_db->timing,
				nt35590_720p_video_timings, TIMING_SIZE);
		break;
	case HX8389B_QHD_VIDEO_PANEL:
		panelstruct->paneldata    = &hx8389b_qhd_video_panel_data;
		panelstruct->panelres     = &hx8389b_qhd_video_panel_res;
		panelstruct->color        = &hx8389b_qhd_video_color;
		panelstruct->videopanel   = &hx8389b_qhd_video_video_panel;
		panelstruct->commandpanel = &hx8389b_qhd_video_command_panel;
		panelstruct->state        = &hx8389b_qhd_video_state;
		panelstruct->laneconfig   = &hx8389b_qhd_video_lane_config;
		panelstruct->paneltiminginfo
					 = &hx8389b_qhd_video_timing_info;
		panelstruct->panelresetseq
					 = &hx8389b_qhd_video_reset_seq;
		panelstruct->backlightinfo = &hx8389b_qhd_video_backlight;
		pinfo->mipi.panel_cmds
					= hx8389b_qhd_video_on_command;
		pinfo->mipi.num_of_panel_cmds
					= HX8389B_QHD_VIDEO_ON_COMMAND;
		memcpy(phy_db->timing,
				hx8389b_qhd_video_timings, TIMING_SIZE);
		break;
	default:
		dprintf(CRITICAL, "Panel ID not detected %d\n", panel_id);
		return false;
	}
	return true;
}

bool oem_panel_select(struct panel_struct *panelstruct,
			struct msm_panel_info *pinfo,
			struct mdss_dsi_phy_ctrl *phy_db)
{
	uint32_t hw_id = board_hardware_id();
	uint32_t platform_subtype = board_hardware_subtype();
	uint32_t target_id = board_target_id();

	target_id = (target_id >> 16) & 0xFF;

	switch (hw_id) {
	case HW_PLATFORM_QRD:
		switch (platform_subtype) {
			case QRD_DEF:
			case QRD_SKUAA:
				panel_id = ILI9806C_WVGA_VIDEO_PANEL;
				break;
			case QRD_SKUAB:
				if (target_id == 0x1)	// 1st HW version
					panel_id = OTM8018B_FWVGA_VIDEO_PANEL;
				else if (target_id == 0x2)	//2nd HW version
					panel_id = HX8389B_QHD_VIDEO_PANEL;
				else
					dprintf(CRITICAL, "SKUAB Display not enabled for %d type\n",
							target_id);
				break;
			default:
				dprintf(CRITICAL, "QRD Display not enabled for %d type\n",
							platform_subtype);
				return false;
		}
		break;
	case HW_PLATFORM_MTP:
		/*  For 720p MTP platform subtype*/
		if (1 == platform_subtype)
			panel_id = NT35590_720P_VIDEO_PANEL;
		else
			panel_id = TRULY_WVGA_VIDEO_PANEL;
		break;

	case HW_PLATFORM_SURF:
		panel_id = TRULY_WVGA_VIDEO_PANEL;
		break;
	default:
		dprintf(CRITICAL, "Display not enabled for %d HW type\n", hw_id);
		return false;
	}

	return init_panel_data(panelstruct, pinfo, phy_db);
}
