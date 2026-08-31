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

#include "taskScheduler.h"

/* ======================================= 私有函数 ======================================= */
static uint8_t Scheduler_Contains(const Scheduler_t *sched,
                                  const SchedTask_t *task)
{
    const SchedTask_t *p;

    if ((sched == 0) ||
        (task == 0) ||
        (sched->head == 0))
    {
        return 0U;
    }

    p = sched->head;

    do
    {
        if (p == task)
        {
            return 1U;
        }

        p = p->next;

    } while ((p != 0) &&
             (p != sched->head));

    return 0U;
}

/* ======================================= 调度器初始化 ======================================= */
void Scheduler_Init(Scheduler_t *sched,
                    uint32_t (*get_tick)(void))
{
    if (sched == 0)
    {
        return;
    }

    sched->head = 0;
    sched->tail = 0;
    sched->current = 0;

    sched->get_tick = get_tick;
}

/* ======================================= 任务初始化 ======================================= */
void Scheduler_TaskInit(SchedTask_t *task,
                        SchedCallback_t callback,
                        void *arg,
                        uint32_t period)
{
    if (task == 0)
    {
        return;
    }

    task->next = 0;
    task->callback = callback;
    task->arg = arg;

    task->period = period;
    task->last_tick = 0U;

    task->enabled = 1U;
    task->linked = 0U;
}

/* ======================================= 添加任务 ======================================= */
SchedResult_t Scheduler_AddTask(Scheduler_t *sched,
                                SchedTask_t *task)
{
    uint32_t now;

    if ((sched == 0) ||
        (task == 0) ||
        (sched->get_tick == 0) ||
        (task->callback == 0) ||
        (task->period == 0U))
    {
        return SCHED_ERR_PARAM;
    }

    if ((task->linked != 0U) ||
        Scheduler_Contains(sched, task))
    {
        return SCHED_ERR_EXIST;
    }

    now = sched->get_tick();

    task->last_tick = now;

    task->enabled = 1U;
    task->linked = 1U;

    if (sched->head == 0)
    {
        task->next = task;

        sched->head = task;
        sched->tail = task;

        sched->current = task;

        return SCHED_OK;
    }

    task->next = sched->head;

    sched->tail->next = task;

    sched->tail = task;

    return SCHED_OK;
}

/* ======================================= 删除任务 ======================================= */
SchedResult_t Scheduler_RemoveTask(Scheduler_t *sched,
                                   SchedTask_t *task)
{
    SchedTask_t *prev;
    SchedTask_t *cur;
    SchedTask_t *next;

    if ((sched == 0) ||
        (task == 0))
    {
        return SCHED_ERR_PARAM;
    }

    if ((sched->head == 0) ||
        (task->linked == 0U))
    {
        return SCHED_ERR_NOT_FOUND;
    }

    prev = sched->tail;
    cur = sched->head;

    do
    {
        if (cur == task)
        {
            next = cur->next;

            if (cur == cur->next)
            {
                sched->head = 0;
                sched->tail = 0;
                sched->current = 0;
            }

            else
            {
                prev->next = cur->next;

                if (sched->head == cur)
                {
                    sched->head = cur->next;
                }

                if (sched->tail == cur)
                {
                    sched->tail = prev;
                }

                if (sched->current == cur)
                {
                    sched->current = next;
                }
            }

            if (sched->head == 0)
            {
                sched->current = 0;
            }

            cur->next = 0;
            cur->linked = 0U;
            cur->enabled = 0U;

            return SCHED_OK;
        }

        prev = cur;
        cur = cur->next;

    } while ((cur != 0) &&
             (cur != sched->head));

    return SCHED_ERR_NOT_FOUND;
}

/* ======================================= 启用任务 ======================================= */
SchedResult_t Scheduler_EnableTask(Scheduler_t *sched,
                                   SchedTask_t *task)
{
    if ((sched == 0) ||
        (task == 0))
    {
        return SCHED_ERR_PARAM;
    }

    if ((task->linked == 0U) ||
        !Scheduler_Contains(sched, task))
    {
        return SCHED_ERR_NOT_FOUND;
    }

    if (sched->get_tick != 0)
    {
        task->last_tick = sched->get_tick();
    }

    task->enabled = 1U;

    return SCHED_OK;
}

/* ======================================= 禁用任务 ======================================= */
SchedResult_t Scheduler_DisableTask(Scheduler_t *sched,
                                    SchedTask_t *task)
{
    if ((sched == 0) ||
        (task == 0))
    {
        return SCHED_ERR_PARAM;
    }

    if ((task->linked == 0U) ||
        !Scheduler_Contains(sched, task))
    {
        return SCHED_ERR_NOT_FOUND;
    }

    task->enabled = 0U;

    return SCHED_OK;
}

/* ======================================= 执行调度 ======================================= */
void Scheduler_Run(Scheduler_t *sched)
{
    SchedTask_t *task;
    SchedTask_t *start;

    uint32_t now;

    if ((sched == 0) ||
        (sched->head == 0) ||
        (sched->current == 0) ||
        (sched->get_tick == 0))
    {
        return;
    }

    now = sched->get_tick();

    task = sched->current;

    start = task;

    do
    {
        if ((task->linked != 0U) &&
            (task->enabled != 0U) &&
            (task->callback != 0) &&
            ((uint32_t)(now - task->last_tick) >=
             task->period))
        {
            SchedCallback_t callback;
            void *arg;

            callback = task->callback;
            arg = task->arg;

            sched->current = task->next;

            task->last_tick = now;

            callback(arg);

            return;
        }

        task = task->next;

    } while ((task != 0) &&
             (task != start));
}
