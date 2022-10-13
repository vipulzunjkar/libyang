/**
 * @file test_emptycont.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for auto delete of empty containers.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

struct state {
    struct ly_ctx *ctx;
    const struct lys_module *mod;
    struct lyd_node *dt;
    char *xml;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *schemafile = TESTS_DIR"/data/files/emptycont.yin";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new_old(NULL, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schema */
    st->mod = lys_parse_path(st->ctx, schemafile, LYS_IN_YIN);
    if (!st->mod) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->xml);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_parse(void **state)
{
    struct state *st = (*state);
    const char *xml = "<topleaf xmlns=\"urn:libyang:tests:emptycont\">X</topleaf>"
                      "<top xmlns=\"urn:libyang:tests:emptycont\"><a>A</a><b><b1>B</b1></b><c><c1>C</c1></c></top>";

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(st->xml, xml);
}

static void
test_parse_autodel1(void **state)
{
    struct state *st = (*state);
    const char *xml = "<topleaf xmlns=\"urn:libyang:tests:emptycont\">X</topleaf>"
                      "<top xmlns=\"urn:libyang:tests:emptycont\"><b><b1>B</b1></b><c><c1>C</c1></c></top>";

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(st->xml, "<topleaf xmlns=\"urn:libyang:tests:emptycont\">X</topleaf><top xmlns=\"urn:libyang:tests:emptycont\"><c/></top>");
}

static void
test_parse_autodel2(void **state)
{
    struct state *st = (*state);
    const char *xml = "<top xmlns=\"urn:libyang:tests:emptycont\"><b><b1>B</b1></b><c><c1>C</c1></c></top>";

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(st->xml, "<top xmlns=\"urn:libyang:tests:emptycont\"><c/></top>");
}

static void
test_parse_autodel3(void **state)
{
    struct state *st = (*state);
    const char *xml = "<topleaf xmlns=\"urn:libyang:tests:emptycont\">X</topleaf>"
                      "<top xmlns=\"urn:libyang:tests:emptycont\"><b><b1>B</b1></b></top>";

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(st->xml, "<topleaf xmlns=\"urn:libyang:tests:emptycont\">X</topleaf>");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_parse, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_parse_autodel1, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_parse_autodel2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_parse_autodel3, setup_f, teardown_f), };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
