#ifndef _PARSER_H__
#define _PARSER_H__

#include "buffer.h"
#include "misc.h"
#include <string.h>

/* RFC2616 */
/* https://www.w3.org/Protocols/rfc2616/rfc2616.html */
/* This is a simple implementation */

/**
 *
 * Request       = Request-Line                   ; Section 5.1
                          (( general-header       ; Section 4.5
                         | request-header         ; Section 5.3
                         | entity-header ) CRLF)  ; Section 7.1
                        CRLF
                        [ message-body ]          ; Section 4.3
 */

/**
 *  Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
 */

/**
 *        request-header = Accept                 ; Section 14.1
                      | Accept-Charset           ; Section 14.2
                      | Accept-Encoding          ; Section 14.3
                      | Accept-Language          ; Section 14.4
                      | Authorization            ; Section 14.8
                      | Expect                   ; Section 14.20
                      | From                     ; Section 14.22
                      | Host                     ; Section 14.23
                      | If-Match                 ; Section 14.24
                      | If-Modified-Since        ; Section 14.25
                      | If-None-Match            ; Section 14.26
                      | If-Range                 ; Section 14.27
                      | If-Unmodified-Since      ; Section 14.28
                      | Max-Forwards             ; Section 14.31
                      | Proxy-Authorization      ; Section 14.34
                      | Range                    ; Section 14.35
                      | Referer                  ; Section 14.36
                      | TE                       ; Section 14.39
                      | User-Agent               ; Section 14.43
 */

#define INVALID_REQUEST (-1)
#define MAX_HEADERS (20)
#define MAX_ELEMENT_SIZE (2048)

typedef int (*method_cb_t)(int method);
typedef int (*data_cb_t)(const char *begin, const char *end);

/* basic http method */
typedef enum {
  HTTP_DELETE,
  HTTP_GET,
  HTTP_HEAD,
  HTTP_POST,
  HTTP_PUT,
  HTTP_INVALID,
} http_method;

/* HTTP protocol version */
typedef struct {
  unsigned short http_major;
  unsigned short http_minor;
} http_version;

/* parser state used in fsm */
typedef enum {
  /* request line states */
  S_RL_BEGIN = 0,
  S_RL_METHOD,
  S_RL_SP_BEFORE_URL,
  S_RL_URL,
  S_RL_SP_BEFORE_VERSION,
  S_RL_VERSION_H,
  S_RL_VERSION_HT,
  S_RL_VERSION_HTT,
  S_RL_VERSION_HTTP,
  S_RL_VERSION_HTTP_SLASH,
  S_RL_VERSION_MAJOR,
  S_RL_VERSION_DOT,
  S_RL_VERSION_MINOR,
  S_RL_CR_AFTER_VERSION,
  S_RL_LF_AFTER_VERSION,

  /* header states */
  S_HD_BEGIN,

} parser_state;

typedef struct {
  /* preserve buffer_t state, so when recv new data, we can keep parsing */
  char *next_parse_pos; /* parser position in buffer_t */
  int state;            /* parser state */

  /* callback functions */
  method_cb_t on_method;
  data_cb_t on_header_field;
  data_cb_t on_header_value;

  /* parsed request line result */
  http_method method;
  http_version version;
  char request_url[MAX_ELEMENT_SIZE];
  char request_path[MAX_ELEMENT_SIZE];
  char query_string[MAX_ELEMENT_SIZE];

  /* parsed header lines result */
  bool keep_alive;
  int content_length;
  int num_headers;
  char headers[MAX_HEADERS][2][MAX_ELEMENT_SIZE];

  /* private members, do not modify !!! */
  char *method_begin;
  char *url_begin;

} parse_settings;

static inline void parse_settings_init(parse_settings *st, buffer_t *b) {
  memset(st, 0, sizeof(parse_settings));
  st->next_parse_pos = b->buf;
}

/* status code */
#define HTTP_STATUS_MAP(GEN)                                                   \
  GEN(100, CONTINUE, Continue)                                                 \
  GEN(101, SWITCHING_PROTOCOLS, Switching Protocols)                           \
  GEN(102, PROCESSING, Processing)                                             \
  GEN(200, OK, OK)                                                             \
  GEN(201, CREATED, Created)                                                   \
  GEN(202, ACCEPTED, Accepted)                                                 \
  GEN(203, NON_AUTHORITATIVE_INFORMATION, Non - Authoritative Information)     \
  GEN(204, NO_CONTENT, No Content)                                             \
  GEN(205, RESET_CONTENT, Reset Content)                                       \
  GEN(206, PARTIAL_CONTENT, Partial Content)                                   \
  GEN(207, MULTI_STATUS, Multi - Status)                                       \
  GEN(208, ALREADY_REPORTED, Already Reported)                                 \
  GEN(226, IM_USED, IM Used)                                                   \
  GEN(300, MULTIPLE_CHOICES, Multiple Choices)                                 \
  GEN(301, MOVED_PERMANENTLY, Moved Permanently)                               \
  GEN(302, FOUND, Found)                                                       \
  GEN(303, SEE_OTHER, See Other)                                               \
  GEN(304, NOT_MODIFIED, Not Modified)                                         \
  GEN(305, USE_PROXY, Use Proxy)                                               \
  GEN(307, TEMPORARY_REDIRECT, Temporary Redirect)                             \
  GEN(308, PERMANENT_REDIRECT, Permanent Redirect)                             \
  GEN(400, BAD_REQUEST, Bad Request)                                           \
  GEN(401, UNAUTHORIZED, Unauthorized)                                         \
  GEN(402, PAYMENT_REQUIRED, Payment Required)                                 \
  GEN(403, FORBIDDEN, Forbidden)                                               \
  GEN(404, NOT_FOUND, Not Found)                                               \
  GEN(405, METHOD_NOT_ALLOWED, Method Not Allowed)                             \
  GEN(406, NOT_ACCEPTABLE, Not Acceptable)                                     \
  GEN(407, PROXY_AUTHENTICATION_REQUIRED, Proxy Authentication Required)       \
  GEN(408, REQUEST_TIMEOUT, Request Timeout)                                   \
  GEN(409, CONFLICT, Conflict)                                                 \
  GEN(410, GONE, Gone)                                                         \
  GEN(411, LENGTH_REQUIRED, Length Required)                                   \
  GEN(412, PRECONDITION_FAILED, Precondition Failed)                           \
  GEN(413, PAYLOAD_TOO_LARGE, Payload Too Large)                               \
  GEN(414, URI_TOO_LONG, URI Too Long)                                         \
  GEN(415, UNSUPPORTED_MEDIA_TYPE, Unsupported Media Type)                     \
  GEN(416, RANGE_NOT_SATISFIABLE, Range Not Satisfiable)                       \
  GEN(417, EXPECTATION_FAILED, Expectation Failed)                             \
  GEN(421, MISDIRECTED_REQUEST, Misdirected Request)                           \
  GEN(422, UNPROCESSABLE_ENTITY, Unprocessable Entity)                         \
  GEN(423, LOCKED, Locked)                                                     \
  GEN(424, FAILED_DEPENDENCY, Failed Dependency)                               \
  GEN(426, UPGRADE_REQUIRED, Upgrade Required)                                 \
  GEN(428, PRECONDITION_REQUIRED, Precondition Required)                       \
  GEN(429, TOO_MANY_REQUESTS, Too Many Requests)                               \
  GEN(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large)   \
  GEN(451, UNAVAILABLE_FOR_LEGAL_REASONS, Unavailable For Legal Reasons)       \
  GEN(500, INTERNAL_SERVER_ERROR, Internal Server Error)                       \
  GEN(501, NOT_IMPLEMENTED, Not Implemented)                                   \
  GEN(502, BAD_GATEWAY, Bad Gateway)                                           \
  GEN(503, SERVICE_UNAVAILABLE, Service Unavailable)                           \
  GEN(504, GATEWAY_TIMEOUT, Gateway Timeout)                                   \
  GEN(505, HTTP_VERSION_NOT_SUPPORTED, HTTP Version Not Supported)             \
  GEN(506, VARIANT_ALSO_NEGOTIATES, Variant Also Negotiates)                   \
  GEN(507, INSUFFICIENT_STORAGE, Insufficient Storage)                         \
  GEN(508, LOOP_DETECTED, Loop Detected)                                       \
  GEN(510, NOT_EXTENDED, Not Extended)                                         \
  GEN(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)

typedef enum {
#define GEN(num, name, string) HTTP_STATUS_##name = num,
  HTTP_STATUS_MAP(GEN)
#undef GEN
} http_status;

extern int parse_request_line(buffer_t *b, parse_settings *st);

#endif
