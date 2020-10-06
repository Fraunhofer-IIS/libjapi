/*!
 * \file
 * \author Christopher Stender
 * \date 2018-02-15
 * \version 0.1
 *
 * \brief Universal JSON API library.
 *
 * \details
 * libjapi is a universal JSON API library.
 *
 * \copyright
 * Copyright (c) 2018 Fraunhofer IIS.
 * All rights reserved.
 */

#ifndef __JAPI_H__
#define __JAPI_H__

#include <json-c/json.h>
#include <creadline.h>
#include <pthread.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief JAPI context struct.
 *
 * A JAPI context struct stores several internal information.
 */
typedef struct __japi_context {
	void *userptr; /*!< Pointer to user data */
	uint16_t num_clients; /*!< Number of connected clients */
	uint16_t max_clients; /*!< Number of maximal allowed clients */
	pthread_mutex_t lock; /*!< Mutual access lock */
	struct __japi_request *requests; /*!< Pointer to the JAPI request list */
	struct __japi_pushsrv_context *push_services; /*!< Pointer to the JAPI push service list */
	struct __japi_client *clients; /*!< Pointer to the JAPI client context */
	bool include_args_in_response; /*!< Flag to include request args in response */
	bool shutdown; /*!< Flag to shutdown the JAPI server */
} japi_context;

/*!
 * \brief JAPI client context.
 *
 * Stores information for client connection
 */
typedef struct __japi_client {
	int socket; /*!< Socket to connect */
	creadline_buf_t crl_buffer; /*!< Buffer used by creadline_r() */
	struct __japi_client* next; /*!< Pointer to the next client struct or NULL */
} japi_client;

/*!
 * \brief JAPI request handler type.
 */
typedef void (*japi_req_handler)(japi_context *ctx, json_object *request, json_object *response);

/*!
 * \brief JAPI request struct.
 *
 * A JAPI request struct is a mapping between a unique request name and a JAPI request handler.
 */
typedef struct __japi_request {
	const char *name; /*!< Printable name of the request */
	japi_req_handler func; /*!< Function to call */
	struct __japi_request* next; /*!< Pointer to the next request struct or NULL */
} japi_request;

/*!
 * \brief Create and initialize a new JAPI context.
 *
 * Create, initialize and return a new japi_context object.
 *
 * \param userptr	Pointer to user data or NULL. If a valid pointer is
 *					provided a request handler can access that data later.
 *
 * \returns	On success, a japi_context object is returned. On error, NULL is returned.
 */
japi_context* japi_init(void *userptr);

/*!
 * \brief Destroy a JAPI context.
 *
 * Destroy the JAPI context pointed to by ctx and release allocated memory.
 *
 * \param ctx	JAPI context
 *
 * \returns On succes, zero is returned. On error, -1 is returned.
 */
int japi_destroy(japi_context *ctx);

/*!
 * \brief Register a JAPI request handler
 *
 * Register a JAPI request handler provided as a function pointer req_handler
 * for the request specified by req_name.
 *
 * \param ctx		JAPI context
 * \param req_name	Request name
 * \param req_handler	Function pointer
 *
 * \returns	On success, zero is returned. On error, -1 for empty JAPI context,
 * -2 for empty request name,
 * -3 for empty request handler,
 * -4 for duplicate naming,
 * -5 for failed memory allocation, is returned.
 */
int japi_register_request(japi_context *ctx, const char *req_name, japi_req_handler req_handler);

/*!
 * \brief Start a JAPI server
 *
 * Start a JAPI server on the given port.
 *
 * \param ctx	JAPI context
 * \param port	Port to be used by the JAPI server
 *
 * \returns	Only returns in case of an error.
 */
int japi_start_server(japi_context *ctx, const char *port);

/*!
 * \brief Set the number of allowed clients
 *
 * Set the maximal number of allowed clients.
 *
 * \param ctx	JAPI context
 * \param num	Number of clients to be allowed. 0 stands for unlimited.
 *
 * \returns	On success, zero is returned. On error, -1 for empty JAPI context, is returned.
 */
int japi_set_max_allowed_clients(japi_context *ctx, uint16_t num);

/*!
 * \brief Configure context to include request arguments in response
 *
 * Configure context to include request arguments in response
 *
 * \param ctx		JAPI context
 * \param include_args	Include request arguments in response.
 *
 * \returns	On success, zero is returned. On error, -1 for empty JAPI context, is returned.
 */
int japi_include_args_in_response(japi_context *ctx, bool include_args);

/*!
 * \brief Shutdown the JAPI server
 *
 * Shutdown the JAPI server.
 *
 * \param ctx	JAPI context
 *
 * \returns	On success, zero is returned. On error, -1 for empty JAPI context, is returned.
 */
int japi_shutdown(japi_context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* __JAPI_H__ */
