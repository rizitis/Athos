/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2016-2024 Matthias Klumpp <matthias@tenstral.net>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ascli-actions-misc.h"

#include <config.h>
#include <glib/gi18n-lib.h>

#include "as-utils-private.h"
#include "as-macros-private.h"
#include "as-pool-private.h"
#include "ascli-utils.h"
#include "as-component.h"
#include "as-news-convert.h"

/**
 * ascli_show_status:
 *
 * Print various interesting status information.
 */
gint
ascli_show_status (void)
{
	g_autoptr(AsPool) pool = NULL;
	g_autoptr(GError) error = NULL;
	gboolean os_metadata_found = FALSE;
	gboolean other_metadata_found = FALSE;

	/* TRANSLATORS: In the status report of ascli: Header */
	ascli_print_highlight (_("AppStream Status:"));
	ascli_print_stdout (_("Version: %s"), PACKAGE_VERSION);
	g_print ("\n");

	pool = as_pool_new ();
	as_pool_remove_flags (pool, AS_POOL_FLAG_MONITOR);

	/* TRANSLATORS: In the status report of ascli: Refers to the metadata shipped by distributions */
	ascli_print_highlight (_("OS metadata sources:"));
	os_metadata_found = as_pool_print_std_data_locations_info_private (pool, TRUE, FALSE);
	if (!os_metadata_found)
		/* TRANSLATORS: In ascli status, the OS had no metadata (which may be a bug) */
		g_print ("%s %s\n", ASCLI_CHAR_FAIL, _("No OS metadata found. This is unusual."));

	/* TRANSLATORS: In the status report of ascli: Refers to the metadata that isn't shipped by the OS (e.g. Flatpak) */
	ascli_print_highlight (_("Other metadata sources:"));
	other_metadata_found = os_metadata_found = as_pool_print_std_data_locations_info_private (
	    pool,
	    FALSE,
	    TRUE);
	if (!other_metadata_found)
		/* TRANSLATORS: In ascli status, no additional metadata sources have been found */
		g_print ("• %s\n", _("No metadata."));

	/* TRANSLATORS: Status summary in ascli */
	ascli_print_highlight (_("Summary:"));

	as_pool_load (pool, NULL, &error);
	if (error == NULL) {
		g_autoptr(AsComponentBox) cbox = NULL;
		g_autofree gchar *tmp = NULL;
		const gchar *marker;

		cbox = as_pool_get_components (pool);
		if (as_component_box_is_empty (cbox))
			marker = ASCLI_CHAR_FAIL;
		else
			marker = ASCLI_CHAR_SUCCESS;

		tmp = g_strdup_printf (_("We have information on %i software components."),
					  as_component_box_len (cbox));
		ascli_print_stdout ("%s %s", marker, tmp);
	} else {
		g_autofree gchar *tmp = NULL;
		tmp = g_strdup_printf (_("Error while loading the metadata pool: %s"),
					  error->message);
		ascli_print_stderr ("%s %s", ASCLI_CHAR_FAIL, tmp);
	}

	return 0;
}

/**
 * ascli_make_desktop_entry_file:
 *
 * Create a .desktop file from a metainfo file.
 */
gint
ascli_make_desktop_entry_file (const gchar *mi_fname, const gchar *de_fname, const gchar *exec_line)
{
	g_autoptr(AsMetadata) mdata = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GFile) mi_file = NULL;
	g_autoptr(GKeyFile) de_file = NULL;
	g_autofree gchar *de_fname_basename = NULL;
	g_autofree gchar *mi_fname_basename = NULL;
	AsComponent *cpt;
	GHashTableIter ht_iter;
	gpointer ht_key, ht_value;

	if (mi_fname == NULL) {
		ascli_print_stderr (_("You need to specify a metainfo file as input."));
		return 3;
	}
	if (de_fname == NULL) {
		ascli_print_stderr (
		    _("You need to specify a desktop-entry file to create or augment as output."));
		return 3;
	}

	de_fname_basename = g_path_get_basename (de_fname);
	mi_fname_basename = g_path_get_basename (mi_fname);

	/* load metainfo file */
	mi_file = g_file_new_for_path (mi_fname);
	if (!g_file_query_exists (mi_file, NULL)) {
		ascli_print_stderr (_("Metainfo file '%s' does not exist."), mi_fname);
		return 4;
	}

	mdata = as_metadata_new ();
	as_metadata_set_locale (mdata, "ALL");

	as_metadata_parse_file (mdata, mi_file, AS_FORMAT_KIND_XML, &error);
	if (error != NULL) {
		g_printerr ("%s\n", error->message);
		return 1;
	}

	cpt = as_metadata_get_component (mdata);
	de_file = g_key_file_new ();

	/* load desktop-entry file to augment, if it exists */
	if (g_file_test (de_fname, G_FILE_TEST_EXISTS)) {
		ascli_print_stdout (
		    _("Augmenting existing desktop-entry file '%s' with data from '%s'."),
		       de_fname_basename,
		       mi_fname_basename);
		if (!g_key_file_load_from_file (de_file,
						de_fname,
						G_KEY_FILE_KEEP_COMMENTS |
						    G_KEY_FILE_KEEP_TRANSLATIONS,
						&error)) {
			ascli_print_stderr (
			    _("Unable to load existing desktop-entry file template: %s"),
			       error->message);
			return 1;
		}
	} else {
		ascli_print_stdout (_("Creating new desktop-entry file '%s' using data from '%s'"),
				       de_fname_basename,
				       mi_fname_basename);
	}

	g_key_file_set_string (de_file,
			       G_KEY_FILE_DESKTOP_GROUP,
			       G_KEY_FILE_DESKTOP_KEY_TYPE,
			       G_KEY_FILE_DESKTOP_TYPE_APPLICATION);

	as_component_set_context_locale (cpt, "C");

	/* Name */
	g_key_file_set_string (de_file,
			       G_KEY_FILE_DESKTOP_GROUP,
			       G_KEY_FILE_DESKTOP_KEY_NAME,
			       as_component_get_name (cpt));

	g_hash_table_iter_init (&ht_iter, as_component_get_name_table (cpt));
	while (g_hash_table_iter_next (&ht_iter, &ht_key, &ht_value)) {
		if (g_strcmp0 ((const gchar *) ht_key, "C") != 0) {
			g_autofree gchar *name_key = g_strdup_printf ("Name[%s]",
								      (const gchar *) ht_key);
			g_key_file_set_string (de_file,
					       G_KEY_FILE_DESKTOP_GROUP,
					       name_key,
					       (const gchar *) ht_value);
		}
	}

	/* Comment */
	g_key_file_set_string (de_file,
			       G_KEY_FILE_DESKTOP_GROUP,
			       G_KEY_FILE_DESKTOP_KEY_COMMENT,
			       as_component_get_summary (cpt));

	g_hash_table_iter_init (&ht_iter, as_component_get_summary_table (cpt));
	while (g_hash_table_iter_next (&ht_iter, &ht_key, &ht_value)) {
		if (g_strcmp0 ((const gchar *) ht_key, "C") != 0) {
			g_autofree gchar *comment_key = g_strdup_printf ("Comment[%s]",
									 (const gchar *) ht_key);
			g_key_file_set_string (de_file,
					       G_KEY_FILE_DESKTOP_GROUP,
					       comment_key,
					       (const gchar *) ht_value);
		}
	}

	/* Icon */
	{
		AsIcon *stock_icon = NULL;
		GPtrArray *icons = as_component_get_icons (cpt);
		for (guint i = 0; i < icons->len; i++) {
			AsIcon *icon = AS_ICON (g_ptr_array_index (icons, i));
			if (as_icon_get_kind (icon) == AS_ICON_KIND_STOCK) {
				stock_icon = icon;
				break;
			}
		}
		if (stock_icon == NULL) {
			ascli_print_stderr (_("No stock icon name was provided in the metainfo file. Can not continue."));
			return 4;
		}
		g_key_file_set_string (de_file,
				       G_KEY_FILE_DESKTOP_GROUP,
				       G_KEY_FILE_DESKTOP_KEY_ICON,
				       as_icon_get_name (stock_icon));
	}

	/* Exec */
	if (exec_line == NULL) {
		GPtrArray *bin_items = NULL;
		AsProvided *prov = as_component_get_provided_for_kind (cpt,
								       AS_PROVIDED_KIND_BINARY);
		if (prov != NULL) {
			bin_items = as_provided_get_items (prov);
			if (bin_items != NULL) {
				if (bin_items->len < 1)
					bin_items = NULL;
			}
		}

		if (bin_items == NULL) {
			ascli_print_stderr (
			    _("No provided binary specified in metainfo file, and no exec command specified via '--exec'. Can not create 'Exec=' key."));
			return 4;
		}

		exec_line = g_ptr_array_index (bin_items, 0);
	}
	g_key_file_set_string (de_file,
			       G_KEY_FILE_DESKTOP_GROUP,
			       G_KEY_FILE_DESKTOP_KEY_EXEC,
			       exec_line);

	/* OnlyShowIn */
	{
		g_autofree gchar *only_show_in = as_ptr_array_to_str (
		    as_component_get_compulsory_for_desktops (cpt),
		    ";");
		if (only_show_in != NULL)
			g_key_file_set_string (de_file,
					       G_KEY_FILE_DESKTOP_GROUP,
					       G_KEY_FILE_DESKTOP_KEY_ONLY_SHOW_IN,
					       only_show_in);
	}

	/* MimeType */
	{
		AsProvided *prov = as_component_get_provided_for_kind (cpt,
								       AS_PROVIDED_KIND_MEDIATYPE);
		if (prov != NULL) {
			g_autofree gchar *mimetypes = as_ptr_array_to_str (
			    as_provided_get_items (prov),
			    ";");
			if (mimetypes != NULL)
				g_key_file_set_string (de_file,
						       G_KEY_FILE_DESKTOP_GROUP,
						       G_KEY_FILE_DESKTOP_KEY_MIME_TYPE,
						       mimetypes);
		}
	}

	/* Categories */
	{
		g_autofree gchar *categories = as_ptr_array_to_str (
		    as_component_get_categories (cpt),
		    ";");
		if (categories != NULL)
			g_key_file_set_string (de_file,
					       G_KEY_FILE_DESKTOP_GROUP,
					       G_KEY_FILE_DESKTOP_KEY_CATEGORIES,
					       categories);
	}

	/* Keywords */
	g_hash_table_iter_init (&ht_iter, as_component_get_keywords_table (cpt));
	while (g_hash_table_iter_next (&ht_iter, &ht_key, &ht_value)) {
		GPtrArray *kws = ht_value;
		g_autoptr(GString) keywords = g_string_new ("");

		for (guint i = 0; i < kws->len; ++i) {
			g_string_append_printf (keywords,
						"%s;",
						(const gchar *) g_ptr_array_index (kws, i));
		}

		if (g_strcmp0 ((const gchar *) ht_key, "C") == 0) {
			g_key_file_set_string (de_file,
					       G_KEY_FILE_DESKTOP_GROUP,
					       "Keywords",
					       keywords->str);
		} else {
			g_autofree gchar *keywords_key = g_strdup_printf ("Keywords[%s]",
									  (const gchar *) ht_key);
			g_key_file_set_string (de_file,
					       G_KEY_FILE_DESKTOP_GROUP,
					       keywords_key,
					       keywords->str);
		}
	}

	/* save the resulting desktop-entry file */
	if (!g_key_file_save_to_file (de_file, de_fname, &error)) {
		ascli_print_stderr (_("Unable to save desktop entry file: %s"), error->message);
		return 1;
	}

	return 0;
}

/**
 * ascli_news_to_metainfo:
 *
 * Convert NEWS data to a metainfo file.
 */
gint
ascli_news_to_metainfo (const gchar *news_fname,
			const gchar *mi_fname,
			const gchar *out_fname,
			guint entry_limit,
			guint translate_limit,
			const gchar *format_str)
{
	g_autoptr(GPtrArray) releases = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(AsMetadata) metad = NULL;
	g_autoptr(GFile) infile = NULL;
	AsComponent *cpt = NULL;
	AsReleaseList *cpt_releases = NULL;

	if (news_fname == NULL) {
		ascli_print_stderr (_("You need to specify a NEWS file as input."));
		return 3;
	}
	if (mi_fname == NULL) {
		ascli_print_stderr (_("You need to specify a metainfo file to augment, or '-' to print to stdout."));
		return 3;
	}
	if (out_fname == NULL) {
		if (g_strcmp0 (mi_fname, "-") != 0) {
			ascli_print_stdout (
			    _("No output filename specified, modifying metainfo file directly."));
			out_fname = mi_fname;
		}
	}

	releases = as_news_to_releases_from_filename (news_fname,
						      as_news_format_kind_from_string (format_str),
						      entry_limit,
						      translate_limit,
						      &error);
	if (error != NULL) {
		g_printerr ("%s\n", error->message);
		return 1;
	}
	if (releases == NULL) {
		g_printerr ("No releases retrieved and no error was emitted.\n");
		return 1;
	}

	if (g_strcmp0 (mi_fname, "-") == 0) {
		g_autofree gchar *tmp = NULL;

		tmp = as_releases_to_metainfo_xml_chunk (releases, &error);
		if (error != NULL) {
			g_printerr ("%s\n", error->message);
			return 1;
		}
		g_print ("%s\n", tmp);

		return 0;
	}

	infile = g_file_new_for_path (mi_fname);
	if (!g_file_query_exists (infile, NULL)) {
		ascli_print_stderr (_("Metainfo file '%s' does not exist."), mi_fname);
		return 4;
	}

	metad = as_metadata_new ();
	as_metadata_set_locale (metad, "ALL");

	as_metadata_parse_file (metad, infile, AS_FORMAT_KIND_XML, &error);
	if (error != NULL) {
		g_printerr ("%s\n", error->message);
		return 1;
	}

	cpt = as_metadata_get_component (metad);
	cpt_releases = as_component_get_releases_plain (cpt);

	/* remove all existing releases, we only include data from the specified file */
	as_release_list_clear (cpt_releases);

	for (guint i = 0; i < releases->len; ++i) {
		AsRelease *release = AS_RELEASE (g_ptr_array_index (releases, i));
		as_release_list_add (cpt_releases, release);
	}

	if (g_strcmp0 (out_fname, "-") == 0) {
		g_autofree gchar *tmp = as_metadata_component_to_metainfo (metad,
									   AS_FORMAT_KIND_XML,
									   &error);
		if (error != NULL) {
			g_printerr ("%s\n", error->message);
			return 1;
		}

		g_print ("%s\n", tmp);
		return 0;
	} else {
		as_metadata_save_metainfo (metad, out_fname, AS_FORMAT_KIND_XML, &error);
		if (error != NULL) {
			g_printerr ("%s\n", error->message);
			return 1;
		}

		return 0;
	}
}

/**
 * ascli_metainfo_to_news:
 *
 * Convert metainfo file to NEWS textfile.
 */
gint
ascli_metainfo_to_news (const gchar *mi_fname, const gchar *news_fname, const gchar *format_str)
{
	g_autoptr(AsMetadata) metad = NULL;
	g_autoptr(GFile) infile = NULL;
	g_autoptr(GError) error = NULL;
	AsComponent *cpt = NULL;
	AsNewsFormatKind format_kind;

	if (mi_fname == NULL) {
		ascli_print_stderr (_("You need to specify a metainfo file as input."));
		return 3;
	}
	if (news_fname == NULL) {
		ascli_print_stderr (
		    _("You need to specify a NEWS file as output, or '-' to print to stdout."));
		return 3;
	}

	infile = g_file_new_for_path (mi_fname);
	if (!g_file_query_exists (infile, NULL)) {
		ascli_print_stderr (_("Metainfo file '%s' does not exist."), mi_fname);
		return 4;
	}

	/* read the metainfo file */
	metad = as_metadata_new ();
	as_metadata_set_locale (metad, "ALL");

	as_metadata_parse_file (metad, infile, AS_FORMAT_KIND_XML, &error);
	if (error != NULL) {
		g_printerr ("%s\n", error->message);
		return 1;
	}
	cpt = as_metadata_get_component (metad);
	as_component_set_context_locale (cpt, "C");

	format_kind = as_news_format_kind_from_string (format_str);
	if (g_strcmp0 (news_fname, "-") == 0) {
		g_autofree gchar *news_data = NULL;

		if (format_kind == AS_NEWS_FORMAT_KIND_UNKNOWN) {
			ascli_print_stderr (
			    _("You need to specify a NEWS format to write the output in."));
			return 3;
		}

		as_releases_to_news_data (
		    as_release_list_get_entries (as_component_get_releases_plain (cpt)),
		    format_kind,
		    &news_data,
		    &error);
		if (error != NULL) {
			g_printerr ("%s\n", error->message);
			return 1;
		}

		g_print ("%s\n", news_data);
		return 0;
	} else {
		as_releases_to_news_file (
		    as_release_list_get_entries (as_component_get_releases_plain (cpt)),
		    news_fname,
		    format_kind,
		    &error);
		if (error != NULL) {
			g_printerr ("%s\n", error->message);
			return 1;
		}

		return 0;
	}
}

/**
 * ascli_show_sysinfo:
 *
 * Display information about the current operating system from the AppStream
 * metadata cache, as well as system information that we know about and that
 * is relevant for AppStream components.
 */
int
ascli_show_sysinfo (const gchar *cachepath, gboolean no_cache, gboolean detailed)
{
	g_autoptr(AsPool) pool = NULL;
	g_autoptr(AsComponentBox) result = NULL;
	g_autoptr(AsSystemInfo) sysinfo = NULL;
	g_autoptr(GError) error = NULL;
	gulong total_memory;
	GPtrArray *modaliases;

	sysinfo = as_system_info_new ();
	pool = ascli_data_pool_new_and_open (cachepath, no_cache, &error);
	if (error != NULL) {
		g_printerr ("%s\n", error->message);
		return 1;
	}

	ascli_print_highlight ("%s:", _("Operating System Details"));
	result = as_pool_get_components_by_id (pool, as_system_info_get_os_cid (sysinfo));
	if (as_component_box_is_empty (result)) {
		g_printerr ("• ");
		ascli_print_stderr (_("Unable to find operating system component '%s'!"),
				       as_system_info_get_os_cid (sysinfo));
	}

	for (guint i = 0; i < as_component_box_len (result); i++) {
		AsComponent *cpt = as_component_box_index (result, i);

		ascli_print_stdout ("%s: %s", _("ID"), as_component_get_id (cpt));
		ascli_print_stdout ("%s: %s", _("Name"), as_component_get_name (cpt));
		ascli_print_stdout ("%s: %s", _("Summary"), as_component_get_summary (cpt));
		if (as_system_info_get_os_version (sysinfo) != NULL)
			ascli_print_stdout (_("Version: %s"),
					       as_system_info_get_os_version (sysinfo));
		ascli_print_stdout ("%s: %s",
				    _("Homepage"),
				       as_component_get_url (cpt, AS_URL_KIND_HOMEPAGE));
		ascli_print_stdout ("%s: %s",
				    _("Developer"),
				       as_developer_get_name (as_component_get_developer (cpt)));
		if (detailed) {
			g_autofree gchar *tmp2 = NULL;
			g_autofree gchar *tmp1 = as_markup_convert (
			    as_component_get_description (cpt),
			    AS_MARKUP_KIND_TEXT,
			    NULL);
			tmp2 = ascli_format_long_output (tmp1, 100, 2);
			ascli_print_stdout ("%s:\n%s", _("Description"), tmp2);
		}

		if (i < as_component_box_len (result) - 1)
			ascli_print_separator ();
	}

	g_print ("\n");
	ascli_print_highlight ("%s:", _("Kernel"));
	ascli_print_stdout ("%s: %s", _("Name"), as_system_info_get_kernel_name (sysinfo));
	ascli_print_stdout ("%s: %s", _("Version"), as_system_info_get_kernel_version (sysinfo));

	g_print ("\n");
	ascli_print_highlight ("%s:", _("Hardware"));
	total_memory = as_system_info_get_memory_total (sysinfo);
	ascli_print_stdout ("%s: %lu MiB (%.2f GiB)",
			    _("Physical Memory"), total_memory, total_memory / 1024.0);

	modaliases = as_system_info_get_modaliases (sysinfo);
	if (modaliases->len > 0) {
		ascli_print_stdout ("%s:", _("Devices with Modaliases"));
		for (guint i = 0; i < modaliases->len; i++) {
			g_autoptr(GError) tmp_error = NULL;
			g_autofree gchar *dev_name = NULL;
			const gchar *modalias = (const gchar *) g_ptr_array_index (modaliases, i);

			dev_name = as_system_info_get_device_name_for_modalias (sysinfo,
										modalias,
										FALSE,
										&tmp_error);
			if (dev_name == NULL && !g_error_matches (tmp_error,
								  AS_SYSTEM_INFO_ERROR,
								  AS_SYSTEM_INFO_ERROR_NOT_FOUND)) {
				g_warning ("Unable to read device info: %s", tmp_error->message);
				continue;
			}

			if (detailed) {
				ascli_print_stdout (" • %s", dev_name ? dev_name : modalias);
				if (dev_name != NULL)
					ascli_print_stdout ("     %s", modalias);
			} else {
				if (dev_name != NULL)
					ascli_print_stdout (" • %s", dev_name);
			}
		}
	}

	g_print ("\n");
	ascli_print_highlight ("%s:", _("User Input Controls"));
	for (guint i = 1; i < AS_CONTROL_KIND_LAST; i++) {
		g_autoptr(GError) tmp_error = NULL;
		const gchar *found_str = _("unknown");
		AsCheckResult res = as_system_info_has_input_control (sysinfo, i, &tmp_error);
		if (res == AS_CHECK_RESULT_ERROR) {
			g_warning ("Unable to read input info: %s", tmp_error->message);
			continue;
		}
		if (res == AS_CHECK_RESULT_TRUE)
			found_str = _("yes");
		else if (res == AS_CHECK_RESULT_FALSE)
			found_str = _("no");
		else if (!detailed)
			continue;
		ascli_print_stdout (" • %s: %s", as_control_kind_to_string (i), found_str);
	}

	return 0;
}
