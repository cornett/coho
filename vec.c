/*
 * Copyright (c) 2017-2018 Ben Cornett <ben@lantern.is>
 *
 * Permission to use, copy, modify, and distribute this software for any
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
 */

#include <err.h>
#include <stdlib.h>

#include "coho.h"
#include "vec.h"


void *
vec_ensure(void *v, size_t n, size_t size, size_t *v_alloc)
{
	size_t nalloc;
	void *nv;

	if (n <= *v_alloc)
		return v;

	nalloc = *v_alloc;
	while (n > nalloc)
		nalloc = nalloc ? nalloc * 2 : 8;

	if ((nv = reallocarray(v, nalloc, size)) == NULL)
		return NULL;
	*v_alloc = nalloc;
	return nv;
}


void *
xvec_ensure(void *v, size_t n, size_t size, size_t *v_alloc)
{
	void *nv;

	if ((nv = vec_ensure(v, n, size, v_alloc)) == NULL)
		err(1,"%s", __func__);
	return nv;
}


void *
vec_ensure_append(void *v, size_t n, size_t size, size_t v_sz, size_t *v_alloc)
{
	return vec_ensure(v, v_sz + n, size, v_alloc);
}


void *
xvec_ensure_append(void *v, size_t n, size_t size, size_t v_sz, size_t *v_alloc)
{
	void *nv;

	if ((nv = vec_ensure_append(v, n, size, v_sz, v_alloc)) == NULL)
		err(1,"%s", __func__);
	return nv;
}
