/*
 * Copyright (c) 2009, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/* vsc8601.h
 *
 * History:
 * Jan 14, 2007 wclewis ready common BDI/ECOS/Linux
 * May 24, 2007 Tag before BSP resturcture 
 */

#ifndef _VSC8601_PHY_H
#define _VSC8601_PHY_H

#ifndef CEXTERN
#define  CEXTERN static inline
#endif

#include "ag7100.h"

/* These must be extern to allow linking to ag7100 */
int vsc8601_phy_setup(int unit);

unsigned int 
vsc8601_phy_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed, unsigned int *cfg);

int
vsc8601_phy_print_link_status(int unit);

#endif
