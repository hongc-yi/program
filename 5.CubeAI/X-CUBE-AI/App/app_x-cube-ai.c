
/**
  ******************************************************************************
  * @file    app_x-cube-ai.c
  * @author  X-CUBE-AI C code generator
  * @brief   AI program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

 /*
  * Description
  *   v1.0 - Minimum template to show how to use the Embedded Client API
  *          model. Only one input and one output is supported. All
  *          memory resources are allocated statically (AI_NETWORK_XX, defines
  *          are used).
  *          Re-target of the printf function is out-of-scope.
  *   v2.0 - add multiple IO and/or multiple heap support
  *
  *   For more information, see the embeded documentation:
  *
  *       [1] %X_CUBE_AI_DIR%/Documentation/index.html
  *
  *   X_CUBE_AI_DIR indicates the location where the X-CUBE-AI pack is installed
  *   typical : C:\Users\[user_name]\STM32Cube\Repository\STMicroelectronics\X-CUBE-AI\7.1.0
  */

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#if defined ( __ICCARM__ )
#elif defined ( __CC_ARM ) || ( __GNUC__ )
#endif

/* System headers */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "app_x-cube-ai.h"
#include "main.h"
#include "ai_datatypes_defines.h"
#include "sin_calc.h"
#include "sin_calc_data.h"

/* USER CODE BEGIN includes */
#include <math.h>

/* 保存当前输入角度，防止 ai_run() 覆盖激活缓冲区后无法回读 */
static float g_current_angle_deg = 0.0f;
/* USER CODE END includes */

/* IO buffers ----------------------------------------------------------------*/

#if !defined(AI_SIN_CALC_INPUTS_IN_ACTIVATIONS)
AI_ALIGNED(4) ai_i8 data_in_1[AI_SIN_CALC_IN_1_SIZE_BYTES];
ai_i8* data_ins[AI_SIN_CALC_IN_NUM] = {
data_in_1
};
#else
ai_i8* data_ins[AI_SIN_CALC_IN_NUM] = {
NULL
};
#endif

#if !defined(AI_SIN_CALC_OUTPUTS_IN_ACTIVATIONS)
AI_ALIGNED(4) ai_i8 data_out_1[AI_SIN_CALC_OUT_1_SIZE_BYTES];
ai_i8* data_outs[AI_SIN_CALC_OUT_NUM] = {
data_out_1
};
#else
ai_i8* data_outs[AI_SIN_CALC_OUT_NUM] = {
NULL
};
#endif

/* Activations buffers -------------------------------------------------------*/

AI_ALIGNED(32)
static uint8_t pool0[AI_SIN_CALC_DATA_ACTIVATION_1_SIZE];

ai_handle data_activations0[] = {pool0};

/* AI objects ----------------------------------------------------------------*/

static ai_handle sin_calc = AI_HANDLE_NULL;

static ai_buffer* ai_input;
static ai_buffer* ai_output;

static void ai_log_err(const ai_error err, const char *fct)
{
  /* USER CODE BEGIN log */
  if (fct)
    printf("TEMPLATE - Error (%s) - type=0x%02x code=0x%02x\r\n", fct,
        err.type, err.code);
  else
    printf("TEMPLATE - Error - type=0x%02x code=0x%02x\r\n", err.type, err.code);

  do {} while (1);
  /* USER CODE END log */
}

static int ai_boostrap(ai_handle *act_addr)
{
  ai_error err;

  /* Create and initialize an instance of the model */
  err = ai_sin_calc_create_and_init(&sin_calc, act_addr, NULL);
  if (err.type != AI_ERROR_NONE) {
    ai_log_err(err, "ai_sin_calc_create_and_init");
    return -1;
  }

  ai_input = ai_sin_calc_inputs_get(sin_calc, NULL);
  ai_output = ai_sin_calc_outputs_get(sin_calc, NULL);

#if defined(AI_SIN_CALC_INPUTS_IN_ACTIVATIONS)
  /*  In the case where "--allocate-inputs" option is used, memory buffer can be
   *  used from the activations buffer. This is not mandatory.
   */
  for (int idx=0; idx < AI_SIN_CALC_IN_NUM; idx++) {
	data_ins[idx] = ai_input[idx].data;
  }
#else
  for (int idx=0; idx < AI_SIN_CALC_IN_NUM; idx++) {
	  ai_input[idx].data = data_ins[idx];
  }
#endif

#if defined(AI_SIN_CALC_OUTPUTS_IN_ACTIVATIONS)
  /*  In the case where "--allocate-outputs" option is used, memory buffer can be
   *  used from the activations buffer. This is no mandatory.
   */
  for (int idx=0; idx < AI_SIN_CALC_OUT_NUM; idx++) {
	data_outs[idx] = ai_output[idx].data;
  }
#else
  for (int idx=0; idx < AI_SIN_CALC_OUT_NUM; idx++) {
	ai_output[idx].data = data_outs[idx];
  }
#endif

  return 0;
}

static int ai_run(void)
{
  ai_i32 batch;

  batch = ai_sin_calc_run(sin_calc, ai_input, ai_output);
  if (batch != 1) {
    ai_log_err(ai_sin_calc_get_error(sin_calc),
        "ai_sin_calc_run");
    return -1;
  }

  return 0;
}

/* USER CODE BEGIN 2 */
int acquire_and_process_data(ai_i8* data[])
{
  /*
   * 输入缓冲区是 ai_i8 类型，但模型实际输入格式是 FLOAT（4字节）
   * 需要把 ai_i8* 强制转为 float* 来写入浮点数据
   *
   * data[0] 指向 data_in_1，大小为 AI_SIN_CALC_IN_1_SIZE_BYTES = 4 字节 = 1 个 float
   */
  float* input = (float*)data[0];

  /*
   * 连续扫角：模型是用度数（1°~360°）训练的，不是弧度！
   * 输入范围必须和训练数据一致：1.0 ~ 360.0
   * 间隔 50ms，步进约 3.6°/帧，每秒 20 个点，约 5 秒一个周期
   */
  static float angle_deg = 1.0f;
  static uint32_t last_run_tick = 0;
  const float step_deg = 3.6f;  // 步进 3.6°/帧，一个周期约 100 帧 ≈ 5 秒

  /* 限速：50ms 一帧 */
  if (HAL_GetTick() - last_run_tick < 50) {
      return -1;
  }
  last_run_tick = HAL_GetTick();

  /* 归一化到 [-1, 1]：新模型用归一化输入训练的，必须匹配 */
  *input = (angle_deg - 180.5f) / 179.5f;
  g_current_angle_deg = angle_deg;

  /* 递增，超过 360° 归零 */
  angle_deg += step_deg;
  if (angle_deg > 360.0f) {
    angle_deg -= 360.0f;
  }

  return 0;
}

int post_process(ai_i8* data[])
{
  /*
   * Vofa+ FireWater 协议：每行纯数字，逗号分隔
   * 格式: angle_deg, ai_sin, std_sin
   *
   * 模型输入是度数，标准 sin 计算需要转换为弧度
   */
  float ai_output = *(float*)data[0];       // AI 模型输出
  float angle_deg = g_current_angle_deg;    // 使用推理前保存的副本（激活缓冲区已被 ai_run 覆盖）
  float angle_rad = angle_deg * 3.14159265f / 180.0f;
  float std_sin   = sinf(angle_rad);        // 标准 math 库参考值

  printf("%.3f,%.6f,%.6f\r\n",
         (angle_deg - 180.0f) / 180.0f,   // 归一化角度 [-1, 1]，与 sin 同量级便于同轴对比
         ai_output,                        // AI sin 输出
         std_sin);                         // 标准 sin 输出

  return 0;
}
/* USER CODE END 2 */

/* Entry points --------------------------------------------------------------*/

void MX_X_CUBE_AI_Init(void)
{
    /* USER CODE BEGIN 5 */
  printf("\r\nTEMPLATE - initialization\r\n");

  ai_boostrap(data_activations0);
    /* USER CODE END 5 */
}

void MX_X_CUBE_AI_Process(void)
{
    /* USER CODE BEGIN 6 */
  int res = -1;

  if (sin_calc) {
    /*
     * 三步流水线:
     *   1. 采集数据 → 2. AI 推理 → 3. 后处理输出
     *
     * acquire_and_process_data 返回值:
     *    0  = 有数据，继续推理
     *   -1  = 被限速跳过，本次不推理
     *   其他 = 错误
     */
    res = acquire_and_process_data(data_ins);

    if (res == 0) {
      /* 2 - process the data - call inference engine */
      res = ai_run();
      /* 3- post-process the predictions */
      if (res == 0) {
        res = post_process(data_outs);
      }
    }
  }

  /* 仅真正的错误（>0 或 < -1）才触发错误处理，-1 是正常的限速跳过 */
  if (res != 0 && res != -1) {
    ai_error err = {AI_ERROR_INVALID_STATE, AI_ERROR_CODE_NETWORK};
    ai_log_err(err, "Process has FAILED");
  }
    /* USER CODE END 6 */
}
#ifdef __cplusplus
}
#endif
