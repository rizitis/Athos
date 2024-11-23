/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2019-2024 Matthias Klumpp <matthias@tenstral.net>
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

#ifndef __AS_VIDEO_PRIVATE_H
#define __AS_VIDEO_PRIVATE_H

#include "as-video.h"
#include "as-xml.h"
#include "as-yaml.h"

G_BEGIN_DECLS
#pragma GCC visibility push(hidden)

gboolean as_video_load_from_xml (AsVideo *video, AsContext *ctx, xmlNode *node, GError **error);
void	 as_video_to_xml_node (AsVideo *video, AsContext *ctx, xmlNode *root);

gboolean as_video_load_from_yaml (AsVideo *video, AsContext *ctx, GNode *node, GError **error);
void	 as_video_emit_yaml (AsVideo *video, AsContext *ctx, yaml_emitter_t *emitter);

#pragma GCC visibility pop
G_END_DECLS

#endif /* __AS_VIDEO_PRIVATE_H */
