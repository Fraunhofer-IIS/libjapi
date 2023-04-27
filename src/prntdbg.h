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
 * Copyright (c) 2023 Fraunhofer IIS
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
