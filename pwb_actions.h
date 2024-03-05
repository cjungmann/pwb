#ifndef PWB_ACTIONS_H
#define PWB_ACTIONS_H

#include "pwb_builtin.h"

#include "pwb_handle.h"
#include "pwb_verbs.h"

PWB_RESULT pwb_action_dummy(PWBH *handle, ACLONE *args);

PWB_RESULT pwb_action_init(PWBH *handle, ACLONE *args);
PWB_RESULT pwb_action_restore(PWBH *handle, ACLONE *args);

PWB_RESULT pwb_action_declare(PWBH *handle, ACLONE *args);
PWB_RESULT pwb_action_set_margins(PWBH *handle, ACLONE *args);
PWB_RESULT pwb_action_report(PWBH * handle, ACLONE *args);
PWB_RESULT pwb_action_trigger(PWBH * handle, ACLONE *args);
PWB_RESULT pwb_action_plot_line(PWBH *handle, ACLONE *args);
PWB_RESULT pwb_action_plot_screen(PWBH *handle, ACLONE *args);

PWB_RESULT pwb_action_declare_keymap(PWBH *handle, ACLONE *args);

// int pwb_action_start.c:
PWB_RESULT pwb_action_start(PWBH *handle, ACLONE *args);

#endif
