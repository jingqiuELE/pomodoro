typedef enum state_id {
    STATE_IDLE,
    STATE_WORKING,
    STATE_PRE_BREAK,
    STATE_BREAK
} STATE_ID;

typedef enum side {
    SIDE_UP,
    SIDE_DOWN
} SIDE;

typedef struct state {
    STATE_ID state_id;
    unsigned long timeout;         //timeout in millis.
    uint32_t ledColor;
    bool ledBlink;
} STATE;
