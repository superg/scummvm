/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:   
 
    Isometric level module

 Notes: 
*/

#include "reinherit.h"

#include "yslib.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "game_mod.h"
#include "gfx_mod.h"

/*
 * Begin module component
\*--------------------------------------------------------------------------*/
#include "isomap_mod.h"
#include "isomap.h"

namespace Saga {

static R_ISOMAP_MODULE IsoModule;

int ISOMAP_Init(void)
{
	IsoModule.init = 1;

	return R_SUCCESS;
}

int ISOMAP_LoadTileset(const uchar * tileres_p, size_t tileres_len)
{

	R_ISOTILE_ENTRY first_entry;
	R_ISOTILE_ENTRY *tile_tbl;

	uint i;

	const uchar *read_p = tileres_p;
	size_t read_len = tileres_len;

	assert((IsoModule.init) && (!IsoModule.tiles_loaded));
	assert((tileres_p != NULL) && (tileres_len > 0));

	read_p += 2;
	first_entry.tile_offset = ys_read_u16_le(read_p, &read_p);

	IsoModule.tile_ct = first_entry.tile_offset / SAGA_ISOTILE_ENTRY_LEN;

	read_p = tileres_p;
	read_len = tileres_len;

	tile_tbl = (R_ISOTILE_ENTRY *)malloc(IsoModule.tile_ct * sizeof *tile_tbl);
	if (tile_tbl == NULL) {
		return R_MEM;
	}

	for (i = 0; i < IsoModule.tile_ct; i++) {

		tile_tbl[i].tile_h = ys_read_u8(read_p, &read_p);
		tile_tbl[i].unknown01 = ys_read_u8(read_p, &read_p);

		tile_tbl[i].tile_offset = ys_read_u16_le(read_p, &read_p);
		tile_tbl[i].unknown04 = ys_read_s16_le(read_p, &read_p);
		tile_tbl[i].unknown06 = ys_read_s16_le(read_p, &read_p);
	}

	IsoModule.tiles_loaded = 1;
	IsoModule.tile_tbl = tile_tbl;
	IsoModule.tileres_p = tileres_p;
	IsoModule.tileres_len = tileres_len;

	return R_SUCCESS;
}

int ISOMAP_LoadMetaTileset(const uchar * mtileres_p, size_t mtileres_len)
{

	R_ISO_METATILE_ENTRY *mtile_tbl;

	const uchar *read_p = mtileres_p;
	size_t read_len = mtileres_len;

	uint mtile_ct;
	uint ct;

	int i;

	assert(IsoModule.init);
	assert((mtileres_p != NULL) && (mtileres_len > 0));

	(void)read_len;

	mtile_ct = mtileres_len / SAGA_METATILE_ENTRY_LEN;

	mtile_tbl = (R_ISO_METATILE_ENTRY *)malloc(mtile_ct * sizeof *mtile_tbl);
	if (mtile_tbl == NULL) {
		return R_MEM;
	}

	for (ct = 0; ct < mtile_ct; ct++) {

		mtile_tbl[ct].mtile_n = ys_read_u16_le(read_p, &read_p);
		mtile_tbl[ct].unknown02 = ys_read_s16_le(read_p, &read_p);
		mtile_tbl[ct].unknown04 = ys_read_s16_le(read_p, &read_p);
		mtile_tbl[ct].unknown06 = ys_read_s16_le(read_p, &read_p);

		for (i = 0; i < SAGA_METATILE_SIZE; i++) {
			mtile_tbl[ct].tile_tbl[i] =
			    ys_read_u16_le(read_p, &read_p);
		}
	}

	IsoModule.mtile_ct = mtile_ct;
	IsoModule.mtile_tbl = mtile_tbl;
	IsoModule.mtileres_p = mtileres_p;
	IsoModule.mtileres_len = mtileres_len;

	IsoModule.mtiles_loaded = 1;

	return R_SUCCESS;
}

int ISOMAP_LoadMetamap(const uchar * mm_res_p, size_t mm_res_len)
{

	const uchar *read_p = mm_res_p;
	size_t read_len = mm_res_len;

	int i;

	(void)read_len;

	IsoModule.metamap_n = ys_read_s16_le(read_p, &read_p);

	for (i = 0; i < SAGA_METAMAP_SIZE; i++) {

		IsoModule.metamap_tbl[i] = ys_read_u16_le(read_p, &read_p);
	}

	IsoModule.mm_res_p = mm_res_p;
	IsoModule.mm_res_len = mm_res_len;
	IsoModule.metamap_loaded = 1;

	return R_SUCCESS;
}

int ISOMAP_Draw(R_SURFACE * dst_s)
{

	R_GAME_DISPLAYINFO disp_info;

	GAME_GetDisplayInfo(&disp_info);

	R_RECT iso_rect(disp_info.logical_w - 1, disp_info.scene_h - 1);

	GFX_DrawRect(dst_s, &iso_rect, 0);

	ISOMAP_DrawMetamap(dst_s, -1000, -500);

	return R_SUCCESS;
}

int ISOMAP_DrawMetamap(R_SURFACE * dst_s, int map_x, int map_y)
{
	int meta_base_x = map_x;
	int meta_base_y = map_y;

	int meta_xi;
	int meta_yi;

	int meta_x;
	int meta_y;

	int meta_idx;

	for (meta_yi = SAGA_METAMAP_H - 1; meta_yi >= 0; meta_yi--) {

		meta_x = meta_base_x;
		meta_y = meta_base_y;

		for (meta_xi = SAGA_METAMAP_W - 1; meta_xi >= 0; meta_xi--) {

			meta_idx = meta_xi + (meta_yi * 16);

			ISOMAP_DrawMetaTile(dst_s,
			    IsoModule.metamap_tbl[meta_idx], meta_x, meta_y);

			meta_x += 128;
			meta_y += 64;
		}

		meta_base_x -= 128;
		meta_base_y += 64;
	}

	return R_SUCCESS;
}

int
ISOMAP_DrawMetaTile(R_SURFACE * dst_s, uint mtile_i, int mtile_x, int mtile_y)
{

	int tile_xi;
	int tile_yi;

	int tile_x;
	int tile_y;

	int tile_base_x;
	int tile_base_y;

	int tile_i;

	R_ISO_METATILE_ENTRY *mtile_p;
	assert(IsoModule.init && IsoModule.mtiles_loaded);

	if (mtile_i >= IsoModule.mtile_ct) {
		return R_FAILURE;
	}

	mtile_p = &IsoModule.mtile_tbl[mtile_i];

	tile_base_x = mtile_x;
	tile_base_y = mtile_y;

	for (tile_yi = SAGA_METATILE_H - 1; tile_yi >= 0; tile_yi--) {

		tile_y = tile_base_y;
		tile_x = tile_base_x;

		for (tile_xi = SAGA_METATILE_W - 1; tile_xi >= 0; tile_xi--) {

			tile_i = tile_xi + (tile_yi * SAGA_METATILE_W);

			ISOMAP_DrawTile(dst_s,
			    mtile_p->tile_tbl[tile_i], tile_x, tile_y);

			tile_x += SAGA_ISOTILE_WIDTH / 2;
			tile_y += SAGA_ISOTILE_BASEHEIGHT / 2 + 1;

		}

		tile_base_x -= SAGA_ISOTILE_WIDTH / 2;
		tile_base_y += SAGA_ISOTILE_BASEHEIGHT / 2 + 1;

	}

	return R_SUCCESS;
}

int ISOMAP_DrawTile(R_SURFACE * dst_s, uint tile_i, int tile_x, int tile_y)
{

	const uchar *tile_p;
	const uchar *read_p;

	uchar *draw_p;

	int draw_x;
	int draw_y;

	int tile_h;
	int w_count = 0;
	int row;

	int bg_runct;
	int fg_runct;
	int ct;

	assert(IsoModule.init && IsoModule.tiles_loaded);

	if (tile_i >= IsoModule.tile_ct) {
		return R_FAILURE;
	}

	/* temporary x clip */
	if (tile_x < 0) {
		return R_SUCCESS;
	}

	/* temporary x clip */
	if (tile_x >= 320 - 32) {
		return R_SUCCESS;
	}

	tile_p = IsoModule.tileres_p + IsoModule.tile_tbl[tile_i].tile_offset;
	tile_h = IsoModule.tile_tbl[tile_i].tile_h;

	read_p = tile_p;
	draw_p = dst_s->buf + tile_x + (tile_y * dst_s->buf_pitch);

	draw_x = tile_x;
	draw_y = tile_y;

	if (tile_h > SAGA_ISOTILE_BASEHEIGHT) {
		draw_y = tile_y - (tile_h - SAGA_ISOTILE_BASEHEIGHT);
	}

	/* temporary y clip */
	if (draw_y < 0) {
		return R_SUCCESS;
	}

	for (row = 0; row < tile_h; row++) {

		draw_p =
		    dst_s->buf + draw_x + ((draw_y + row) * dst_s->buf_pitch);
		w_count = 0;

		/* temporary y clip */
		if ((draw_y + row) >= 137) {
			return R_SUCCESS;
		}

		for (;;) {

			bg_runct = *read_p++;
			w_count += bg_runct;

			if (w_count >= SAGA_ISOTILE_WIDTH) {
				break;
			}

			draw_p += bg_runct;

			fg_runct = *read_p++;
			w_count += fg_runct;

			for (ct = 0; ct < fg_runct; ct++) {

				*draw_p++ = *read_p++;
			}
		}
	}

	return R_SUCCESS;
}

} // End of namespace Saga
