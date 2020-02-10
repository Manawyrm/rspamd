/*-
 * Copyright 2020 Vsevolod Stakhov
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RSPAMD_LOGGER_PRIVATE_H
#define RSPAMD_LOGGER_PRIVATE_H

/* How much message should be repeated before it is count to be repeated one */
#define REPEATS_MIN 3
#define REPEATS_MAX 300
#define LOG_ID 6
#define LOGBUF_LEN 8192

struct rspamd_log_module {
	gchar *mname;
	guint id;
};

struct rspamd_log_modules {
	guchar *bitset;
	guint bitset_len; /* Number of BITS used in bitset */
	guint bitset_allocated; /* Size of bitset allocated in BYTES */
	GHashTable *modules;
};

struct rspamd_logger_error_elt {
	gint completed;
	GQuark ptype;
	pid_t pid;
	gdouble ts;
	gchar id[LOG_ID + 1];
	gchar module[9];
	gchar message[];
};

struct rspamd_logger_error_log {
	struct rspamd_logger_error_elt *elts;
	rspamd_mempool_t *pool;
	guint32 max_elts;
	guint32 elt_len;
	/* Avoid false cache sharing */
	guchar __padding[64 - sizeof(gpointer) * 2 - sizeof(guint64)];
	guint cur_row;
};

/**
 * Static structure that store logging parameters
 * It is NOT shared between processes and is created by main process
 */
struct rspamd_logger_s {
	struct rspamd_logger_funcs ops;
	gint log_facility;
	gint log_level;

	struct rspamd_logger_error_log *errlog;
	struct rspamd_cryptobox_pubkey *pk;
	struct rspamd_cryptobox_keypair *keypair;

	guint flags;
	gboolean enabled;
	gboolean is_debug;
	gboolean no_lock;
	gboolean opened;

	pid_t pid;
	GQuark process_type;
	struct rspamd_radix_map_helper *debug_ip;
	rspamd_mempool_mutex_t *mtx;
};

/*
 * Common logging prototypes
 */

/*
 * File logging
 */
void * rspamd_log_file_init (rspamd_logger_t *logger, struct rspamd_config *cfg,
							 uid_t uid, gid_t gid, GError **err);
void * rspamd_log_file_reload (rspamd_logger_t *logger, struct rspamd_config *cfg,
							   gpointer arg, uid_t uid, gid_t gid, GError **err);
void rspamd_log_file_dtor (rspamd_logger_t *logger, gpointer arg);
void rspamd_log_file_log (const gchar *module, const gchar *id,
						  const gchar *function,
						  gint level_flags,
						  const gchar *message,
						  gsize mlen,
						  rspamd_logger_t *rspamd_log,
						  gpointer arg);

const static struct rspamd_logger_funcs file_log_funcs = {
		.init = rspamd_log_file_init,
		.dtor = rspamd_log_file_dtor,
		.reload = rspamd_log_file_reload,
		.log = rspamd_log_file_log,
};

#endif
