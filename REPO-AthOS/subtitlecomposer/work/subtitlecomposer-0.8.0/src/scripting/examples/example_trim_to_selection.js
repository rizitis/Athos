/*
	SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>

	SPDX-License-Identifier: GPL-2.0-or-later

	@category Examples
	@name Remove Unselected Lines
	@version 1.0
	@summary Example script to remove all lines except the selected ones.
	@author SubtitleComposer Team
*/

subtitle.instance().removeLines(ranges.newSelectionRangeList().complement());
