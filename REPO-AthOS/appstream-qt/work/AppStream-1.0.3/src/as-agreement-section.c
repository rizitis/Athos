/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2018 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2018-2024 Matthias Klumpp <matthias@tenstral.net>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:as-agreement-section
 * @short_description: Object representing a agreement section
 * @include: appstream.h
 * @stability: Unstable
 *
 * Agreements are typically split up into sections.
 * This class describes one agreement section.
 *
 * See also: #AsAgreement
 */

#include "as-agreement-section-private.h"
#include "as-utils-private.h"
#include "as-context-private.h"

typedef struct {
	gchar *kind;
	GHashTable *name;
	GHashTable *description;

	AsContext *context;
} AsAgreementSectionPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AsAgreementSection, as_agreement_section, G_TYPE_OBJECT)

#define GET_PRIVATE(o) (as_agreement_section_get_instance_private (o))

static void
as_agreement_section_finalize (GObject *object)
{
	AsAgreementSection *agreement_section = AS_AGREEMENT_SECTION (object);
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);

	g_free (priv->kind);
	g_hash_table_unref (priv->name);
	g_hash_table_unref (priv->description);

	if (priv->context != NULL)
		g_object_unref (priv->context);

	G_OBJECT_CLASS (as_agreement_section_parent_class)->finalize (object);
}

static void
as_agreement_section_init (AsAgreementSection *agreement_section)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);

	priv->name = g_hash_table_new_full (g_str_hash,
					    g_str_equal,
					    (GDestroyNotify) as_ref_string_release,
					    g_free);
	priv->description = g_hash_table_new_full (g_str_hash,
						   g_str_equal,
						   (GDestroyNotify) as_ref_string_release,
						   g_free);
}

static void
as_agreement_section_class_init (AsAgreementSectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = as_agreement_section_finalize;
}

/**
 * as_agreement_section_get_kind:
 * @agreement_section: a #AsAgreementSection instance.
 *
 * Gets the agreement section kind.
 *
 * Returns: a string, e.g. "GDPR", or NULL
 *
 * Since: 0.12.1
 **/
const gchar *
as_agreement_section_get_kind (AsAgreementSection *agreement_section)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);
	return priv->kind;
}

/**
 * as_agreement_section_set_kind:
 * @agreement_section: a #AsAgreementSection instance.
 * @kind: the agreement kind, e.g. "GDPR"
 *
 * Sets the agreement section kind.
 *
 * Since: 0.12.1
 **/
void
as_agreement_section_set_kind (AsAgreementSection *agreement_section, const gchar *kind)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);

	g_free (priv->kind);
	priv->kind = g_strdup (kind);
}

/**
 * as_agreement_section_get_name:
 * @agreement_section: a #AsAgreementSection instance.
 *
 * Gets the agreement section name.
 *
 * Returns: a string, e.g. "GDPR", or NULL
 *
 * Since: 0.12.1
 **/
const gchar *
as_agreement_section_get_name (AsAgreementSection *agreement_section)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);
	return as_context_localized_ht_get (priv->context, priv->name, NULL /* locale override */);
}

/**
 * as_agreement_section_set_name:
 * @agreement_section: a #AsAgreementSection instance.
 * @name: the agreement name, e.g. "GDPR"
 * @locale: (nullable): the locale. e.g. "en_GB"
 *
 * Sets the agreement section name.
 *
 * Since: 0.12.1
 **/
void
as_agreement_section_set_name (AsAgreementSection *agreement_section,
			       const gchar *name,
			       const gchar *locale)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);
	as_context_localized_ht_set (priv->context, priv->name, name, locale);
}

/**
 * as_agreement_section_get_description:
 * @agreement_section: a #AsAgreementSection instance.
 *
 * Gets the agreement section desc.
 *
 * Returns: a string, e.g. "GDPR", or NULL
 *
 * Since: 0.12.1
 **/
const gchar *
as_agreement_section_get_description (AsAgreementSection *agreement_section)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);
	return as_context_localized_ht_get (priv->context,
					    priv->description,
					    NULL /* locale override */);
}

/**
 * as_agreement_section_set_description:
 * @agreement_section: a #AsAgreementSection instance.
 * @locale: (nullable): the locale in BCP47 format. e.g. "en-GB"
 * @desc: the agreement description, e.g. "GDPR"
 *
 * Sets the agreement section desc.
 *
 * Since: 0.12.1
 **/
void
as_agreement_section_set_description (AsAgreementSection *agreement_section,
				      const gchar *desc,
				      const gchar *locale)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);
	as_context_localized_ht_set (priv->context, priv->description, desc, locale);
}

/**
 * as_agreement_section_get_context:
 * @agreement_section: An instance of #AsAgreementSection.
 *
 * Returns the #AsContext associated with this section.
 * This function may return %NULL if no context is set.
 *
 * Returns: (transfer none) (nullable): the #AsContext used by this agreement section.
 *
 * Since: 0.12.1
 */
AsContext *
as_agreement_section_get_context (AsAgreementSection *agreement_section)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);
	return priv->context;
}

/**
 * as_agreement_section_set_context:
 * @agreement_section: An instance of #AsAgreementSection.
 * @context: the #AsContext.
 *
 * Sets the document context this release is associated
 * with.
 *
 * Since: 0.12.1
 */
void
as_agreement_section_set_context (AsAgreementSection *agreement_section, AsContext *context)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);
	if (priv->context != NULL)
		g_object_unref (priv->context);
	priv->context = context == NULL ? NULL : g_object_ref (context);
}

/**
 * as_agreement_section_load_from_xml:
 * @agreement_section: an #AsAgreement
 * @ctx: the AppStream document context.
 * @node: the XML node.
 * @error: a #GError.
 *
 * Loads data from an XML node.
 **/
gboolean
as_agreement_section_load_from_xml (AsAgreementSection *agreement_section,
				    AsContext *ctx,
				    xmlNode *node,
				    GError **error)
{
	xmlNode *iter;
	gchar *prop;

	/* propagate context */
	as_agreement_section_set_context (agreement_section, ctx);

	prop = as_xml_get_prop_value (node, "type");
	if (prop != NULL) {
		as_agreement_section_set_kind (agreement_section, prop);
		g_free (prop);
	}

	for (iter = node->children; iter != NULL; iter = iter->next) {
		g_autofree gchar *lang = NULL;
		if (iter->type != XML_ELEMENT_NODE)
			continue;

		lang = as_xml_get_node_locale_match (ctx, iter);

		if (g_strcmp0 ((gchar *) iter->name, "name") == 0) {
			g_autofree gchar *content = NULL;

			content = as_xml_get_node_value (iter);
			if (content == NULL)
				continue;
			if (lang != NULL)
				as_agreement_section_set_name (agreement_section, content, lang);

			continue;
		}

		if (g_strcmp0 ((gchar *) iter->name, "description") == 0) {
			g_autofree gchar *content = NULL;

			content = as_xml_dump_node_children (iter);
			if (lang != NULL)
				as_agreement_section_set_description (agreement_section,
								      content,
								      lang);

			continue;
		}
	}

	return TRUE;
}

/**
 * as_agreement_section_to_xml_node:
 * @agreement_section: an #AsAgreement
 * @ctx: the AppStream document context.
 * @root: XML node to attach the new nodes to.
 *
 * Serializes the data to an XML node.
 **/
void
as_agreement_section_to_xml_node (AsAgreementSection *agreement_section,
				  AsContext *ctx,
				  xmlNode *root)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);
	xmlNode *asnode;

	asnode = as_xml_add_node (root, "agreement_section");
	as_xml_add_text_prop (asnode, "type", priv->kind);

	as_xml_add_localized_text_node (asnode, "name", priv->name);
	as_xml_add_description_node (ctx, asnode, priv->description, TRUE);
}

/**
 * as_agreement_section_load_from_yaml:
 * @agreement_section: an #AsAgreementSection
 * @ctx: the AppStream document context.
 * @node: the YAML node.
 * @error: a #GError.
 *
 * Loads data from a YAML field.
 **/
gboolean
as_agreement_section_load_from_yaml (AsAgreementSection *agreement_section,
				     AsContext *ctx,
				     GNode *node,
				     GError **error)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);
	GNode *n;

	/* propagate context */
	as_agreement_section_set_context (agreement_section, ctx);

	for (n = node->children; n != NULL; n = n->next) {
		const gchar *key = as_yaml_node_get_key (n);

		if (g_strcmp0 (key, "type") == 0) {
			as_agreement_section_set_kind (agreement_section,
						       as_yaml_node_get_value (n));
		} else if (g_strcmp0 (key, "name") == 0) {
			as_yaml_set_localized_table (ctx, n, priv->name);
		} else if (g_strcmp0 (key, "description") == 0) {
			as_yaml_set_localized_table (ctx, n, priv->description);
		} else {
			as_yaml_print_unknown ("agreement_section", key);
		}
	}

	return TRUE;
}

/**
 * as_agreement_section_emit_yaml:
 * @agreement_section: an #AsAgreementSection
 * @ctx: the AppStream document context.
 * @emitter: The YAML emitter to emit data on.
 *
 * Emit YAML data for this object.
 **/
void
as_agreement_section_emit_yaml (AsAgreementSection *agreement_section,
				AsContext *ctx,
				yaml_emitter_t *emitter)
{
	AsAgreementSectionPrivate *priv = GET_PRIVATE (agreement_section);

	/* start mapping for this agreement */
	as_yaml_mapping_start (emitter);

	/* type */
	as_yaml_emit_entry (emitter, "type", priv->kind);

	/* name */
	as_yaml_emit_localized_entry (emitter, "name", priv->name);

	/* description */
	as_yaml_emit_long_localized_entry (emitter, "description", priv->description);

	/* end mapping for the agreement */
	as_yaml_mapping_end (emitter);
}

/**
 * as_agreement_section_new:
 *
 * Creates a new #AsAgreementSection.
 *
 * Returns: (transfer full): a #AsAgreementSection
 *
 * Since: 0.12.1
 **/
AsAgreementSection *
as_agreement_section_new (void)
{
	AsAgreementSection *agreement_section;
	agreement_section = g_object_new (AS_TYPE_AGREEMENT_SECTION, NULL);
	return AS_AGREEMENT_SECTION (agreement_section);
}
