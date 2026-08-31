# A-lightweight-cooperative-task-scheduler-suitable-for-embedded-systems
This scheduler is essentially a lightweight task management shell. It demonstrates to you that even without an operating system, you can still write elegant bare-metal code using the concepts of "time slices" and "state machines".

In embedded development, when it comes to "multi-task scheduling", most people's first reaction is to use an RTOS (such as FreeRTOS, RT-Thread). Indeed, RTOS addresses the pain points of "blocking delay" and "concurrency", but have you encountered the following scenarios:
1. A status indicator light that needs to blink regularly;
2. Motor drive/PID control operations that require rapid response;
3. Sensor acquisition, periodic reading of sensor values;
4. BootLoader type, wanting to quickly jump without introducing a complex system;
5. Other scenarios that require strong time sensitivity.

Just needing a periodic task scheduling function requires running a real-time operating system on the MCU?
Especially on resource-sensitive chips like Cortex-M0/M3, a complete FreeRTOS kernel often has several KB of RAM/ROM overhead. More importantly, when doing motor FOV (field-oriented control) or high-frequency AD sampling, the frequent interrupt (critical section) and context switching of RTOS will make your control loop response "sluggish".

During my studies, in a competition project, I attempted to use an RTOS. At that time, because there were indeed many tasks that needed to be executed periodically in the project, my first reaction was to use an RTOS, but I found that when implementing precise motor control and performing PID calculations, it was impossible to achieve the desired effect. Due to time constraints, I had to use a rather clumsy method: designing the periodic scheduling flags of each task in the timer interrupt and calling them one by one in the system loop. This sacrificed some performance. So, for "turning on a light", do we really need to run an operating system kernel on the MCU?

Later, I wrote a lightweight task scheduler for bare metal. Implemented using a linked list, without system kernel mode, it completed 90% of the task scheduling functions of RTOS while retaining the most precious quality of bare metal: hard real-time.

It is undeniable that the position of RTOS in embedded systems is unquestionable. The key is to distinguish the usage scenarios, not all scenarios can or need to use RTOS.
What can be solved by bare metal, don't use the system; what has to be used with the system, don't force bare metal.

Why do we sometimes need to "escape" from RTOS?
1. Each task stack of RTOS often requires allocating several hundred bytes (even several KB). If your project has many functions or for 8051 or Cortex-M0+ with only 2KB RAM, this is almost a fatal flaw.
2. The "glass ceiling" of real-time. Many people mistakenly think that RTOS is the synonym of real-time, but that's not the case. The "real-time" core of RTOS is not about "fast", but about "certainty" and "predictability". The interrupt response of bare metal is a hardware-level jump, the delay is usually in nanoseconds (several ns), and the execution order is completely controlled by you (priority grouping). While RTOS maintains the consistency of the kernel data structure by shielding interrupts when entering the critical section. Although the time is very short, the cumulative microsecond-level (several us to several tens of us) delay in the FOV current loop (PWM interrupt frequency 20KHz to 50KHz) is sufficient to make PID parameters soft.

Conclusion: If your application requires deterministic nanosecond-level jitter, bare metal almost is the only choice.

The architecture design of this scheduler: coroutines + time slice rotation (without OS kernel mode)
Different from preemptive RTOS (where high-priority tasks can interrupt low-priority tasks at any time), this scheduler adopts a cooperative approach. Tasks must actively give up the CPU.
Advantages: There is no on-site protection stack action for task switching, and the switching speed is extremely fast (pure C function pointer jump), and there is no need to worry about the re-entry problem of shared resources (because it will not be preempted at any time).

Therefore: The master of "control"
RTOS makes embedded development simple, but it is a layer of "armor". When you need precise control, this layer of armor may become a constraint.
The scheduler I wrote is essentially a lightweight task management shell. It tells you: Even without an operating system, you can still write elegant bare-metal code using the concepts of "time slice" and "state machine".
It allows you to enjoy the nanosecond-level response of bare-metal while getting rid of the piles of flag bit flags in the endless while(1) loop.

How to use:

/*Define the scheduler instance and each task node*/
Scheduler_t scheduler;
SchedTask_t task1_t;
SchedTask_t task2_t;
SchedTask_t task3_t;

/*Define the task callback function*/
SchedCallback_t task1_cb_func(void)
{
	......
}

SchedCallback_t task2_cb_func(void)
{
	......
}

SchedCallback_t task3_cb_func(void)
{
	......
}

/*Initialize the scheduler and tasks*/
Scheduler_Init(&scheduler, SYS_GetTick); /*Use system TICK, incrementing by 1 MS*/
Scheduler_TaskInit(&task1_t, task1_cb_func, 0, 200U); /*Task 1 is executed every 200 ms*/
Scheduler_TaskInit(&task2_t, task2_cb_func, 0, 500U); /*Task 2 is executed every 500 ms*/
Scheduler_TaskInit(&task3_t, task3_cb_func, 0, 1000U); /*Task 3 is executed every 1000 ms*/

/*Add the task to this scheduler*/
Scheduler_AddTask(&scheduler, &task1_t);
Scheduler_AddTask(&scheduler, &task2_t);
Scheduler_AddTask(&scheduler, &task3_t);

/*Call within the main loop*/
while(1)
{
	Scheduler_Run(&scheduler);
}
