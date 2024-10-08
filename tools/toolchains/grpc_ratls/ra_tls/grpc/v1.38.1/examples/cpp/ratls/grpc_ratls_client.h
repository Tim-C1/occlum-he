#ifndef _GRPC_RATLS_CLIENT_H_
#define _GRPC_RATLS_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

// client get secret
extern int grpc_ratls_get_secret(
    const char *server_addr, // grpc server address+port, such as "localhost:50051"
    const char *config_json, // ratls handshake config json file
    const char *name, // secret name to be requested
    const char *secret_file // secret file to be saved
);

extern int grpc_ratls_get_secret_to_buf(
    const char *server_addr, // grpc server address+port, such as "localhost:50051"
    const char *config_json, // ratls handshake config json file
    const char *name, // secret name to be requested
    char *secret_buf, // buffer to save secret
    unsigned int *buf_len // buffer size
);

#ifdef __cplusplus
}
#endif

#endif  // _GRPC_RATLS_CLIENT_H_