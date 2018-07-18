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

/*!
 * \brief JAPI context struct.
 *
 * A JAPI context struct stores several internal information.
 */
typedef struct __japi_context {
	int socket; /*!< File descriptor to reach the client */
	void *userptr; /*!< Pointer to user data */
	creadline_buf_t crl_buffer; /*!< Buffer used by creadline_r() */
	struct __japi_command *commands; /*!< Pointer to the JAPI command list */
} japi_context;

/**
 * \brief JAPI command handler type.
 */
typedef void (*japi_cmd_handler)(japi_context *ctx, json_object *request, json_object **response);

/*!
 * \brief JAPI command struct.
 *
 * A JAPI command struct is a mapping between a unique command name and a JAPI command handler.
 */
typedef struct __japi_command {
	const char *name; /*!< Printable name of the command */
	japi_cmd_handler func; /*!< Function to call */
	struct __japi_command* next; /*!< Pointer to the next command or NULL */
} japi_command;

/*!
 * \brief Create and initialize a new JAPI context.
 *
 * Create, initialize and return a new japi_context object.
 *
 * \param userptr	Pointer to user data or NULL. If a valid pointer is
 *					provided a command handler can access that data later.
 *
 * \returns	On success, a japi_context object is returned. On error, NULL is returned.
 */
japi_context* japi_init(void *userptr);

/*!
 * \brief Destroy a JAPI context.
 *
 * Destroy the JAPI context pointed to by ctx and release allocated memory.
 *
 * \param ctx			JAPI context
 */
void japi_destroy(japi_context *ctx);

/*!
 * \brief Register a JAPI command handler
 *
 * Register a JAPI command handler provided as a function pointer cmd_handler
 * for the command specified by cmd_name.
 *
 * \param ctx			JAPI context
 * \param cmd_name		Command name
 * \param cmd_handler	Function pointer
 *
 * \returns	On success, zero is returned. On error, -1 is returned.
 */
int japi_register_command(japi_context *ctx, const char *cmd_name, japi_cmd_handler cmd_handler);

/*!
 * \brief Start a JAPI server
 *
 * Start a JAPI server on the given port.
 *
 * \param ctx		JAPI context
 * \param port		Port to be used by the JAPI server
 *
 * \returns	Only returns in case of an error.
 */
int japi_start_server(japi_context *ctx, const char *port);

#endif /* __JAPI_H__ */

