/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
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

#ifndef __AS_ARTIFACT_PRIVATE_H
#define __AS_ARTIFACT_PRIVATE_H

#include "as-artifact.h"
#include "as-xml.h"
#include "as-yaml.h"

G_BEGIN_DECLS
#pragma GCC visibility push(hidden)

gboolean	       as_artifact_load_from_xml (AsArtifact *artifact,
						  AsContext  *ctx,
						  xmlNode    *node,
						  GError    **error);
void	 as_artifact_to_xml_node (AsArtifact *artifact, AsContext *ctx, xmlNode *root);

gboolean as_artifact_load_from_yaml (AsArtifact *artifact,
				     AsContext	*ctx,
				     GNode	*node,
				     GError    **error);
void	 as_artifact_emit_yaml (AsArtifact *artifact, AsContext *ctx, yaml_emitter_t *emitter);

#pragma GCC visibility pop
G_END_DECLS

#endif /* __AS_ARTIFACT_PRIVATE_H */
