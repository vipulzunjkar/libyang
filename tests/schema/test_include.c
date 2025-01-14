/**
 * \file test_include.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - submodule includes
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cmocka.h>

#include "libyang.h"
#include "tests/config.h"

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/files"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/files"

static int
setup_ctx(void **state)
{
    *state = ly_ctx_new_old(NULL, 0);
    if (!*state) {
        return -1;
    }
    return 0;
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy(*state, NULL);
    return 0;
}

static void
test_mult_revisions(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *sch_yang = "module main_mod {"
        "namespace \"urn:cesnet:test:a\";"
        "prefix \"a\";"
        "include submod_r { revision-date \"2016-06-19\";}"
        "include submod1;}";
    const char *sch_correct_yang = "module main_mod {"
        "namespace \"urn:cesnet:test:a\";"
        "prefix \"a\";"
        "include submod_r;"
        "include submod1;}";
    const char *sch_yin = "<module name=\"main_mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
        "<namespace uri=\"urn:cesnet:test:a\"/><prefix value=\"a\"/>"
        "<include module=\"submod_r\"><revision-date date=\"2016-06-19\"/></include>"
        "<include module=\"submod1\"/></module>";
    const char *sch_correct_yin = "<module name=\"main_mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
        "<namespace uri=\"urn:cesnet:test:a\"/><prefix value=\"a\"/>"
        "<include module=\"submod_r\"/>"
        "<include module=\"submod1\"/></module>";

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YIN);

    assert_ptr_equal(lys_parse_mem(ctx, sch_yin, LYS_IN_YIN), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, sch_correct_yin, LYS_IN_YIN), NULL);

    ly_ctx_destroy(*state, NULL);
    *state = ctx = ly_ctx_new_old(SCHEMA_FOLDER_YANG, 0);

    assert_ptr_equal(lys_parse_mem(ctx, sch_yang, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, sch_correct_yang, LYS_IN_YANG), NULL);
}

static void
test_circular_include(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *sch_yang = "module main-mod {"
        "namespace \"urn:cesnet:test:a\";"
        "prefix \"a\";"
        "include circ_inc1;}";
    const char *sch_yin = "<module name=\"main-mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
        "<namespace uri=\"urn:cesnet:test:a\"/><prefix value=\"a\"/>"
        "<include module=\"circ_inc1\"/></module>";

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YIN);

    assert_ptr_equal(lys_parse_mem(ctx, sch_yin, LYS_IN_YIN), NULL);
    assert_int_equal(ly_vecode, LYVE_CIRC_INCLUDES);

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);

    assert_ptr_equal(lys_parse_mem(ctx, sch_yang, LYS_IN_YANG), NULL);
    assert_int_equal(ly_vecode, LYVE_CIRC_INCLUDES);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_mult_revisions, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_circular_include, setup_ctx, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
