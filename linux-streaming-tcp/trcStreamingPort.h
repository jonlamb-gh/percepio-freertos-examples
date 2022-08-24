#ifndef TRC_STREAMING_PORT_H
#define TRC_STREAMING_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t stream_port_read(void* data, uint32_t size, int32_t* bytes_read);
int32_t stream_port_write(void* data, uint32_t size, int32_t *bytes_written);

#define TRC_STREAM_PORT_READ_DATA(_ptrData, _size, _ptrBytesRead) stream_port_read(_ptrData, _size, _ptrBytesRead)

#if (TRC_STREAM_PORT_USE_INTERNAL_BUFFER == 1)    
    #define TRC_STREAM_PORT_WRITE_DATA(_ptrData, _size, _ptrBytesWritten) stream_port_write(_ptrData, _size, _ptrBytesWritten)
#else
    /* In the direct mode, _ptrBytesWritten is not used, so it is assumed that "all or nothing" is written. */
    //#define TRC_STREAM_PORT_WRITE_DATA(_ptrData, _size, UNUSED) stream_port_write(_ptrData, _size, NULL)
    #error "TRC_STREAM_PORT_USE_INTERNAL_BUFFER != 1"
#endif

#ifdef __cplusplus
}
#endif

#endif /* TRC_STREAMING_PORT_H */
