// message definitions

#define MSG_EVENT_KEY           0
#define MSG_EVENT_KILL_THREAD   1
#define MSG_ACK                 2
#define MSG_ERROR               3

struct message
{
    uint8_t *data;      // buffer for message
    uint32_t data_len;  // message length
    uint8_t data_ready; // 1 = data not written yet, 0 = data ready, message is in buffer
    uint32_t thread_num;
    uint32_t thread_sender_num;
    struct message *next;
};
