#pragma once

#include "InputState.h"

// 初始化调试串口
void print_init();

// 打印当前物理输入状态
void print_physical_input(const InputState &state);