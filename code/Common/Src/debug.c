#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "debug.h"



static debug_context_t debug_context;
uint32_t debug_mode;

typedef struct {
    debug_command_t *handler;
} help_command_t;

typedef struct {
    uint32_t address;
    uint32_t len;
    uint8_t size;
} md_command_t;

static union {
    md_command_t md;
    help_command_t help;
} command_context;

int debug_md_handler(debug_context_t *pContext);

debug_context_t *get_dbg()
{
  return &debug_context;
}
  
void debug_init(void)
{
  debug_context.input.wr_index = 0;
  debug_context.input.rd_index = 0;
  debug_context.line.wr_index = 0;
  debug_context.line.rd_index = 0;
  debug_context.output.wr_index = 0;
  debug_context.output.rd_index = 0;
  debug_context.field_start = 0;
  debug_context.field_next = 0;
  debug_context.state = DEBUG_PROMPT;
  debug_context.user_context = NULL;
}

void debug_set_user_context(void *user_context)
{
  debug_context.user_context = user_context;
}


void debug_putc(uint8_t c)
{
    debug_context_t *pContext = &debug_context;
    
    pContext->output.data[pContext->output.wr_index] = c;
    pContext->output.wr_index++;
    if (pContext->output.wr_index == sizeof(pContext->output.data)) {
        pContext->output.wr_index = 0;
    }
}

int debug_tx_empty()
{
  debug_context_t *pContext = &debug_context;
  return pContext->output.wr_index == pContext->output.rd_index;
}

void debug_puts(char *s)
{
    int len = strlen(s);
    debug_context_t *pContext = &debug_context;
    
    if (pContext->output.wr_index + len >= sizeof(pContext->output.data)) {
        int first_size = sizeof(pContext->output.data)- pContext->output.wr_index;
        memcpy(&pContext->output.data[pContext->output.wr_index],s,first_size);
        len = len - first_size;
       if (len > 0) {
            if (len > pContext->output.wr_index) {
                len = pContext->output.wr_index;
            }
            memcpy(pContext->output.data,&s[first_size],len);
            pContext->output.wr_index = len;
      }
    } else {
        memcpy(&pContext->output.data[pContext->output.wr_index],s,len);
        pContext->output.wr_index += len;
    }
    
}

int debug_next_field(void)
{
    debug_context_t *pContext = &debug_context;
    uint8_t field_start;
    uint8_t field_end;
    
    field_start = pContext->field_next;
    
    while ((field_start < pContext->line.wr_index) &&
            (pContext->line.data[field_start] == ' ')) {
        field_start++;
    }
    if (field_start >= pContext->line.wr_index) {
        return 0;
    }
    pContext->field_start = field_start;
    field_end = field_start;
    while ((field_end < pContext->line.wr_index) &&
            (pContext->line.data[field_end] != ' ')) {
        field_end++;
    }
    pContext->line.data[field_end] = '\0';
    pContext->field_next = field_end + 1;
    return 1;
}

void debug_process(void);

void debug_loop(void)
{
    debug_context_t *pContext = &debug_context;

    switch (pContext->state) {
      case DEBUG_IDLE:
        if (pContext->output.rd_index != pContext->output.wr_index) {
            break;
        }
        pContext->output.wr_index = 0;
        pContext->output.rd_index = 0;
        if (pContext->input.rd_index != pContext->input.wr_index) {
            uint8_t c;
            c = pContext->input.data[pContext->input.rd_index];
            pContext->input.rd_index++;
            if (pContext->input.rd_index >= sizeof(pContext->input.data)) {
                pContext->input.rd_index = 0;
            }
            
            if ((c == '\r') || (c == '\n')) {
                if (pContext->line.wr_index != 0) {
                    debug_putc('\r');
                    debug_putc('\n');
                    pContext->field_start = 0;
                    debug_process();
                }
                pContext->line.wr_index = 0;
            } else {
                if ((c >= ' ') && (c != 0x7F)) {
                    // Keep room for \0
                    if (pContext->line.wr_index < DEBUG_LINE-2) {
                        pContext->line.data[pContext->line.wr_index++] = c;
                        debug_putc(c);
                    }
                } else if (c == 0x08) {
                    if (pContext->line.wr_index != 0) {
                        pContext->line.wr_index--;
                        debug_putc('\b');
                        debug_putc(' ');
                        debug_putc('\b');
                    }
                }
            }
        }
      break;
    case DEBUG_IN_PROGRESS:
      if (pContext->output.rd_index == pContext->output.wr_index) {
          pContext->output.wr_index = 0;
          pContext->output.rd_index = 0;
          pContext->handler(pContext);
      }
      break;
    case DEBUG_PROMPT:
      if (pContext->output.rd_index == pContext->output.wr_index) {
          pContext->output.data[0] = '>';
          pContext->output.wr_index = 1;
          pContext->output.rd_index = 0;
          pContext->handler = NULL;
          memset(&command_context,0,sizeof(command_context));
          pContext->state = DEBUG_FLUSHING;
      }
      break;
      
    case DEBUG_FLUSHING:
      if (pContext->output.rd_index == pContext->output.wr_index) {
          pContext->output.wr_index = 0;
          pContext->output.rd_index = 0;
          pContext->state = DEBUG_IDLE;
      }
      break;
  }
}

void debug_line_memory_display_8(uint32_t addr, int len)
{
    int i;
    int l;
    uint8_t c;
    uint8_t *pH;
    uint8_t *pT;
    debug_context_t *pContext = &debug_context;
    
    l = pContext->output.wr_index;
    fmt_hexa32(&pContext->output.data[l],addr);
    pContext->output.data[l+8] = ' ';
    pH = &pContext->output.data[l+9];
    pT = &pContext->output.data[l+9+16*3];
    *pT++ = ' ';
    for (i = 0;i < len;i++) {
        c = *(uint8_t *)(addr+i);
        fmt_hexa8(pH,c);
        pH += 2;
        if ((c >= ' ') && (c <= 0x7e)) {
            *pT = c;
        } else {
            *pT = '.';
        }
        pT++;
        *pH++ = ' ';
    }
    while (i < 16) {
        *pH++ = ' ';
        *pH++ = ' ';
        *pH++ = ' ';
        *pT++ = ' ';
        i++;
    }
    *pT++ = '\n';
    *pT = '\0';
    pContext->output.wr_index = pT - pContext->output.data;
}    

void debug_line_memory_display_16(uint32_t addr, int len)
{
    int i;
    int l;
    uint16_t v;
    uint8_t *pH;
    debug_context_t *pContext = &debug_context;
    
    l = pContext->output.wr_index;
    fmt_hexa32(&pContext->output.data[l],addr);
    pContext->output.data[l+8] = ' ';
    pH = &pContext->output.data[l+9];
    for (i = 0;i < len;i+=2) {
        v = *(uint16_t *)(addr+i);
        fmt_hexa16(pH,v);
        pH += 4;
        *pH++ = ' ';
    }
    *pH++ = '\n';
    *pH = '\0';
    pContext->output.wr_index = pH - pContext->output.data;
}    


void debug_line_memory_display_32(uint32_t addr, int len)
{
    int i;
    int l;
    uint32_t v;
    uint8_t *pH;
    debug_context_t *pContext = &debug_context;
    
    l = pContext->output.wr_index;
    fmt_hexa32(&pContext->output.data[l],addr);
    pContext->output.data[l+8] = ' ';
    pH = &pContext->output.data[l+9];
    for (i = 0;i < len;i+=4) {
        v = *(uint32_t *)(addr+i);
        fmt_hexa32(pH,v);
        pH += 8;
        *pH++ = ' ';
    }
    *pH++ = '\n';
    *pH = '\0';
    pContext->output.wr_index = pH - pContext->output.data;
}    

void debug_context_memory_display(void)
{
    int line;
    int len = command_context.md.len;

    if (len > 16) {
        line = 16;
    } else {
        line = len;
    }
    if (command_context.md.size == 8) {
        debug_line_memory_display_8(command_context.md.address, line);
    } else if (command_context.md.size == 16) {
        debug_line_memory_display_16(command_context.md.address, line);
    } else if (command_context.md.size == 32) {
        debug_line_memory_display_32(command_context.md.address, line);
    }
    command_context.md.len -= line;
    command_context.md.address += line;
}   
void debug_memory_display(uint32_t addr, uint32_t len, uint8_t size)
{
    debug_context_t *pContext = &debug_context;
    if (len != 0) {
        pContext->state = DEBUG_IN_PROGRESS;
        pContext->handler = debug_md_handler;
        command_context.md.address = addr;
        command_context.md.len     = len;
        command_context.md.size    = size;
        debug_context_memory_display();
    }
}

static debug_command_t *first_handler;
static debug_command_t *last_handler;

void debug_add_command(debug_command_t *handler)
{
    if (first_handler == NULL) {
        first_handler = handler;
    } else {
        last_handler->next = handler;
    }
    last_handler = handler;
    last_handler->next = NULL;
}

void debug_add_commands(debug_command_t *handlers, int n)
{
    int i;
    for (i = 0;i < n;i++) {
        debug_add_command(&handlers[i]);
    }
}
int debug_help_handler(debug_context_t *pContext);

int debug_help_handler(debug_context_t *pContext)
{
    debug_command_t *handler;
    int l;
    int i;
    
    if (command_context.help.handler == NULL) {
        command_context.help.handler = first_handler;
        pContext->state = DEBUG_IN_PROGRESS;
        pContext->handler = debug_help_handler;
    }
    
    handler = command_context.help.handler;

    l = pContext->output.wr_index;
    i = 0;
    while (handler->name[i]) {
        pContext->output.data[l++] = handler->name[i++];
    }
    
    while (i < 16) {
      pContext->output.data[l++] = ' ';
      i++;
    }
    pContext->output.data[l++] = ':';
    pContext->output.data[l++] = ' ';
    i = 0;
    while (handler->help[i]) {
        pContext->output.data[l++] = handler->help[i++];
    }
    pContext->output.data[l++] = '\n';
    pContext->output.wr_index = l;
    command_context.help.handler = handler->next;
    if (handler->next == NULL) {
        pContext->state = DEBUG_PROMPT;
    }
    return 1;
}

int debug_md_handler(debug_context_t *pContext)
{
    uint32_t addr;
    uint32_t len = 0x100;
    int size = 8;
    char c;
    
    if (pContext->state != DEBUG_IN_PROGRESS) {
        c = pContext->line.data[pContext->field_start+2];    
        if ((c == '\0') || (c == 'b')) {
            size = 8;
        } else if (c == 'h') {
            size = 16;
        } else if (c == 'w') {
            size = 32;
        }
        
        if (debug_next_field() == 0) {
            return 0;
        }
        addr = ltoi((char *)&pContext->line.data[pContext->field_start],16);
        if (debug_next_field()) {
            len = ltoi((char *)&pContext->line.data[pContext->field_start],16);
        }
        pContext->state = DEBUG_IN_PROGRESS;
        pContext->handler = debug_md_handler;
        command_context.md.address = addr;
        command_context.md.len     = len;
        command_context.md.size    = size;
    } else {
        if (command_context.md.len == 0) {
            pContext->state = DEBUG_PROMPT;
        } else {
            debug_context_memory_display();
        }
    }
    return 1;

}

int debug_mode_handler(debug_context_t *pContext)
{
    if (debug_next_field() == 0) {
        int l;
    
        l = pContext->output.wr_index;
        fmt_hexa32(&pContext->output.data[l],debug_mode);
        pContext->output.data[l+8] = '\n';
        pContext->output.wr_index = l + 9;
        
        return 1;
    }
    debug_mode = ltoi((char *)&pContext->line.data[pContext->field_start],16);
    return 1;

}

int debug_mm_handler(debug_context_t *pContext)
{
    uint32_t addr;
    uint32_t value;
    int size = 8;
    char c;
    
    c = pContext->line.data[pContext->field_start+2];    
    if ((c == '\0') || (c == 'b')) {
        size = 8;
    } else if (c == 'h') {
        size = 16;
    } else if (c == 'w') {
        size = 32;
    }
    if (debug_next_field() == 0) {
        return 0;
    }
    addr = ltoi((char *)&pContext->line.data[pContext->field_start],16);
    if (debug_next_field() == 0) {
        return 0;
    }
    value = ltoi((char *)&pContext->line.data[pContext->field_start],16);
    
    if (size == 8) {
        *(uint8_t *)addr = value;
    } else if (size == 16) {
        *(uint16_t *)addr = value;
    } else if (size == 32) {
        *(uint32_t *)addr = value;
    }
    return 1;

}

debug_command_t basics[] = {
    { .name = "help",
      .help = "List available commands",
      .handler = debug_help_handler,
      .next = NULL},
    { .name = "mode",
      .help = "mode [value], displays/sets debug mode",
      .handler = debug_mode_handler,
      .next = NULL},
    { .name = "md",
      .help = "md addr [len], displays memory content as bytes",
      .handler = debug_md_handler,
      .next = NULL},
    { .name = "mdb",
      .help = "mdb addr [len], displays memory content as bytes",
      .handler = debug_md_handler,
      .next = NULL},
    { .name = "mdh",
      .help = "mdh addr [len], displays memory content as halfwords",
      .handler = debug_md_handler,
      .next = NULL},
    { .name = "mdw",
      .help = "mdw addr [len], displays memory content as words",
      .handler = debug_md_handler,
      .next = NULL},
    { .name = "mm",
      .help = "mm addr value, modifies memory content as byte",
      .handler = debug_mm_handler,
      .next = NULL},
    { .name = "mmb",
      .help = "mmb addr value, modifies memory content as byte",
      .handler = debug_mm_handler,
      .next = NULL},
    { .name = "mmh",
      .help = "mmh addr value, modifies memory content as haldword",
      .handler = debug_mm_handler,
      .next = NULL},
    { .name = "mmw",
      .help = "mmw addr value, modifies memory content as word",
      .handler = debug_mm_handler,
      .next = NULL}
  };
  
void debug_process(void)
{
    debug_context_t *pContext = &debug_context;
    uint8_t field_start;

    pContext->field_next = 0;
    
    if (debug_next_field()) {
        debug_command_t *handler = first_handler;
        field_start = pContext->field_start;
        
        while (handler != NULL) {
            if (strcmp((char *)&pContext->line.data[field_start],
                       handler->name) == 0) {
                pContext->state = DEBUG_PROMPT;
                (handler->handler)(pContext);
                break;
            }
            handler = handler->next;
        }
    }
}
int debug_get_next_char(void)
{
    debug_context_t *pContext = &debug_context;
    if (pContext->output.wr_index != pContext->output.rd_index) {
        int c = pContext->output.data[pContext->output.rd_index];
        pContext->output.rd_index++;
        if (pContext->output.rd_index == sizeof(pContext->output.data)) {
            pContext->output.rd_index = 0;
        }
        return c;
    }
    return -1;
}

int debug_get_next_string(uint8_t **start, uint32_t *length)
{
    debug_context_t *pContext = &debug_context;
    if (pContext->output.wr_index != pContext->output.rd_index) {
        *start = &pContext->output.data[pContext->output.rd_index];
        if (pContext->output.wr_index > pContext->output.rd_index) {
            *length = pContext->output.wr_index - pContext->output.rd_index;
        } else {
            *length = sizeof(pContext->output.data) - pContext->output.rd_index;
        }
        return 0;
    }
    return -1;
}
void debug_update_next_string(uint32_t length)
{
    debug_context_t *pContext = &debug_context;
    pContext->output.rd_index += length;
    if (pContext->output.rd_index >= sizeof(pContext->output.data)) {
        pContext->output.rd_index -= sizeof(pContext->output.data);
    }
}

void debug_add_char(int c)
{
    debug_context_t *pContext = &debug_context;
    pContext->input.data[pContext->input.wr_index] = c;
    pContext->input.wr_index++;
    if (pContext->input.wr_index == sizeof(pContext->input.data)) {
        pContext->input.wr_index = 0;
    }
}


void debug_start(void)
{
    debug_add_commands(basics,SIZEOF(basics));
    debug_putc('=');
    debug_putc('>');
}
