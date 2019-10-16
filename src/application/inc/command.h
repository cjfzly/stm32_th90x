//#include "stm32l1xx.h"
#include <stdint.h>//头文件描述int_8类型

#define size_cmd_tbl (sizeof(char *)*2+sizeof(int))//1个cmd_tbl结构的大小=0x14
#define NUM_OF_CMD (sizeof(cmd_tbl)/size_cmd_tbl)
#define CFG_MAX_BUF 128 //定义一个命令所有的最大buffer
#define CFG_MAX_ARG 10 //定义一个命令最多支持的参数个数，包括命令本身
#define ASCII_SPACE 0x20
#define ASCII_BACKSPACE 8
#define CFG_MAX_HISTORY 5

#define NEXT_COMMAND 1
#define PREVIOUS_COMMAND 0



struct cmd_tbl_s {
	char		*name;		/* Command Name			*/
//	int		maxargs;	/* maximum number of arguments	*/
//	int		repeatable;	/* autorepeat allowed?		*/
					/* Implementation function	*/
	uint8_t		(*cmd)(int, uint8_t *[]);//有变化
	char		*usage;		/* Usage message	(short)	*/
};

typedef struct cmd_tbl_s	cmd_tbl_t;

extern uint8_t usart1_buf;
extern uint8_t usart1_rec_data_flag;


uint8_t do_version (int argc, uint8_t *argv[]);
uint8_t do_test(int argc, uint8_t *argv[]);
uint8_t do_help(int argc, uint8_t *argv[]);

cmd_tbl_t *find_cmd(uint8_t *cmd);
uint8_t parse_line(uint8_t *line, uint8_t *argv[]);
uint8_t run_commnad(uint8_t *cmd);
void copy_new_command_line(void);
void get_command_line(uint8_t bool_next);
void clean_cursor_word(void);
void clean_command_line(void);
//int size_of_str(uint8_t *buf);
void print_log(void);
void send_uart_cursor(uint8_t cmd);
void isr_uart(uint8_t inBuf);
void USART1_REC_DATA(void);
void check_uart_data(void);
//extern cmd_tbl_t  __u_boot_cmd_start;
//extern cmd_tbl_t  __u_boot_cmd_end;


/* common/command.c */
//cmd_tbl_t *find_cmd(const char *cmd);

#ifdef CONFIG_AUTO_COMPLETE
extern void install_auto_complete(void);
extern int cmd_auto_complete(const char *const prompt, char *buf, int *np, int *colp);
#endif


