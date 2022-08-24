#ifndef TRC_STREAMING_PORT_H
#define TRC_STREAMING_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#define TRC_STREAM_PORT_USE_INTERNAL_BUFFER 1

int32_t trace_io_read(void* data, uint32_t size, int32_t* bytes_read);

int32_t trace_io_write(void* data, uint32_t size, int32_t* bytes_written);

#define TRC_STREAM_PORT_READ_DATA(_data, _size, _bytes_read) trace_io_read(_data, _size, _bytes_read)

#define TRC_STREAM_PORT_WRITE_DATA(_data, _size, _bytes_written) trace_io_write(_data, _size, _bytes_written)

#ifdef __cplusplus
}
#endif

#endif /* TRC_STREAMING_PORT_H */
