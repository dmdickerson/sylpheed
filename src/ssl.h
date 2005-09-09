/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2002 Hiroyuki Yamamoto
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __SSL_H__
#define __SSL_H__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if USE_SSL

#include <glib.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "socket.h"

typedef enum {
	SSL_METHOD_SSLv23,
	SSL_METHOD_TLSv1
} SSLMethod;

typedef enum {
	SSL_NONE,
	SSL_TUNNEL,
	SSL_STARTTLS
} SSLType;

void ssl_init				(void);
void ssl_done				(void);
gboolean ssl_init_socket		(SockInfo	*sockinfo);
gboolean ssl_init_socket_with_method	(SockInfo	*sockinfo,
					 SSLMethod	 method);
void ssl_done_socket			(SockInfo	*sockinfo);

#endif /* USE_SSL */

#endif /* __SSL_H__ */