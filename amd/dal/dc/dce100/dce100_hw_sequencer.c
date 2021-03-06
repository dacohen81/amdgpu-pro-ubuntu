/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */
#include "dm_services.h"
#include "dc.h"
#include "core_dc.h"
#include "core_types.h"
#include "hw_sequencer.h"
#include "dce100_hw_sequencer.h"
#include "dce110/dce110_hw_sequencer.h"

/* include DCE10 register header files */
#include "dce/dce_10_0_d.h"
#include "dce/dce_10_0_sh_mask.h"

struct dce100_hw_seq_reg_offsets {
	uint32_t blnd;
	uint32_t crtc;
};

enum blender_mode {
	BLENDER_MODE_CURRENT_PIPE = 0,/* Data from current pipe only */
	BLENDER_MODE_OTHER_PIPE, /* Data from other pipe only */
	BLENDER_MODE_BLENDING,/* Alpha blending - blend 'current' and 'other' */
	BLENDER_MODE_STEREO
};

static const struct dce100_hw_seq_reg_offsets reg_offsets[] = {
{
	.blnd = (mmBLND0_BLND_CONTROL - mmBLND_CONTROL),
	.crtc = (mmCRTC0_CRTC_GSL_CONTROL - mmCRTC_GSL_CONTROL),
},
{
	.blnd = (mmBLND1_BLND_CONTROL - mmBLND_CONTROL),
	.crtc = (mmCRTC1_CRTC_GSL_CONTROL - mmCRTC_GSL_CONTROL),
},
{
	.blnd = (mmBLND2_BLND_CONTROL - mmBLND_CONTROL),
	.crtc = (mmCRTC2_CRTC_GSL_CONTROL - mmCRTC_GSL_CONTROL),
},
{
	.blnd = (mmBLND3_BLND_CONTROL - mmBLND_CONTROL),
	.crtc = (mmCRTC3_CRTC_GSL_CONTROL - mmCRTC_GSL_CONTROL),
},
{
	.blnd = (mmBLND4_BLND_CONTROL - mmBLND_CONTROL),
	.crtc = (mmCRTC4_CRTC_GSL_CONTROL - mmCRTC_GSL_CONTROL),
},
{
	.blnd = (mmBLND5_BLND_CONTROL - mmBLND_CONTROL),
	.crtc = (mmCRTC5_CRTC_GSL_CONTROL - mmCRTC_GSL_CONTROL),
}
};

#define HW_REG_BLND(reg, id)\
	(reg + reg_offsets[id].blnd)

#define HW_REG_CRTC(reg, id)\
	(reg + reg_offsets[id].crtc)

/*******************************************************************************
 * Private definitions
 ******************************************************************************/
/***************************PIPE_CONTROL***********************************/
static void dce100_enable_fe_clock(
	struct dc_context *ctx, uint8_t controller_id, bool enable)
{
	uint32_t value = 0;
	uint32_t addr;

	addr = HW_REG_CRTC(mmDCFE_CLOCK_CONTROL, controller_id);

	value = dm_read_reg(ctx, addr);

	set_reg_field_value(
		value,
		enable,
		DCFE_CLOCK_CONTROL,
		DCFE_CLOCK_ENABLE);

	dm_write_reg(ctx, addr, value);
}

static bool dce100_pipe_control_lock(
	struct dc_context *ctx,
	uint8_t controller_idx,
	uint32_t control_mask,
	bool lock)
{
	uint32_t addr = HW_REG_BLND(mmBLND_V_UPDATE_LOCK, controller_idx);
	uint32_t value = dm_read_reg(ctx, addr);

	if (control_mask & PIPE_LOCK_CONTROL_GRAPHICS)
		set_reg_field_value(
			value,
			lock,
			BLND_V_UPDATE_LOCK,
			BLND_DCP_GRPH_V_UPDATE_LOCK);

	if (control_mask & PIPE_LOCK_CONTROL_SCL)
		set_reg_field_value(
			value,
			lock,
			BLND_V_UPDATE_LOCK,
			BLND_SCL_V_UPDATE_LOCK);

	if (control_mask & PIPE_LOCK_CONTROL_SURFACE)
		set_reg_field_value(
			value,
			lock,
			BLND_V_UPDATE_LOCK,
			BLND_DCP_GRPH_SURF_V_UPDATE_LOCK);

	if (control_mask & PIPE_LOCK_CONTROL_BLENDER) {
		set_reg_field_value(
			value,
			lock,
			BLND_V_UPDATE_LOCK,
			BLND_BLND_V_UPDATE_LOCK);
	}

	if (control_mask & PIPE_LOCK_CONTROL_MODE)
		set_reg_field_value(
			value,
			lock,
			BLND_V_UPDATE_LOCK,
			BLND_V_UPDATE_LOCK_MODE);

	dm_write_reg(ctx, addr, value);


	return true;
}

static void dce100_set_blender_mode(
	struct core_dc *dc,
	uint8_t controller_id,
	uint32_t mode)
{
	uint32_t value;
	uint32_t addr = HW_REG_BLND(mmBLND_CONTROL, controller_id);
	uint32_t blnd_mode;
	uint32_t feedthrough = 0;

	struct dc_context *ctx = dc->ctx;

	switch (mode) {
	case BLENDER_MODE_OTHER_PIPE:
		feedthrough = 0;
		blnd_mode = 1;
		break;
	case BLENDER_MODE_BLENDING:
		feedthrough = 0;
		blnd_mode = 2;
		break;
	case BLENDER_MODE_CURRENT_PIPE:
	default:
		feedthrough = 1;
		blnd_mode = 0;
		break;
	}

	value = dm_read_reg(ctx, addr);

	set_reg_field_value(
		value,
		feedthrough,
		BLND_CONTROL,
		BLND_FEEDTHROUGH_EN);

	set_reg_field_value(
		value,
		blnd_mode,
		BLND_CONTROL,
		BLND_MODE);

	dm_write_reg(ctx, addr, value);
}

static bool dce100_enable_display_power_gating(
	struct core_dc *dc,
	uint8_t controller_id,
	struct dc_bios *dcb,
	enum pipe_gating_control power_gating)
{
	enum bp_result bp_result = BP_RESULT_OK;
	enum bp_pipe_control_action cntl;
	struct dc_context *ctx = dc->ctx;

	if (power_gating == PIPE_GATING_CONTROL_INIT)
		cntl = ASIC_PIPE_INIT;
	else if (power_gating == PIPE_GATING_CONTROL_ENABLE)
		cntl = ASIC_PIPE_ENABLE;
	else
		cntl = ASIC_PIPE_DISABLE;

	if (!(power_gating == PIPE_GATING_CONTROL_INIT && controller_id != 0)){

		bp_result = dcb->funcs->enable_disp_power_gating(
						dcb, controller_id + 1, cntl);

		/* Revert MASTER_UPDATE_MODE to 0 because bios sets it 2
		 * by default when command table is called
		 */
		dm_write_reg(ctx,
			HW_REG_CRTC(mmMASTER_UPDATE_MODE, controller_id),
			0);
	}

	if (bp_result == BP_RESULT_OK)
		return true;
	else
		return false;
}

static void set_display_mark_for_pipe_if_needed(struct core_dc *dc,
		struct pipe_ctx *pipe_ctx,
		struct validate_context *context)
{
	/* Do nothing until we have proper bandwitdth calcs */
}

static void set_displaymarks(
		const struct core_dc *dc, struct validate_context *context)
{
	/* Do nothing until we have proper bandwitdth calcs */
}

static void set_bandwidth(struct core_dc *dc)
{
	/* Do nothing until we have proper bandwitdth calcs */
}

static void enable_hw_base_light_sleep(void)
{
	/* TODO: implement */
}

static void disable_sw_manual_control_light_sleep(void)
{
	/* TODO: implement */
}

static void enable_sw_manual_control_light_sleep(void)
{
	/* TODO: implement */
}

static void dal_dc_clock_gating_dce100_power_up(struct dc_context *ctx, bool enable)
{
	if (enable) {
		enable_hw_base_light_sleep();
		disable_sw_manual_control_light_sleep();
	} else {
		enable_sw_manual_control_light_sleep();
	}
}

/**************************************************************************/

bool dce100_hw_sequencer_construct(struct core_dc *dc)
{
	dce110_hw_sequencer_construct(dc);

	/* TODO: dce80 is empty implementation at the moment*/
	dc->hwss.clock_gating_power_up = dal_dc_clock_gating_dce100_power_up;

	dc->hwss.enable_display_power_gating = dce100_enable_display_power_gating;
	dc->hwss.enable_fe_clock = dce100_enable_fe_clock;
	dc->hwss.pipe_control_lock = dce100_pipe_control_lock;
	dc->hwss.set_blender_mode = dce100_set_blender_mode;
	dc->hwss.set_displaymarks = set_displaymarks;
	dc->hwss.increase_watermarks_for_pipe = set_display_mark_for_pipe_if_needed;
	dc->hwss.set_bandwidth = set_bandwidth;

	return true;
}

