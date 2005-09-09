/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2005 Hiroyuki Yamamoto
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

#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#if HAVE_ALLOCA_H
#  include <alloca.h>
#endif
#if HAVE_WCHAR_H
#  include <wchar.h>
#endif

/* The AC_CHECK_SIZEOF() in configure fails for some machines.
 * we provide some fallback values here */
#if !SIZEOF_UNSIGNED_SHORT
  #undef SIZEOF_UNSIGNED_SHORT
  #define SIZEOF_UNSIGNED_SHORT 2
#endif
#if !SIZEOF_UNSIGNED_INT
  #undef SIZEOF_UNSIGNED_INT
  #define SIZEOF_UNSIGNED_INT 4
#endif
#if !SIZEOF_UNSIGNED_LONG
  #undef SIZEOF_UNSIGNED_LONG
  #define SIZEOF_UNSIGNED_LONG 4
#endif

#ifndef HAVE_U32_TYPEDEF
  #undef u32	    /* maybe there is a macro with this name */
  typedef guint32 u32;
  #define HAVE_U32_TYPEDEF
#endif

#ifndef BIG_ENDIAN_HOST
  #if (G_BYTE_ORDER == G_BIG_ENDIAN)
    #define BIG_ENDIAN_HOST 1
  #endif
#endif

#define CHDIR_RETURN_IF_FAIL(dir) \
{ \
	if (change_dir(dir) < 0) return; \
}

#define CHDIR_RETURN_VAL_IF_FAIL(dir, val) \
{ \
	if (change_dir(dir) < 0) return val; \
}

#define Xalloca(ptr, size, iffail) \
{ \
	if ((ptr = alloca(size)) == NULL) { \
		g_warning("can't allocate memory\n"); \
		iffail; \
	} \
}

#define Xstrdup_a(ptr, str, iffail) \
{ \
	gchar *__tmp; \
 \
	if ((__tmp = alloca(strlen(str) + 1)) == NULL) { \
		g_warning("can't allocate memory\n"); \
		iffail; \
	} else \
		strcpy(__tmp, str); \
 \
	ptr = __tmp; \
}

#define Xstrndup_a(ptr, str, len, iffail) \
{ \
	gchar *__tmp; \
 \
	if ((__tmp = alloca(len + 1)) == NULL) { \
		g_warning("can't allocate memory\n"); \
		iffail; \
	} else { \
		strncpy(__tmp, str, len); \
		__tmp[len] = '\0'; \
	} \
 \
	ptr = __tmp; \
}

#define Xstrcat_a(ptr, str1, str2, iffail) \
{ \
	gchar *__tmp; \
	gint len1, len2; \
 \
	len1 = strlen(str1); \
	len2 = strlen(str2); \
	if ((__tmp = alloca(len1 + len2 + 1)) == NULL) { \
		g_warning("can't allocate memory\n"); \
		iffail; \
	} else { \
		memcpy(__tmp, str1, len1); \
		memcpy(__tmp + len1, str2, len2 + 1); \
	} \
 \
	ptr = __tmp; \
}

#define AUTORELEASE_STR(str, iffail) \
{ \
	gchar *__str; \
	Xstrdup_a(__str, str, iffail); \
	g_free(str); \
	str = __str; \
}

#define FILE_OP_ERROR(file, func) \
{ \
	fprintf(stderr, "%s: ", file); \
	perror(func); \
}

/* for macro expansion */
#define Str(x)	#x
#define Xstr(x)	Str(x)

void list_free_strings		(GList		*list);
void slist_free_strings		(GSList		*list);

void hash_free_strings		(GHashTable	*table);
void hash_free_value_mem	(GHashTable	*table);

gint str_case_equal		(gconstpointer	 v,
				 gconstpointer	 v2);
guint str_case_hash		(gconstpointer	 key);

void ptr_array_free_strings	(GPtrArray	*array);

typedef gboolean (*StrFindFunc) (const gchar	*haystack,
				 const gchar	*needle);

gboolean str_find		(const gchar	*haystack,
				 const gchar	*needle);
gboolean str_case_find		(const gchar	*haystack,
				 const gchar	*needle);
gboolean str_find_equal		(const gchar	*haystack,
				 const gchar	*needle);
gboolean str_case_find_equal	(const gchar	*haystack,
				 const gchar	*needle);

/* number-string conversion */
gint to_number			(const gchar *nstr);
gchar *itos_buf			(gchar	     *nstr,
				 gint	      n);
gchar *itos			(gint	      n);
gchar *to_human_readable	(off_t	      size);

/* alternative string functions */
gint strcmp2		(const gchar	*s1,
			 const gchar	*s2);
gint path_cmp		(const gchar	*s1,
			 const gchar	*s2);
gchar *strretchomp	(gchar		*str);
gchar *strtailchomp	(gchar		*str,
			 gchar		 tail_char);
gchar *strcrchomp	(gchar		*str);
gchar *strcasestr	(const gchar	*haystack,
			 const gchar	*needle);
gpointer my_memmem	(gconstpointer	 haystack,
			 size_t		 haystacklen,
			 gconstpointer	 needle,
			 size_t		 needlelen);
gchar *strncpy2		(gchar		*dest,
			 const gchar	*src,
			 size_t		 n);

/* wide-character functions */
#if !HAVE_ISWALNUM
int iswalnum		(wint_t wc);
#endif
#if !HAVE_ISWSPACE
int iswspace		(wint_t wc);
#endif
#if !HAVE_TOWLOWER
wint_t towlower		(wint_t wc);
#endif

#if !HAVE_WCSLEN
size_t wcslen		(const wchar_t *s);
#endif
#if !HAVE_WCSCPY
wchar_t *wcscpy		(wchar_t       *dest,
			 const wchar_t *src);
#endif
#if !HAVE_WCSNCPY
wchar_t *wcsncpy	(wchar_t       *dest,
			 const wchar_t *src,
			 size_t		n);
#endif

wchar_t *wcsdup			(const wchar_t *s);
wchar_t *wcsndup		(const wchar_t *s,
				 size_t		n);
wchar_t *strdup_mbstowcs	(const gchar   *s);
gchar *strdup_wcstombs		(const wchar_t *s);
gint wcsncasecmp		(const wchar_t *s1,
				 const wchar_t *s2,
				 size_t		n);
wchar_t *wcscasestr		(const wchar_t *haystack,
				 const wchar_t *needle);
gint get_mbs_len		(const gchar	*s);

gboolean is_next_nonascii	(const guchar *s);
gint get_next_word_len		(const guchar *s);

/* functions for string parsing */
gint subject_compare			(const gchar	*s1,
					 const gchar	*s2);
gint subject_compare_for_sort		(const gchar	*s1,
					 const gchar	*s2);
void trim_subject_for_compare		(gchar		*str);
void trim_subject_for_sort		(gchar		*str);
void trim_subject			(gchar		*str);
void eliminate_parenthesis		(gchar		*str,
					 gchar		 op,
					 gchar		 cl);
void extract_parenthesis		(gchar		*str,
					 gchar		 op,
					 gchar		 cl);

void extract_parenthesis_with_skip_quote	(gchar		*str,
						 gchar		 quote_chr,
						 gchar		 op,
						 gchar		 cl);

void eliminate_quote			(gchar		*str,
					 gchar		 quote_chr);
void extract_quote			(gchar		*str,
					 gchar		 quote_chr);
void eliminate_address_comment		(gchar		*str);
gchar *strchr_with_skip_quote		(const gchar	*str,
					 gint		 quote_chr,
					 gint		 c);
gchar *strrchr_with_skip_quote		(const gchar	*str,
					 gint		 quote_chr,
					 gint		 c);
void extract_address			(gchar		*str);
void extract_list_id_str		(gchar		*str);

GSList *address_list_append		(GSList		*addr_list,
					 const gchar	*str);
GSList *references_list_prepend		(GSList		*msgid_list,
					 const gchar	*str);
GSList *references_list_append		(GSList		*msgid_list,
					 const gchar	*str);
GSList *newsgroup_list_append		(GSList		*group_list,
					 const gchar	*str);

GList *add_history			(GList		*list,
					 const gchar	*str);

void remove_return			(gchar		*str);
void remove_space			(gchar		*str);
void unfold_line			(gchar		*str);
void subst_char				(gchar		*str,
					 gchar		 orig,
					 gchar		 subst);
void subst_chars			(gchar		*str,
					 gchar		*orig,
					 gchar		 subst);
void subst_null				(gchar		*str,
					 gint		 len,
					 gchar		 subst);
void subst_for_filename			(gchar		*str);
gboolean is_header_line			(const gchar	*str);
gboolean is_ascii_str			(const guchar	*str);
gint get_quote_level			(const gchar	*str);
gint check_line_length			(const gchar	*str,
					 gint		 max_chars,
					 gint		*line);

gchar *strstr_with_skip_quote		(const gchar	*haystack,
					 const gchar	*needle);
gchar *strchr_parenthesis_close		(const gchar	*str,
					 gchar		 op,
					 gchar		 cl);

gchar **strsplit_parenthesis		(const gchar	*str,
					 gchar		 op,
					 gchar		 cl,
					 gint		 max_tokens);
gchar **strsplit_with_quote		(const gchar	*str,
					 const gchar	*delim,
					 gint		 max_tokens);

gchar *get_abbrev_newsgroup_name	(const gchar	*group,
					 gint		 len);
gchar *trim_string			(const gchar	*str,
					 gint		 len);
gchar *trim_string_before		(const gchar	*str,
					 gint		 len);

GList *uri_list_extract_filenames	(const gchar	*uri_list);
gboolean is_uri_string			(const gchar	*str);
gchar *get_uri_path			(const gchar	*uri);
gint get_uri_len			(const gchar	*str);
void decode_uri				(gchar		*decoded_uri,
					 const gchar	*encoded_uri);
gchar *encode_uri			(const gchar	*filename);
gint scan_mailto_url			(const gchar	*mailto,
					 gchar	       **to,
					 gchar	       **cc,
					 gchar	       **bcc,
					 gchar	       **subject,
					 gchar	       **body);

/* return static strings */
const gchar *get_home_dir		(void);
const gchar *get_rc_dir			(void);
const gchar *get_old_rc_dir		(void);
const gchar *get_news_cache_dir		(void);
const gchar *get_imap_cache_dir		(void);
const gchar *get_mime_tmp_dir		(void);
const gchar *get_template_dir		(void);
const gchar *get_tmp_dir		(void);
gchar *get_tmp_file			(void);
const gchar *get_domain_name		(void);

/* file / directory handling */
off_t get_file_size		(const gchar	*file);
off_t get_file_size_as_crlf	(const gchar	*file);
off_t get_left_file_size	(FILE		*fp);

gboolean file_exist		(const gchar	*file,
				 gboolean	 allow_fifo);
gboolean is_dir_exist		(const gchar	*dir);
gboolean is_file_entry_exist	(const gchar	*file);
gboolean dirent_is_regular_file	(struct dirent	*d);
gboolean dirent_is_directory	(struct dirent	*d);

#define is_file_exist(file)		file_exist(file, FALSE)
#define is_file_or_fifo_exist(file)	file_exist(file, TRUE)

gint change_dir			(const gchar	*dir);
gint make_dir			(const gchar	*dir);
gint make_dir_hier		(const gchar	*dir);
gint remove_all_files		(const gchar	*dir);
gint remove_numbered_files	(const gchar	*dir,
				 guint		 first,
				 guint		 last);
gint remove_all_numbered_files	(const gchar	*dir);
gint remove_expired_files	(const gchar	*dir,
				 guint		 hours);
gint remove_dir_recursive	(const gchar	*dir);
gint copy_file			(const gchar	*src,
				 const gchar	*dest,
				 gboolean	 keep_backup);
gint copy_dir			(const gchar	*src,
				 const gchar	*dest);
gint move_file			(const gchar	*src,
				 const gchar	*dest,
				 gboolean	 overwrite);
gint copy_file_part		(FILE		*fp,
				 off_t		 offset,
				 size_t		 length,
				 const gchar	*dest);

gchar *canonicalize_str		(const gchar	*str);
gint canonicalize_file		(const gchar	*src,
				 const gchar	*dest);
gint canonicalize_file_replace	(const gchar	*file);
gint uncanonicalize_file	(const gchar	*src,
				 const gchar	*dest);
gint uncanonicalize_file_replace(const gchar	*file);

gchar *normalize_newlines	(const gchar	*str);

gchar *get_outgoing_rfc2822_str	(FILE		*fp);
gchar *generate_mime_boundary	(const gchar	*prefix);

gint change_file_mode_rw	(FILE		*fp,
				 const gchar	*file);
FILE *my_tmpfile		(void);
FILE *str_open_as_stream	(const gchar	*str);
gint str_write_to_file		(const gchar	*str,
				 const gchar	*file);
gchar *file_read_to_str		(const gchar	*file);
gchar *file_read_stream_to_str	(FILE		*fp);

/* process execution */
gint execute_async		(gchar *const	 argv[]);
gint execute_sync		(gchar *const	 argv[]);
gint execute_command_line	(const gchar	*cmdline,
				 gboolean	 async);
gchar *get_command_output	(const gchar	*cmdline);

/* open URI with external browser */
gint open_uri(const gchar *uri, const gchar *cmdline);

/* time functions */
time_t remote_tzoffset_sec	(const gchar	*zone);
time_t tzoffset_sec		(time_t		*now);
gchar *tzoffset			(time_t		*now);
void get_rfc822_date		(gchar		*buf,
				 gint		 len);

size_t my_strftime		(gchar			*s,
				 size_t			 max,
				 const gchar		*format,
				 const struct tm	*tm);

/* logging */
void set_log_file	(const gchar *filename);
void close_log_file	(void);
void log_verbosity_set	(gboolean verbose);
void debug_print	(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void log_print		(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void log_message	(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void log_warning	(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void log_error		(const gchar *format, ...) G_GNUC_PRINTF(1, 2);

#endif /* __UTILS_H__ */