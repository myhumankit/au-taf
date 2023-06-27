
#define MAX_WORDS_NB 4

typedef struct {
  //pv_picovoice_t *handle;
  // pv_picovoice_t *handle;
  char    *access_key;
  int32_t num_keywords;
  float   *sensitivities;
  int32_t *sizes;
  const void *models[MAX_WORDS_NB];
  pv_porcupine_t *handle;
} pico_context_t;

pv_status_t pico_init(pico_context_t *pContext);
pv_status_t pico_process(pico_context_t *pContext, const int16_t *pData);

#define printf myprintf
void myprintf(char *fmt,...);

enum {
  CMD_NEXT = 1,
  CMD_OK,
  CMD_BACK,
  CMD_COMMAND};

int hotword_handler(int hot);
void error_handler(void);
