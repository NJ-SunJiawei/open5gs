#ifndef __EVENT_H__
#define __EVENT_H__

#include "core_msgq.h"
#include "core_time.h"
#include "core_timer.h"
#include "core_fsm.h"
#include "core_pkbuf.h"
#include "core_net.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    EVT_BASE = FSM_USER_SIG,

    EVT_LO_ENB_S1AP_ACCEPT,
    EVT_LO_ENB_S1AP_CONNREFUSED,

    EVT_MSG_ENB_S1AP,
    EVT_MSG_UE_NAS,
    
    EVT_TOP,

} event_e;

typedef struct {
    fsm_event_t event;
    c_uintptr_t param1;
    c_uintptr_t param2;
    c_uintptr_t param3;
    c_uintptr_t param4;
} event_t;

#define EVENT_SIZE sizeof(event_t)

#define event_set(__ptr_e, __evnt) \
    ((__ptr_e)->event = (__evnt))

#define event_get(__ptr_e) \
    ((__ptr_e)->event)


#define event_set_param1(__ptr_e, __param) \
    ((__ptr_e)->param1 = (c_uintptr_t)(__param))

#define event_set_param2(__ptr_e, __param) \
    ((__ptr_e)->param2 = (c_uintptr_t)(__param))

#define event_set_param3(__ptr_e, __param) \
    ((__ptr_e)->param3 = (c_uintptr_t)(__param))

#define event_set_param4(__ptr_e, __param) \
    ((__ptr_e)->param4 = (c_uintptr_t)(__param))

#define event_get_param1(__ptr_e) \
    ((__ptr_e)->param1)

#define event_get_param2(__ptr_e) \
    ((__ptr_e)->param2)

#define event_get_param3(__ptr_e) \
    ((__ptr_e)->param3)

#define event_get_param4(__ptr_e) \
    ((__ptr_e)->param4)

/**
 * Create event message queue
 *
 * @return event queue or 0
 */
CORE_DECLARE(msgq_id) event_create(void);

/**
 * Delete event message queue
 *
 * @return CORE_OK or CORE_ERROR
 */
CORE_DECLARE(status_t) event_delete(msgq_id queue_id);


/**
 * Send a event to event queue
 *
 * @return If success, return the size to be sent.
 *         If else, return -1
 */
CORE_DECLARE(int) event_send(msgq_id queue_id, event_t *e);

/**
 * Receive a event from event queue with timeout
 *
 * @return If success, return the size to be received.
 *         If timout occurs, return CORE_TIMEUP.
 *         If else, return -1.
 */
CORE_DECLARE(int) event_timedrecv(
        msgq_id queue_id, event_t *e, c_time_t timeout);

/**
 * Create a timer
 */
CORE_DECLARE(tm_block_id) event_timer_create(tm_service_t *tm_service);

/**
 * Set a timer
 */
status_t event_timer_set(tm_block_id id, event_e te, tm_type_e type, 
        c_uint32_t duration, c_uintptr_t queue_id, c_uintptr_t param);

/**
 * Delete a timer
 */
CORE_DECLARE(status_t) event_timer_delete(tm_block_id id);

/*
 * Execute timer engine
 */
CORE_DECLARE(status_t) event_timer_execute(tm_service_t *tm_service);

char* event_get_name(event_t *e);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__EVENT_H__ */
