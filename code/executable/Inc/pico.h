


typedef struct {
  //pv_picovoice_t *handle;
  // pv_picovoice_t *handle;
  char    *access_key;
  int32_t num_keywords;
  float   *sensitivities;
  int32_t *sizes;
  void    **models;
  void    *handle;
  char    *rhino_context;
} pico_context_t;

pv_status_t pico_init(pico_context_t *pContext);
pv_status_t pico_process(pico_context_t *pContext, const int16_t *pData);

#define printf myprintf
void myprintf(char *fmt,...);

enum {
  CMD_COMMAND = 1,
  CMD_NEXT,
  CMD_OK,
  CMD_BACK};

int hotword_handler(int hot);
