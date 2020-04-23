/*!
 * \file
 * \author Deniz Armagan
 * \date 2019-06-11
 * \version 0.1
 *
 * \brief Debug macros for the JSON API library.
 *
 * \details
 * This module collects macros for debugging.
 *
 * \copyright
 * Copyright (c) 2019 Fraunhofer IIS.
 * All rights reserved.
 */

/*
 * some preprocessor workarounds according to www.decompile.com/cpp/faq/file_and_line_error_string.htm
 */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(__GNUC__)
	/* gcc's cpp has extensions; it allows for macros with a variable number of
		 arguments. */
	#if !defined(NDEBUG)
		#define prntdbg(format, ...) fprintf(stderr, __FILE__ "(" TOSTRING(__LINE__) "): \t" format, ##__VA_ARGS__)
	#else
		#define prntdbg(format, ...) ((void)0)
	#endif /* defined(NODEBUG) */
#else
	/* This compiler does not have extensions, so we just rename the function */
	#define prntdbg printf
#endif
