/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2003 Hiroyuki Yamamoto
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

#ifndef __QUOTED_PRINTABLE_H__
#define __QUOTED_PRINTABLE_H__

#include <glib.h>

void qp_encode_line		(gchar		*out,
				 const guchar	*in);
gint qp_decode_line		(gchar		*str);

gint qp_decode_q_encoding	(guchar		*out,
				 const gchar	*in,
				 gint		 inlen);
gint qp_get_q_encoding_len	(const guchar	*str);
void qp_q_encode		(gchar		*out,
				 const guchar	*in);

#endif /* __QUOTED_PRINTABLE_H__ */