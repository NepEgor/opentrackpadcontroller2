#ifndef USB_CALLBACKS_H
#define USB_CALLBACKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void sendReportData(uint8_t *data, uint16_t len);
void recvReportData(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif