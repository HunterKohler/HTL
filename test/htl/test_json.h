#ifndef HTL_TEST_JSON_H_
#define HTL_TEST_JSON_H_

#include <cstddef>
#include <string>
#include <htl/json.h>
#include "./util/allocator.h"

namespace htl::test {

using namespace htl::json;
using namespace std::literals;

using TestAlloc = IdentityAllocator<std::byte>;
using TestDocument = BasicDocument<IdentityAllocator<std::byte>>;
using TestString = BasicString<IdentityAllocator<std::byte>>;
using TestArray = BasicArray<IdentityAllocator<std::byte>>;
using TestObject = BasicObject<IdentityAllocator<std::byte>>;

struct ParseSuccessCase {
    std::string name;
    std::string input;
    TestDocument value;
    ParseOptions opts = {};
    std::string remaining = {};
};

struct ParseFailCase {
    std::string name;
    std::string input;
    ParseErrorCode code;
    ParseOptions opts = {};
};

inline const std::vector<ParseSuccessCase> parse_success_cases = {
    {
        "array_arraysWithSpaces",
        "[[]   ]",
        TestArray{ TestArray{} },
    },
    {
        "array_empty-string",
        "[\"\"]",
        TestArray{ "" },
    },
    {
        "array_empty",
        "[]",
        TestArray{},
    },
    {
        "array_ending_with_newline",
        "[\"a\"]\n",
        TestArray{ "a" },
    },
    {
        "array_false",
        "[false]",
        TestArray{ false },
    },
    {
        "array_heterogeneous",
        "[null, 1, \"1\", {}]",
        TestArray{ nullptr, 1, "1", TestObject{} },
    },
    {
        "array_null",
        "[null]",
        TestArray{ nullptr },
    },
    {
        "array_with_1_and_newline",
        "[1\n]",
        TestArray{ 1 },
    },
    {
        "array_with_leading_space",
        " [1]",
        TestArray{ 1 },
    },
    {
        "array_with_several_null",
        "[1,null,null,null,2]",
        TestArray{ 1, nullptr, nullptr, nullptr, 2 },
    },
    {
        "array_with_trailing_space",
        "[2] ",
        TestArray{ 2 },
    },
    {
        "number",
        "123e65",
        123e65,
    },
    {
        "number_0e+1",
        "0e+1",
        0e+1,
    },
    {
        "number_0e1",
        "0e1",
        0e1,
    },
    {
        "number_after_space",
        " 4",
        4,
    },
    {
        "number_double_close_to_zero",
        "-0.000000000000000000000000000000000000000000000000000000000000000000"
        "000000000001\n",
        -1e-78,
    },
    {
        "number_int_with_exp",
        "20e1",
        20e1,
    },
    {
        "number_minus_zero",
        "-0",
        0,
    },
    {
        "number_negative_int",
        "-123",
        -123,
    },
    {
        "number_negative_one",
        "-1",
        -1,
    },
    {
        "number_real_capital_e",
        "1E22",
        1e22,
    },
    {
        "number_real_capital_e_neg_exp",
        "1E-2",
        1e-2,
    },
    {
        "number_real_capital_e_pos_exp",
        "1E+2",
        1e2,
    },
    {
        "number_real_exponent",
        "123e45",
        123e45,
    },
    {
        "number_real_fraction_exponent",
        "123.456e78",
        123.456e78,
    },
    {
        "number_real_neg_exp",
        "1e-2",
        1e-2,
    },
    {
        "number_real_pos_exponent",
        "1e+2",
        1e2,
    },
    {
        "number_simple_int",
        "123",
        123,
    },
    {
        "number_simple_real",
        "123.456789",
        123.456789,
    },
    {
        "object",
        "{\"asd\":\"sdf\", \"dfg\":\"fgh\"}",
        TestObject{ { "asd", "sdf" }, { "dfg", "fgh" } },
    },
    {
        "object_basic",
        "{\"asd\":\"sdf\"}",
        TestObject{ { "asd", "sdf" } },
    },
    {
        "object_duplicated_key_with_accept_duplicate_keys",
        "{\"a\":\"b\",\"a\":\"c\"}",
        TestObject{ { "a", "c" } },
        { .accept_duplicate_keys = true },
    },
    {
        "object_empty",
        "{}",
        TestObject{},
    },
    {
        "object_empty_key",
        "{\"\":0}",
        TestObject{ { "", 0 } },
    },
    {
        "object_escaped_null_in_key",
        "{\"foo\\u0000bar\": 42}",
        TestObject{ { TestString("foo\u0000bar"s), 42 } },
    },
    {
        "object_extreme_numbers",
        "{ \"min\": -1.0e+28, \"max\": 1.0e+28 }",
        TestObject{ { "min", -1e28 }, { "max", 1e28 } },
    },
    {
        "object_simple",
        "{\"a\":[]}",
        TestObject{ { "a", TestArray{} } },
    },
    {
        "object_with_newlines",
        "{\n\"a\": \"b\"\n}",
        TestObject{ { "a", "b" } },
    },
    {
        "string_1_2_3_bytes_UTF-8_sequences",
        "\"\\u0060\\u012A\\u12AB\"",
        "\u0060\u012A\u12AB",
    },
    {
        "string_accepted_surrogate_pair",
        "\"\\uD801\\udc37\"",
        "\U00010437",
    },
    {
        "string_accepted_surrogate_pairs",
        "\"\\ud83d\\ude39\\ud83d\\udc8d\"",
        "\U0001F639\U0001F48D",
    },
    {
        "string_accepted_escapes",
        "\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"",
        "\"\\/\b\f\n\r\t",
    },
    {
        "string_backslash_and_u_escaped_zero",
        "\"\\\\u0000\"",
        "\\u0000",
    },
    {
        "string_backslash_doublequotes",
        "\"\\\"\"",
        "\"",
    },
    {
        "string_comments",
        "\"a/*b*/c/*d//e\"",
        "a/*b*/c/*d//e",
    },
    {
        "string_double_escape_a",
        "\"\\\\a\"",
        "\\a",
    },
    {
        "string_double_escape_n",
        "\"\\\\n\"",
        "\\n",
    },
    {
        "string_escaped_control_character",
        "\"\\u0012\"",
        "\u0012",
    },
    {
        "string_in_array",
        "[\"asd\"]",
        TestArray{ "asd" },
    },
    {
        "string_in_array_with_leading_space",
        "[ \"asd\"]",
        TestArray{ "asd" },
    },
    {
        "string_last_surrogates_1_and_2",
        "\"\\uDBFF\\uDFFF\"",
        "\uFFFD",
    },
    {
        "string_last_surrogates_1_and_2_without_replace_invalid_code_points",
        "\"\\uDBFF\\uDFFF\"",
        "\U0010FFFF",
        { .replace_invalid_code_points = false },
    },
    {
        "string_nbsp_uescaped",
        "\"new\\u00A0line\"",
        "new\u00A0line",
    },
    {
        "string_nonCharacterInUTF-8_U+10FFFF",
        "\"\U0010ffff\"",
        "\uFFFD",
    },
    {
        "string_nonCharacterInUTF-8_U+10FFFF_without_replace_invalid_code_poin"
        "ts",
        "\"\U0010ffff\"",
        "\U0010FFFF",
        { .replace_invalid_code_points = false },
    },
    {
        "string_nonCharacterInUTF-8_U+FFFF",
        "\"\uffff\"",
        "\uFFFD",
    },
    {
        "string_nonCharacterInUTF-8_U+FFFF_without_replace_invalid_code_points",
        "\"\uffff\"",
        "\uFFFF",
        { .replace_invalid_code_points = false },
    },
    {
        "string_null_escape",
        "\"\\u0000\"",
        TestString("\u0000"s),
    },
    {
        "string_one-byte-utf-8",
        "\"\\u002c\"",
        "\u002c",
    },
    {
        "string_pi",
        "\"\u03C0\"",
        "\u03C0",
    },
    {
        "string_reservedCharacterInUTF-8_U+1BFFF",
        "\"\U0001BFFF\"",
        "\U0001BFFF",
    },
    {
        "string_simple_ascii",
        "\"asd \"",
        "asd ",
    },
    {
        "string_space",
        "\" \"",
        " ",
    },
    {
        "string_surrogates_U+1D11E_MUSICAL_SYMBOL_G_CLEF",
        "\"\\uD834\\uDD1E\"",
        "\U0001D11E",
    },
    {
        "string_three-byte-utf-8",
        "\"\\u0821\"",
        "\u0821",
    },
    {
        "string_two-byte-utf-8",
        "\"\\u0123\"",
        "\u0123",
    },
    {
        "string_u+2028_line_sep",
        "\"\u2028\"",
        "\u2028",
    },
    {
        "string_u+2029_par_sep",
        "\"\u2029\"",
        "\u2029",
    },
    {
        "string_uEscape",
        "\"\\u0061\\u30AF\\u30EA\\u30B9\"",
        "\u0061\u30AF\u30EA\u30B9",
    },
    {
        "string_uescaped_newline",
        "\"new\\u000Aline\"",
        "new\nline",
    },
    {
        "string_unescaped_char_delete",
        "\"\x7F\"",
        "\x7F",
    },
    {
        "string_unicode",
        "\"\\uA66D\"",
        "\uA66D",
    },
    {
        "string_unicodeEscapedBackslash",
        "\"\\u005C\"",
        "\u005C",
    },
    {
        "string_unicode_2",
        "\"\u2342\u3234\u2342\"",
        "\u2342\u3234\u2342",
    },
    {
        "string_unicode_U+10FFFE_nonchar",
        "\"\\uDBFF\\uDFFE\"",
        "\uFFFD",
    },
    {
        "string_unicode_U+10FFFE_nonchar_without_replace_invalid_code_points",
        "\"\\uDBFF\\uDFFE\"",
        "\U0010FFFE",
        { .replace_invalid_code_points = false },
    },
    {
        "string_unicode_U+10FFFE_nonchar",
        "\"\\uD83F\\uDFFE\"",
        "\uFFFD",
    },
    {
        "string_unicode_U+1FFFE_nonchar_without_replace_invalid_code_points",
        "\"\\uD83F\\uDFFE\"",
        "\U0001FFFE",
        { .replace_invalid_code_points = false },
    },
    {
        "string_unicode_U+200B_ZERO_WIDTH_SPACE",
        "\"\\u200B\"",
        "\u200B",
    },
    {
        "string_unicode_U+2064_invisible_plus",
        "\"\\u2064\"",
        "\u2064",
    },
    {
        "string_unicode_U+FDD0_nonchar",
        "\"\\uFDD0\"",
        "\uFFFD",
    },
    {
        "string_unicode_U+FDD0_nonchar_without_replace_invalid_code_points",
        "\"\\uFDD0\"",
        "\uFDD0",
        { .replace_invalid_code_points = false },
    },
    {
        "string_unicode_U+FFFE_nonchar",
        "\"\\uFFFE\"",
        "\uFFFD",
    },
    {
        "string_unicode_U+FFFE_nonchar_without_replace_invalid_code_points",
        "\"\\uFFFE\"",
        "\uFFFE",
        { .replace_invalid_code_points = false },
    },
    {
        "string_unicode_escaped_double_quote",
        "\"\\u0022\"",
        "\u0022",
    },
    {
        "string_utf8",
        "\"\u20ac\U0001d11e\"",
        "\u20AC\U0001D11E",
    },
    {
        "string_with_del_character",
        "\"a\x7Fz\"",
        "a\x7Fz",
    },
    {
        "structure_lonely_false",
        "false",
        false,
    },
    {
        "structure_lonely_int",
        "42",
        42,
    },
    {
        "structure_lonely_negative_real",
        "-0.1",
        -0.1,
    },
    {
        "structure_lonely_null",
        "null",
        nullptr,
    },
    {
        "structure_lonely_string",
        "\"asd\"",
        "asd",
    },
    {
        "structure_lonely_true",
        "true",
        true,
    },
    { "structure_string_empty", "\"\"", "" },
    {
        "structure_trailing_newline",
        "[\"a\"]\n",
        TestArray{ "a" },
        {},
        "\n",
    },
    {
        "structure_true_in_array",
        "[true]",
        TestArray{ true },
    },
    {
        "structure_whitespace_array",
        " [] ",
        TestArray{},
        {},
        " ",
    },
    {
        "structure_trailing_#",
        "{\"a\":\"b\"}#{}",
        TestObject{ { "a", "b" } },
        {},
        "#{}",
    },
    {
        "object_trailing_comment",
        "{\"a\":\"b\"}/**/",
        TestObject{ { "a", "b" } },
        {},
        "/**/",
    },
    {
        "object_trailing_comment_open",
        "{\"a\":\"b\"}/**//",
        TestObject{ { "a", "b" } },
        {},
        "/**//",
    },
    {
        "object_trailing_comment_slash_open",
        "{\"a\":\"b\"}//",
        TestObject{ { "a", "b" } },
        {},
        "//",
    },
    {
        "object_trailing_comment_slash_open_incomplete",
        "{\"a\":\"b\"}/",
        TestObject{ { "a", "b" } },
        {},
        "/",
    },
    {
        "structure_object_with_trailing_garbage",
        "{\"a\": true} \"x\"",
        TestObject{ { "a", true } },
        {},
        " \"x\"",
    },
    {
        "structure_number_with_trailing_garbage",
        "2@",
        2,
        {},
        "@",
    },
    {
        "structure_array_trailing_garbage",
        "[1]x",
        TestArray{ 1 },
        {},
        "x",
    },
    {
        "string_with_trailing_garbage",
        "\"\"x",
        "",
        {},
        "x",
    },
    {
        "object_with_trailing_garbage",
        "{\"a\":\"b\"}#",
        TestObject{ { "a", "b" } },
        {},
        "#",
    },
    {
        "object_size_1_trailing_comma_with_accept_trailing_commas",
        "{\"id\":0,}",
        TestObject{ { "id", 0 } },
        { .accept_trailing_commas = true },
    },
    {
        "array_size_1_trailing_comma_with_accept_trailing_commas",
        "[\"id\",]",
        TestArray{ "id" },
        { .accept_trailing_commas = true },
    },
    {
        "object_size_2_trailing_comma_with_accept_trailing_commas",
        "{\"a\": 0,\"id\":0,}",
        TestObject{ { "a", 0 }, { "id", 0 } },
        { .accept_trailing_commas = true },
    },
    {
        "array_size_2_trailing_comma_with_accept_trailing_commas",
        "[\"id\", 0,]",
        TestArray{ "id", 0 },
        { .accept_trailing_commas = true },
    },
    {
        "string_unpaired_surrogate",
        "\"\\uD800\"",
        "\uFFFD",
    },
    {
        "string_unpaired_surrogate_then_escape_without_replace_invalid_code_po"
        "ints",
        "\"\\uD800\"",
        "\xED\xA0\x80",
        { .replace_invalid_code_points = false },
    },
    {
        "string_incomplete_surrogate_escape",
        "\"\\uD800\\uD800\"",
        "\uFFFD\uFFFD",
    },
    {
        "string_incomplete_surrogate_escape_without_replace_invalid_code_point"
        "s",
        "\"\\uD800\\uD800\"",
        "\xED\xA0\x80\xED\xA0\x80",
        { .replace_invalid_code_points = false },
    },
    {
        "string_1_surrogate_then_escape_n",
        "\"\\uD800\\n\"",
        "\uFFFD\n",
    },
    {
        "string_1_surrogate_then_escape_n_without_replace_invalid_code_points",
        "\"\\uD800\\n\"",
        "\xED\xA0\x80\n",
        { .replace_invalid_code_points = false },
    },
    {
        "array_max_depth_5",
        "[[[[[]]]]]",
        TestArray{ TestArray{ TestArray{ TestArray{ TestArray{} } } } },
        { .max_depth = 5 },
    },
    {
        "object_max_depth_5",
        "{\"_\":{\"_\":{\"_\":{\"_\":{}}}}}",
        TestObject{
            { "_",
              TestObject{
                  { "_",
                    TestObject{
                        { "_", TestObject{ { "_", TestObject{} } } } } } } } },
        { .max_depth = 5 },
    },
};

inline const std::vector<ParseFailCase> parse_fail_cases = {
    {
        "array_1_true_without_comma",
        "[1 true]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_a_invalid_utf8",
        "[a\xE5]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_colon_instead_of_comma",
        "[\"\": 1]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_comma_and_number",
        "[,1]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_double_comma",
        "[1,,2]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_double_extra_comma",
        "[\"x\",,]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_extra_comma",
        "[\"\",]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_incomplete",
        "[\"x\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_incomplete_invalid_value",
        "[x",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_inner_array_no_comma",
        "[3[4]]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_invalid_utf8",
        "[123\xFF]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_items_separated_by_semicolon",
        "[1:2]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_just_comma",
        "[,]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_just_minus",
        "[-]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_missing_value",
        "[   , \"\"]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_newlines_unclosed",
        "[\"a\",\n4\n,1,",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_number_and_comma",
        "[1,]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_number_and_several_commas",
        "[1,,]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_spaces_vertical_tab_formfeed",
        "[\"\u000ba\"\\f]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_star_inside",
        "[*]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_unclosed",
        "[\"\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_unclosed_trailing_comma",
        "[1,",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_unclosed_with_new_lines",
        "[1,\n1\n,1",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "array_unclosed_with_object_inside",
        "[{}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "incomplete_false",
        "[fals]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "incomplete_null",
        "[nul]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "incomplete_true",
        "[tru]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_++",
        "[++1234]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_+1",
        "[+1]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_+Inf",
        "[+Inf]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_-01",
        "[-01]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_-1.0.",
        "[-1.0.]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_-2.",
        "[-2.]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_-NaN",
        "[-NaN]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_.-1",
        "[.-1]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_.2e-3",
        "[.2e-3]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_0.1.2",
        "[0.1.2]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_0.3e+",
        "[0.3e+]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_0.3e",
        "[0.3e]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_0.e1",
        "[0.e1]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_0_capital_E+",
        "[0E+]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_0_capital_E",
        "[0E]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_0e+",
        "[0e+]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_0e",
        "[0e]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_1.0e+",
        "[1.0e+]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_1.0e-",
        "[1.0e-]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_1.0e",
        "[1.0e]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_1_000",
        "[1 000.0]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_1eE2",
        "[1eE2]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_2.e+3",
        "[2.e+3]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_2.e-3",
        "[2.e-3]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_2.e3",
        "[2.e3]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_9.e+",
        "[9.e+]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_Inf",
        "[Inf]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_NaN",
        "[NaN]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_U+FF11_fullwidth_digit_one",
        "[\uff11]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_expression",
        "[1+2]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_hex_1_digit",
        "[0x1]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_hex_2_digits",
        "[0x42]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_infinity",
        "[Infinity]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_invalid+-",
        "[0e+-1]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_invalid-negative-real",
        "[-123.123foo]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_invalid-utf-8-in-bigger-int",
        "[123\xE5]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_invalid-utf-8-in-exponent",
        "[1e1\xE5]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_invalid-utf-8-in-int",
        "[0\xE5]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_minus_infinity",
        "[-Infinity]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_minus_space_1",
        "[- 1]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_neg_int_starting_with_zero",
        "[-012]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_neg_real_without_int_part",
        "[-.123]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_neg_with_garbage_at_end",
        "[-1x]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_real_garbage_after_e",
        "[1ea]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_real_with_invalid_utf8_after_e",
        "[1e\xE5]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_real_without_fractional_part",
        "[1.]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_starting_with_dot",
        "[.123]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_with_alpha",
        "[1.2a-3]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_with_alpha_char",
        "[1.8011670033376514H-308]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_with_leading_zero",
        "[012]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_bad_value",
        "[\"x\", truth]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_bracket_key",
        "{[: \"x\"}\n",
        ParseErrorCode::UnexpectedToken,
    },

    {
        "object_comma_instead_of_colon",
        "{\"x\", null}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_double_colon",
        "{\"x\"::\"b\"}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_duplicated_key",
        "{\"a\":\"b\",\"a\":\"c\"}",
        ParseErrorCode::DuplicateKey,
    },
    {
        "object_duplicated_key_and_value",
        "{\"a\":\"b\",\"a\":\"b\"}",
        ParseErrorCode::DuplicateKey,
    },
    {
        "object_emoji",
        "{\U0001f1e8\U0001f1ed}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_garbage_at_end",
        "{\"a\":\"a\" 123}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_key_with_single_quotes",
        "{key: 'value'}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_missing_colon",
        "{\"a\" b}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_missing_key",
        "{:\"b\"}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_missing_semicolon",
        "{\"a\" \"b\"}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_missing_value",
        "{\"a\":",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_no-colon",
        "{\"a\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_non_string_key",
        "{1:1}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_non_string_key_but_huge_number_instead",
        "{9999E9999:1}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_repeated_null_null",
        "{null:null,null:null}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_several_trailing_commas",
        "{\"id\":0,,,,,}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_single_quote",
        "{'a':0}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_trailing_comma",
        "{\"id\":0,}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_two_commas_in_a_row",
        "{\"a\":\"b\",,\"c\":\"d\"}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_unquoted_key",
        "{a: \"b\"}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_unterminated-value",
        "{\"a\":\"a",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_with_single_string",
        "{ \"foo\" : \"bar\", \"a\" }",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "single_space",
        " ",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_1_surrogate_then_escape_without_accept_invalid_code_points",
        "\"\\uD800\"",
        ParseErrorCode::InvalidEscape,
        { .accept_invalid_code_points = false },
    },
    {
        "string_1_surrogate_then_escape_u_without_accept_invalid_code_points",
        "\"\\uD800\\u\"",
        ParseErrorCode::InvalidEscape,
        { .accept_invalid_code_points = false },
    },
    {
        "string_1_surrogate_then_escape_u1_without_accept_invalid_code_points",
        "\"\\uD800\\u1\"",
        ParseErrorCode::InvalidEscape,
        { .accept_invalid_code_points = false },
    },
    {
        "string_1_surrogate_then_escape_n_without_accept_invalid_code_points",
        "\"\\uD800\\n\"",
        ParseErrorCode::InvalidEscape,
        { .accept_invalid_code_points = false },
    },
    {
        "string_accentuated_char_no_quotes",
        "[\xe9]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_backslash_00",
        "[\"\\\x00\"]"s,
        ParseErrorCode::InvalidEscape,
    },
    {
        "string_escape_x",
        "\"\\x00\"",
        ParseErrorCode::InvalidEscape,
    },
    {
        "string_escaped_backslash_bad",
        "\"\\\\\\\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_escaped_ctrl_char_tab",
        "\"\\\t\"",
        ParseErrorCode::InvalidEscape,
    },
    {
        "string_escaped_emoji",
        "\"\\\U0001F600\"",
        ParseErrorCode::InvalidEscape,
    },
    {
        "string_incomplete_escape",
        "\"\\\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_incomplete_escaped_character",
        "\"\\u00A\"",
        ParseErrorCode::InvalidEscape,
    },
    {
        "string_incomplete_surrogate",
        "\"\\uD834\\uDd\"",
        ParseErrorCode::InvalidEscape,
    },
    {
        "string_incomplete_surrogate_escape_without_accept_invalid_code_points",
        "\"\\uD800\\uD800\"",
        ParseErrorCode::InvalidEscape,
        { .accept_invalid_code_points = false },
    },
    {
        "string_invalid-utf-8-in-escape",
        "\"\\u\xE5\"",
        ParseErrorCode::InvalidEscape,
    },
    {
        "string_invalid_backslash_esc",
        "\"\\a\"",
        ParseErrorCode::InvalidEscape,
    },
    {
        "string_invalid_unicode_escape",
        "\"\\uqqqq\"",
        ParseErrorCode::InvalidEscape,
    },
    {
        "string_invalid_utf8_after_escape",
        "\"\\\\\xE5\"",
        ParseErrorCode::InvalidEncoding,
    },
    {
        "string_leading_uescaped_thinspace",
        "\\u0020\"asd\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_no_quotes_with_bad_escape",
        "\\n",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_single_doublequote",
        "\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_single_quote",
        "'single quote'",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_single_string_no_double_quotes",
        "abc",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_start_escape_unclosed",
        "\"\\",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_unescaped_null_char",
        "\"a\x00a\""s,
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_unescaped_newline",
        "\"new\nline\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_unescaped_tab",
        "\"\t\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "string_unicode_CapitalU",
        "\"\\UA66D\"",
        ParseErrorCode::InvalidEscape,
    },
    {
        "structure_U+2060_word_joined",
        "[\u2060]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_UTF8_BOM_no_data",
        "\uFEFF",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_angle_bracket_.",
        "<.>",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_angle_bracket_null",
        "[<null>]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_array_with_unclosed_string",
        "[\"asd]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_ascii-unicode-identifier",
        "a\xe5",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_capitalized_True",
        "[True]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_comma_instead_of_closing_brace",
        "{\"x\": true,",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_end_array",
        "]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_incomplete_UTF8_BOM",
        "\xEF\xBB{}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_lone-invalid-utf-8",
        "\xE5",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_lone-open-bracket",
        "[",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_no_data",
        "",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_null-byte-outside-string",
        "[\x00]"s,
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_object_unclosed_no_value",
        "{\"\":",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_object_with_comment",
        "{\"a\":/*comment*/\"b\"}",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_array_apostrophe",
        "['",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_array_comma",
        "[,",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_array_open_object",
        "[{",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_array_open_string",
        "[\"a",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_array_string",
        "[\"a\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_object",
        "{",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_object_close_array",
        "{]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_object_comma",
        "{,",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_object_open_array",
        "{[",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_object_open_string",
        "{\"a",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_object_string_with_apostrophes",
        "{'a'",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_open_open",
        "[\"\\{[\"\\{[\"\\{[\"\\{",
        ParseErrorCode::InvalidEscape,
    },
    {
        "structure_single_eacute",
        "\xE9",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_single_star",
        "*",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_lone_continuation_byte",
        "{\"\xB9\":\"0\"}",
        ParseErrorCode::InvalidEncoding,
    },
    {
        "structure_uescaped_LF_before_string",
        "[\\u000A\"\"]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_unclosed_array",
        "[1",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_unclosed_array_partial_null",
        "[ false, nul",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_unclosed_array_unfinished_false",
        "[ true, fals",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_unclosed_array_unfinished_true",
        "[ false, tru",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_unclosed_object",
        "{\"asd\":\"asd\"",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_unicode-identifier",
        "\xe5",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_whitespace_U+2060_word_joiner",
        "[\u2060]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "structure_whitespace_formfeed",
        "[\x0c]",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "number_minus_foo",
        "-foo",
        ParseErrorCode::UnexpectedToken,
    },
    {
        "object_size_0_trailing_comma_with_accept_trailing_commas",
        "{,}",
        ParseErrorCode::UnexpectedToken,
        { .accept_trailing_commas = true },
    },
    {
        "array_size_0_trailing_comma_with_accept_trailing_commas",
        "[,]",
        ParseErrorCode::UnexpectedToken,
        { .accept_trailing_commas = true },
    },
    {
        "array_max_depth_5+1",
        "[[[[[[]]]]]]",
        ParseErrorCode::MaxDepth,
        { .max_depth = 5 },
    },
    {
        "object_max_depth_5+1",
        "{\"_\":{\"_\":{\"_\":{\"_\":{\"_\":{}}}}}}",
        ParseErrorCode::MaxDepth,
        { .max_depth = 5 },
    },
};

// { "i_number_double_huge_neg_exp" },
// { "i_number_huge_exp" },
// { "i_number_neg_int_huge_exp" },
// { "i_number_pos_double_huge_exp" },
// { "i_number_real_neg_overflow" },
// { "i_number_real_pos_overflow" },
// { "i_number_real_underflow" },
// { "i_number_too_big_neg_int" },
// { "i_number_too_big_pos_int" },
// { "i_number_very_big_negative_int" },
// { "i_object_key_lone_2nd_surrogate" },
// { "i_string_1st_surrogate_but_2nd_missing" },
// { "i_string_1st_valid_surrogate_2nd_invalid" },
// { "i_string_UTF-16LE_with_BOM" },
// { "i_string_UTF-8_invalid_sequence" },
// { "i_string_UTF8_surrogate_U+D800" },
// { "i_string_incomplete_surrogate_and_escape_valid" },
// { "i_string_incomplete_surrogate_pair" },
// { "i_string_incomplete_surrogates_escape_valid" },
// { "i_string_invalid_lonely_surrogate" },
// { "i_string_invalid_surrogate" },
// { "i_string_invalid_utf-8" },
// { "i_string_inverted_surrogates_U+1D11E" },
// { "i_string_iso_latin_1" },
// { "i_string_lone_second_surrogate" },
// { "i_string_lone_utf8_continuation_byte" },
// { "i_string_not_in_unicode_range" },
// { "i_string_overlong_sequence_2_bytes" },
// { "i_string_overlong_sequence_6_bytes" },
// { "i_string_overlong_sequence_6_bytes_null" },
// { "i_string_truncated-utf-8" },
// { "i_string_utf16BE_no_BOM" },
// { "i_string_utf16LE_no_BOM" },
// { "i_structure_500_nested_arrays" },
// { "i_structure_UTF-8_BOM_empty_object" },

} // namespace htl::test

#endif
