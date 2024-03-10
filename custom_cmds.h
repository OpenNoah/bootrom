#pragma once

typedef enum {
    CRNop               = 0,
    CRShowImage         = 1,
    CRStmpeGPIOIn       = 2,
    CRStmpeGPIOOut      = 3,
    CRStmpeGPIODir      = 4,
    CRKeyboardTest      = 5,
} custom_cmd_op_t;
