#include <stdio.h>
#include <ultra64.h>

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>

#include "controller_api.h"

#define DEADZONE    24
#define DEADZONE_SQ (DEADZONE * DEADZONE)

static u8 padbuf[256] __attribute__((aligned(64)));
static int init_done = 0;

static int joy_port = -1;
static int joy_slot = -1;
static int joy_id = -1;
static struct padButtonStatus joy_buttons __attribute__((aligned(64)));

static struct {
    u32 n64_btn;
    u32 sce_btn;
} joy_binds[] = {
    { A_BUTTON,     PAD_CROSS  },
    { B_BUTTON,     PAD_SQUARE },
    { L_TRIG,       PAD_L2     },
    { R_TRIG,       PAD_R2     },
    { Z_TRIG,       PAD_L1     },
    { Z_TRIG,       PAD_R1,    },
    { START_BUTTON, PAD_START  },
    { L_CBUTTONS,   PAD_LEFT   },
    { R_CBUTTONS,   PAD_RIGHT  },
    { U_CBUTTONS,   PAD_UP     },
    { D_CBUTTONS,   PAD_DOWN   },
};

static int num_joy_binds = sizeof(joy_binds) / sizeof(joy_binds[0]);

static inline int wait_pad(int tries) {
    int state = padGetState(joy_port, joy_slot);
    if (state == PAD_STATE_DISCONN) {
        joy_id = -1;
        return -1;
    }

    while ((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
        state = padGetState(joy_port, joy_slot);
        if (--tries == 0) break;
    }

    return 0;
}

static int detect_pad(void) {
    int id = padInfoMode(joy_port, joy_slot, PAD_MODECURID, 0);
    if (id <= 0) return -1;

    const int ext = padInfoMode(joy_port, joy_slot, PAD_MODECUREXID, 0);
    if (ext) id = ext;

    printf("controller_ps2: detected pad type %d\n", id);

    if (id == PAD_TYPE_DIGITAL || id == PAD_TYPE_DUALSHOCK)
        padSetMainMode(joy_port, joy_slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);

    return id;
}

static void controller_ps2_init(void) {
    if (SifLoadModule("rom0:SIO2MAN", 0, NULL) < 0) {
        printf("controller_ps2: SIO2MAN failed to load\n");
        return;
    }

    if (SifLoadModule("rom0:PADMAN", 0, NULL) < 0) {
        printf("controller_ps2: PADMAN failed to load\n");
        return;
    }

    padInit(0);

    const int numports = padGetPortMax();

    for (int port = 0; port < numports && joy_port < 0; ++port) {
        const int maxslots = padGetSlotMax(port);
        for (int slot = 0; slot < maxslots; ++slot) {
            if (padPortOpen(port, slot, padbuf) >= 0) {
                joy_port = port;
                joy_slot = slot;
                printf("controller_ps2: using pad (%d, %d)\n", port, slot);
                break;
            }
        }
    }

    if (joy_slot < 0 || joy_port < 0) {
        printf("controller_ps2: could not open a single port\n");
        return;
    }

    init_done = 1;
}

static void controller_ps2_read(OSContPad *pad) {
    if (!init_done) return;

    if (wait_pad(10) < 0)
        return; // nothing received

    if (joy_id < 0) {
        // pad not detected yet, do it
        joy_id = detect_pad();
        if (joy_id < 0) return; // still nothing
        if (wait_pad(10) < 0) return;
    }

    if (padRead(joy_port, joy_slot, &joy_buttons)) {
        const u32 btns = 0xffff ^ joy_buttons.btns;

        for (int i = 0; i < num_joy_binds; ++i)
            if (btns & joy_binds[i].sce_btn)
                pad->button |= joy_binds[i].n64_btn;

        const int lstick_x = (int)joy_buttons.ljoy_h - 128;
        const int lstick_y = (int)joy_buttons.ljoy_v - 128;
        const int rstick_x = (int)joy_buttons.rjoy_h - 128;
        const int rstick_y = (int)joy_buttons.rjoy_v - 128;

        if (rstick_x < -64)     pad->button |= L_CBUTTONS;
        else if (rstick_x > 63) pad->button |= R_CBUTTONS;
        if (rstick_y < -64)     pad->button |= U_CBUTTONS;
        else if (rstick_y > 63) pad->button |= D_CBUTTONS;

        const uint32_t lstick_mag = (u32)(lstick_x * lstick_x) + (u32)(lstick_y * lstick_y);
        if (lstick_mag > (u32)DEADZONE_SQ) {
            pad->stick_x = roundf(((float) lstick_x) / 128.f * 80.f);
            pad->stick_y = roundf(((float)-lstick_y) / 128.f * 80.f);
        }
    }
}

struct ControllerAPI controller_ps2 = {
    controller_ps2_init,
    controller_ps2_read
};
