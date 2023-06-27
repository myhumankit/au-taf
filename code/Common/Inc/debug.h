#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdint.h>

#include "base.h"

#ifndef TRACE_ON
#define TRACE_ON 1
#endif


/*
 *  Debug part
 */
 
#define DEBUG_LINE 2048

typedef struct {
    uint8_t data[DEBUG_LINE];
    uint8_t wr_index;
    uint8_t rd_index;
} debug_buffer_t;

struct s_debug_context_t;

typedef int (*debug_command_handler_t)(struct s_debug_context_t *pContext);

typedef enum {
  DEBUG_IDLE,
  DEBUG_IN_PROGRESS,
  DEBUG_PROMPT,
  DEBUG_FLUSHING
  } debug_state_e;


typedef struct s_debug_context_t
{
    uint8_t field_start;
    uint8_t field_next;
    uint8_t state;
    uint8_t command;
    debug_command_handler_t handler;
    debug_buffer_t input;
    debug_buffer_t line;
    debug_buffer_t output;
  void *user_context;
} debug_context_t;

/**@brief Function for initializing the UARTs. */

void debug_start(void);
void debug_loop(void);

typedef struct {
    char *name;
    char *help;
    debug_command_handler_t handler;
    void *next;
} debug_command_t;

int debug_next_field(void);

void debug_memory_display(uint32_t addr, uint32_t len, uint8_t size);

void debug_set_user_context(void *user_context);

void debug_add_command(debug_command_t *handler);
void debug_add_commands(debug_command_t *handlers, int n);

void debug_process(void);

void debug_init(void);
void debug_loop(void);

int debug_get_next_char(void);
int debug_get_next_string(uint8_t **start, uint32_t *length);
void debug_update_next_string(uint32_t length);
void debug_add_char(int c);
void debug_putc(uint8_t c);
void debug_puts(char *s);
int debug_tx_empty();

debug_context_t *get_dbg();

extern uint32_t debug_mode;

#define DEBUG_MODE(n) if (debug_mode & (1<<(n)))



#endif
