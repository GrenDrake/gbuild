#include <stdlib.h>
#include <check.h>

#include "../gbuild.h"

int count_tokens(tokenlist_t *tokens);


int count_tokens(tokenlist_t *tokens) {
    int token_count = 0;
    lexertoken_t *cur = tokens->first;
    while (cur) {
        ++token_count;
        cur = cur->next;
    }
    return token_count;
}


START_TEST(test_lex_identifier_basic)
{
    const char *test_string = "abc";
    tokenlist_t* tokens = lex_string(0, "test", test_string, strlen(test_string));
    ck_assert_int_eq(1, count_tokens(tokens));
    ck_assert_int_eq(IDENTIFIER, tokens->first->type);
    ck_assert_str_eq(tokens->first->text, "abc");
    free_tokens(tokens);
}
END_TEST

START_TEST(test_lex_integer_basic)
{
    const char *test_string = "956357";
    tokenlist_t* tokens = lex_string(0, "test", test_string, strlen(test_string));
    ck_assert_int_eq(1, count_tokens(tokens));
    ck_assert_int_eq(INTEGER, tokens->first->type);
    ck_assert_int_eq(tokens->first->integer, 956357);
    free_tokens(tokens);
}
END_TEST

START_TEST(test_lex_integer_hex)
{
    const char *test_string = "0x456fa1";
    tokenlist_t* tokens = lex_string(0, "test", test_string, strlen(test_string));
    ck_assert_int_eq(1, count_tokens(tokens));
    ck_assert_int_eq(INTEGER, tokens->first->type);
    ck_assert_int_eq(tokens->first->integer, 4550561);
    free_tokens(tokens);
}
END_TEST

START_TEST(test_lex_integer_char_constant)
{
    const char *test_string = "'a'";
    tokenlist_t* tokens = lex_string(0, "test", test_string, strlen(test_string));
    ck_assert_int_eq(1, count_tokens(tokens));
    ck_assert_int_eq(INTEGER, tokens->first->type);
    ck_assert_int_eq(tokens->first->integer, 97);
    free_tokens(tokens);
}
END_TEST

START_TEST(test_lex_integer_char_constant_tightbordered)
{
    const char *test_string = "fad'z'{";
    tokenlist_t* tokens = lex_string(0, "test", test_string, strlen(test_string));
    ck_assert_int_eq(3, count_tokens(tokens));
    ck_assert_int_eq(INTEGER, tokens->first->next->type);
    ck_assert_int_eq(tokens->first->next->integer, 122);
    free_tokens(tokens);
}
END_TEST

START_TEST(test_lex_integer_char_constant_bordered)
{
    const char *test_string = "\"452\" 'z' afd";
    tokenlist_t* tokens = lex_string(0, "test", test_string, strlen(test_string));
    ck_assert_int_eq(3, count_tokens(tokens));
    ck_assert_int_eq(INTEGER, tokens->first->next->type);
    ck_assert_int_eq(tokens->first->next->integer, 122);
    free_tokens(tokens);
}
END_TEST


Suite* lexer_suite(void) {
    Suite *s = suite_create("Lexer");
    TCase *tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_lex_identifier_basic);
    tcase_add_test(tc_core, test_lex_integer_basic);
    tcase_add_test(tc_core, test_lex_integer_hex);
    tcase_add_test(tc_core, test_lex_integer_char_constant);
    tcase_add_test(tc_core, test_lex_integer_char_constant_tightbordered);
    tcase_add_test(tc_core, test_lex_integer_char_constant_bordered);
    suite_add_tcase(s, tc_core);
    return s;
}

int main(void) {

    Suite *s = lexer_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}