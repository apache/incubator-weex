/**
 * \file
 * <PRE>
 * High performance base64 encoder / decoder
 * Version 1.3 -- 17-Mar-2006
 *
 * Copyright &copy; 2005, 2006, Nick Galbreath -- nickg [at] modp [dot] com
 * All rights reserved.
 *
 * http://modp.com/release/base64
 *
 * Released under bsd license.  See modp_b64.c for details.
 * </pre>
 *
 * The default implementation is the standard b64 encoding with padding.
 * It's easy to change this to use "URL safe" characters and to remove
 * padding.  See the modp_b64.c source code for details.
 *
 */

#include "./modp_base64/modp_b64.h"

// Encodes the input string in base64.  Returns true if successful and false
// otherwise.  The output string is only modified if successful.
bool Base64Encode(const std::string &input, std::string *output);

// Decodes the base64 input string.  Returns true if successful and false
// otherwise.  The output string is only modified if successful.
bool Base64Decode(const std::string &input, std::string *output);
