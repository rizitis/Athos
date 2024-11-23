/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2018-2024 Matthias Klumpp <matthias@tenstral.net>
 * Copyright (C) 2018 Richard Hughes <richard@hughsie.com>
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

#ifndef __AS_AGREEMENT_SECTION_PRIVATE_H
#define __AS_AGREEMENT_SECTION_PRIVATE_H

#include <glib-object.h>

#include "as-macros-private.h"
#include "as-agreement-section.h"
#include "as-xml.h"
#include "as-yaml.h"

AS_BEGIN_PRIVATE_DECLS

gboolean as_agreement_section_load_from_xml (AsAgreementSection *agreement_section,
					     AsContext		*ctx,
					     xmlNode		*node,
					     GError	       **error);
void	 as_agreement_section_to_xml_node (AsAgreementSection *agreement_section,
					   AsContext	      *ctx,
					   xmlNode	      *root);

gboolean as_agreement_section_load_from_yaml (AsAgreementSection *agreement_section,
					      AsContext		 *ctx,
					      GNode		 *node,
					      GError		**error);
void	 as_agreement_section_emit_yaml (AsAgreementSection *agreement_section,
					 AsContext	    *ctx,
					 yaml_emitter_t	    *emitter);

AS_END_PRIVATE_DECLS

#endif /* __AS_AGREEMENT_SECTION_PRIVATE_H */
