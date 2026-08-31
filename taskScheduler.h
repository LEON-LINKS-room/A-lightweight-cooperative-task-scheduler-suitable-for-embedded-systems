/*******************************************************************************
MIT License

Copyright (c) 2021 LEON-LINKS-room

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#ifndef __TASKSCHEDULER_H__
#define __TASKSCHEDULER_H__

#include <stdint.h>

typedef enum
{
    SCHED_OK = 0,
    SCHED_ERR_PARAM = -1,
    SCHED_ERR_EXIST = -2,
    SCHED_ERR_NOT_FOUND = -3
} SchedResult_t;

typedef void (*SchedCallback_t)(void *arg);

/* ======================================= 任务控制块 ======================================= */
typedef struct SchedTask
{
    struct SchedTask *next;
    SchedCallback_t callback;
    void *arg;
    uint32_t period;
    uint32_t last_tick;
    uint8_t enabled;
    uint8_t linked;

} SchedTask_t;

/* ======================================= 调度器对象 ======================================= */
typedef struct
{
    SchedTask_t *head;
    SchedTask_t *tail;
    SchedTask_t *current;
    uint32_t (*get_tick)(void);
} Scheduler_t;

/* ======================================= 调度器初始化 ======================================= */
void Scheduler_Init(Scheduler_t *sched,
                    uint32_t (*get_tick)(void));

/* ======================================= 任务初始化 ======================================= */
void Scheduler_TaskInit(SchedTask_t *task,
                        SchedCallback_t callback,
                        void *arg,
                        uint32_t period);

/* ======================================= 任务管理 ======================================= */
SchedResult_t Scheduler_AddTask(Scheduler_t *sched,
                                SchedTask_t *task);

SchedResult_t Scheduler_RemoveTask(Scheduler_t *sched,
                                   SchedTask_t *task);

SchedResult_t Scheduler_EnableTask(Scheduler_t *sched,
                                   SchedTask_t *task);

SchedResult_t Scheduler_DisableTask(Scheduler_t *sched,
                                    SchedTask_t *task);

/* ======================================= 调度 ======================================= */

void Scheduler_Run(Scheduler_t *sched);

#endif
