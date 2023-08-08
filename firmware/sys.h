#ifndef SYS_H
#define SYS_H

void sys_init(void);

// Some constants regarding configuration
#define OSCHF_FREQ 24000000UL
#define CLK_MAIN_FREQ OSCHF_FREQ
#define CLK_PER_FREQ CLK_MAIN_FREQ
#define F_CPU CLK_MAIN_FREQ

#endif
