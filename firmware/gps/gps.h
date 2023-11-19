#ifndef GPS_H
#define GPS_H

void gps_init(void);
void gps_reinit(void);

// process incoming UART messages, and update slate with GPS result.
void gps_poll_uart(void);

#endif
