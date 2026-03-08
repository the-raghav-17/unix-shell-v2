#ifndef SIG_H_
#define SIG_H_


void reset_signal_disposition(void);
void set_sigchld_disposition(void);
void ignore_sigchld(void);
void block_sigchld(void);
void unblock_sigchld(void);


#endif
